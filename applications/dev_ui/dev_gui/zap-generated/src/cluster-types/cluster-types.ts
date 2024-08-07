//This file is generated automatically. Don't try to change something here.
//To add support for new clusters, modify addon-helper.js
//To change the stucture of the ClusterTypes, modify cluster-types.zapt

export enum ClusterTypes {
    BarrierControl = "BarrierControl", 
	Basic = "Basic", 
	ColorControl = "ColorControl", 
	CarbonDioxide = "CarbonDioxide", 
	CarbonMonoxide = "CarbonMonoxide", 
	Descriptor = "Descriptor", 
	DoorLock = "DoorLock", 
	ElectricalConductivityMeasurement = "ElectricalConductivityMeasurement", 
	ElectricalMeasurement = "ElectricalMeasurement", 
	FlowMeasurement = "FlowMeasurement", 
	IASZone = "IASZone", 
	Identify = "Identify", 
	IlluminanceMeasurement = "IlluminanceMeasurement", 
	Level = "Level", 
	Metering = "Metering", 
	OccupancySensing = "OccupancySensing", 
	OnOff = "OnOff", 
	PhMeasurement = "PhMeasurement", 
	PM25 = "PM25", 
	PowerConfiguration = "PowerConfiguration", 
	PressureMeasurement = "PressureMeasurement", 
	RelativityHumidity = "RelativityHumidity", 
	Scenes = "Scenes", 
	SoilMoisture = "SoilMoisture", 
	TemperatureMeasurement = "TemperatureMeasurement", 
	Thermostat = "Thermostat", 
	WindSpeedMeasurement = "WindSpeedMeasurement", 
	WindowCovering = "WindowCovering", 
	UserCredential = "UserCredential"
}

export enum ControllerTypes {
    ProtocolController = "ProtocolController"
}

export type NodeTypes = ClusterTypes | ControllerTypes;

export const NodeTypesList = {...ClusterTypes, ...ControllerTypes};