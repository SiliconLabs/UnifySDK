// License
// <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>

// The licensor of this software is Silicon Laboratories Inc. Your use of this
// software is governed by the terms of Silicon Labs Master Software License
// Agreement (MSLA) available at
// www.silabs.com/about-us/legal/master-software-license-agreement. This
// software is distributed to you in Source Code format and is governed by the
// sections of the MSLA applicable to Source Code.

//! Cache is an object that bookkeeps unid & endpoint to group relations.
//! it also returns information about changed records.
use std::collections::BTreeMap;
use std::iter::FromIterator;
use std::{collections::HashSet, vec};
use uic_log::log_warning;

#[derive(Clone, Debug, PartialEq)]
pub struct EntryChanged {
    pub group_id: u16,
    pub unid: String,
    pub endpoints: Vec<u16>,
}

impl EntryChanged {
    pub fn is_unretain(&self) -> bool {
        self.endpoints.is_empty()
    }
}

type UnidEpPair = (String, u16);
#[derive(Debug, PartialEq)]
pub struct Cache {
    groups_for_endpoints: BTreeMap<UnidEpPair, Vec<u16>>,
    reverse_lookup: BTreeMap<u16, BTreeMap<String, Vec<u16>>>,
    group_names: BTreeMap<u16, String>,
    // Each endpoint (UnidEpPair) has a BTreeMap, indexed by a cluster-name, that contains the
    // vector of supported commands for that cluster.
    supported_commands_for_endpoints: BTreeMap<UnidEpPair, BTreeMap<String, HashSet<String>>>,
}

impl Cache {
    pub fn new() -> Self {
        Cache {
            groups_for_endpoints: BTreeMap::new(),
            reverse_lookup: BTreeMap::new(),
            group_names: BTreeMap::new(),
            supported_commands_for_endpoints: BTreeMap::new(),
        }
    }

    /// Update the cache with new groups for an given key (unid-endpoint combination)
    /// if unid.is_empty(), the function returns and does not update anything.
    /// returns a list with changed endpoints for all effected keys.
    /// if the endpoint vector inside EntryChanged is empty, its considered unretained
    pub fn set_group_list_for_node(
        &mut self,
        unid: &str,
        endpoint: u16,
        groups: &[u16],
    ) -> Vec<EntryChanged> {
        if unid.is_empty() {
            return Vec::new();
        }

        let mut to_be_removed: Vec<u16> = Vec::new();
        let mut to_be_added: Vec<u16> = Vec::new();

        let key = (unid.to_string(), endpoint);
        if let Some(cached_group) = self.groups_for_endpoints.get(&key) {
            for group in cached_group {
                if !groups.contains(group) {
                    to_be_removed.push(group.clone());
                }
            }

            for group in groups {
                if !cached_group.contains(group) {
                    to_be_added.push(group.clone());
                }
            }
        } else {
            to_be_added.extend(groups);
        }

        // update cache entry
        self.groups_for_endpoints
            .insert(key.clone(), groups.to_owned());

        self.update_reverse_lookup();

        self.make_changed_entries(key, &to_be_added, &to_be_removed)
    }

    /// Get a list of groups a node belongs to.
    /// Returns an empty vector if unid is empty or if node doesn't belong to a group.
    pub fn get_group_list_for_node(&mut self, unid: &str, endpoint: u16) -> Vec<u16> {
        if unid.is_empty() {
            return Vec::new();
        }

        if let Some(groups) = self.groups_for_endpoints.get(&(unid.to_string(), endpoint)) {
            return Vec::from_iter(groups.clone());
        } else {
            return Vec::new();
        }
    }

    /// creates EntryChanged objects for the records that will be effected by the incoming change.
    /// this function assumes the groups_for_endpoints already reflects the new state.
    /// if the endpoint vector inside EntryChanged is empty, its considered unretained
    fn make_changed_entries(
        &self,
        key: UnidEpPair,
        added_groups: &[u16],
        removed_groups: &[u16],
    ) -> Vec<EntryChanged> {
        let mut changed_entries: Vec<EntryChanged> = Vec::new();

        for added in added_groups {
            let end_points = self.reverse_lookup[added][&key.0].to_vec();
            changed_entries.push(EntryChanged {
                group_id: *added,
                unid: key.0.to_owned(),
                endpoints: end_points,
            });
        }

        for removed in removed_groups {
            let mut end_points = Vec::new();
            if let Some(unid_list) = self.reverse_lookup.get(removed) {
                if let Some(ep_list) = unid_list.get(&key.0) {
                    end_points = ep_list.clone();
                }
            }

            changed_entries.push(EntryChanged {
                group_id: *removed,
                unid: key.0.to_owned(),
                endpoints: end_points,
            });
        }
        changed_entries
    }

