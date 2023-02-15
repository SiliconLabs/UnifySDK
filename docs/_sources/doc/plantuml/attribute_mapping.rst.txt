.. uml::
  :align: center

  legend top
      |Background color | description |
      |<#ff9999>| Unify Component |
      |<#acffa0>| Protocol Controller Component |
  endlegend

  !theme plain
  allowmixing
  title Attribute Mapping

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
        Reported => [03]
        Desired  => [01 02 03]
      }
      map "DotDot Attribute 2" as dotdot_attribute_2 {
        Reported => ["Device Name"]
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

  Package "Attribute Mapper" as attribute_mapper #ff9999 {
    () "Maps Callbacks" as maps_callbacks
    () "Maps Eval" as maps_evaluations
    map "Mappings" as maps #acffa0 {
      Rule 1 => r'1 =  2 * r'2
      Rule 2 => d'2 =  d'1 / 2
      Rule 3 => r'3 =  r'4 + r'2
    }
    maps_callbacks --> maps
    maps_evaluations <-- maps
  }

  skinparam {
  ArrowColor #FF6655
  ArrowFontColor #FF6655
  }

  attribute_1 -d-> maps_callbacks : Read
  attribute_2 -d-> maps_callbacks
  dotdot_attribute_1 -d-> maps_callbacks
  dotdot_attribute_2 -d-> maps_callbacks

  skinparam {
  ArrowColor #5566FF
  ArrowFontColor #5566FF
  }

  maps_evaluations -u-> attribute_1
  maps_evaluations -u-> attribute_2
  maps_evaluations -u-> dotdot_attribute_1
  maps_evaluations -u-> dotdot_attribute_2 : Write
