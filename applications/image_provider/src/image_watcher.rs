use serde::{Deserialize, Serialize};
use std::fs::File;
use std::io::Read;
use std::path::Path;
use std::path::PathBuf;
use std::time::Duration;
use unify_config_sys::config_get_as_int;
use unify_config_sys::config_get_as_string;
use unify_config_sys::ConfigLoader;
use unify_middleware::unify_mqtt_client::{sl_status_t, MqttClientTrait, UnifyMqttClient};

use std::thread;

use std::collections::HashMap;
use std::sync::Mutex;

use unify_log_sys::*;

#[allow(non_snake_case)]
#[derive(Serialize, Deserialize, Default)]
pub struct ImageList {
    pub Version: String,
    pub Images: Vec<ImageEntry>,
}

#[allow(non_snake_case)]
#[derive(Serialize, Deserialize, std::clone::Clone, PartialEq, Default)]
pub struct ImageEntry {
    pub FileName: String,
    pub Uiid: String,
    pub Unid: Vec<String>,
    pub Version: String,
    pub ApplyAfter: String,
    pub Md5: String,
}

#[allow(non_snake_case)]
#[derive(PartialEq, Eq)]
pub struct MapEntry {
    pub FileName: String,
    pub Uiid: String,
    pub Unid: String,
    pub Version: String,
    pub ApplyAfter: String,
    pub Md5: String,
}

#[derive(Eq, PartialEq, Hash, Clone)]
struct TwoStringsKey {
    uiid: String,
    unid: String,
}

pub struct ImageWatcher {
    ota_config: OtaConfig,
}

pub struct MetaFile {
    md5_digest: String,
    detected: bool,
}

#[derive(Debug)]
pub enum WatchError {
    Md5Error,
    JsonError,
    PathEror,
    FileError,
    MapEntryError,
    MapKeyError,
}

// Use a HashMap to store the images metadata
lazy_static! {
    static ref IMAGE_MAP: Mutex<HashMap<TwoStringsKey, MapEntry>> = {
        let m = HashMap::new();
        Mutex::new(m)
    };
}

static mut IMG_PATH: String = String::new();

#[derive(Default, Clone)]
pub struct OtaConfig {
    image_path: String,
    poll_period: i32,
}

declare_app_name!("image_watcher");

impl ConfigLoader for OtaConfig {
    fn from_config() -> Result<Self, unify_config_sys::config_status_t>
    where
        Self: Sized,
    {
        let image_path = config_get_as_string("image_provider.image_path")?;
        let poll_period = config_get_as_int("image_provider.poll_period")?;

        Ok(OtaConfig {
            image_path,
            poll_period,
        })
    }
}

impl ImageWatcher {
    pub fn new<T: MqttClientTrait + std::clone::Clone>(ota_config: OtaConfig) -> Self {
        ImageWatcher { ota_config }
    }

    /// Build topics from map key and send empty payload to topics
    /// ucl/OTA/info/{}/{}
    /// Clear map
    pub fn remove_image_advertisement() -> Result<(), sl_status_t> {
        let mqtt_client = UnifyMqttClient::default();
        let mut map = IMAGE_MAP.lock().unwrap();
        // Unretain message by sending empty payload
        for key in map.keys() {
            let topic = format!("ucl/OTA/info/{}/{}", key.uiid, key.unid);
            log_info!("Removing image advertisement to \"{}\"", topic);
            let _msg_id = mqtt_client
                .publish(&topic, &[], true)
                .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
        }
        // remove all entry from map
        map.clear();
        Ok(())
    }

    /// Process metadata and insert JSON fields to map
    fn process_metadata(images: Vec<ImageEntry>) {
        let image_path_s: String = unsafe { IMG_PATH.clone() };
        for image in images {
            // Build cross platform image file path
            let mut path = PathBuf::new();
            path.push(image_path_s.clone());
            path.push(image.FileName.clone());
            // Check if file exists and compare md5
            let mut buffer = Vec::new();
            if file_exists(&path, &mut buffer) {
                if calc_md5_digest(&buffer) != image.Md5 {
                    error_message(WatchError::Md5Error, image.FileName.to_string());
                } else {
                    Self::fill_map(image);
                }
                // Remove all data from buffer
                buffer.clear();
            }
        }
    }

