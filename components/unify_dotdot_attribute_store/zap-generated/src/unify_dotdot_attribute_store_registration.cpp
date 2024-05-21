/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_registration.h"
#include "unify_dotdot_defined_attribute_types.h"

// Unify components
#include "attribute_store_type_registration.h"
#include "sl_log.h"

// Generic includes
#include <string>
#include <vector>

constexpr const char *LOG_TAG = "unify_dotdot_attribute_store_registration";

typedef struct {
  attribute_store_type_t type;
  const char *name;
  attribute_store_type_t parent_type;
  attribute_store_storage_type_t storage_type;
} attribute_schema_t;

// clang-format off
static const std::vector<attribute_schema_t> zcl_additional_attribute_schema = {
  /////////////////////////////////////////////////////////////////////
  //         ZCL Scenes cluster attributes
  /////////////////////////////////////////////////////////////////////
  // SceneTable attribute is present in the XML files.
  //{DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_TABLE,                  "ZCL Scenes SceneTable",  ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,                            EMPTY_STORAGE_TYPE},
  {DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_ID_GROUP_ID,            "Scene ID / Group ID",    DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_TABLE,           FIXED_SIZE_STRUCT_STORAGE_TYPE},
  {DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_TRANSITION_TIME,        "Transition Time",        DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_ID_GROUP_ID,     U16_STORAGE_TYPE},
  {DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_TRANSITION_TIME_100MS,  "Transition Time 100ms",  DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_ID_GROUP_ID,     U8_STORAGE_TYPE},
  {DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_NAME,                   "Name",                   DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_ID_GROUP_ID,     C_STRING_STORAGE_TYPE},
  {DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_FIELD_SETS,             "Extension FieldSets",    DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_ID_GROUP_ID,     EMPTY_STORAGE_TYPE},
  /////////////////////////////////////////////////////////////////////
  //         ZCL Level cluster attributes
  /////////////////////////////////////////////////////////////////////
  {DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL_LAST_NON_ZERO_VALUE,   "ZCL Level Last Non Zero Level",   ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,   U8_STORAGE_TYPE},
  //         ZCL Group cluster attributes
  {DOTDOT_ATTRIBUTE_ID_GROUPS_GROUP_ID,   "ZCL Group ID",   ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,   U16_STORAGE_TYPE},
  {DOTDOT_ATTRIBUTE_ID_GROUPS_GROUP_NAME,   "Name",   DOTDOT_ATTRIBUTE_ID_GROUPS_GROUP_ID,   C_STRING_STORAGE_TYPE},
};
// clang-format on

attribute_store_storage_type_t
  attribute_storage_type_conversion(std::string &attribute_type_string)
{
  attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

  // clang-format on
  if ((attribute_type_string == "uint8_t") || (attribute_type_string == "bool")
      || (attribute_type_string == "BatterySize")
      || (attribute_type_string == "DrlkOperMode")
      || (attribute_type_string == "CCColorLoopDirection")) {
    storage_type = U8_STORAGE_TYPE;
  } else if ((attribute_type_string == "uint16_t")
             || (attribute_type_string == "SGroupId")) {
    storage_type = U16_STORAGE_TYPE;
  } else if ((attribute_type_string == "uint32_t")
             || (attribute_type_string == "UTC")) {
    storage_type = U32_STORAGE_TYPE;
  } else if (attribute_type_string == "uint64_t") {
    storage_type = U64_STORAGE_TYPE;
  } else if (attribute_type_string == "int8_t") {
    storage_type = I8_STORAGE_TYPE;
  } else if (attribute_type_string == "int16_t") {
    storage_type = I16_STORAGE_TYPE;
  } else if (attribute_type_string == "int32_t") {
    storage_type = I32_STORAGE_TYPE;
  } else if (attribute_type_string == "uint64_t") {
    storage_type = I64_STORAGE_TYPE;
  } else if (attribute_type_string == "float") {
    storage_type = FLOAT_STORAGE_TYPE;
  } else if (attribute_type_string == "double") {
    storage_type = DOUBLE_STORAGE_TYPE;
  } else if (attribute_type_string == "EUI64") {
    storage_type = FIXED_SIZE_STRUCT_STORAGE_TYPE;
  } else if ((attribute_type_string == "PumpOperationMode")
             || (attribute_type_string == "PumpControlMode")
             || (attribute_type_string == "IasZoneType")
             || (attribute_type_string == "LoggingLevelEnum")
             || (attribute_type_string == "NodeStateSecurity")
             || (attribute_type_string == "NodeStateNetworkStatus")) {
    storage_type = ENUM_STORAGE_TYPE;
  }
  return storage_type;
}

