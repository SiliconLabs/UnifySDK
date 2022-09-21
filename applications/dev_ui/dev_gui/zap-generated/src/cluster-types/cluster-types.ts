//This file is generated automatically. Don't try to change something here.
//To add support for new clusters, modify addon-helper.js
//To change the stucture of the ClusterTypes, modify cluster-types.zapt

export enum ClusterTypes {
    Basic = "Basic", 
	ColorControl = "ColorControl", 
	DoorLock = "DoorLock", 
	ElectricalMeasurement = "ElectricalMeasurement", 
	IASZone = "IASZone", 
	Identify = "Identify", 
	Level = "Level", 
	OccupancySensing = "OccupancySensing", 
	OnOff = "OnOff", 
	Scenes = "Scenes", 
	Thermostat = "Thermostat", 
	Metering = "Metering", 
	PowerConfiguration = "PowerConfiguration"
}

export enum ControllerTypes {
    ProtocolController = "ProtocolController"
}

export type NodeTypes = ClusterTypes | ControllerTypes;

export const NodeTypesList = {...ClusterTypes, ...ControllerTypes};