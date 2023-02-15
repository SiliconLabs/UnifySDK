.. uml::
  :align: center

  !theme plain
  title Attribute Store tree networking addressing example

  object root
  root : Attribute Type = TREE_ROOT
  root : value = Desired: [], Reported: []

  object "HomeID" as home_id #fffae6
  home_id : Attribute Type = ATTRIBUTE_HOME_ID
  home_id : value = Desired: [], Reported: [FB E6 8C CE]

  object "NodeID" as node_id_1 #f2ffe6
  node_id_1 : Attribute Type = ATTRIBUTE_NODE_ID
  node_id_1 : value = Desired: [], Reported: [03]

  object "NodeID" as node_id_2 #f2ffe6
  node_id_2 : Attribute Type = ATTRIBUTE_NODE_ID
  node_id_2 : value = Desired: [], Reported: [04]

  object "Endpoint" as endpoint_1 #e6fff7
  endpoint_1 : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint_1 : value = Desired: [], Reported: [00]

  object "Endpoint" as endpoint_2 #e6fff7
  endpoint_2 : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint_2 : value = Desired: [], Reported: [01]

  object "Endpoint" as endpoint_3 #e6fff7
  endpoint_3 : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint_3 : value = Desired: [], Reported: [00]

  object "OnOff attribute" as node_1_ep_2_on_off #FFFFFF
  node_1_ep_2_on_off : Attribute Type = DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF
  node_1_ep_2_on_off : value = Desired: [01], Reported: [00]

  root *-- home_id
  home_id *-- node_id_1
  home_id *-- node_id_2
  node_id_1 *-- endpoint_1
  node_id_1 *-- endpoint_2
  node_id_2 *-- endpoint_3
  endpoint_2 *-- node_1_ep_2_on_off
