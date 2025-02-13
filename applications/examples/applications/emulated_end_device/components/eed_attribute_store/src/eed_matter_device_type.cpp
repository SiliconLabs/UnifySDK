/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <cstring>
#include "sl_log.h"

#include "eed_matter_device_type.h"

// Declare DeviceClusterData structure
struct DeviceClusterData {
    std::string cluster_name;
    bool is_supported;
};

// Device type IDs mapping
const std::map<std::string, std::vector<DeviceClusterData>> matter_device_type_vs_clusters_map = {
    { "doorlock", { 
                    { "DoorLock", true }, 
                    { "Identify", true }} },
    { "onofflight", {
                    { "Groups", true },
                    { "Identify", true },
                    { "OnOff", true } } },
    { "dimmablelight", {
                    { "Groups", true },
                    { "Identify", true },
                    { "Level", true },
                    { "OnOff", true } } },
    { "onofflightswitch", {
                    { "Binding", true },
                    { "Identify", true },
                    { "OnOff", true } } },
    { "lightsensor", { 
                    { "IlluminanceMeasurement", true } } },
    { "occupancysensor", { 
                    { "Identify", true },
                    { "OccupancySensing", true } } },
    { "onoffpluginunit", {
                    { "Groups", true  },
                    { "Identify", true },
                    { "OnOff", true } } },
    { "dimmablepluginunit", {
                    { "Groups", true  },
                    { "Identify", true },
                    { "Level", true },
                    { "OnOff", true } } },
    { "extendedcolorlight", {
                    { "ColorControl", true },
                    { "Groups", true },
                    { "Identify", true },
                    { "Level", true },
                    { "OnOff", true } } },
    { "windowcovering", { 
                    { "Identify", true },
                    { "WindowCovering", true } } },
    { "thermostat", { 
                    { "Identify", true },
                    { "Thermostat", true }} },
    { "pressuresensor", { 
                    { "Identify", true },
                    { "PressureMeasurement", true } } },
    { "flowsensor", {  
                    { "FlowMeasurement", true }, 
                    { "Identify", true } } },
    { "allclustersdevice", { { "Basic", true},
                    { "PowerConfiguration", true},
                    { "DeviceTemperatureConfiguration", true},
                    { "Identify", true},
                    { "Groups", true},
                    { "Scenes", true},
                    { "OnOff", true},
                    { "Level", true},
                    { "Alarms", true},
                    { "Time", true},
                    { "PollControl", true},
                    { "ShadeConfiguration", true},
                    { "DoorLock", true},
                    { "WindowCovering", true},
                    { "BarrierControl", true},
                    { "PumpConfigurationAndControl", true},
                    { "Thermostat", true},
                    { "FanControl", true},
                    { "DehumidificationControl", true},
                    { "ThermostatUserInterfaceConfiguration", true},
                    { "ColorControl", true},
                    { "BallastConfiguration", true},
                    { "IlluminanceMeasurement", true},
                    { "IlluminanceLevelSensing", true},
                    { "TemperatureMeasurement", true},
                    { "PressureMeasurement", true},
                    { "FlowMeasurement", true},
                    { "RelativityHumidity", true},
                    { "OccupancySensing", true},
                    { "SoilMoisture", true},
                    { "PhMeasurement", true},
                    { "ElectricalConductivityMeasurement", true},
                    { "WindSpeedMeasurement", true},
                    { "CarbonMonoxide", true},
                    { "CarbonDioxide", true},
                    { "PM25", true},
                    { "IASZone", true},
                    { "IASWD", true},
                    { "Metering", true},
                    { "ElectricalMeasurement", true},
                    { "Diagnostics", true},
                    { "Binding", true},
                    { "NameAndLocation", true},
                    { "ConfigurationParameters", true},
                    { "Descriptor", true},
                    { "UnifyFanControl", true},
                    { "UnifyThermostat", true},
                    { "UnifyHumidityControl", true},}}
};

sl_status_t getClustersForDeviceType(const char* deviceType, char* clusterList, size_t clusterListSize) {
    auto it = matter_device_type_vs_clusters_map.find(deviceType);
    if (it == matter_device_type_vs_clusters_map.end()) {
        return SL_STATUS_NOT_SUPPORTED;
    }

    // TODO: Filter out unsupported clusters. Currently, we are supporting all device types,
    // so filtering out unsupported clusters is not necessary.
    // getSupportedClusters(it->second)

    std::string tempClusterList;
    for (const auto& clusterData : it->second) {
        tempClusterList += clusterData.cluster_name;
        tempClusterList += ";";
    }

    if (tempClusterList.length() >= clusterListSize) {
        return SL_STATUS_FAIL;
    }

    std::strncpy(clusterList, tempClusterList.c_str(), clusterListSize);

    return SL_STATUS_OK;
}

bool isDeviceType(const std::string& config) {
    return matter_device_type_vs_clusters_map.find(config) != matter_device_type_vs_clusters_map.end();
}
