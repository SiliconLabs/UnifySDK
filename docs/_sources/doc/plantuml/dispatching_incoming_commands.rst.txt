.. uml::
  :align: center

  legend top
      |Background color | description |
      |<#ff9999>| Unify Component |
      |<#acffa0>| Protocol Controller Component |
  endlegend

  !theme plain
  title Dispatching incoming UCL commands

  Package "Unify DotDot\nAttribute Store" as dotdot_attribute_store #ff9999 {
    map "Command callbacks" as dotdot_attribute_store_command_callbacks #ff9999 {
      OnOff_On => function1
      OnOff_Off => function2
      OnOff_toggle => function3
      Level_move => function4
    }
    map "Protocol Controller configuration" as dotdot_attribute_store_configuration #acffa0 {
      update_attribute_desired_values => true
      clear_reported_values => false
      supported_commands => true
      force_read_attributes => false
      write_attributes => true
      publish_attributes => true
    }
    map "Protocol Controller functions" as dotdot_attribute_store_functions #acffa0 {
      get_endpoint_attribute => function_A
      get_unid_endpoint_id => function_B
    }

     dotdot_attribute_store_command_callbacks -[hidden]down- dotdot_attribute_store_configuration
     dotdot_attribute_store_configuration -[hidden]down- dotdot_attribute_store_functions
  }

  Package "UIC DotDot MQTT" as uic_dotdot_mqtt #ff9999 {
    object "DotDot MQTT handler" as uic_dotdot_mqtt_component #FFFFFF
    uic_dotdot_mqtt_component : Saves callbacks for commands from user
    uic_dotdot_mqtt_component : components, makes the corresponding
    uic_dotdot_mqtt_component : MQTT subscriptions and dispatches
    uic_dotdot_mqtt_component : incoming messages
  }

  rectangle "Optional if using Attribute Resolver" {
    Package "Protocol Controller\nCommand Dispatch" as protocol_controller_command_dispatch #acffa0 {
      map "Command callbacks" as protocol_controller_command_callbacks #acffa0 {
        OnOff_On => function1
        OnOff_Off => function2
        OnOff_toggle => function3
        Level_move => function4
      }
    }
    Package "Command Mapper" as command_mapper #acffa0 {
      object "Command/Address selection" as command_address_selection #FFFFFF
      command_address_selection : Resolve PAN node address
      command_address_selection : Maps dotdot command to PAN node command
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
  }

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

  Package "MQTT Client" as mqtt_client #ff9999 {
    map "Subscriptions callbacks" as mqtt_client_subscriptions #FFFFFF {
      topic_filter_1 => function1
      topic_filter_2 => function2
      topic_filter_3 => function3
    }
  }


  skinparam {
    ArrowColor #5566FF
    ArrowFontColor #5566FF
  }

  mqtt_client_subscriptions::topic_filter_2 -> uic_dotdot_mqtt_component
  uic_dotdot_mqtt_component -> dotdot_attribute_store_command_callbacks::OnOff_toggle
  uic_dotdot_mqtt_component -down-> protocol_controller_command_callbacks

  dotdot_attribute_store_command_callbacks -u-> dotdot_attribute_1 : Write the desired value\nin the DotDot space

  () "MQTT DotDot\nCommand Publication" as command_publication #5566FF
  command_publication --> mqtt_client_subscriptions

  skinparam {
    ArrowColor #FF6655
    ArrowFontColor #FF6655
  }

  dotdot_attribute_store_command_callbacks -> uic_dotdot_mqtt_component : Register command handlers
  protocol_controller_command_callbacks -u-> uic_dotdot_mqtt_component : Register additional command callbacks
  uic_dotdot_mqtt_component -> mqtt_client_subscriptions : Subscribe to relevant topics to\nreceving incoming publications

  skinparam {
    ArrowColor #5566FF
    ArrowFontColor #5566FF
  }
  protocol_controller_command_dispatch -l-> command_address_selection
  command_address_selection -d-> tx_rx_module : Send Command to PAN Node
  tx_rx_module -l-> pan_node : Send Command