    /// Fill map entry
    fn fill_map(image: ImageEntry) {
        let mut map = IMAGE_MAP.lock().unwrap();
        if !image.Unid.is_empty() {
            for unid in image.Unid {
                if unid.to_string().trim().is_empty() || image.Uiid.to_string().trim().is_empty() {
                    continue;
                }
                let entry = MapEntry {
                    FileName: image.FileName.to_string(),
                    Uiid: image.Uiid.to_string(),
                    Unid: unid.to_string(),
                    Version: image.Version.to_string(),
                    ApplyAfter: image.ApplyAfter.to_string(),
                    Md5: image.Md5.to_string(),
                };
                let key = TwoStringsKey {
                    uiid: image.Uiid.to_owned(),
                    unid: unid.to_owned(),
                };
                map.insert(key.clone(), entry);
            }
        } else if !image.Uiid.trim().is_empty() {
            let entry = MapEntry {
                FileName: image.FileName.to_string(),
                Uiid: image.Uiid.to_string(),
                Unid: String::from("all"),
                Version: image.Version.to_string(),
                ApplyAfter: image.ApplyAfter.to_string(),
                Md5: image.Md5.to_string(),
            };
            let key = TwoStringsKey {
                uiid: image.Uiid,
                unid: "all".to_owned(),
            };
            map.insert(key, entry);
        }
    }

    /// Build topics from map key, build payload from map value, send payload to
    /// ucl/OTA/info/{}/{} topics
    fn advertise_images<T: MqttClientTrait + std::clone::Clone>(mqtt_client: &T) {
        let map = IMAGE_MAP.lock().unwrap();
        // Advertise images to MQTT
        for key in map.keys() {
            let topic = format!("ucl/OTA/info/{}/{}", key.uiid, key.unid);
            let entry = map.get(key);
            match entry {
                Some(entry) => {
                    let data = json::object! {
                        "Version" => entry.Version.to_string(),
                        "ApplyAfter" => entry.ApplyAfter.to_string(),
                        "Filename" => entry.FileName.to_string()
                    };
                    log_info!("Advertising new image at \"{}\"", topic);
                    let payload = data.dump().as_bytes().to_vec();
                    let _msg_id = mqtt_client
                        .publish(&topic, &payload, true)
                        .map_err(|e| format!("error publishing topic '{}' : {}", topic, e));
                }
                None => {
                    error_message(
                        WatchError::MapEntryError,
                        format!("{}/{}", key.uiid, key.unid),
                    );
                }
            }
        }
    }

    /// Check for changes
    pub fn file_changed(metafile: &mut MetaFile, buffer: &Vec<u8>) -> bool {
        let digest = calc_md5_digest(buffer);
        let is_changed = digest != metafile.md5_digest;
        metafile.md5_digest = digest;
        is_changed
    }

    /// Poll changes in images.json
    /// If changes occures: send denounce messages, process metadata from file,
    /// send announce messages
    pub fn run_image_watcher(&mut self) {
        let watcher = std::thread::Builder::new().name("IMAGE_WATCHER".into());
        let images_path_s = self.ota_config.image_path.clone();

        unsafe {
            IMG_PATH = images_path_s.clone();
        }
        unsafe {
            log_debug!("Image storage path {}", IMG_PATH);
        }
        // Build cross platform metadata file path
        let mut metadata_path = PathBuf::new();
        metadata_path.push(images_path_s);
        metadata_path.push("images.json");

        let poll_period = Duration::from_secs(self.ota_config.poll_period as u64);

        let mut metadata_file = MetaFile {
            md5_digest: String::new(),
            detected: false,
        };

        watcher
            .spawn(move || loop {
                // workaround; we can instantiate an new instance of mqttclient here because we know
                // that MosquittoClient implementation is created with an single instance.
                // We dont need a thread here. When we refactor this thread out we can restore a proper
                // clone or borrow to the mosquitto client.
                // note: this call should never be earlier as the MosquittoClient::new call in the main.rs
                // file.
                let mqttclient = UnifyMqttClient::default();

                let mut buffer = Vec::new();
                if !file_exists(&metadata_path, &mut buffer) {
                    if metadata_file.detected {
                        ImageWatcher::remove_image_advertisement().unwrap();
                        metadata_file.detected = false;
                        metadata_file.md5_digest = String::new();
                    }
                } else {
                    metadata_file.detected = true;
                    if Self::file_changed(&mut metadata_file, &mut buffer) {
                        match serde_json::from_slice::<ImageList>(buffer.as_slice()) {
                            Ok(json_data) => {
                                ImageWatcher::remove_image_advertisement().unwrap();
                                Self::process_metadata(json_data.Images);
                                ImageWatcher::advertise_images(&mqttclient);
                            }
                            Err(e) => {
                                error_message(
                                    WatchError::JsonError,
                                    format!("Could not parse images.json metadata: {}", e),
                                );
                                if metadata_file.detected {
                                    ImageWatcher::remove_image_advertisement().unwrap();
                                    metadata_file.detected = false;
                                }
                            }
                        }
                    }
                }
                thread::sleep(poll_period);
            })
            .unwrap();
    }
}

