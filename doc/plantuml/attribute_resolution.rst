.. uml::
  :align: center

  legend top
      |Background color | description |
      |<#ff9999>| Unify Component |
      |<#acffa0>| Protocol Controller Component |
  endlegend

  !theme plain
  title Attribute Resolution

  Package "Attribute Store" <<Database>> #ff9999 {

    object "Root" as root #DEDEDE
    object "UNID Attribute" as unid #f2ffe6

    unid : Attribute Type = ATTRIBUTE_UNID
    unid : value = Desired: [] - Reported: ["unid-123"]

    object "Endpoint Attribute" as ep #e6fff7
    ep : Attribute Type = ATTRIBUTE_ENDPOINT_ID
    ep : value = Desired: [] - Reported: [01]

    root *-- unid
    unid *-- ep

    Package "DotDot\nAttribute Space" #f3acb1 {
      map "DotDot Attribute" as dotdot_attribute_1 {
        Reported => [03]
        Desired  => [01 02 03]
      }
      ep *-- dotdot_attribute_1
    }

    Package "Protocol Controller\nAttribute Space" #acffa0 {
      map "Proprietary Attribute" as attribute_1 #FFFFFF {
        Reported => [FF]
        Desired  => [00]
      }
      ep *-- attribute_1
    }
  }

  Package "Attribute Resolver" as attribute_resolver #ff9999 {
    map "Rule book" as rule_book #acffa0 {
      Function 1 => Attribute type A
      Function 2 => Attribute type B
      Function 3 => Attribute type C
    }
  }

  Package "Addressing resolver" as address_resolver #acffa0 {
    object "Address resolver" as dedicated_address_resolver #FFFFFF
    dedicated_address_resolver : Takes an attribute, navigate up the tree
    dedicated_address_resolver : and find out proprietary addressing
    dedicated_address_resolver : information and forwards payload/frame
    dedicated_address_resolver : to the NCP component
  }

  Package "Frame/Message handlers" as frame_handlers #acffa0 {
    object "Frame handler" as frame_handler #FFFFFF
    frame_handler : Parses frames, converts to
    frame_handler : attributes and saves values
    frame_handler : in the Attribute Store
  }

  Package "Frame/Message generator" as frame_generator #acffa0 {
    object "Attribute Rule" as rule_1 #FFFFFF
    rule_1 : Takes an attribute of a certain
    rule_1 : type and generates a payload/frame
    rule_1 : to send to a node
  }

  Package "NCP" as ncp_component #acffa0 {
    object "Tx/Rx Module" as tx_rx_module #FFFFFF
    tx_rx_module : send and/or receive
    tx_rx_module : network messages
  }

  package "PAN" <<Cloud>> {
    package "Node / Endpoint" as pan_node <<Node>> #FFFFFF {
    }
  }

  rule_1 -l-> rule_book : Registers itself\nfor Attribute Type X

  attribute_1 -> attribute_resolver : Notify on value update

  attribute_resolver -u-> dedicated_address_resolver : Send Frame / Attribute
  dedicated_address_resolver -u-> tx_rx_module : Send Frame with address
  pan_node -d-> tx_rx_module : Receive Frame
  tx_rx_module -u-> pan_node : Send Frame

  ncp_component --> frame_handler : Incoming frames
  frame_handler --> attribute_1 : Update value