    /// rebuilds reverse_lookup completely from groups_for_endpoints
    fn update_reverse_lookup(&mut self) {
        let mut reverse_lookup: BTreeMap<u16, BTreeMap<String, Vec<u16>>> = BTreeMap::new();
        for (k, v) in &self.groups_for_endpoints {
            for group in v {
                if let Some(unid_list) = reverse_lookup.get_mut(&group) {
                    if let Some(ep_list) = unid_list.get_mut(&k.0) {
                        ep_list.push(k.1);
                    } else {
                        unid_list.insert(k.0.to_owned(), vec![k.1]);
                    }
                } else {
                    let mut map = BTreeMap::new();
                    map.insert(k.0.to_owned(), vec![k.1]);
                    reverse_lookup.insert(group.clone(), map);
                }
            }
        }

        self.reverse_lookup = reverse_lookup;
    }

    /// returns true if the name was updated.
    /// returns false when trying to update an existing name to an empty string
    /// or when the new name == the old name
    pub fn update_group_name(&mut self, groupid: u16, name: &str) -> bool {
        if self.get_group_name(groupid).is_some() && name.is_empty() {
            log_warning(
                "cache",
                format!(
                    "omitting groupname update of group {} because of an empty string.",
                    self.group_names.get(&groupid).unwrap()
                ),
            );
            return false;
        }

        let old = self.group_names.insert(groupid, name.to_string());
        old != Some(name.to_string())
    }

    pub fn get_group_name(&self, groupid: u16) -> Option<String> {
        self.group_names.get(&groupid).map(|s| s.into())
    }

    /// Update the list of supported-commands, per cluster, for the given endpoint.
    pub fn set_endpoint_cluster_supported_commands(
        &mut self,
        key: UnidEpPair,
        cluster_name: &str,
        commands: HashSet<String>,
    ) {
        let endpoint = self
            .supported_commands_for_endpoints
            .entry(key.clone())
            .or_insert(BTreeMap::new());
        endpoint.insert(cluster_name.to_string(), commands);
    }

    /// Get the list of supported commands, for a given cluster, for a given endpoint.
    pub fn get_endpoint_cluster_supported_commands(
        &self,
        key: &UnidEpPair,
        cluster_name: &str,
    ) -> Option<HashSet<String>> {
        if let Some(supported_commands) = self.supported_commands_for_endpoints.get(key) {
            if let Some(cluster_commands) = supported_commands.get(cluster_name) {
                return Some(cluster_commands.clone());
            }
        }
        return None;
    }

    /// Get the list of endpoints that belong to group.
    pub fn get_endpoints_for_group(&self, group_id: &u16) -> Option<Vec<(String, u16)>> {
        if let Some(unid_list) = self.reverse_lookup.get(group_id) {
            let mut return_vec: Vec<(String, u16)> = Vec::new();
            for (unid, eps) in unid_list {
                for ep in eps {
                    return_vec.push((unid.clone(), ep.clone()));
                }
            }
            return Some(return_vec);
        }
        return None;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn change_contains(changes: &[EntryChanged], unid: &str, group_id: u16, ep: Vec<u16>) -> bool {
        let entr = EntryChanged {
            group_id,
            unid: unid.to_string(),
            endpoints: ep,
        };
        changes.iter().find(|&x| x == &entr).is_some()
    }

    #[test]
    fn set_groups_for_nodes_test() {
        let mut cache = Cache::new();
        let changes = cache.set_group_list_for_node("zw-1235", 0, &vec![3, 4, 5]);
        assert_eq!(changes.len(), 3);
        assert!(change_contains(&changes, "zw-1235", 3, vec![0]));
        assert!(change_contains(&changes, "zw-1235", 4, vec![0]));
        assert!(change_contains(&changes, "zw-1235", 5, vec![0]));

        // no data change equals no update
        let changes = cache.set_group_list_for_node("zw-1235", 0, &vec![3, 4, 5]);
        assert_eq!(changes.len(), 0);

        // remove group 5
        let changes = cache.set_group_list_for_node("zw-1235", 0, &vec![3, 4]);
        assert_eq!(changes.len(), 1);
        assert!(change_contains(&changes, "zw-1235", 5, vec![]));

        // add different unid doesnt effect result
        let changes = cache.set_group_list_for_node("aa-000", 0, &vec![0, 3]);
        assert_eq!(changes.len(), 2);
        assert!(change_contains(&changes, "aa-000", 0, vec![0]));
        assert!(change_contains(&changes, "aa-000", 3, vec![0]));

        // add extra end-point
        let changes = cache.set_group_list_for_node("zw-1235", 2, &vec![3, 66]);
        assert_eq!(changes.len(), 2);
        assert!(change_contains(&changes, "zw-1235", 3, vec![0, 2]));
        assert!(change_contains(&changes, "zw-1235", 66, vec![2]));

        // remove end-point
        let changes = cache.set_group_list_for_node("zw-1235", 2, &Vec::new());
        assert_eq!(changes.len(), 2);
        assert!(change_contains(&changes, "zw-1235", 3, vec![0]));
        assert!(change_contains(&changes, "zw-1235", 66, vec![]));
    }

    #[test]
    fn update_group_name() {
        let mut cache = Cache::new();
        assert!(cache.update_group_name(2, ""));
        assert!(cache.update_group_name(2, "test group"));
        assert!(!cache.update_group_name(2, "test group"));
        assert!(!cache.update_group_name(2, ""));
        assert!(cache.update_group_name(2, "test group new"));
    }
}
