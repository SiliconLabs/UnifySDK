.. uml::
  :align: center

  legend top
      |Background color | description |
      |<#ff9999>| Unify Component |
      |<#acffa0>| Protocol Controller Component |
  endlegend

  !theme plain
  title Publishing the UCL node state

  Package "Attribute Store" <<Database>> #ff9999 {

    object "Root" as root #DEDEDE
    object "UNID Attribute" as unid #f2ffe6

    unid : Attribute Type = ATTRIBUTE_UNID
    unid : value = Desired: [] - Reported: ["unid-123"]

    object "Endpoint Attribute" as ep #e6fff7
    ep : Attribute Type = ATTRIBUTE_ENDPOINT_ID
    ep : value = Desired: [] - Reported: [01]

    root *- unid
    unid *- ep

    Package "DotDot\nAttribute Space" #f3acb1 {
      map "DotDot Attribute 1" as dotdot_attribute_1 {
        Reported => [00]
        Desired  => [01]
      }
      map "DotDot Attribute 2" as dotdot_attribute_2 {
        Reported => [00 01]
        Desired  => []
      }
      ep *-- dotdot_attribute_1
      ep *-- dotdot_attribute_2
    }

    Package "Protocol Controller\nAttribute Space" #acffa0 {
      map "Proprietary Attribute 1" as attribute_1 #FFFFFF {
        Reported => [FF]
        Desired  => [00]
      }
      map "Proprietary Attribute 2" as attribute_2 #FFFFFF {
        Reported => [3C]
        Desired  => []
      }
      ep *-- attribute_1
      ep *-- attribute_2
    }
  }

  Package "Other\nComponent" as other_component #acffa0 {
    object "Other component" as other_component #FFFFFF
    other_component : May generate relevant events to the
    other_component : node states, like when the Protocol
    other_component : Controller shuts down
  }

  Package "UCL\nNode State" as ucl_node_state #acffa0 {
    object "Node State publisher" as ucl_node_state_publisher #FFFFFF
    ucl_node_state_publisher : Listens to events from the Attribute
    ucl_node_state_publisher : store or from other components
    ucl_node_state_publisher : to publish node states.
  }

  Package "MQTT Client" as mqtt_client #ff9999 {
    map "Publication queue" as mqtt_client_publications #FFFFFF {
      topic1 => payload1
      topic2 => payload2
      topic3 => payload2
    }
  }

  skinparam {
  ArrowColor #5566FF
  ArrowFontColor #5566FF
  }

  ucl_node_state_publisher -> mqtt_client_publications : Publish node state

  skinparam {
  ArrowColor #FF6655
  ArrowFontColor #FF6655
  }

  attribute_1 --> ucl_node_state_publisher : Read
  attribute_2 --> ucl_node_state_publisher
  dotdot_attribute_1 --> ucl_node_state_publisher
  dotdot_attribute_2 --> ucl_node_state_publisher
  other_component -> ucl_node_state_publisher : Relevant events
