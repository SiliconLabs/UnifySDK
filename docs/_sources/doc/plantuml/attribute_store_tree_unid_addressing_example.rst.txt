.. uml::
  :align: center

  !theme plain
  title Attribute Store tree UNID addressing example

  object "Root" as root #DEDEDE
  root : Attribute Type = TREE_ROOT
  root : value = Desired: [], Reported: []

  object "UNID" as node_1 #fffae6
  node_1 : Attribute Type = ATTRIBUTE_UNID
  node_1 : value = Desired: [], Reported: ["zb-04CD15FFFE730912"]

  object "Endpoint" as endpoint_1 #f2ffe6
  endpoint_1 : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint_1 : value = Desired: [], Reported: [00]

  object "Endpoint" as endpoint_2 #f2ffe6
  endpoint_2 : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint_2 : value = Desired: [], Reported: [01]

  object "UNID" as node_2 #fffae6
  node_2 : Attribute Type = ATTRIBUTE_UNID
  node_2 : value = Desired: [], Reported: ["zb-04CD15FFFE730912"]

  object "Endpoint" as endpoint_3 #f2ffe6
  endpoint_3 : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint_3 : value = Desired: [], Reported: [00]

  object "OnOff attribute" as node_1_ep_2_on_off #FFFFFF
  node_1_ep_2_on_off : Attribute Type = DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF
  node_1_ep_2_on_off : value = Desired: [01], Reported: [00]

  root *-- node_1
  root *-- node_2
  node_1 *-- endpoint_1
  node_1 *-- endpoint_2
  node_2 *-- endpoint_3
  endpoint_2 *-- node_1_ep_2_on_off