/// Calculate and return md5 base64 encoded digest from buffer
fn calc_md5_digest(buffer: &Vec<u8>) -> String {
    base64::encode(md5::compute(buffer).to_vec())
}

/// Get file from Map
/// Should be implemented as queue
/// But it looks mosquitto itself already has queue for on-message handler
pub fn get_image_file(uiid: String, unid: String, buffer: &mut Vec<u8>) {
    // Check if key exists in map
    let map = IMAGE_MAP.lock().unwrap();
    let key = TwoStringsKey {
        uiid: uiid.to_owned(),
        unid: unid.to_owned(),
    };
    let path_c: String = unsafe { IMG_PATH.clone() };

    if map.contains_key(&key) {
        log_info!("Image requested for {}/{}", uiid, unid);
        // Get value for key
        let entry = map.get(&key);
        match entry {
            Some(entry) => {
                // Extract image file name
                let filename = &entry.FileName;
                // Build cross platform image file path
                let mut path = PathBuf::new();
                path.push(path_c);
                path.push(filename);

                if file_exists(&path, buffer) && calc_md5_digest(buffer) != entry.Md5 {
                    error_message(WatchError::Md5Error, filename.to_string());
                    // Remove all data from buffer
                    buffer.clear();
                }
            }
            None => {
                error_message(
                    WatchError::MapEntryError,
                    format!("No Entry for existing {}/{}", uiid, unid),
                );
            }
        }
    } else {
        error_message(
            WatchError::MapKeyError,
            format!("Image Requested for non existing {}/{}", uiid, unid),
        );
    }
}

/// Check if file exists and return buffer with content
fn file_exists(path: &Path, buffer: &mut Vec<u8>) -> bool {
    let file = File::open(&path);
    match file {
        Ok(mut f) => {
            // Read file to buffer
            match f.read_to_end(buffer) {
                Err(_) => {
                    error_message(WatchError::FileError, path.display().to_string());
                    buffer.clear();
                    false
                }
                Ok(_) => true,
            }
        }
        Err(_) => {
            error_message(WatchError::PathEror, path.display().to_string());
            false
        }
    }
}

/// Print error messages
fn error_message(e: WatchError, text: String) {
    match e {
        WatchError::FileError => {
            log_error!("Could not open file {}", text);
        }
        WatchError::PathEror => {
            log_error!("Could not open path {}", text);
        }
        WatchError::JsonError => {
            log_info!("Could not parse JSON {}", text);
        }
        WatchError::Md5Error => {
            log_info!("Md5 digest error for {}", text);
        }
        WatchError::MapEntryError => {
            log_error!("Map Entry Error for {}", text);
        }
        WatchError::MapKeyError => {
            log_info!("Map Key Error {}", text);
        }
    }
}

// Test JSON format
#[cfg(test)]
mod tests {
    use super::*;
    use serial_test::serial;

    #[test]
    #[serial]
    fn metadata_changes_test() {
        // Define some json objects
        let json_1 = json::object! {
            "Version" => String::from("1"),
            "Images" => Vec::<String>::new()
        };

        let json_2 = json::object! {
            "Version" => String::from("2"),
            "Images" => Vec::<String>::new()
        };

        // Dump json objects to vectors
        let file_buffer_1 = json_1.dump().as_bytes().to_vec();
        let file_buffer_2 = json_2.dump().as_bytes().to_vec();

        let mut metadata_file = MetaFile {
            md5_digest: String::new(),
            detected: true,
        };

        // Check if changes detected
        assert_eq!(
            ImageWatcher::file_changed(&mut metadata_file, &file_buffer_1),
            true
        );
        assert_eq!(
            ImageWatcher::file_changed(&mut metadata_file, &file_buffer_1),
            false
        );
        assert_eq!(
            ImageWatcher::file_changed(&mut metadata_file, &file_buffer_2),
            true
        );
    }