sl_status_t unify_dotdot_attribute_store_registration_init()
{
  sl_status_t status = SL_STATUS_OK;
  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic ZCLVersion, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_ZCL_VERSION,
      "ZCL Basic ZCLVersion",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic ApplicationVersion, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_APPLICATION_VERSION,
      "ZCL Basic ApplicationVersion",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic StackVersion, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_STACK_VERSION,
      "ZCL Basic StackVersion",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic HWVersion, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_HW_VERSION,
      "ZCL Basic HWVersion",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic ManufacturerName, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_NAME,
      "ZCL Basic ManufacturerName",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic ModelIdentifier, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_MODEL_IDENTIFIER,
      "ZCL Basic ModelIdentifier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic DateCode, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_DATE_CODE,
      "ZCL Basic DateCode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic PowerSource, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_POWER_SOURCE,
      "ZCL Basic PowerSource",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic GenericDevice-Class, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_GENERIC_DEVICE_CLASS,
      "ZCL Basic GenericDevice-Class",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic GenericDevice-Type, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_GENERIC_DEVICE_TYPE,
      "ZCL Basic GenericDevice-Type",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // octstr // octstr // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic ProductCode, "
                     "type:  octstr // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_PRODUCT_CODE,
      "ZCL Basic ProductCode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic ProductURL, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_PRODUCTURL,
      "ZCL Basic ProductURL",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic ManufacturerVersionDetails, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_MANUFACTURER_VERSION_DETAILS,
      "ZCL Basic ManufacturerVersionDetails",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic SerialNumber, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_SERIAL_NUMBER,
      "ZCL Basic SerialNumber",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic ProductLabel, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_PRODUCT_LABEL,
      "ZCL Basic ProductLabel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic LocationDescription, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_LOCATION_DESCRIPTION,
      "ZCL Basic LocationDescription",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic PhysicalEnvironment, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_PHYSICAL_ENVIRONMENT,
      "ZCL Basic PhysicalEnvironment",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic DeviceEnabled, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_DEVICE_ENABLED,
      "ZCL Basic DeviceEnabled",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic AlarmMask, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_ALARM_MASK,
      "ZCL Basic AlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic DisableLocalConfig, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_DISABLE_LOCAL_CONFIG,
      "ZCL Basic DisableLocalConfig",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Basic SWBuildID, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BASIC_SW_BUILDID,
      "ZCL Basic SWBuildID",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration MainsVoltage, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_MAINS_VOLTAGE,
      "ZCL PowerConfiguration MainsVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration MainsFrequency, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_MAINS_FREQUENCY,
      "ZCL PowerConfiguration MainsFrequency",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration MainsAlarmMask, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_MAINS_ALARM_MASK,
      "ZCL PowerConfiguration MainsAlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration MainsVoltageMinThreshold, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_MAINS_VOLTAGE_MIN_THRESHOLD,
      "ZCL PowerConfiguration MainsVoltageMinThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration MainsVoltageMaxThreshold, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_MAINS_VOLTAGE_MAX_THRESHOLD,
      "ZCL PowerConfiguration MainsVoltageMaxThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration MainsVoltageDwellTripPoint, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_MAINS_VOLTAGE_DWELL_TRIP_POINT,
      "ZCL PowerConfiguration MainsVoltageDwellTripPoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryVoltage, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_VOLTAGE,
      "ZCL PowerConfiguration BatteryVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryPercentageRemaining, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_REMAINING,
      "ZCL PowerConfiguration BatteryPercentageRemaining",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryManufacturer, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_MANUFACTURER,
      "ZCL PowerConfiguration BatteryManufacturer",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatterySize // BatterySize // BatterySize
    std::string attribute_type_string           = "BatterySize";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatterySize, "
                     "type:  BatterySize // BatterySize");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_SIZE,
      "ZCL PowerConfiguration BatterySize",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryAHrRating, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERYA_HR_RATING,
      "ZCL PowerConfiguration BatteryAHrRating",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryQuantity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_QUANTITY,
      "ZCL PowerConfiguration BatteryQuantity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryRatedVoltage, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_RATED_VOLTAGE,
      "ZCL PowerConfiguration BatteryRatedVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatteryAlarmMask // BatteryAlarmMask // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryAlarmMask, "
                     "type:  BatteryAlarmMask // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_ALARM_MASK,
      "ZCL PowerConfiguration BatteryAlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryVoltageMinThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_VOLTAGE_MIN_THRESHOLD,
      "ZCL PowerConfiguration BatteryVoltageMinThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryVoltageThreshold1, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD1,
      "ZCL PowerConfiguration BatteryVoltageThreshold1",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryVoltageThreshold2, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD2,
      "ZCL PowerConfiguration BatteryVoltageThreshold2",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryVoltageThreshold3, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD3,
      "ZCL PowerConfiguration BatteryVoltageThreshold3",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryPercentageMinThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_MIN_THRESHOLD,
      "ZCL PowerConfiguration BatteryPercentageMinThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryPercentageThreshold1, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD1,
      "ZCL PowerConfiguration BatteryPercentageThreshold1",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryPercentageThreshold2, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD2,
      "ZCL PowerConfiguration BatteryPercentageThreshold2",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryPercentageThreshold3, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD3,
      "ZCL PowerConfiguration BatteryPercentageThreshold3",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatteryAlarmState // BatteryAlarmState // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration BatteryAlarmState, "
                     "type:  BatteryAlarmState // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY_ALARM_STATE,
      "ZCL PowerConfiguration BatteryAlarmState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2Voltage, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_VOLTAGE,
      "ZCL PowerConfiguration Battery2Voltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2PercentageRemaining, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_PERCENTAGE_REMAINING,
      "ZCL PowerConfiguration Battery2PercentageRemaining",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2Manufacturer, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_MANUFACTURER,
      "ZCL PowerConfiguration Battery2Manufacturer",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatterySize // BatterySize // BatterySize
    std::string attribute_type_string           = "BatterySize";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2Size, "
                     "type:  BatterySize // BatterySize");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_SIZE,
      "ZCL PowerConfiguration Battery2Size",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2AHrRating, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2A_HR_RATING,
      "ZCL PowerConfiguration Battery2AHrRating",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2Quantity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_QUANTITY,
      "ZCL PowerConfiguration Battery2Quantity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2RatedVoltage, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_RATED_VOLTAGE,
      "ZCL PowerConfiguration Battery2RatedVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatteryAlarmMask // BatteryAlarmMask // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2AlarmMask, "
                     "type:  BatteryAlarmMask // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_ALARM_MASK,
      "ZCL PowerConfiguration Battery2AlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2VoltageMinThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_VOLTAGE_MIN_THRESHOLD,
      "ZCL PowerConfiguration Battery2VoltageMinThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2VoltageThreshold1, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_VOLTAGE_THRESHOLD1,
      "ZCL PowerConfiguration Battery2VoltageThreshold1",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2VoltageThreshold2, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_VOLTAGE_THRESHOLD2,
      "ZCL PowerConfiguration Battery2VoltageThreshold2",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2VoltageThreshold3, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_VOLTAGE_THRESHOLD3,
      "ZCL PowerConfiguration Battery2VoltageThreshold3",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2PercentageMinThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_PERCENTAGE_MIN_THRESHOLD,
      "ZCL PowerConfiguration Battery2PercentageMinThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2PercentageThreshold1, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_PERCENTAGE_THRESHOLD1,
      "ZCL PowerConfiguration Battery2PercentageThreshold1",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2PercentageThreshold2, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_PERCENTAGE_THRESHOLD2,
      "ZCL PowerConfiguration Battery2PercentageThreshold2",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2PercentageThreshold3, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_PERCENTAGE_THRESHOLD3,
      "ZCL PowerConfiguration Battery2PercentageThreshold3",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatteryAlarmState // BatteryAlarmState // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery2AlarmState, "
                     "type:  BatteryAlarmState // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY2_ALARM_STATE,
      "ZCL PowerConfiguration Battery2AlarmState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3Voltage, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_VOLTAGE,
      "ZCL PowerConfiguration Battery3Voltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3PercentageRemaining, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_PERCENTAGE_REMAINING,
      "ZCL PowerConfiguration Battery3PercentageRemaining",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3Manufacturer, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_MANUFACTURER,
      "ZCL PowerConfiguration Battery3Manufacturer",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatterySize // BatterySize // BatterySize
    std::string attribute_type_string           = "BatterySize";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3Size, "
                     "type:  BatterySize // BatterySize");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_SIZE,
      "ZCL PowerConfiguration Battery3Size",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3AHrRating, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3A_HR_RATING,
      "ZCL PowerConfiguration Battery3AHrRating",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3Quantity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_QUANTITY,
      "ZCL PowerConfiguration Battery3Quantity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3RatedVoltage, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_RATED_VOLTAGE,
      "ZCL PowerConfiguration Battery3RatedVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatteryAlarmMask // BatteryAlarmMask // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3AlarmMask, "
                     "type:  BatteryAlarmMask // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_ALARM_MASK,
      "ZCL PowerConfiguration Battery3AlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3VoltageMinThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_VOLTAGE_MIN_THRESHOLD,
      "ZCL PowerConfiguration Battery3VoltageMinThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3VoltageThreshold1, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_VOLTAGE_THRESHOLD1,
      "ZCL PowerConfiguration Battery3VoltageThreshold1",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3VoltageThreshold2, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_VOLTAGE_THRESHOLD2,
      "ZCL PowerConfiguration Battery3VoltageThreshold2",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3VoltageThreshold3, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_VOLTAGE_THRESHOLD3,
      "ZCL PowerConfiguration Battery3VoltageThreshold3",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3PercentageMinThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_PERCENTAGE_MIN_THRESHOLD,
      "ZCL PowerConfiguration Battery3PercentageMinThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3PercentageThreshold1, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_PERCENTAGE_THRESHOLD1,
      "ZCL PowerConfiguration Battery3PercentageThreshold1",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3PercentageThreshold2, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_PERCENTAGE_THRESHOLD2,
      "ZCL PowerConfiguration Battery3PercentageThreshold2",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3PercentageThreshold3, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_PERCENTAGE_THRESHOLD3,
      "ZCL PowerConfiguration Battery3PercentageThreshold3",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BatteryAlarmState // BatteryAlarmState // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PowerConfiguration Battery3AlarmState, "
                     "type:  BatteryAlarmState // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POWER_CONFIGURATION_BATTERY3_ALARM_STATE,
      "ZCL PowerConfiguration Battery3AlarmState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration CurrentTemperature, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_CURRENT_TEMPERATURE,
      "ZCL DeviceTemperatureConfiguration CurrentTemperature",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration MinTempExperienced, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_MIN_TEMP_EXPERIENCED,
      "ZCL DeviceTemperatureConfiguration MinTempExperienced",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration MaxTempExperienced, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_MAX_TEMP_EXPERIENCED,
      "ZCL DeviceTemperatureConfiguration MaxTempExperienced",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration OverTempTotalDwell, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_OVER_TEMP_TOTAL_DWELL,
      "ZCL DeviceTemperatureConfiguration OverTempTotalDwell",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration DeviceTempAlarmMask, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_DEVICE_TEMP_ALARM_MASK,
      "ZCL DeviceTemperatureConfiguration DeviceTempAlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration LowTempThreshold, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_LOW_TEMP_THRESHOLD,
      "ZCL DeviceTemperatureConfiguration LowTempThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration HighTempThreshold, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_HIGH_TEMP_THRESHOLD,
      "ZCL DeviceTemperatureConfiguration HighTempThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration LowTempDwellTripPoint, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_LOW_TEMP_DWELL_TRIP_POINT,
      "ZCL DeviceTemperatureConfiguration LowTempDwellTripPoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DeviceTemperatureConfiguration HighTempDwellTripPoint, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEVICE_TEMPERATURE_CONFIGURATION_HIGH_TEMP_DWELL_TRIP_POINT,
      "ZCL DeviceTemperatureConfiguration HighTempDwellTripPoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Identify IdentifyTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
      "ZCL Identify IdentifyTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Groups NameSupport, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_GROUPS_NAME_SUPPORT,
      "ZCL Groups NameSupport",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Scenes SceneCount, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_COUNT,
      "ZCL Scenes SceneCount",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Scenes CurrentScene, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SCENES_CURRENT_SCENE,
      "ZCL Scenes CurrentScene",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // SGroupId // SGroupId // SGroupId
    std::string attribute_type_string           = "SGroupId";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Scenes CurrentGroup, "
                     "type:  SGroupId // SGroupId");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SCENES_CURRENT_GROUP,
      "ZCL Scenes CurrentGroup",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Scenes SceneValid, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_VALID,
      "ZCL Scenes SceneValid",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Scenes NameSupport, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SCENES_NAME_SUPPORT,
      "ZCL Scenes NameSupport",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // EUI64 // EUI64 // EUI64
    std::string attribute_type_string           = "EUI64";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Scenes LastConfiguredBy, "
                     "type:  EUI64 // EUI64");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SCENES_LAST_CONFIGURED_BY,
      "ZCL Scenes LastConfiguredBy",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // SSceneTable // SSceneTable // SSceneTable
    std::string attribute_type_string           = "SSceneTable";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Scenes SceneTable, "
                     "type:  SSceneTable // SSceneTable");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SCENES_SCENE_TABLE,
      "ZCL Scenes SceneTable",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OnOff OnOff, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
      "ZCL OnOff OnOff",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OnOff GlobalSceneControl, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ON_OFF_GLOBAL_SCENE_CONTROL,
      "ZCL OnOff GlobalSceneControl",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OnOff OnTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME,
      "ZCL OnOff OnTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OnOff OffWaitTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ON_OFF_OFF_WAIT_TIME,
      "ZCL OnOff OffWaitTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OnOff StartUpOnOff, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ON_OFF_START_UP_ON_OFF,
      "ZCL OnOff StartUpOnOff",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level CurrentLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
      "ZCL Level CurrentLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level RemainingTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_REMAINING_TIME,
      "ZCL Level RemainingTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level MinLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_MIN_LEVEL,
      "ZCL Level MinLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level MaxLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_MAX_LEVEL,
      "ZCL Level MaxLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level CurrentFrequency, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_FREQUENCY,
      "ZCL Level CurrentFrequency",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level MinFrequency, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_MIN_FREQUENCY,
      "ZCL Level MinFrequency",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level MaxFrequency, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_MAX_FREQUENCY,
      "ZCL Level MaxFrequency",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // LevelOptions // LevelOptions // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level Options, "
                     "type:  LevelOptions // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_OPTIONS,
      "ZCL Level Options",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level OnOffTransitionTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
      "ZCL Level OnOffTransitionTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level OnLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_ON_LEVEL,
      "ZCL Level OnLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level OnTransitionTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_ON_TRANSITION_TIME,
      "ZCL Level OnTransitionTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level OffTransitionTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_OFF_TRANSITION_TIME,
      "ZCL Level OffTransitionTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level DefaultMoveRate, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_DEFAULT_MOVE_RATE,
      "ZCL Level DefaultMoveRate",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Level StartUpCurrentLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_LEVEL_START_UP_CURRENT_LEVEL,
      "ZCL Level StartUpCurrentLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Alarms AlarmCount, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ALARMS_ALARM_COUNT,
      "ZCL Alarms AlarmCount",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // UTC // UTC // UTC
    std::string attribute_type_string           = "UTC";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time Time, "
                     "type:  UTC // UTC");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_TIME,
      "ZCL Time Time",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time TimeStatus, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_TIME_STATUS,
      "ZCL Time TimeStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time TimeZone, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_TIME_ZONE,
      "ZCL Time TimeZone",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time DstStart, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_DST_START,
      "ZCL Time DstStart",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time DstEnd, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_DST_END,
      "ZCL Time DstEnd",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time DstShift, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_DST_SHIFT,
      "ZCL Time DstShift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time StandardTime, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_STANDARD_TIME,
      "ZCL Time StandardTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time LocalTime, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_LOCAL_TIME,
      "ZCL Time LocalTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // UTC // UTC // UTC
    std::string attribute_type_string           = "UTC";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time LastSetTime, "
                     "type:  UTC // UTC");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_LAST_SET_TIME,
      "ZCL Time LastSetTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // UTC // UTC // UTC
    std::string attribute_type_string           = "UTC";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Time ValidUntilTime, "
                     "type:  UTC // UTC");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TIME_VALID_UNTIL_TIME,
      "ZCL Time ValidUntilTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PollControl CheckInInterval, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POLL_CONTROL_CHECK_IN_INTERVAL,
      "ZCL PollControl CheckInInterval",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PollControl LongPollInterval, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POLL_CONTROL_LONG_POLL_INTERVAL,
      "ZCL PollControl LongPollInterval",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PollControl ShortPollInterval, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POLL_CONTROL_SHORT_POLL_INTERVAL,
      "ZCL PollControl ShortPollInterval",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PollControl FastPollTimeout, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POLL_CONTROL_FAST_POLL_TIMEOUT,
      "ZCL PollControl FastPollTimeout",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PollControl CheckInIntervalMin, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POLL_CONTROL_CHECK_IN_INTERVAL_MIN,
      "ZCL PollControl CheckInIntervalMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PollControl LongPollIntervalMin, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POLL_CONTROL_LONG_POLL_INTERVAL_MIN,
      "ZCL PollControl LongPollIntervalMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PollControl FastPollTimeoutMax, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_POLL_CONTROL_FAST_POLL_TIMEOUT_MAX,
      "ZCL PollControl FastPollTimeoutMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ShadeConfiguration PhysicalClosedLimit, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SHADE_CONFIGURATION_PHYSICAL_CLOSED_LIMIT,
      "ZCL ShadeConfiguration PhysicalClosedLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ShadeConfiguration MotorStepSize, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SHADE_CONFIGURATION_MOTOR_STEP_SIZE,
      "ZCL ShadeConfiguration MotorStepSize",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ShadeConfiguration Status, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SHADE_CONFIGURATION_STATUS,
      "ZCL ShadeConfiguration Status",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ShadeConfiguration ClosedLimit, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SHADE_CONFIGURATION_CLOSED_LIMIT,
      "ZCL ShadeConfiguration ClosedLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ShadeConfiguration Mode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SHADE_CONFIGURATION_MODE,
      "ZCL ShadeConfiguration Mode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock LockState, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LOCK_STATE,
      "ZCL DoorLock LockState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock LockType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LOCK_TYPE,
      "ZCL DoorLock LockType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock ActuatorEnabled, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_ACTUATOR_ENABLED,
      "ZCL DoorLock ActuatorEnabled",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock DoorState, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_DOOR_STATE,
      "ZCL DoorLock DoorState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock DoorOpenEvents, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_DOOR_OPEN_EVENTS,
      "ZCL DoorLock DoorOpenEvents",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock DoorClosedEvents, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_DOOR_CLOSED_EVENTS,
      "ZCL DoorLock DoorClosedEvents",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock OpenPeriod, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_OPEN_PERIOD,
      "ZCL DoorLock OpenPeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock NumberOfLogRecordsSupported, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_NUMBER_OF_LOG_RECORDS_SUPPORTED,
      "ZCL DoorLock NumberOfLogRecordsSupported",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock NumberOfTotalUsersSupported, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_NUMBER_OF_TOTAL_USERS_SUPPORTED,
      "ZCL DoorLock NumberOfTotalUsersSupported",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock NumberOfPINUsersSupported, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_NUMBER_OF_PIN_USERS_SUPPORTED,
      "ZCL DoorLock NumberOfPINUsersSupported",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock NumberOfRFIDUsersSupported, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_NUMBER_OF_RFID_USERS_SUPPORTED,
      "ZCL DoorLock NumberOfRFIDUsersSupported",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock NumberOfWeekDaySchedulesSupportedPerUser, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_NUMBER_OF_WEEK_DAY_SCHEDULES_SUPPORTED_PER_USER,
      "ZCL DoorLock NumberOfWeekDaySchedulesSupportedPerUser",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock NumberOfYearDaySchedulesSupportedPerUser, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_NUMBER_OF_YEAR_DAY_SCHEDULES_SUPPORTED_PER_USER,
      "ZCL DoorLock NumberOfYearDaySchedulesSupportedPerUser",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock NumberOfHolidaySchedulesSupported, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_NUMBER_OF_HOLIDAY_SCHEDULES_SUPPORTED,
      "ZCL DoorLock NumberOfHolidaySchedulesSupported",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock MaxPINCodeLength, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_MAX_PIN_CODE_LENGTH,
      "ZCL DoorLock MaxPINCodeLength",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock MinPINCodeLength, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_MIN_PIN_CODE_LENGTH,
      "ZCL DoorLock MinPINCodeLength",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock MaxRFIDCodeLength, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_MAX_RFID_CODE_LENGTH,
      "ZCL DoorLock MaxRFIDCodeLength",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock MinRFIDCodeLength, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_MIN_RFID_CODE_LENGTH,
      "ZCL DoorLock MinRFIDCodeLength",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock CredentialRulesSupport, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_CREDENTIAL_RULES_SUPPORT,
      "ZCL DoorLock CredentialRulesSupport",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock NumberOfCredentialsSupportedPerUser, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_NUMBER_OF_CREDENTIALS_SUPPORTED_PER_USER,
      "ZCL DoorLock NumberOfCredentialsSupportedPerUser",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock EnableLogging, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_ENABLE_LOGGING,
      "ZCL DoorLock EnableLogging",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock Language, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LANGUAGE,
      "ZCL DoorLock Language",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock LEDSettings, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LED_SETTINGS,
      "ZCL DoorLock LEDSettings",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock AutoRelockTime, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_AUTO_RELOCK_TIME,
      "ZCL DoorLock AutoRelockTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock SoundVolume, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_SOUND_VOLUME,
      "ZCL DoorLock SoundVolume",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // DrlkOperMode // DrlkOperMode // DrlkOperMode
    std::string attribute_type_string           = "DrlkOperMode";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock OperatingMode, "
                     "type:  DrlkOperMode // DrlkOperMode");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_OPERATING_MODE,
      "ZCL DoorLock OperatingMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock SupportedOperatingModes, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_SUPPORTED_OPERATING_MODES,
      "ZCL DoorLock SupportedOperatingModes",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock DefaultConfigurationRegister, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_DEFAULT_CONFIGURATION_REGISTER,
      "ZCL DoorLock DefaultConfigurationRegister",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock EnableLocalProgramming, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_ENABLE_LOCAL_PROGRAMMING,
      "ZCL DoorLock EnableLocalProgramming",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock EnableOneTouchLocking, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_ENABLE_ONE_TOUCH_LOCKING,
      "ZCL DoorLock EnableOneTouchLocking",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock EnableInsideStatusLED, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_ENABLE_INSIDE_STATUSLED,
      "ZCL DoorLock EnableInsideStatusLED",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock EnablePrivacyModeButton, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_ENABLE_PRIVACY_MODE_BUTTON,
      "ZCL DoorLock EnablePrivacyModeButton",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock LocalProgrammingFeatures, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LOCAL_PROGRAMMING_FEATURES,
      "ZCL DoorLock LocalProgrammingFeatures",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock WrongCodeEntryLimit, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_WRONG_CODE_ENTRY_LIMIT,
      "ZCL DoorLock WrongCodeEntryLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock UserCodeTemporaryDisableTime, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_USER_CODE_TEMPORARY_DISABLE_TIME,
      "ZCL DoorLock UserCodeTemporaryDisableTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock SendPINOverTheAir, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_SEND_PIN_OVER_THE_AIR,
      "ZCL DoorLock SendPINOverTheAir",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock RequirePINforRFOperation, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_REQUIRE_PI_NFOR_RF_OPERATION,
      "ZCL DoorLock RequirePINforRFOperation",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock SecurityLevel, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_SECURITY_LEVEL,
      "ZCL DoorLock SecurityLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock ExpiringUserTimeout, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_EXPIRING_USER_TIMEOUT,
      "ZCL DoorLock ExpiringUserTimeout",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock AlarmMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_ALARM_MASK,
      "ZCL DoorLock AlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock KeypadOperationEventMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_KEYPAD_OPERATION_EVENT_MASK,
      "ZCL DoorLock KeypadOperationEventMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock RFOperationEventMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_RF_OPERATION_EVENT_MASK,
      "ZCL DoorLock RFOperationEventMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock ManualOperationEventMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_MANUAL_OPERATION_EVENT_MASK,
      "ZCL DoorLock ManualOperationEventMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock RFIDOperationEventMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_RFID_OPERATION_EVENT_MASK,
      "ZCL DoorLock RFIDOperationEventMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock KeypadProgrammingEventMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_KEYPAD_PROGRAMMING_EVENT_MASK,
      "ZCL DoorLock KeypadProgrammingEventMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock RFProgrammingEventMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_RF_PROGRAMMING_EVENT_MASK,
      "ZCL DoorLock RFProgrammingEventMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock RFIDProgrammingEventMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_RFID_PROGRAMMING_EVENT_MASK,
      "ZCL DoorLock RFIDProgrammingEventMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DoorLock FeatureMap, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_FEATURE_MAP,
      "ZCL DoorLock FeatureMap",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering WindowCoveringType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_WINDOW_COVERING_TYPE,
      "ZCL WindowCovering WindowCoveringType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering PhysicalClosedLimitLift, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_PHYSICAL_CLOSED_LIMIT_LIFT,
      "ZCL WindowCovering PhysicalClosedLimitLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering PhysicalClosedLimitTilt, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_PHYSICAL_CLOSED_LIMIT_TILT,
      "ZCL WindowCovering PhysicalClosedLimitTilt",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering CurrentPositionLift, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_CURRENT_POSITION_LIFT,
      "ZCL WindowCovering CurrentPositionLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering CurrentPositionTilt, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_CURRENT_POSITION_TILT,
      "ZCL WindowCovering CurrentPositionTilt",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering NumberOfActuationsLift, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_NUMBER_OF_ACTUATIONS_LIFT,
      "ZCL WindowCovering NumberOfActuationsLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering NumberOfActuationsTilt, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_NUMBER_OF_ACTUATIONS_TILT,
      "ZCL WindowCovering NumberOfActuationsTilt",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering ConfigOrStatus, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_CONFIG_OR_STATUS,
      "ZCL WindowCovering ConfigOrStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering CurrentPositionLiftPercentage, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_CURRENT_POSITION_LIFT_PERCENTAGE,
      "ZCL WindowCovering CurrentPositionLiftPercentage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering CurrentPositionTiltPercentage, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_CURRENT_POSITION_TILT_PERCENTAGE,
      "ZCL WindowCovering CurrentPositionTiltPercentage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering InstalledOpenLimitLift, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_INSTALLED_OPEN_LIMIT_LIFT,
      "ZCL WindowCovering InstalledOpenLimitLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering InstalledClosedLimitLift, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_INSTALLED_CLOSED_LIMIT_LIFT,
      "ZCL WindowCovering InstalledClosedLimitLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering InstalledOpenLimitTilt, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_INSTALLED_OPEN_LIMIT_TILT,
      "ZCL WindowCovering InstalledOpenLimitTilt",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering InstalledClosedLimitTilt, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_INSTALLED_CLOSED_LIMIT_TILT,
      "ZCL WindowCovering InstalledClosedLimitTilt",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering VelocityLift, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_VELOCITY_LIFT,
      "ZCL WindowCovering VelocityLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering AccelerationTimeLift, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_ACCELERATION_TIME_LIFT,
      "ZCL WindowCovering AccelerationTimeLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering DecelerationTimeLift, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_DECELERATION_TIME_LIFT,
      "ZCL WindowCovering DecelerationTimeLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering Mode, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_MODE,
      "ZCL WindowCovering Mode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // octstr // octstr // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering IntermediateSetpointsLift, "
                     "type:  octstr // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_INTERMEDIATE_SETPOINTS_LIFT,
      "ZCL WindowCovering IntermediateSetpointsLift",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // octstr // octstr // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindowCovering IntermediateSetpointsTilt, "
                     "type:  octstr // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WINDOW_COVERING_INTERMEDIATE_SETPOINTS_TILT,
      "ZCL WindowCovering IntermediateSetpointsTilt",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl MovingState, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_MOVING_STATE,
      "ZCL BarrierControl MovingState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl SafetyStatus, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_SAFETY_STATUS,
      "ZCL BarrierControl SafetyStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl Capabilities, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_CAPABILITIES,
      "ZCL BarrierControl Capabilities",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl OpenEvents, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_OPEN_EVENTS,
      "ZCL BarrierControl OpenEvents",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl CloseEvents, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_CLOSE_EVENTS,
      "ZCL BarrierControl CloseEvents",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl CommandOpenEvents, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_COMMAND_OPEN_EVENTS,
      "ZCL BarrierControl CommandOpenEvents",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl CommandCloseEvents, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_COMMAND_CLOSE_EVENTS,
      "ZCL BarrierControl CommandCloseEvents",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl OpenPeriod, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_OPEN_PERIOD,
      "ZCL BarrierControl OpenPeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl ClosePeriod, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_CLOSE_PERIOD,
      "ZCL BarrierControl ClosePeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BarrierControl BarrierPosition, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BARRIER_CONTROL_BARRIER_POSITION,
      "ZCL BarrierControl BarrierPosition",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MaxPressure, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MAX_PRESSURE,
      "ZCL PumpConfigurationAndControl MaxPressure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MaxSpeed, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MAX_SPEED,
      "ZCL PumpConfigurationAndControl MaxSpeed",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MaxFlow, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MAX_FLOW,
      "ZCL PumpConfigurationAndControl MaxFlow",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MinConstPressure, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MIN_CONST_PRESSURE,
      "ZCL PumpConfigurationAndControl MinConstPressure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MaxConstPressure, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MAX_CONST_PRESSURE,
      "ZCL PumpConfigurationAndControl MaxConstPressure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MinCompPressure, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MIN_COMP_PRESSURE,
      "ZCL PumpConfigurationAndControl MinCompPressure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MaxCompPressure, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MAX_COMP_PRESSURE,
      "ZCL PumpConfigurationAndControl MaxCompPressure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MinConstSpeed, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MIN_CONST_SPEED,
      "ZCL PumpConfigurationAndControl MinConstSpeed",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MaxConstSpeed, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MAX_CONST_SPEED,
      "ZCL PumpConfigurationAndControl MaxConstSpeed",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MinConstFlow, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MIN_CONST_FLOW,
      "ZCL PumpConfigurationAndControl MinConstFlow",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MaxConstFlow, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MAX_CONST_FLOW,
      "ZCL PumpConfigurationAndControl MaxConstFlow",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MinConstTemp, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MIN_CONST_TEMP,
      "ZCL PumpConfigurationAndControl MinConstTemp",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl MaxConstTemp, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_MAX_CONST_TEMP,
      "ZCL PumpConfigurationAndControl MaxConstTemp",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl PumpStatus, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_PUMP_STATUS,
      "ZCL PumpConfigurationAndControl PumpStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // PumpOperationMode // PumpOperationMode // PumpOperationMode
    std::string attribute_type_string           = "PumpOperationMode";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl EffectiveOperationMode, "
                     "type:  PumpOperationMode // PumpOperationMode");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_EFFECTIVE_OPERATION_MODE,
      "ZCL PumpConfigurationAndControl EffectiveOperationMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // PumpControlMode // PumpControlMode // PumpControlMode
    std::string attribute_type_string           = "PumpControlMode";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl EffectiveControlMode, "
                     "type:  PumpControlMode // PumpControlMode");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_EFFECTIVE_CONTROL_MODE,
      "ZCL PumpConfigurationAndControl EffectiveControlMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl Capacity, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_CAPACITY,
      "ZCL PumpConfigurationAndControl Capacity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl Speed, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_SPEED,
      "ZCL PumpConfigurationAndControl Speed",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl LifetimeRunningHours, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_LIFETIME_RUNNING_HOURS,
      "ZCL PumpConfigurationAndControl LifetimeRunningHours",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl Power, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_POWER,
      "ZCL PumpConfigurationAndControl Power",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl LifetimeEnergyConsumed, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_LIFETIME_ENERGY_CONSUMED,
      "ZCL PumpConfigurationAndControl LifetimeEnergyConsumed",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // PumpOperationMode // PumpOperationMode // PumpOperationMode
    std::string attribute_type_string           = "PumpOperationMode";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl OperationMode, "
                     "type:  PumpOperationMode // PumpOperationMode");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_OPERATION_MODE,
      "ZCL PumpConfigurationAndControl OperationMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // PumpControlMode // PumpControlMode // PumpControlMode
    std::string attribute_type_string           = "PumpControlMode";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl ControlMode, "
                     "type:  PumpControlMode // PumpControlMode");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_CONTROL_MODE,
      "ZCL PumpConfigurationAndControl ControlMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PumpConfigurationAndControl AlarmMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PUMP_CONFIGURATION_AND_CONTROL_ALARM_MASK,
      "ZCL PumpConfigurationAndControl AlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat LocalTemperature, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_LOCAL_TEMPERATURE,
      "ZCL Thermostat LocalTemperature",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat OutdoorTemperature, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OUTDOOR_TEMPERATURE,
      "ZCL Thermostat OutdoorTemperature",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat Occupancy, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPANCY,
      "ZCL Thermostat Occupancy",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat AbsMinHeatSetpointLimit, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_ABS_MIN_HEAT_SETPOINT_LIMIT,
      "ZCL Thermostat AbsMinHeatSetpointLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat AbsMaxHeatSetpointLimit, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_ABS_MAX_HEAT_SETPOINT_LIMIT,
      "ZCL Thermostat AbsMaxHeatSetpointLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat AbsMinCoolSetpointLimit, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_ABS_MIN_COOL_SETPOINT_LIMIT,
      "ZCL Thermostat AbsMinCoolSetpointLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat AbsMaxCoolSetpointLimit, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_ABS_MAX_COOL_SETPOINT_LIMIT,
      "ZCL Thermostat AbsMaxCoolSetpointLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat PICoolingDemand, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_PI_COOLING_DEMAND,
      "ZCL Thermostat PICoolingDemand",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat PIHeatingDemand, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_PI_HEATING_DEMAND,
      "ZCL Thermostat PIHeatingDemand",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat HVACSystemTypeConfiguration, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_HVAC_SYSTEM_TYPE_CONFIGURATION,
      "ZCL Thermostat HVACSystemTypeConfiguration",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat LocalTemperatureCalibration, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_LOCAL_TEMPERATURE_CALIBRATION,
      "ZCL Thermostat LocalTemperatureCalibration",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat OccupiedCoolingSetpoint, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_COOLING_SETPOINT,
      "ZCL Thermostat OccupiedCoolingSetpoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat OccupiedHeatingSetpoint, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_HEATING_SETPOINT,
      "ZCL Thermostat OccupiedHeatingSetpoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat UnoccupiedCoolingSetpoint, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_UNOCCUPIED_COOLING_SETPOINT,
      "ZCL Thermostat UnoccupiedCoolingSetpoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat UnoccupiedHeatingSetpoint, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_UNOCCUPIED_HEATING_SETPOINT,
      "ZCL Thermostat UnoccupiedHeatingSetpoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat MinHeatSetpointLimit, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT,
      "ZCL Thermostat MinHeatSetpointLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat MaxHeatSetpointLimit, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT,
      "ZCL Thermostat MaxHeatSetpointLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat MinCoolSetpointLimit, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT,
      "ZCL Thermostat MinCoolSetpointLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat MaxCoolSetpointLimit, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT,
      "ZCL Thermostat MaxCoolSetpointLimit",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat MinSetpointDeadBand, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MIN_SETPOINT_DEAD_BAND,
      "ZCL Thermostat MinSetpointDeadBand",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat RemoteSensing, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_REMOTE_SENSING,
      "ZCL Thermostat RemoteSensing",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ControlSequenceOfOperation, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_CONTROL_SEQUENCE_OF_OPERATION,
      "ZCL Thermostat ControlSequenceOfOperation",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat SystemMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_SYSTEM_MODE,
      "ZCL Thermostat SystemMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat AlarmMask, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_ALARM_MASK,
      "ZCL Thermostat AlarmMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ThermostatRunningMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_THERMOSTAT_RUNNING_MODE,
      "ZCL Thermostat ThermostatRunningMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat StartOfWeek, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_START_OF_WEEK,
      "ZCL Thermostat StartOfWeek",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat NumberOfWeeklyTransitions, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_NUMBER_OF_WEEKLY_TRANSITIONS,
      "ZCL Thermostat NumberOfWeeklyTransitions",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat NumberOfDailyTransitions, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_NUMBER_OF_DAILY_TRANSITIONS,
      "ZCL Thermostat NumberOfDailyTransitions",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat TemperatureSetpointHold, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD,
      "ZCL Thermostat TemperatureSetpointHold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat TemperatureSetpointHoldDuration, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_TEMPERATURE_SETPOINT_HOLD_DURATION,
      "ZCL Thermostat TemperatureSetpointHoldDuration",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ThermostatProgrammingOperationMode, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE,
      "ZCL Thermostat ThermostatProgrammingOperationMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ThermostatRunningState, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_THERMOSTAT_RUNNING_STATE,
      "ZCL Thermostat ThermostatRunningState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat SetpointChangeSource, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_SETPOINT_CHANGE_SOURCE,
      "ZCL Thermostat SetpointChangeSource",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat SetpointChangeAmount, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_SETPOINT_CHANGE_AMOUNT,
      "ZCL Thermostat SetpointChangeAmount",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // UTC // UTC // UTC
    std::string attribute_type_string           = "UTC";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat SetpointChangeSourceTimestamp, "
                     "type:  UTC // UTC");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_SETPOINT_CHANGE_SOURCE_TIMESTAMP,
      "ZCL Thermostat SetpointChangeSourceTimestamp",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat OccupiedSetback, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_SETBACK,
      "ZCL Thermostat OccupiedSetback",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat OccupiedSetbackMin, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_SETBACK_MIN,
      "ZCL Thermostat OccupiedSetbackMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat OccupiedSetbackMax, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_SETBACK_MAX,
      "ZCL Thermostat OccupiedSetbackMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat UnoccupiedSetback, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_UNOCCUPIED_SETBACK,
      "ZCL Thermostat UnoccupiedSetback",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat UnoccupiedSetbackMin, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_UNOCCUPIED_SETBACK_MIN,
      "ZCL Thermostat UnoccupiedSetbackMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat UnoccupiedSetbackMax, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_UNOCCUPIED_SETBACK_MAX,
      "ZCL Thermostat UnoccupiedSetbackMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat EmergencyHeatDelta, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_EMERGENCY_HEAT_DELTA,
      "ZCL Thermostat EmergencyHeatDelta",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ACType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_AC_TYPE,
      "ZCL Thermostat ACType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ACCapacity, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_AC_CAPACITY,
      "ZCL Thermostat ACCapacity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ACRefrigerantType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_AC_REFRIGERANT_TYPE,
      "ZCL Thermostat ACRefrigerantType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ACCompressorType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_AC_COMPRESSOR_TYPE,
      "ZCL Thermostat ACCompressorType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map32 // map32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ACErrorCode, "
                     "type:  map32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_AC_ERROR_CODE,
      "ZCL Thermostat ACErrorCode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ACLouverPosition, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_AC_LOUVER_POSITION,
      "ZCL Thermostat ACLouverPosition",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ACCoilTemperature, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_AC_COIL_TEMPERATURE,
      "ZCL Thermostat ACCoilTemperature",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Thermostat ACCapacityFormat, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_AC_CAPACITY_FORMAT,
      "ZCL Thermostat ACCapacityFormat",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL FanControl FanMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_FAN_CONTROL_FAN_MODE,
      "ZCL FanControl FanMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL FanControl FanModeSequence, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_FAN_CONTROL_FAN_MODE_SEQUENCE,
      "ZCL FanControl FanModeSequence",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DehumidificationControl RelativeHumidity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEHUMIDIFICATION_CONTROL_RELATIVE_HUMIDITY,
      "ZCL DehumidificationControl RelativeHumidity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DehumidificationControl DehumidificationCooling, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEHUMIDIFICATION_CONTROL_DEHUMIDIFICATION_COOLING,
      "ZCL DehumidificationControl DehumidificationCooling",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DehumidificationControl RHDehumidificationSetpoint, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEHUMIDIFICATION_CONTROL_RH_DEHUMIDIFICATION_SETPOINT,
      "ZCL DehumidificationControl RHDehumidificationSetpoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DehumidificationControl RelativeHumidityMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEHUMIDIFICATION_CONTROL_RELATIVE_HUMIDITY_MODE,
      "ZCL DehumidificationControl RelativeHumidityMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DehumidificationControl DehumidificationLockout, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEHUMIDIFICATION_CONTROL_DEHUMIDIFICATION_LOCKOUT,
      "ZCL DehumidificationControl DehumidificationLockout",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DehumidificationControl DehumidificationHysteresis, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEHUMIDIFICATION_CONTROL_DEHUMIDIFICATION_HYSTERESIS,
      "ZCL DehumidificationControl DehumidificationHysteresis",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DehumidificationControl DehumidificationMaxCool, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEHUMIDIFICATION_CONTROL_DEHUMIDIFICATION_MAX_COOL,
      "ZCL DehumidificationControl DehumidificationMaxCool",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL DehumidificationControl RelativeHumidityDisplay, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DEHUMIDIFICATION_CONTROL_RELATIVE_HUMIDITY_DISPLAY,
      "ZCL DehumidificationControl RelativeHumidityDisplay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ThermostatUserInterfaceConfiguration TemperatureDisplayMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_USER_INTERFACE_CONFIGURATION_TEMPERATURE_DISPLAY_MODE,
      "ZCL ThermostatUserInterfaceConfiguration TemperatureDisplayMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ThermostatUserInterfaceConfiguration KeypadLockout, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_USER_INTERFACE_CONFIGURATION_KEYPAD_LOCKOUT,
      "ZCL ThermostatUserInterfaceConfiguration KeypadLockout",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ThermostatUserInterfaceConfiguration ScheduleProgrammingVisibility, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_THERMOSTAT_USER_INTERFACE_CONFIGURATION_SCHEDULE_PROGRAMMING_VISIBILITY,
      "ZCL ThermostatUserInterfaceConfiguration ScheduleProgrammingVisibility",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl CurrentHue, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
      "ZCL ColorControl CurrentHue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl CurrentSaturation, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
      "ZCL ColorControl CurrentSaturation",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl RemainingTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_REMAINING_TIME,
      "ZCL ColorControl RemainingTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl CurrentX, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENTX,
      "ZCL ColorControl CurrentX",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl CurrentY, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENTY,
      "ZCL ColorControl CurrentY",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl DriftCompensation, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_DRIFT_COMPENSATION,
      "ZCL ColorControl DriftCompensation",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl CompensationText, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COMPENSATION_TEXT,
      "ZCL ColorControl CompensationText",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorTemperatureMireds, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_TEMPERATURE_MIREDS,
      "ZCL ColorControl ColorTemperatureMireds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_MODE,
      "ZCL ColorControl ColorMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // CCColorOptions // CCColorOptions // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Options, "
                     "type:  CCColorOptions // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_OPTIONS,
      "ZCL ColorControl Options",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl NumberOfPrimaries, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_NUMBER_OF_PRIMARIES,
      "ZCL ColorControl NumberOfPrimaries",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary1X, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY1X,
      "ZCL ColorControl Primary1X",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary1Y, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY1Y,
      "ZCL ColorControl Primary1Y",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary1Intensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY1_INTENSITY,
      "ZCL ColorControl Primary1Intensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary2X, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY2X,
      "ZCL ColorControl Primary2X",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary2Y, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY2Y,
      "ZCL ColorControl Primary2Y",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary2Intensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY2_INTENSITY,
      "ZCL ColorControl Primary2Intensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary3X, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY3X,
      "ZCL ColorControl Primary3X",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary3Y, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY3Y,
      "ZCL ColorControl Primary3Y",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary3Intensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY3_INTENSITY,
      "ZCL ColorControl Primary3Intensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary4X, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY4X,
      "ZCL ColorControl Primary4X",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary4Y, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY4Y,
      "ZCL ColorControl Primary4Y",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary4Intensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY4_INTENSITY,
      "ZCL ColorControl Primary4Intensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary5X, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY5X,
      "ZCL ColorControl Primary5X",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary5Y, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY5Y,
      "ZCL ColorControl Primary5Y",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary5Intensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY5_INTENSITY,
      "ZCL ColorControl Primary5Intensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary6X, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY6X,
      "ZCL ColorControl Primary6X",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary6Y, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY6Y,
      "ZCL ColorControl Primary6Y",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl Primary6Intensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_PRIMARY6_INTENSITY,
      "ZCL ColorControl Primary6Intensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl WhitePointX, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_WHITE_POINTX,
      "ZCL ColorControl WhitePointX",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl WhitePointY, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_WHITE_POINTY,
      "ZCL ColorControl WhitePointY",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointRX, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTRX,
      "ZCL ColorControl ColorPointRX",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointRY, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTRY,
      "ZCL ColorControl ColorPointRY",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointRIntensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTR_INTENSITY,
      "ZCL ColorControl ColorPointRIntensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointGX, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTGX,
      "ZCL ColorControl ColorPointGX",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointGY, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTGY,
      "ZCL ColorControl ColorPointGY",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointGIntensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTG_INTENSITY,
      "ZCL ColorControl ColorPointGIntensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointBX, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTBX,
      "ZCL ColorControl ColorPointBX",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointBY, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTBY,
      "ZCL ColorControl ColorPointBY",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorPointBIntensity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_POINTB_INTENSITY,
      "ZCL ColorControl ColorPointBIntensity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl EnhancedCurrentHue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_ENHANCED_CURRENT_HUE,
      "ZCL ColorControl EnhancedCurrentHue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl EnhancedColorMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_ENHANCED_COLOR_MODE,
      "ZCL ColorControl EnhancedColorMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorLoopActive, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_LOOP_ACTIVE,
      "ZCL ColorControl ColorLoopActive",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // CCColorLoopDirection // CCColorLoopDirection // CCColorLoopDirection
    std::string attribute_type_string           = "CCColorLoopDirection";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorLoopDirection, "
                     "type:  CCColorLoopDirection // CCColorLoopDirection");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_LOOP_DIRECTION,
      "ZCL ColorControl ColorLoopDirection",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorLoopTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_LOOP_TIME,
      "ZCL ColorControl ColorLoopTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorLoopStartEnhancedHue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_LOOP_START_ENHANCED_HUE,
      "ZCL ColorControl ColorLoopStartEnhancedHue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorLoopStoredEnhancedHue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_LOOP_STORED_ENHANCED_HUE,
      "ZCL ColorControl ColorLoopStoredEnhancedHue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorCapabilities, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
      "ZCL ColorControl ColorCapabilities",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorTempPhysicalMinMireds, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN_MIREDS,
      "ZCL ColorControl ColorTempPhysicalMinMireds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl ColorTempPhysicalMaxMireds, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX_MIREDS,
      "ZCL ColorControl ColorTempPhysicalMaxMireds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl CoupleColorTempToLevelMinMireds, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COUPLE_COLOR_TEMP_TO_LEVEL_MIN_MIREDS,
      "ZCL ColorControl CoupleColorTempToLevelMinMireds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ColorControl StartUpColorTemperatureMireds, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_START_UP_COLOR_TEMPERATURE_MIREDS,
      "ZCL ColorControl StartUpColorTemperatureMireds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration PhysicalMinLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_PHYSICAL_MIN_LEVEL,
      "ZCL BallastConfiguration PhysicalMinLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration PhysicalMaxLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_PHYSICAL_MAX_LEVEL,
      "ZCL BallastConfiguration PhysicalMaxLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration BallastStatus, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_BALLAST_STATUS,
      "ZCL BallastConfiguration BallastStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration MinLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_MIN_LEVEL,
      "ZCL BallastConfiguration MinLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration MaxLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_MAX_LEVEL,
      "ZCL BallastConfiguration MaxLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration PowerOnLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_POWER_ON_LEVEL,
      "ZCL BallastConfiguration PowerOnLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration PowerOnFadeTime, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_POWER_ON_FADE_TIME,
      "ZCL BallastConfiguration PowerOnFadeTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration IntrinsicBallastFactor, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_INTRINSIC_BALLAST_FACTOR,
      "ZCL BallastConfiguration IntrinsicBallastFactor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration BallastFactorAdjustment, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_BALLAST_FACTOR_ADJUSTMENT,
      "ZCL BallastConfiguration BallastFactorAdjustment",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration LampQuantity, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_LAMP_QUANTITY,
      "ZCL BallastConfiguration LampQuantity",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration LampType, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_LAMP_TYPE,
      "ZCL BallastConfiguration LampType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration LampManufacturer, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_LAMP_MANUFACTURER,
      "ZCL BallastConfiguration LampManufacturer",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration LampRatedHours, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_LAMP_RATED_HOURS,
      "ZCL BallastConfiguration LampRatedHours",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration LampBurnHours, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_LAMP_BURN_HOURS,
      "ZCL BallastConfiguration LampBurnHours",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration LampAlarmMode, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_LAMP_ALARM_MODE,
      "ZCL BallastConfiguration LampAlarmMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL BallastConfiguration LampBurnHoursTripPoint, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BALLAST_CONFIGURATION_LAMP_BURN_HOURS_TRIP_POINT,
      "ZCL BallastConfiguration LampBurnHoursTripPoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IlluminanceMeasurement MeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE,
      "ZCL IlluminanceMeasurement MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IlluminanceMeasurement MinMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ILLUMINANCE_MEASUREMENT_MIN_MEASURED_VALUE,
      "ZCL IlluminanceMeasurement MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IlluminanceMeasurement MaxMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ILLUMINANCE_MEASUREMENT_MAX_MEASURED_VALUE,
      "ZCL IlluminanceMeasurement MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IlluminanceMeasurement Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ILLUMINANCE_MEASUREMENT_TOLERANCE,
      "ZCL IlluminanceMeasurement Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IlluminanceMeasurement LightSensorType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ILLUMINANCE_MEASUREMENT_LIGHT_SENSOR_TYPE,
      "ZCL IlluminanceMeasurement LightSensorType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IlluminanceLevelSensing LevelStatus, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ILLUMINANCE_LEVEL_SENSING_LEVEL_STATUS,
      "ZCL IlluminanceLevelSensing LevelStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IlluminanceLevelSensing LightSensorType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ILLUMINANCE_LEVEL_SENSING_LIGHT_SENSOR_TYPE,
      "ZCL IlluminanceLevelSensing LightSensorType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IlluminanceLevelSensing IlluminanceTargetLevel, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ILLUMINANCE_LEVEL_SENSING_ILLUMINANCE_TARGET_LEVEL,
      "ZCL IlluminanceLevelSensing IlluminanceTargetLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL TemperatureMeasurement MeasuredValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE,
      "ZCL TemperatureMeasurement MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL TemperatureMeasurement MinMeasuredValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MIN_MEASURED_VALUE,
      "ZCL TemperatureMeasurement MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL TemperatureMeasurement MaxMeasuredValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MAX_MEASURED_VALUE,
      "ZCL TemperatureMeasurement MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL TemperatureMeasurement Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_TOLERANCE,
      "ZCL TemperatureMeasurement Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement MeasuredValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_MEASURED_VALUE,
      "ZCL PressureMeasurement MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement MinMeasuredValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_MIN_MEASURED_VALUE,
      "ZCL PressureMeasurement MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement MaxMeasuredValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_MAX_MEASURED_VALUE,
      "ZCL PressureMeasurement MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_TOLERANCE,
      "ZCL PressureMeasurement Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement ScaledValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_SCALED_VALUE,
      "ZCL PressureMeasurement ScaledValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement MinScaledValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_MIN_SCALED_VALUE,
      "ZCL PressureMeasurement MinScaledValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement MaxScaledValue, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_MAX_SCALED_VALUE,
      "ZCL PressureMeasurement MaxScaledValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement ScaledTolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_SCALED_TOLERANCE,
      "ZCL PressureMeasurement ScaledTolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PressureMeasurement Scale, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PRESSURE_MEASUREMENT_SCALE,
      "ZCL PressureMeasurement Scale",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL FlowMeasurement MeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_FLOW_MEASUREMENT_MEASURED_VALUE,
      "ZCL FlowMeasurement MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL FlowMeasurement MinMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_FLOW_MEASUREMENT_MIN_MEASURED_VALUE,
      "ZCL FlowMeasurement MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL FlowMeasurement MaxMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_FLOW_MEASUREMENT_MAX_MEASURED_VALUE,
      "ZCL FlowMeasurement MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL FlowMeasurement Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_FLOW_MEASUREMENT_TOLERANCE,
      "ZCL FlowMeasurement Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL RelativityHumidity MeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_RELATIVITY_HUMIDITY_MEASURED_VALUE,
      "ZCL RelativityHumidity MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL RelativityHumidity MinMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_RELATIVITY_HUMIDITY_MIN_MEASURED_VALUE,
      "ZCL RelativityHumidity MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL RelativityHumidity MaxMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_RELATIVITY_HUMIDITY_MAX_MEASURED_VALUE,
      "ZCL RelativityHumidity MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL RelativityHumidity Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_RELATIVITY_HUMIDITY_TOLERANCE,
      "ZCL RelativityHumidity Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing Occupancy, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_OCCUPANCY,
      "ZCL OccupancySensing Occupancy",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing OccupancySensorType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_OCCUPANCY_SENSOR_TYPE,
      "ZCL OccupancySensing OccupancySensorType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing OccupancySensorTypeBitmap, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_OCCUPANCY_SENSOR_TYPE_BITMAP,
      "ZCL OccupancySensing OccupancySensorTypeBitmap",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing PIROccupiedToUnoccupiedDelay, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_PIR_OCCUPIED_TO_UNOCCUPIED_DELAY,
      "ZCL OccupancySensing PIROccupiedToUnoccupiedDelay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing PIRUnoccupiedToOccupiedDelay, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_PIR_UNOCCUPIED_TO_OCCUPIED_DELAY,
      "ZCL OccupancySensing PIRUnoccupiedToOccupiedDelay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing PIRUnoccupiedToOccupiedThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_PIR_UNOCCUPIED_TO_OCCUPIED_THRESHOLD,
      "ZCL OccupancySensing PIRUnoccupiedToOccupiedThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing UltrasonicOccupiedToUnoccupiedDelay, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_ULTRASONIC_OCCUPIED_TO_UNOCCUPIED_DELAY,
      "ZCL OccupancySensing UltrasonicOccupiedToUnoccupiedDelay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing UltrasonicUnoccupiedToOccupiedDelay, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_DELAY,
      "ZCL OccupancySensing UltrasonicUnoccupiedToOccupiedDelay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing UltrasonicUnoccupiedToOccupiedThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_THRESHOLD,
      "ZCL OccupancySensing UltrasonicUnoccupiedToOccupiedThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing PhysicalContactOccupiedToUnoccupiedDelay, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_OCCUPIED_TO_UNOCCUPIED_DELAY,
      "ZCL OccupancySensing PhysicalContactOccupiedToUnoccupiedDelay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing PhysicalContactUnoccupiedToOccupiedDelay, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_UNOCCUPIED_TO_OCCUPIED_DELAY,
      "ZCL OccupancySensing PhysicalContactUnoccupiedToOccupiedDelay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL OccupancySensing PhysicalContactUnoccupiedToOccupiedThreshold, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_OCCUPANCY_SENSING_PHYSICAL_CONTACT_UNOCCUPIED_TO_OCCUPIED_THRESHOLD,
      "ZCL OccupancySensing PhysicalContactUnoccupiedToOccupiedThreshold",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SoilMoisture MeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SOIL_MOISTURE_MEASURED_VALUE,
      "ZCL SoilMoisture MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SoilMoisture MinMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SOIL_MOISTURE_MIN_MEASURED_VALUE,
      "ZCL SoilMoisture MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SoilMoisture MaxMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SOIL_MOISTURE_MAX_MEASURED_VALUE,
      "ZCL SoilMoisture MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SoilMoisture Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SOIL_MOISTURE_TOLERANCE,
      "ZCL SoilMoisture Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PhMeasurement MeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PH_MEASUREMENT_MEASURED_VALUE,
      "ZCL PhMeasurement MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PhMeasurement MinMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PH_MEASUREMENT_MIN_MEASURED_VALUE,
      "ZCL PhMeasurement MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PhMeasurement MaxMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PH_MEASUREMENT_MAX_MEASURED_VALUE,
      "ZCL PhMeasurement MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PhMeasurement Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PH_MEASUREMENT_TOLERANCE,
      "ZCL PhMeasurement Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalConductivityMeasurement MeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_CONDUCTIVITY_MEASUREMENT_MEASURED_VALUE,
      "ZCL ElectricalConductivityMeasurement MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalConductivityMeasurement MinMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_CONDUCTIVITY_MEASUREMENT_MIN_MEASURED_VALUE,
      "ZCL ElectricalConductivityMeasurement MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalConductivityMeasurement MaxMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_CONDUCTIVITY_MEASUREMENT_MAX_MEASURED_VALUE,
      "ZCL ElectricalConductivityMeasurement MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalConductivityMeasurement Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_CONDUCTIVITY_MEASUREMENT_TOLERANCE,
      "ZCL ElectricalConductivityMeasurement Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindSpeedMeasurement MeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WIND_SPEED_MEASUREMENT_MEASURED_VALUE,
      "ZCL WindSpeedMeasurement MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindSpeedMeasurement MinMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WIND_SPEED_MEASUREMENT_MIN_MEASURED_VALUE,
      "ZCL WindSpeedMeasurement MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindSpeedMeasurement MaxMeasuredValue, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WIND_SPEED_MEASUREMENT_MAX_MEASURED_VALUE,
      "ZCL WindSpeedMeasurement MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL WindSpeedMeasurement Tolerance, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_WIND_SPEED_MEASUREMENT_TOLERANCE,
      "ZCL WindSpeedMeasurement Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL CarbonMonoxide MeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CARBON_MONOXIDE_MEASURED_VALUE,
      "ZCL CarbonMonoxide MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL CarbonMonoxide MinMeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CARBON_MONOXIDE_MIN_MEASURED_VALUE,
      "ZCL CarbonMonoxide MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL CarbonMonoxide MaxMeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CARBON_MONOXIDE_MAX_MEASURED_VALUE,
      "ZCL CarbonMonoxide MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL CarbonMonoxide Tolerance, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CARBON_MONOXIDE_TOLERANCE,
      "ZCL CarbonMonoxide Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL CarbonDioxide MeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CARBON_DIOXIDE_MEASURED_VALUE,
      "ZCL CarbonDioxide MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL CarbonDioxide MinMeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CARBON_DIOXIDE_MIN_MEASURED_VALUE,
      "ZCL CarbonDioxide MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL CarbonDioxide MaxMeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CARBON_DIOXIDE_MAX_MEASURED_VALUE,
      "ZCL CarbonDioxide MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL CarbonDioxide Tolerance, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CARBON_DIOXIDE_TOLERANCE,
      "ZCL CarbonDioxide Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PM25 MeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PM25_MEASURED_VALUE,
      "ZCL PM25 MeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PM25 MinMeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PM25_MIN_MEASURED_VALUE,
      "ZCL PM25 MinMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PM25 MaxMeasuredValue, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PM25_MAX_MEASURED_VALUE,
      "ZCL PM25 MaxMeasuredValue",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // single // single // float
    std::string attribute_type_string           = "float";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL PM25 Tolerance, "
                     "type:  single // float");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PM25_TOLERANCE,
      "ZCL PM25 Tolerance",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IASZone ZoneState, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IAS_ZONE_ZONE_STATE,
      "ZCL IASZone ZoneState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // IasZoneType // IasZoneType // IasZoneType
    std::string attribute_type_string           = "IasZoneType";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IASZone ZoneType, "
                     "type:  IasZoneType // IasZoneType");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IAS_ZONE_ZONE_TYPE,
      "ZCL IASZone ZoneType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // IasZoneStatus // IasZoneStatus // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IASZone ZoneStatus, "
                     "type:  IasZoneStatus // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IAS_ZONE_ZONE_STATUS,
      "ZCL IASZone ZoneStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // EUI64 // EUI64 // EUI64
    std::string attribute_type_string           = "EUI64";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IASZone IASCIEAddress, "
                     "type:  EUI64 // EUI64");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IAS_ZONE_IASCIE_ADDRESS,
      "ZCL IASZone IASCIEAddress",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IASZone ZoneID, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IAS_ZONE_ZONEID,
      "ZCL IASZone ZoneID",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IASZone NumberOfZoneSensitivityLevelsSupported, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IAS_ZONE_NUMBER_OF_ZONE_SENSITIVITY_LEVELS_SUPPORTED,
      "ZCL IASZone NumberOfZoneSensitivityLevelsSupported",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IASZone CurrentZoneSensitivityLevel, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IAS_ZONE_CURRENT_ZONE_SENSITIVITY_LEVEL,
      "ZCL IASZone CurrentZoneSensitivityLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL IASWD MaxDuration, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_IASWD_MAX_DURATION,
      "ZCL IASWD MaxDuration",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering CurrentSummationDelivered, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_CURRENT_SUMMATION_DELIVERED,
      "ZCL Metering CurrentSummationDelivered",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering CurrentSummationReceived, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_CURRENT_SUMMATION_RECEIVED,
      "ZCL Metering CurrentSummationReceived",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering CurrentMaxDemandDelivered, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_CURRENT_MAX_DEMAND_DELIVERED,
      "ZCL Metering CurrentMaxDemandDelivered",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering CurrentMaxDemandReceived, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_CURRENT_MAX_DEMAND_RECEIVED,
      "ZCL Metering CurrentMaxDemandReceived",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering PowerFactor, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_POWER_FACTOR,
      "ZCL Metering PowerFactor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // UTC // UTC // UTC
    std::string attribute_type_string           = "UTC";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering ReadingSnapShotTime, "
                     "type:  UTC // UTC");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_READING_SNAP_SHOT_TIME,
      "ZCL Metering ReadingSnapShotTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // UTC // UTC // UTC
    std::string attribute_type_string           = "UTC";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering CurrentMaxDemandDeliveredTime, "
                     "type:  UTC // UTC");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_CURRENT_MAX_DEMAND_DELIVERED_TIME,
      "ZCL Metering CurrentMaxDemandDeliveredTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // UTC // UTC // UTC
    std::string attribute_type_string           = "UTC";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering CurrentMaxDemandReceivedTime, "
                     "type:  UTC // UTC");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_CURRENT_MAX_DEMAND_RECEIVED_TIME,
      "ZCL Metering CurrentMaxDemandReceivedTime",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering DefaultUpdatePeriod, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_DEFAULT_UPDATE_PERIOD,
      "ZCL Metering DefaultUpdatePeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering SupplyStatus, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_SUPPLY_STATUS,
      "ZCL Metering SupplyStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering CurrentInletEnergyCarrierSummation, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_CURRENT_INLET_ENERGY_CARRIER_SUMMATION,
      "ZCL Metering CurrentInletEnergyCarrierSummation",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering CurrentOutletEnergyCarrierSummation, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_CURRENT_OUTLET_ENERGY_CARRIER_SUMMATION,
      "ZCL Metering CurrentOutletEnergyCarrierSummation",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int24 // int24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering InletTemperature, "
                     "type:  int24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_INLET_TEMPERATURE,
      "ZCL Metering InletTemperature",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int24 // int24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering OutletTemperature, "
                     "type:  int24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_OUTLET_TEMPERATURE,
      "ZCL Metering OutletTemperature",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering UnitofMeasure, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_UNITOF_MEASURE,
      "ZCL Metering UnitofMeasure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering Multiplier, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_MULTIPLIER,
      "ZCL Metering Multiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint24 // uint24 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering Divisor, "
                     "type:  uint24 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_DIVISOR,
      "ZCL Metering Divisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering SummationFormatting, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_SUMMATION_FORMATTING,
      "ZCL Metering SummationFormatting",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering DemandFormatting, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_DEMAND_FORMATTING,
      "ZCL Metering DemandFormatting",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering HistoricalConsumptionFormatting, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_HISTORICAL_CONSUMPTION_FORMATTING,
      "ZCL Metering HistoricalConsumptionFormatting",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering MeteringDeviceType, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_METERING_DEVICE_TYPE,
      "ZCL Metering MeteringDeviceType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering EnergyCarrierUnitOfMeasure, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_ENERGY_CARRIER_UNIT_OF_MEASURE,
      "ZCL Metering EnergyCarrierUnitOfMeasure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering EnergyCarrierSummationFormatting, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_ENERGY_CARRIER_SUMMATION_FORMATTING,
      "ZCL Metering EnergyCarrierSummationFormatting",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering EnergyCarrierDemandFormatting, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_ENERGY_CARRIER_DEMAND_FORMATTING,
      "ZCL Metering EnergyCarrierDemandFormatting",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering TemperatureUnitOfMeasure, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_TEMPERATURE_UNIT_OF_MEASURE,
      "ZCL Metering TemperatureUnitOfMeasure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Metering TemperatureFormatting, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_METERING_TEMPERATURE_FORMATTING,
      "ZCL Metering TemperatureFormatting",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map32 // map32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement MeasurementType, "
                     "type:  map32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASUREMENT_TYPE,
      "ZCL ElectricalMeasurement MeasurementType",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCVoltage, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE,
      "ZCL ElectricalMeasurement DCVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCVoltageMin, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MIN,
      "ZCL ElectricalMeasurement DCVoltageMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCVoltageMax, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MAX,
      "ZCL ElectricalMeasurement DCVoltageMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_CURRENT,
      "ZCL ElectricalMeasurement DCCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCCurrentMin, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MIN,
      "ZCL ElectricalMeasurement DCCurrentMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCCurrentMax, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MAX,
      "ZCL ElectricalMeasurement DCCurrentMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCPower, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_POWER,
      "ZCL ElectricalMeasurement DCPower",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCPowerMin, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_POWER_MIN,
      "ZCL ElectricalMeasurement DCPowerMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCPowerMax, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_POWER_MAX,
      "ZCL ElectricalMeasurement DCPowerMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCVoltageMultiplier, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MULTIPLIER,
      "ZCL ElectricalMeasurement DCVoltageMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCVoltageDivisor, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_DIVISOR,
      "ZCL ElectricalMeasurement DCVoltageDivisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCCurrentMultiplier, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_CURRENT_MULTIPLIER,
      "ZCL ElectricalMeasurement DCCurrentMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCCurrentDivisor, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_CURRENT_DIVISOR,
      "ZCL ElectricalMeasurement DCCurrentDivisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCPowerMultiplier, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_POWER_MULTIPLIER,
      "ZCL ElectricalMeasurement DCPowerMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCPowerDivisor, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_POWER_DIVISOR,
      "ZCL ElectricalMeasurement DCPowerDivisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACFrequency, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY,
      "ZCL ElectricalMeasurement ACFrequency",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACFrequencyMin, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MIN,
      "ZCL ElectricalMeasurement ACFrequencyMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACFrequencyMax, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MAX,
      "ZCL ElectricalMeasurement ACFrequencyMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement NeutralCurrent, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_NEUTRAL_CURRENT,
      "ZCL ElectricalMeasurement NeutralCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement TotalActivePower, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_TOTAL_ACTIVE_POWER,
      "ZCL ElectricalMeasurement TotalActivePower",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement TotalReactivePower, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_TOTAL_REACTIVE_POWER,
      "ZCL ElectricalMeasurement TotalReactivePower",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement TotalApparentPower, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_TOTAL_APPARENT_POWER,
      "ZCL ElectricalMeasurement TotalApparentPower",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement Measured1stHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED1ST_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement Measured1stHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement Measured3rdHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED3RD_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement Measured3rdHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement Measured5thHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED5TH_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement Measured5thHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement Measured7thHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED7TH_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement Measured7thHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement Measured9thHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED9TH_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement Measured9thHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement Measured11thHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED11TH_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement Measured11thHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement MeasuredPhase1stHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE1ST_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement MeasuredPhase1stHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement MeasuredPhase3rdHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE3RD_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement MeasuredPhase3rdHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement MeasuredPhase5thHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE5TH_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement MeasuredPhase5thHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement MeasuredPhase7thHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE7TH_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement MeasuredPhase7thHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement MeasuredPhase9thHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE9TH_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement MeasuredPhase9thHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement MeasuredPhase11thHarmonicCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_MEASURED_PHASE11TH_HARMONIC_CURRENT,
      "ZCL ElectricalMeasurement MeasuredPhase11thHarmonicCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACFrequencyMultiplier, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_MULTIPLIER,
      "ZCL ElectricalMeasurement ACFrequencyMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACFrequencyDivisor, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_FREQUENCY_DIVISOR,
      "ZCL ElectricalMeasurement ACFrequencyDivisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement PowerMultiplier, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_POWER_MULTIPLIER,
      "ZCL ElectricalMeasurement PowerMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement PowerDivisor, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_POWER_DIVISOR,
      "ZCL ElectricalMeasurement PowerDivisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement HarmonicCurrentMultiplier, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_HARMONIC_CURRENT_MULTIPLIER,
      "ZCL ElectricalMeasurement HarmonicCurrentMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement PhaseHarmonicCurrentMultiplier, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_PHASE_HARMONIC_CURRENT_MULTIPLIER,
      "ZCL ElectricalMeasurement PhaseHarmonicCurrentMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement LineCurrent, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_LINE_CURRENT,
      "ZCL ElectricalMeasurement LineCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActiveCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT,
      "ZCL ElectricalMeasurement ActiveCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ReactiveCurrent, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT,
      "ZCL ElectricalMeasurement ReactiveCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltage, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE,
      "ZCL ElectricalMeasurement RMSVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageMin, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN,
      "ZCL ElectricalMeasurement RMSVoltageMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageMax, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX,
      "ZCL ElectricalMeasurement RMSVoltageMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrent, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT,
      "ZCL ElectricalMeasurement RMSCurrent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrentMin, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN,
      "ZCL ElectricalMeasurement RMSCurrentMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrentMax, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX,
      "ZCL ElectricalMeasurement RMSCurrentMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePower, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER,
      "ZCL ElectricalMeasurement ActivePower",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePowerMin, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN,
      "ZCL ElectricalMeasurement ActivePowerMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePowerMax, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX,
      "ZCL ElectricalMeasurement ActivePowerMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ReactivePower, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER,
      "ZCL ElectricalMeasurement ReactivePower",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ApparentPower, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_APPARENT_POWER,
      "ZCL ElectricalMeasurement ApparentPower",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement PowerFactor, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_POWER_FACTOR,
      "ZCL ElectricalMeasurement PowerFactor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSVoltageMeasurementPeriod, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD,
      "ZCL ElectricalMeasurement AverageRMSVoltageMeasurementPeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSOverVoltageCounter, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER,
      "ZCL ElectricalMeasurement AverageRMSOverVoltageCounter",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSUnderVoltageCounter, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER,
      "ZCL ElectricalMeasurement AverageRMSUnderVoltageCounter",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSExtremeOverVoltagePeriod, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD,
      "ZCL ElectricalMeasurement RMSExtremeOverVoltagePeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSExtremeUnderVoltagePeriod, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD,
      "ZCL ElectricalMeasurement RMSExtremeUnderVoltagePeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageSagPeriod, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD,
      "ZCL ElectricalMeasurement RMSVoltageSagPeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageSwellPeriod, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD,
      "ZCL ElectricalMeasurement RMSVoltageSwellPeriod",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACVoltageMultiplier, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_MULTIPLIER,
      "ZCL ElectricalMeasurement ACVoltageMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACVoltageDivisor, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_DIVISOR,
      "ZCL ElectricalMeasurement ACVoltageDivisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACCurrentMultiplier, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_CURRENT_MULTIPLIER,
      "ZCL ElectricalMeasurement ACCurrentMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACCurrentDivisor, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_CURRENT_DIVISOR,
      "ZCL ElectricalMeasurement ACCurrentDivisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACPowerMultiplier, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_POWER_MULTIPLIER,
      "ZCL ElectricalMeasurement ACPowerMultiplier",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACPowerDivisor, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_POWER_DIVISOR,
      "ZCL ElectricalMeasurement ACPowerDivisor",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCOverloadAlarmsMask, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_OVERLOAD_ALARMS_MASK,
      "ZCL ElectricalMeasurement DCOverloadAlarmsMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCVoltageOverload, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_OVERLOAD,
      "ZCL ElectricalMeasurement DCVoltageOverload",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement DCCurrentOverload, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_DC_CURRENT_OVERLOAD,
      "ZCL ElectricalMeasurement DCCurrentOverload",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map16 // map16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACAlarmsMask, "
                     "type:  map16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_ALARMS_MASK,
      "ZCL ElectricalMeasurement ACAlarmsMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACVoltageOverload, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_VOLTAGE_OVERLOAD,
      "ZCL ElectricalMeasurement ACVoltageOverload",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACCurrentOverload, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_CURRENT_OVERLOAD,
      "ZCL ElectricalMeasurement ACCurrentOverload",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACActivePowerOverload, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_ACTIVE_POWER_OVERLOAD,
      "ZCL ElectricalMeasurement ACActivePowerOverload",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ACReactivePowerOverload, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AC_REACTIVE_POWER_OVERLOAD,
      "ZCL ElectricalMeasurement ACReactivePowerOverload",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSOverVoltage, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE,
      "ZCL ElectricalMeasurement AverageRMSOverVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSUnderVoltage, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE,
      "ZCL ElectricalMeasurement AverageRMSUnderVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSExtremeOverVoltage, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE,
      "ZCL ElectricalMeasurement RMSExtremeOverVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSExtremeUnderVoltage, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE,
      "ZCL ElectricalMeasurement RMSExtremeUnderVoltage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageSag, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG,
      "ZCL ElectricalMeasurement RMSVoltageSag",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageSwell, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL,
      "ZCL ElectricalMeasurement RMSVoltageSwell",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement LineCurrentPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PHB,
      "ZCL ElectricalMeasurement LineCurrentPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActiveCurrentPhB, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PHB,
      "ZCL ElectricalMeasurement ActiveCurrentPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ReactiveCurrentPhB, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PHB,
      "ZCL ElectricalMeasurement ReactiveCurrentPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltagePhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PHB,
      "ZCL ElectricalMeasurement RMSVoltagePhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageMinPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PHB,
      "ZCL ElectricalMeasurement RMSVoltageMinPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageMaxPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PHB,
      "ZCL ElectricalMeasurement RMSVoltageMaxPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrentPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PHB,
      "ZCL ElectricalMeasurement RMSCurrentPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrentMinPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PHB,
      "ZCL ElectricalMeasurement RMSCurrentMinPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrentMaxPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PHB,
      "ZCL ElectricalMeasurement RMSCurrentMaxPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePowerPhB, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PHB,
      "ZCL ElectricalMeasurement ActivePowerPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePowerMinPhB, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PHB,
      "ZCL ElectricalMeasurement ActivePowerMinPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePowerMaxPhB, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PHB,
      "ZCL ElectricalMeasurement ActivePowerMaxPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ReactivePowerPhB, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PHB,
      "ZCL ElectricalMeasurement ReactivePowerPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ApparentPowerPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PHB,
      "ZCL ElectricalMeasurement ApparentPowerPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement PowerFactorPhB, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PHB,
      "ZCL ElectricalMeasurement PowerFactorPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSVoltageMeasurementPeriodPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD_PHB,
      "ZCL ElectricalMeasurement AverageRMSVoltageMeasurementPeriodPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSOverVoltageCounterPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PHB,
      "ZCL ElectricalMeasurement AverageRMSOverVoltageCounterPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSUnderVoltageCounterPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER_PHB,
      "ZCL ElectricalMeasurement AverageRMSUnderVoltageCounterPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSExtremeOverVoltagePeriodPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PHB,
      "ZCL ElectricalMeasurement RMSExtremeOverVoltagePeriodPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSExtremeUnderVoltagePeriodPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PHB,
      "ZCL ElectricalMeasurement RMSExtremeUnderVoltagePeriodPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageSagPeriodPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PHB,
      "ZCL ElectricalMeasurement RMSVoltageSagPeriodPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageSwellPeriodPhB, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PHB,
      "ZCL ElectricalMeasurement RMSVoltageSwellPeriodPhB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement LineCurrentPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_LINE_CURRENT_PHC,
      "ZCL ElectricalMeasurement LineCurrentPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActiveCurrentPhC, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_CURRENT_PHC,
      "ZCL ElectricalMeasurement ActiveCurrentPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ReactiveCurrentPhC, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_REACTIVE_CURRENT_PHC,
      "ZCL ElectricalMeasurement ReactiveCurrentPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltagePhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_PHC,
      "ZCL ElectricalMeasurement RMSVoltagePhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageMinPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MIN_PHC,
      "ZCL ElectricalMeasurement RMSVoltageMinPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageMaxPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_MAX_PHC,
      "ZCL ElectricalMeasurement RMSVoltageMaxPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrentPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_PHC,
      "ZCL ElectricalMeasurement RMSCurrentPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrentMinPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MIN_PHC,
      "ZCL ElectricalMeasurement RMSCurrentMinPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSCurrentMaxPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_CURRENT_MAX_PHC,
      "ZCL ElectricalMeasurement RMSCurrentMaxPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePowerPhC, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_PHC,
      "ZCL ElectricalMeasurement ActivePowerPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePowerMinPhC, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MIN_PHC,
      "ZCL ElectricalMeasurement ActivePowerMinPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ActivePowerMaxPhC, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_ACTIVE_POWER_MAX_PHC,
      "ZCL ElectricalMeasurement ActivePowerMaxPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int16 // int16 // int16_t
    std::string attribute_type_string           = "int16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ReactivePowerPhC, "
                     "type:  int16 // int16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_REACTIVE_POWER_PHC,
      "ZCL ElectricalMeasurement ReactivePowerPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement ApparentPowerPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_APPARENT_POWER_PHC,
      "ZCL ElectricalMeasurement ApparentPowerPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement PowerFactorPhC, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_POWER_FACTOR_PHC,
      "ZCL ElectricalMeasurement PowerFactorPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSVoltageMeasurementPeriodPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_VOLTAGE_MEASUREMENT_PERIOD_PHC,
      "ZCL ElectricalMeasurement AverageRMSVoltageMeasurementPeriodPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSOverVoltageCounterPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_OVER_VOLTAGE_COUNTER_PHC,
      "ZCL ElectricalMeasurement AverageRMSOverVoltageCounterPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement AverageRMSUnderVoltageCounterPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_AVERAGE_RMS_UNDER_VOLTAGE_COUNTER_PHC,
      "ZCL ElectricalMeasurement AverageRMSUnderVoltageCounterPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSExtremeOverVoltagePeriodPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_OVER_VOLTAGE_PERIOD_PHC,
      "ZCL ElectricalMeasurement RMSExtremeOverVoltagePeriodPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSExtremeUnderVoltagePeriodPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_EXTREME_UNDER_VOLTAGE_PERIOD_PHC,
      "ZCL ElectricalMeasurement RMSExtremeUnderVoltagePeriodPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageSagPeriodPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SAG_PERIOD_PHC,
      "ZCL ElectricalMeasurement RMSVoltageSagPeriodPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ElectricalMeasurement RMSVoltageSwellPeriodPhC, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_ELECTRICAL_MEASUREMENT_RMS_VOLTAGE_SWELL_PERIOD_PHC,
      "ZCL ElectricalMeasurement RMSVoltageSwellPeriodPhC",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics NumberOfResets, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_NUMBER_OF_RESETS,
      "ZCL Diagnostics NumberOfResets",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics PersistentMemoryWrites, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_PERSISTENT_MEMORY_WRITES,
      "ZCL Diagnostics PersistentMemoryWrites",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics MacRxBcast, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_MAC_RX_BCAST,
      "ZCL Diagnostics MacRxBcast",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics MacTxBcast, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_MAC_TX_BCAST,
      "ZCL Diagnostics MacTxBcast",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics MacRxUcast, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_MAC_RX_UCAST,
      "ZCL Diagnostics MacRxUcast",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics MacTxUcast, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_MAC_TX_UCAST,
      "ZCL Diagnostics MacTxUcast",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics MacTxUcastRetry, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_MAC_TX_UCAST_RETRY,
      "ZCL Diagnostics MacTxUcastRetry",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics MacTxUcastFail, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_MAC_TX_UCAST_FAIL,
      "ZCL Diagnostics MacTxUcastFail",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSRxBcast, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APS_RX_BCAST,
      "ZCL Diagnostics APSRxBcast",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSTxBcast, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APS_TX_BCAST,
      "ZCL Diagnostics APSTxBcast",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSRxUcast, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APS_RX_UCAST,
      "ZCL Diagnostics APSRxUcast",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSTxUcastSuccess, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APS_TX_UCAST_SUCCESS,
      "ZCL Diagnostics APSTxUcastSuccess",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSTxUcastRetry, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APS_TX_UCAST_RETRY,
      "ZCL Diagnostics APSTxUcastRetry",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSTxUcastFail, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APS_TX_UCAST_FAIL,
      "ZCL Diagnostics APSTxUcastFail",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics RouteDiscInitiated, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_ROUTE_DISC_INITIATED,
      "ZCL Diagnostics RouteDiscInitiated",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics NeighborAdded, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_NEIGHBOR_ADDED,
      "ZCL Diagnostics NeighborAdded",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics NeighborRemoved, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_NEIGHBOR_REMOVED,
      "ZCL Diagnostics NeighborRemoved",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics NeighborStale, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_NEIGHBOR_STALE,
      "ZCL Diagnostics NeighborStale",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics JoinIndication, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_JOIN_INDICATION,
      "ZCL Diagnostics JoinIndication",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics ChildMoved, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_CHILD_MOVED,
      "ZCL Diagnostics ChildMoved",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics NWKFCFailure, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_NWKFC_FAILURE,
      "ZCL Diagnostics NWKFCFailure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSFCFailure, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APSFC_FAILURE,
      "ZCL Diagnostics APSFCFailure",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSUnauthorizedKey, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APS_UNAUTHORIZED_KEY,
      "ZCL Diagnostics APSUnauthorizedKey",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics NWKDecryptFailures, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_NWK_DECRYPT_FAILURES,
      "ZCL Diagnostics NWKDecryptFailures",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics APSDecryptFailures, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_APS_DECRYPT_FAILURES,
      "ZCL Diagnostics APSDecryptFailures",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics PacketBufferAllocateFailures, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_PACKET_BUFFER_ALLOCATE_FAILURES,
      "ZCL Diagnostics PacketBufferAllocateFailures",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics RelayedUcast, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_RELAYED_UCAST,
      "ZCL Diagnostics RelayedUcast",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics PHYToMACQueueLimitReached, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_PHY_TO_MAC_QUEUE_LIMIT_REACHED,
      "ZCL Diagnostics PHYToMACQueueLimitReached",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics PacketValidateDropCount, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_PACKET_VALIDATE_DROP_COUNT,
      "ZCL Diagnostics PacketValidateDropCount",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint16 // uint16 // uint16_t
    std::string attribute_type_string           = "uint16_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics AverageMACRetryPerAPSMessageSent, "
                     "type:  uint16 // uint16_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT,
      "ZCL Diagnostics AverageMACRetryPerAPSMessageSent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics LastMessageLQI, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_LAST_MESSAGELQI,
      "ZCL Diagnostics LastMessageLQI",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Diagnostics LastMessageRSSI, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DIAGNOSTICS_LAST_MESSAGERSSI,
      "ZCL Diagnostics LastMessageRSSI",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ProtocolController-RFTelemetry TxReportEnabled, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_RF_TELEMETRY_TX_REPORT_ENABLED,
      "ZCL ProtocolController-RFTelemetry TxReportEnabled",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ProtocolController-RFTelemetry PTIEnabled, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_RF_TELEMETRY_PTI_ENABLED,
      "ZCL ProtocolController-RFTelemetry PTIEnabled",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL State EndpointIdList, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_STATE_ENDPOINT_ID_LIST,
      "ZCL State EndpointIdList",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // NodeStateNetworkStatus // NodeStateNetworkStatus // NodeStateNetworkStatus
    std::string attribute_type_string           = "NodeStateNetworkStatus";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL State NetworkStatus, "
                     "type:  NodeStateNetworkStatus // NodeStateNetworkStatus");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
      "ZCL State NetworkStatus",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // NodeStateSecurity // NodeStateSecurity // NodeStateSecurity
    std::string attribute_type_string           = "NodeStateSecurity";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL State Security, "
                     "type:  NodeStateSecurity // NodeStateSecurity");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_STATE_SECURITY,
      "ZCL State Security",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL State MaximumCommandDelay, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_STATE_MAXIMUM_COMMAND_DELAY,
      "ZCL State MaximumCommandDelay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL State NetworkList, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_LIST,
      "ZCL State NetworkList",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Binding BindingTableFull, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BINDING_BINDING_TABLE_FULL,
      "ZCL Binding BindingTableFull",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Binding BindableClusterList, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BINDING_BINDABLE_CLUSTER_LIST,
      "ZCL Binding BindableClusterList",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // BindingObject // BindingObject // BindingObject
    std::string attribute_type_string           = "BindingObject";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Binding BindingTable, "
                     "type:  BindingObject // BindingObject");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_BINDING_BINDING_TABLE,
      "ZCL Binding BindingTable",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics ReportingIntervalSeconds, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_REPORTING_INTERVAL_SECONDS,
      "ZCL SystemMetrics ReportingIntervalSeconds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics CPUUsagePercent, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_CPU_USAGE_PERCENT,
      "ZCL SystemMetrics CPUUsagePercent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics CPUFrequencyMHz, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_CPU_FREQUENCYM_HZ,
      "ZCL SystemMetrics CPUFrequencyMHz",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics CPUAverageUsagePercent, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_CPU_AVERAGE_USAGE_PERCENT,
      "ZCL SystemMetrics CPUAverageUsagePercent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics CPUMinUsagePercent, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_CPU_MIN_USAGE_PERCENT,
      "ZCL SystemMetrics CPUMinUsagePercent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics CPUMaxUsagePercent, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_CPU_MAX_USAGE_PERCENT,
      "ZCL SystemMetrics CPUMaxUsagePercent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics RAMTotalMB, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_RAM_TOTALMB,
      "ZCL SystemMetrics RAMTotalMB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics RAMFreeMB, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_RAM_FREEMB,
      "ZCL SystemMetrics RAMFreeMB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics RAMAvailableMB, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_RAM_AVAILABLEMB,
      "ZCL SystemMetrics RAMAvailableMB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics SWAPMemoryTotalMB, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_SWAP_MEMORY_TOTALMB,
      "ZCL SystemMetrics SWAPMemoryTotalMB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics SWAPMemoryUsedMB, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_SWAP_MEMORY_USEDMB,
      "ZCL SystemMetrics SWAPMemoryUsedMB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics VirtualMemoryTotalMB, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_VIRTUAL_MEMORY_TOTALMB,
      "ZCL SystemMetrics VirtualMemoryTotalMB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics VirtualMemoryUsedMB, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_VIRTUAL_MEMORY_USEDMB,
      "ZCL SystemMetrics VirtualMemoryUsedMB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // DiskUsageData // DiskUsageData // DiskUsageData
    std::string attribute_type_string           = "DiskUsageData";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics DisksUsage, "
                     "type:  DiskUsageData // DiskUsageData");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_DISKS_USAGE,
      "ZCL SystemMetrics DisksUsage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // DiskIOCounters // DiskIOCounters // DiskIOCounters
    std::string attribute_type_string           = "DiskIOCounters";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics DisksCounters, "
                     "type:  DiskIOCounters // DiskIOCounters");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_DISKS_COUNTERS,
      "ZCL SystemMetrics DisksCounters",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // NetworkInterfaceData // NetworkInterfaceData // NetworkInterfaceData
    std::string attribute_type_string           = "NetworkInterfaceData";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics NetworkInterfacesData, "
                     "type:  NetworkInterfaceData // NetworkInterfaceData");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_NETWORK_INTERFACES_DATA,
      "ZCL SystemMetrics NetworkInterfacesData",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // WirelessNetworkInterfaceData // WirelessNetworkInterfaceData // WirelessNetworkInterfaceData
    std::string attribute_type_string           = "WirelessNetworkInterfaceData";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics WirelessNetworkInterfacesData, "
                     "type:  WirelessNetworkInterfaceData // WirelessNetworkInterfaceData");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_WIRELESS_NETWORK_INTERFACES_DATA,
      "ZCL SystemMetrics WirelessNetworkInterfacesData",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics Hostname, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_HOSTNAME,
      "ZCL SystemMetrics Hostname",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics FQDN, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_FQDN,
      "ZCL SystemMetrics FQDN",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics UptimeMinutes, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_UPTIME_MINUTES,
      "ZCL SystemMetrics UptimeMinutes",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics CurrentTemperatureCelcius, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_CURRENT_TEMPERATURE_CELCIUS,
      "ZCL SystemMetrics CurrentTemperatureCelcius",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics AverageTemperatureCelcius, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_AVERAGE_TEMPERATURE_CELCIUS,
      "ZCL SystemMetrics AverageTemperatureCelcius",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics MinTemperatureCelcius, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_MIN_TEMPERATURE_CELCIUS,
      "ZCL SystemMetrics MinTemperatureCelcius",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics MaxTemperatureCelcius, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_MAX_TEMPERATURE_CELCIUS,
      "ZCL SystemMetrics MaxTemperatureCelcius",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics PowerPlugged, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_POWER_PLUGGED,
      "ZCL SystemMetrics PowerPlugged",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics BatteryPercentage, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_BATTERY_PERCENTAGE,
      "ZCL SystemMetrics BatteryPercentage",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL SystemMetrics SystemInterrupts, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_SYSTEM_METRICS_SYSTEM_INTERRUPTS,
      "ZCL SystemMetrics SystemInterrupts",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationName, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_NAME,
      "ZCL ApplicationMonitoring ApplicationName",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationVersion, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_VERSION,
      "ZCL ApplicationMonitoring ApplicationVersion",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationConnected, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_CONNECTED,
      "ZCL ApplicationMonitoring ApplicationConnected",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationMQTTTopics, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_MQTT_TOPICS,
      "ZCL ApplicationMonitoring ApplicationMQTTTopics",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring UptimeMinutes, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_UPTIME_MINUTES,
      "ZCL ApplicationMonitoring UptimeMinutes",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ProcessId, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_PROCESS_ID,
      "ZCL ApplicationMonitoring ProcessId",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring Hostname, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_HOSTNAME,
      "ZCL ApplicationMonitoring Hostname",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring FQDN, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_FQDN,
      "ZCL ApplicationMonitoring FQDN",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTLoggingEnabled, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_LOGGING_ENABLED,
      "ZCL ApplicationMonitoring MQTTLoggingEnabled",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // LoggingLevelEnum // LoggingLevelEnum // LoggingLevelEnum
    std::string attribute_type_string           = "LoggingLevelEnum";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTLoggingLevel, "
                     "type:  LoggingLevelEnum // LoggingLevelEnum");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_LOGGING_LEVEL,
      "ZCL ApplicationMonitoring MQTTLoggingLevel",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTStatisticsReportingIntervalSeconds, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_STATISTICS_REPORTING_INTERVAL_SECONDS,
      "ZCL ApplicationMonitoring MQTTStatisticsReportingIntervalSeconds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTMessagesSent, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_MESSAGES_SENT,
      "ZCL ApplicationMonitoring MQTTMessagesSent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTMessagesReceived, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_MESSAGES_RECEIVED,
      "ZCL ApplicationMonitoring MQTTMessagesReceived",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint64 // uint64 // uint64_t
    std::string attribute_type_string           = "uint64_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTSubscriptionCount, "
                     "type:  uint64 // uint64_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_SUBSCRIPTION_COUNT,
      "ZCL ApplicationMonitoring MQTTSubscriptionCount",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTAverageDeliveryTimeSeconds, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_AVERAGE_DELIVERY_TIME_SECONDS,
      "ZCL ApplicationMonitoring MQTTAverageDeliveryTimeSeconds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTMinDeliveryTimeSeconds, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_MIN_DELIVERY_TIME_SECONDS,
      "ZCL ApplicationMonitoring MQTTMinDeliveryTimeSeconds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring MQTTMaxDeliveryTimeSeconds, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_MQTT_MAX_DELIVERY_TIME_SECONDS,
      "ZCL ApplicationMonitoring MQTTMaxDeliveryTimeSeconds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint32 // uint32 // uint32_t
    std::string attribute_type_string           = "uint32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationStatisticsReportingIntervalSeconds, "
                     "type:  uint32 // uint32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_STATISTICS_REPORTING_INTERVAL_SECONDS,
      "ZCL ApplicationMonitoring ApplicationStatisticsReportingIntervalSeconds",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationCPUUsagePercent, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_CPU_USAGE_PERCENT,
      "ZCL ApplicationMonitoring ApplicationCPUUsagePercent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationCPUAverageUsagePercent, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_CPU_AVERAGE_USAGE_PERCENT,
      "ZCL ApplicationMonitoring ApplicationCPUAverageUsagePercent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationCPUMinUsagePercent, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_CPU_MIN_USAGE_PERCENT,
      "ZCL ApplicationMonitoring ApplicationCPUMinUsagePercent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationCPUMaxUsagePercent, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_CPU_MAX_USAGE_PERCENT,
      "ZCL ApplicationMonitoring ApplicationCPUMaxUsagePercent",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ApplicationMonitoring ApplicationRAMUsageMB, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_APPLICATION_MONITORING_APPLICATION_RAM_USAGEMB,
      "ZCL ApplicationMonitoring ApplicationRAMUsageMB",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL NameAndLocation Name, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_NAME_AND_LOCATION_NAME,
      "ZCL NameAndLocation Name",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL NameAndLocation Location, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_NAME_AND_LOCATION_LOCATION,
      "ZCL NameAndLocation Location",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // ConfigurationParameter // ConfigurationParameter // ConfigurationParameter
    std::string attribute_type_string           = "ConfigurationParameter";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ConfigurationParameters ConfigurationParameters, "
                     "type:  ConfigurationParameter // ConfigurationParameter");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_CONFIGURATION_PARAMETERS_CONFIGURATION_PARAMETERS,
      "ZCL ConfigurationParameters ConfigurationParameters",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator ReportingMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_REPORTING_MODE,
      "ZCL AoXLocator ReportingMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator PositionAndOrientationValid, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_POSITION_AND_ORIENTATION_VALID,
      "ZCL AoXLocator PositionAndOrientationValid",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // CoordinateAndOrientation // CoordinateAndOrientation // CoordinateAndOrientation
    std::string attribute_type_string           = "CoordinateAndOrientation";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator PositionAndOrientation, "
                     "type:  CoordinateAndOrientation // CoordinateAndOrientation");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_POSITION_AND_ORIENTATION,
      "ZCL AoXLocator PositionAndOrientation",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // MinMaxPair // MinMaxPair // MinMaxPair
    std::string attribute_type_string           = "MinMaxPair";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AzimuthMask, "
                     "type:  MinMaxPair // MinMaxPair");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_AZIMUTH_MASK,
      "ZCL AoXLocator AzimuthMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // MinMaxPair // MinMaxPair // MinMaxPair
    std::string attribute_type_string           = "MinMaxPair";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator ElevationMask, "
                     "type:  MinMaxPair // MinMaxPair");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_ELEVATION_MASK,
      "ZCL AoXLocator ElevationMask",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // string // string // const char*
    std::string attribute_type_string           = "const char*";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = C_STRING_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AllowList, "
                     "type:  string // const char*");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_ALLOW_LIST,
      "ZCL AoXLocator AllowList",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AoXMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_AOX_MODE,
      "ZCL AoXLocator AoXMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AntennaMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_ANTENNA_MODE,
      "ZCL AoXLocator AntennaMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AntennaArray, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_ANTENNA_ARRAY,
      "ZCL AoXLocator AntennaArray",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator PeriodSamples, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_PERIOD_SAMPLES,
      "ZCL AoXLocator PeriodSamples",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // bool // bool // bool
    std::string attribute_type_string           = "bool";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AngleFiltering, "
                     "type:  bool // bool");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_ANGLE_FILTERING,
      "ZCL AoXLocator AngleFiltering",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // double // double // double
    std::string attribute_type_string           = "double";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AngleFilteringWeight, "
                     "type:  double // double");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_ANGLE_FILTERING_WEIGHT,
      "ZCL AoXLocator AngleFilteringWeight",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AngleCorrectionTimeout, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_ANGLE_CORRECTION_TIMEOUT,
      "ZCL AoXLocator AngleCorrectionTimeout",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator AngleCorrectionDelay, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_ANGLE_CORRECTION_DELAY,
      "ZCL AoXLocator AngleCorrectionDelay",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator CTEMode, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_CTE_MODE,
      "ZCL AoXLocator CTEMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator CTESamplingInterval, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_CTE_SAMPLING_INTERVAL,
      "ZCL AoXLocator CTESamplingInterval",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator CTELength, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_CTE_LENGTH,
      "ZCL AoXLocator CTELength",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int8 // int8 // int8_t
    std::string attribute_type_string           = "int8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXLocator SlotDuration, "
                     "type:  int8 // int8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_LOCATOR_SLOT_DURATION,
      "ZCL AoXLocator SlotDuration",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // EstimatedPosition // EstimatedPosition // EstimatedPosition
    std::string attribute_type_string           = "EstimatedPosition";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL AoXPositionEstimation Position, "
                     "type:  EstimatedPosition // EstimatedPosition");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_AOX_POSITION_ESTIMATION_POSITION,
      "ZCL AoXPositionEstimation Position",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // NetworkManagementState // NetworkManagementState // NetworkManagementState
    std::string attribute_type_string           = "NetworkManagementState";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL ProtocolController-NetworkManagement NetworkManagementState, "
                     "type:  NetworkManagementState // NetworkManagementState");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_NETWORK_MANAGEMENT_NETWORK_MANAGEMENT_STATE,
      "ZCL ProtocolController-NetworkManagement NetworkManagementState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // DeviceTypeStruct // DeviceTypeStruct // DeviceTypeStruct
    std::string attribute_type_string           = "DeviceTypeStruct";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = BYTE_ARRAY_STORAGE_TYPE;

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL Descriptor DeviceTypeList, "
                     "type:  DeviceTypeStruct // DeviceTypeStruct");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_DESCRIPTOR_DEVICE_TYPE_LIST,
      "ZCL Descriptor DeviceTypeList",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // ModeType // ModeType // ModeType
    std::string attribute_type_string           = "ModeType";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl ReportingMode, "
                     "type:  ModeType // ModeType");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_REPORTING_MODE,
      "ZCL UnifyHumidityControl ReportingMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl SupportedReportingMode, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_SUPPORTED_REPORTING_MODE,
      "ZCL UnifyHumidityControl SupportedReportingMode",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // enum8 // enum8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl CurrentState, "
                     "type:  enum8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_CURRENT_STATE,
      "ZCL UnifyHumidityControl CurrentState",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // map8 // map8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl SupportedSetPoints, "
                     "type:  map8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_SUPPORTED_SET_POINTS,
      "ZCL UnifyHumidityControl SupportedSetPoints",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl HumidifierSetpointMin, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_HUMIDIFIER_SETPOINT_MIN,
      "ZCL UnifyHumidityControl HumidifierSetpointMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl HumidifierSetpointMax, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_HUMIDIFIER_SETPOINT_MAX,
      "ZCL UnifyHumidityControl HumidifierSetpointMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl HumidifierSetpoint, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_HUMIDIFIER_SETPOINT,
      "ZCL UnifyHumidityControl HumidifierSetpoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // ScaleType // ScaleType // ScaleType
    std::string attribute_type_string           = "ScaleType";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl HumidifierSetpointScale, "
                     "type:  ScaleType // ScaleType");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_HUMIDIFIER_SETPOINT_SCALE,
      "ZCL UnifyHumidityControl HumidifierSetpointScale",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl HumidifierSetpointPrecision, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_HUMIDIFIER_SETPOINT_PRECISION,
      "ZCL UnifyHumidityControl HumidifierSetpointPrecision",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl DehumidifierSetpointMin, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_DEHUMIDIFIER_SETPOINT_MIN,
      "ZCL UnifyHumidityControl DehumidifierSetpointMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl DehumidifierSetpointMax, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_DEHUMIDIFIER_SETPOINT_MAX,
      "ZCL UnifyHumidityControl DehumidifierSetpointMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl DehumidifierSetpoint, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_DEHUMIDIFIER_SETPOINT,
      "ZCL UnifyHumidityControl DehumidifierSetpoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // ScaleType // ScaleType // ScaleType
    std::string attribute_type_string           = "ScaleType";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl DehumidifierSetpointScale, "
                     "type:  ScaleType // ScaleType");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_DEHUMIDIFIER_SETPOINT_SCALE,
      "ZCL UnifyHumidityControl DehumidifierSetpointScale",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl DehumidifierSetpointPrecision, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_DEHUMIDIFIER_SETPOINT_PRECISION,
      "ZCL UnifyHumidityControl DehumidifierSetpointPrecision",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl AutoSetpointMin, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_AUTO_SETPOINT_MIN,
      "ZCL UnifyHumidityControl AutoSetpointMin",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl AutoSetpointMax, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_AUTO_SETPOINT_MAX,
      "ZCL UnifyHumidityControl AutoSetpointMax",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // int32 // int32 // int32_t
    std::string attribute_type_string           = "int32_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl AutoSetpoint, "
                     "type:  int32 // int32_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_AUTO_SETPOINT,
      "ZCL UnifyHumidityControl AutoSetpoint",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // ScaleType // ScaleType // ScaleType
    std::string attribute_type_string           = "ScaleType";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl AutoSetpointScale, "
                     "type:  ScaleType // ScaleType");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_AUTO_SETPOINT_SCALE,
      "ZCL UnifyHumidityControl AutoSetpointScale",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  {
    // uint8 // uint8 // uint8_t
    std::string attribute_type_string           = "uint8_t";
    attribute_store_storage_type_t storage_type = UNKNOWN_STORAGE_TYPE;

    // clang-format off
    storage_type = attribute_storage_type_conversion(attribute_type_string);

    if (storage_type == UNKNOWN_STORAGE_TYPE) {
      sl_log_warning(LOG_TAG,
                     "Unkown storage type for ZCL UnifyHumidityControl AutoSetpointPrecision, "
                     "type:  uint8 // uint8_t");
    }

    status |= attribute_store_register_type(
      DOTDOT_ATTRIBUTE_ID_UNIFY_HUMIDITY_CONTROL_AUTO_SETPOINT_PRECISION,
      "ZCL UnifyHumidityControl AutoSetpointPrecision",
      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
      storage_type);
  }

  // clang-format off
  // clang-format on

  // Additional attributes:
  for (auto const &a: zcl_additional_attribute_schema) {
    status |= attribute_store_register_type(a.type,
                                            a.name,
                                            a.parent_type,
                                            a.storage_type);
  }

  return status;
}
