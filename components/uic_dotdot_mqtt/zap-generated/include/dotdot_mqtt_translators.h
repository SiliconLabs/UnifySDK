/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef DOTDOT_MQTT_TRANSLATORS_H
#define DOTDOT_MQTT_TRANSLATORS_H
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <nlohmann/json.hpp>

#include <zap-types.h>


/**
 * Parse NM_StateParameters type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, NM_StateParameters& field)
{
  field.ProvisioningMode = jsn.at("ProvisioningMode").get_ptr<const std::string*>()->c_str();
  jsn.at("UserAccept").get_to(field.UserAccept);
  field.SecurityCode = jsn.at("SecurityCode").get_ptr<const std::string*>()->c_str();
  field.Unid = jsn.at("Unid").get_ptr<const std::string*>()->c_str();
  jsn.at("AllowMultipleInclusions").get_to(field.AllowMultipleInclusions);
}


/**
 * Build JSON object from NM_StateParameters type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const NM_StateParameters& field)
{
  jsn["ProvisioningMode"] = field.ProvisioningMode;
  jsn["UserAccept"] = field.UserAccept;
  jsn["SecurityCode"] = field.SecurityCode;
  jsn["Unid"] = field.Unid;
  jsn["AllowMultipleInclusions"] = field.AllowMultipleInclusions;
}


/**
 * Parse NetworkManagementState type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, NetworkManagementState& field)
{
  jsn.at("State").get_to(field.State);
  field.SupportedStateList = jsn.at("SupportedStateList").get_ptr<const std::string*>()->c_str();
  jsn.at("StateParameters").get_to(field.StateParameters);
  field.RequestedStateParameters = jsn.at("RequestedStateParameters").get_ptr<const std::string*>()->c_str();
}


/**
 * Build JSON object from NetworkManagementState type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const NetworkManagementState& field)
{
  jsn["State"] = field.State;
  jsn["SupportedStateList"] = field.SupportedStateList;
  jsn["StateParameters"] = field.StateParameters;
  jsn["RequestedStateParameters"] = field.RequestedStateParameters;
}


/**
 * Parse BindingObject type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, BindingObject& field)
{
  field.ClusterName = jsn.at("ClusterName").get_ptr<const std::string*>()->c_str();
  field.DestinationUnid = jsn.at("DestinationUnid").get_ptr<const std::string*>()->c_str();
  jsn.at("DestinationEp").get_to(field.DestinationEp);
}


/**
 * Build JSON object from BindingObject type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const BindingObject& field)
{
  jsn["ClusterName"] = field.ClusterName;
  jsn["DestinationUnid"] = field.DestinationUnid;
  jsn["DestinationEp"] = field.DestinationEp;
}


/**
 * Parse ConfigurationParameter type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, ConfigurationParameter& field)
{
  jsn.at("ParameterId").get_to(field.ParameterId);
  jsn.at("Value").get_to(field.Value);
  field.Name = jsn.at("Name").get_ptr<const std::string*>()->c_str();
  field.Info = jsn.at("Info").get_ptr<const std::string*>()->c_str();
  jsn.at("MinimumValue").get_to(field.MinimumValue);
  jsn.at("MaximumValue").get_to(field.MaximumValue);
  jsn.at("DefaultValue").get_to(field.DefaultValue);
  jsn.at("DisplayFormat").get_to(field.DisplayFormat);
  jsn.at("ReadOnly").get_to(field.ReadOnly);
  jsn.at("Advanced").get_to(field.Advanced);
  jsn.at("AlteringCapabilities").get_to(field.AlteringCapabilities);
}


/**
 * Build JSON object from ConfigurationParameter type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const ConfigurationParameter& field)
{
  jsn["ParameterId"] = field.ParameterId;
  jsn["Value"] = field.Value;
  jsn["Name"] = field.Name;
  jsn["Info"] = field.Info;
  jsn["MinimumValue"] = field.MinimumValue;
  jsn["MaximumValue"] = field.MaximumValue;
  jsn["DefaultValue"] = field.DefaultValue;
  jsn["DisplayFormat"] = field.DisplayFormat;
  jsn["ReadOnly"] = field.ReadOnly;
  jsn["Advanced"] = field.Advanced;
  jsn["AlteringCapabilities"] = field.AlteringCapabilities;
}


/**
 * Parse CoordinateAndOrientation type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, CoordinateAndOrientation& field)
{
  jsn.at("CoordinateX").get_to(field.CoordinateX);
  jsn.at("CoordinateY").get_to(field.CoordinateY);
  jsn.at("CoordinateZ").get_to(field.CoordinateZ);
  jsn.at("OrientationX").get_to(field.OrientationX);
  jsn.at("OrientationY").get_to(field.OrientationY);
  jsn.at("OrientationZ").get_to(field.OrientationZ);
}


/**
 * Build JSON object from CoordinateAndOrientation type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const CoordinateAndOrientation& field)
{
  jsn["CoordinateX"] = field.CoordinateX;
  jsn["CoordinateY"] = field.CoordinateY;
  jsn["CoordinateZ"] = field.CoordinateZ;
  jsn["OrientationX"] = field.OrientationX;
  jsn["OrientationY"] = field.OrientationY;
  jsn["OrientationZ"] = field.OrientationZ;
}


/**
 * Parse DiskIOCounters type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, DiskIOCounters& field)
{
  field.UUID = jsn.at("UUID").get_ptr<const std::string*>()->c_str();
  jsn.at("ReadCount").get_to(field.ReadCount);
  jsn.at("WriteCount").get_to(field.WriteCount);
  jsn.at("ReadBytes").get_to(field.ReadBytes);
  jsn.at("WriteBytes").get_to(field.WriteBytes);
}


/**
 * Build JSON object from DiskIOCounters type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const DiskIOCounters& field)
{
  jsn["UUID"] = field.UUID;
  jsn["ReadCount"] = field.ReadCount;
  jsn["WriteCount"] = field.WriteCount;
  jsn["ReadBytes"] = field.ReadBytes;
  jsn["WriteBytes"] = field.WriteBytes;
}


/**
 * Parse DiskUsageData type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, DiskUsageData& field)
{
  field.UUID = jsn.at("UUID").get_ptr<const std::string*>()->c_str();
  jsn.at("UsagePercent").get_to(field.UsagePercent);
  jsn.at("FreeSpaceMB").get_to(field.FreeSpaceMB);
}


/**
 * Build JSON object from DiskUsageData type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const DiskUsageData& field)
{
  jsn["UUID"] = field.UUID;
  jsn["UsagePercent"] = field.UsagePercent;
  jsn["FreeSpaceMB"] = field.FreeSpaceMB;
}


/**
 * Parse EstimatedPosition type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, EstimatedPosition& field)
{
  field.ApplicationId = jsn.at("ApplicationId").get_ptr<const std::string*>()->c_str();
  jsn.at("X").get_to(field.X);
  jsn.at("Y").get_to(field.Y);
  jsn.at("Z").get_to(field.Z);
  jsn.at("DeviationX").get_to(field.DeviationX);
  jsn.at("DeviationY").get_to(field.DeviationY);
  jsn.at("DeviationZ").get_to(field.DeviationZ);
  jsn.at("Sequence").get_to(field.Sequence);
}


/**
 * Build JSON object from EstimatedPosition type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const EstimatedPosition& field)
{
  jsn["ApplicationId"] = field.ApplicationId;
  jsn["X"] = field.X;
  jsn["Y"] = field.Y;
  jsn["Z"] = field.Z;
  jsn["DeviationX"] = field.DeviationX;
  jsn["DeviationY"] = field.DeviationY;
  jsn["DeviationZ"] = field.DeviationZ;
  jsn["Sequence"] = field.Sequence;
}


/**
 * Parse IasaceZoneStatusRecord type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, IasaceZoneStatusRecord& field)
{
  jsn.at("ZoneID").get_to(field.ZoneID);
  jsn.at("ZoneStatus").get_to(field.ZoneStatus);
}


/**
 * Build JSON object from IasaceZoneStatusRecord type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const IasaceZoneStatusRecord& field)
{
  jsn["ZoneID"] = field.ZoneID;
  jsn["ZoneStatus"] = field.ZoneStatus;
}


/**
 * Parse MinMaxPair type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, MinMaxPair& field)
{
  jsn.at("Min").get_to(field.Min);
  jsn.at("Max").get_to(field.Max);
}


/**
 * Build JSON object from MinMaxPair type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const MinMaxPair& field)
{
  jsn["Min"] = field.Min;
  jsn["Max"] = field.Max;
}


/**
 * Parse NetworkInterfaceData type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, NetworkInterfaceData& field)
{
  field.InterfaceName = jsn.at("InterfaceName").get_ptr<const std::string*>()->c_str();
  jsn.at("BytesSent").get_to(field.BytesSent);
  jsn.at("BytesReceived").get_to(field.BytesReceived);
  jsn.at("PacketsSent").get_to(field.PacketsSent);
  jsn.at("PacketsReceived").get_to(field.PacketsReceived);
  jsn.at("ErrorsIn").get_to(field.ErrorsIn);
  jsn.at("ErrorsOut").get_to(field.ErrorsOut);
  jsn.at("DropIn").get_to(field.DropIn);
  jsn.at("DropOut").get_to(field.DropOut);
}


/**
 * Build JSON object from NetworkInterfaceData type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const NetworkInterfaceData& field)
{
  jsn["InterfaceName"] = field.InterfaceName;
  jsn["BytesSent"] = field.BytesSent;
  jsn["BytesReceived"] = field.BytesReceived;
  jsn["PacketsSent"] = field.PacketsSent;
  jsn["PacketsReceived"] = field.PacketsReceived;
  jsn["ErrorsIn"] = field.ErrorsIn;
  jsn["ErrorsOut"] = field.ErrorsOut;
  jsn["DropIn"] = field.DropIn;
  jsn["DropOut"] = field.DropOut;
}


/**
 * Parse SExtensionFieldSetList type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, SExtensionFieldSetList& field)
{
  jsn.at("ClusterId").get_to(field.ClusterId);
  field.ExtensionFieldSet = jsn.at("ExtensionFieldSet").get_ptr<const std::string*>()->c_str();
}


/**
 * Build JSON object from SExtensionFieldSetList type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const SExtensionFieldSetList& field)
{
  jsn["ClusterId"] = field.ClusterId;
  jsn["ExtensionFieldSet"] = field.ExtensionFieldSet;
}


/**
 * Parse SSceneTable type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, SSceneTable& field)
{
  jsn.at("SceneID").get_to(field.SceneID);
  jsn.at("GroupID").get_to(field.GroupID);
  field.SceneName = jsn.at("SceneName").get_ptr<const std::string*>()->c_str();
  jsn.at("TransitionTime").get_to(field.TransitionTime);
  jsn.at("TransitionTime100ms").get_to(field.TransitionTime100ms);
  field.SceneTableExtensions = jsn.at("SceneTableExtensions").get_ptr<const std::string*>()->c_str();
}


/**
 * Build JSON object from SSceneTable type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const SSceneTable& field)
{
  jsn["SceneID"] = field.SceneID;
  jsn["GroupID"] = field.GroupID;
  jsn["SceneName"] = field.SceneName;
  jsn["TransitionTime"] = field.TransitionTime;
  jsn["TransitionTime100ms"] = field.TransitionTime100ms;
  jsn["SceneTableExtensions"] = field.SceneTableExtensions;
}


/**
 * Parse SphericalCoordinates type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, SphericalCoordinates& field)
{
  jsn.at("Azimuth").get_to(field.Azimuth);
  jsn.at("Elevation").get_to(field.Elevation);
  jsn.at("Distance").get_to(field.Distance);
}


/**
 * Build JSON object from SphericalCoordinates type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const SphericalCoordinates& field)
{
  jsn["Azimuth"] = field.Azimuth;
  jsn["Elevation"] = field.Elevation;
  jsn["Distance"] = field.Distance;
}


/**
 * Parse TransitionType type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, TransitionType& field)
{
  jsn.at("TransitionTime").get_to(field.TransitionTime);
  jsn.at("HeatSetPoint").get_to(field.HeatSetPoint);
  jsn.at("CoolSetPoint").get_to(field.CoolSetPoint);
}


/**
 * Build JSON object from TransitionType type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const TransitionType& field)
{
  jsn["TransitionTime"] = field.TransitionTime;
  jsn["HeatSetPoint"] = field.HeatSetPoint;
  jsn["CoolSetPoint"] = field.CoolSetPoint;
}


/**
 * Parse WirelessNetworkInterfaceData type from json object.
 *
 * @param jsn     JSON object to parse from.
 * @param field  field to be populated from JSON object.
 */
inline void from_json(const nlohmann::json& jsn, WirelessNetworkInterfaceData& field)
{
  field.InterfaceName = jsn.at("InterfaceName").get_ptr<const std::string*>()->c_str();
  jsn.at("SignalStrengthdBm").get_to(field.SignalStrengthdBm);
}


/**
 * Build JSON object from WirelessNetworkInterfaceData type.
 *
 * @param jsn     JSON object to be populated.
 * @param field   Reference data.
 */
inline void to_json(nlohmann::json& jsn, const WirelessNetworkInterfaceData& field)
{
  jsn["InterfaceName"] = field.InterfaceName;
  jsn["SignalStrengthdBm"] = field.SignalStrengthdBm;
}


// END OF nlohmann parsers

#endif