    #[test]
    #[serial]
    fn map_fill_test() {
        // Create Image entries
        // entry_1 with Uiid = "zw-000-001" and Unid = []
        let entry_1 = ImageEntry {
            FileName: String::from("updates/image_0.jpg"),
            Uiid: String::from("zw-000-001"),
            Unid: Vec::<String>::new(),
            Version: String::from("1.0.0"),
            ApplyAfter: String::from("2021-06-29T16:39:57+02:00"),
            Md5: String::from("GEyaucM09e025pO/FeIuZQ=="),
        };

        // entry_1 with Uiid = "zw-000-001"
        // and Unid = ["zw-BEEF-0002", "zw-BEEF-0003", "zw-BEEF-0004"]
        let mut unid_vector = Vec::<String>::new();

        unid_vector.push(String::from("zw-BEEF-0002"));
        unid_vector.push(String::from("zw-BEEF-0003"));
        unid_vector.push(String::from("zw-BEEF-0004"));

        let entry_2 = ImageEntry {
            FileName: String::from("updates/image_0.jpg"),
            Uiid: String::from("zw-000-002"),
            Unid: unid_vector,
            Version: String::from("1.0.0"),
            ApplyAfter: String::from("2021-06-29T16:39:57+02:00"),
            Md5: String::from("GEyaucM09e025pO/FeIuZQ=="),
        };

        // Add entry to map
        ImageWatcher::fill_map(entry_1);
        ImageWatcher::fill_map(entry_2);

        // Build keys
        // Unid = [] should create "zw-000-001 - all" key
        let key1 = TwoStringsKey {
            uiid: String::from("zw-000-001"),
            unid: String::from("all"),
        };

        // Unid = ["zw-BEEF-0002", "zw-BEEF-0003", "zw-BEEF-0004"] should create 3 keys
        let key2 = TwoStringsKey {
            uiid: String::from("zw-000-002"),
            unid: String::from("zw-BEEF-0002"),
        };
        let key3 = TwoStringsKey {
            uiid: String::from("zw-000-002"),
            unid: String::from("zw-BEEF-0003"),
        };
        let key4 = TwoStringsKey {
            uiid: String::from("zw-000-002"),
            unid: String::from("zw-BEEF-0004"),
        };

        // Some non-existing key
        let key5 = TwoStringsKey {
            uiid: String::from("zw-000-002"),
            unid: String::from("zw-BEEF-0005"),
        };

        let mut map = IMAGE_MAP.lock().unwrap();

        // Check if keys exists and values is correct
        assert!(map.get(&key1).is_some());
        assert!(map.get(&key2).is_some());
        assert!(map.get(&key3).is_some());
        assert!(map.get(&key4).is_some());
        assert!(map.get(&key5).is_none());

        // Remove all entry from map
        map.clear();
    }

    #[test]
    #[serial]
    fn map_fill_empty_string_test() {
        // Create Image entries
        // entry_1 with Uiid = "" and Unid = []
        let entry_1 = ImageEntry {
            FileName: String::from("updates/image_0.jpg"),
            Uiid: String::from(""),
            Unid: Vec::<String>::new(),
            Version: String::from("1.0.0"),
            ApplyAfter: String::from("2021-06-29T16:39:57+02:00"),
            Md5: String::from("GEyaucM09e025pO/FeIuZQ=="),
        };

        // entry_2 with Uiid = "zw-000-001"
        // and Unid = ["", " ", "  "]
        let mut unid_vector_2 = Vec::<String>::new();

        unid_vector_2.push(String::from(""));
        unid_vector_2.push(String::from(" "));
        unid_vector_2.push(String::from("  "));

        let entry_2 = ImageEntry {
            FileName: String::from("updates/image_0.jpg"),
            Uiid: String::from("zw-000-002"),
            Unid: unid_vector_2,
            Version: String::from("1.0.0"),
            ApplyAfter: String::from("2021-06-29T16:39:57+02:00"),
            Md5: String::from("GEyaucM09e025pO/FeIuZQ=="),
        };

        // entry_3 with Uiid = ""
        // and Unid = ["", " ", "  "]
        let mut unid_vector_3 = Vec::<String>::new();

        unid_vector_3.push(String::from("zw-000-002"));
        unid_vector_3.push(String::from("zw-000-003"));
        unid_vector_3.push(String::from("zw-000-004"));

        let entry_3 = ImageEntry {
            FileName: String::from("updates/image_0.jpg"),
            Uiid: String::from(""),
            Unid: unid_vector_3,
            Version: String::from("1.0.0"),
            ApplyAfter: String::from("2021-06-29T16:39:57+02:00"),
            Md5: String::from("GEyaucM09e025pO/FeIuZQ=="),
        };

        // Add entry to map
        ImageWatcher::fill_map(entry_1);
        ImageWatcher::fill_map(entry_2);
        ImageWatcher::fill_map(entry_3);

        let mut map = IMAGE_MAP.lock().unwrap();

        // Check if map is empty
        assert!(map.is_empty());

        // Remove all entry from map
        map.clear();
    }
}
