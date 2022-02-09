import React from 'react';
import * as IoIcons from 'react-icons/io';
import * as MdIcons from 'react-icons/md'
import * as RiIcons from 'react-icons/ri'
import * as AiIcons from 'react-icons/ai'
import * as FiIcons from 'react-icons/fi'
import Tooltip from '@material-ui/core/Tooltip';
import { NavbarItem } from '../../components/navbar/navbar-types';
import { ClusterTypes } from '../../cluster-types/cluster-types';
import { ClusterViewOverride } from './base-cluster-types';

//Here you can find icons that can be used to customize you page: https://react-icons.github.io/react-icons/
//Don`t forgot to check licence if you use something that is not in Licence.txt

export let ClusterViewOverrides = {
    Thermostat: {
        ViewTable: [
            {
                Name: `Temperature`,
                Value: (item: any) => item.Attributes?.LocalTemperature?.Reported ? `${((item.Attributes.LocalTemperature.Reported || 0) / 100).toFixed(2)}°C` : "-"
            },
            {
                Name: `Heating Setpoint`,
                Value: (item: any) => item.Attributes?.OccupiedHeatingSetpoint?.Reported ? `${(item.Attributes.OccupiedHeatingSetpoint.Reported / 100).toFixed(2)}°C` : `-`
            },
            {
                Name: `Cooling Setpoint`,
                Value: (item: any) => item.Attributes?.OccupiedCoolingSetpoint?.Reported ? `${(item.Attributes.OccupiedCoolingSetpoint.Reported / 100).toFixed(2)}°C` : `-`
            },
            {
                Name: `System Mode`,
                Value: (item: any) => item.Attributes?.SystemMode?.Reported || "-"
            },
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Thermostat`}><span className="cursor-defult"><IoIcons.IoMdThermometer /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.Thermostat,
            title: 'Thermostats',
            path: '/thermostats',
            icon: <IoIcons.IoMdThermometer />,
            cName: 'nav-text'
        } as NavbarItem
    } as ClusterViewOverride,
    OnOff: {
        ViewTable: [
            {
                Name: `State`,
                Value: (item: any) => item.Attributes?.OnOff?.Reported !== undefined ?
                    (item.Attributes.OnOff.Reported === true
                        ? <Tooltip title="On"><span className="cursor-defult"><RiIcons.RiLightbulbFill color="#28a745" /></span></Tooltip>
                        : <Tooltip title="Off"><span className="cursor-defult"><RiIcons.RiLightbulbLine color="grey" /></span></Tooltip>)
                    : "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: On/Off`}><span className="cursor-defult"><IoIcons.IoIosSwitch /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.OnOff,
            title: 'On/Offs',
            path: '/onoffs',
            icon: <IoIcons.IoIosSwitch />,
            cName: 'nav-text'
        } as NavbarItem
    } as ClusterViewOverride,
    DoorLock: {
        ViewTable: [
            {
                Name: `Type`,
                Value: (item: any) => item.Attributes?.LockType?.Reported || "-"
            },
            {
                Name: `State`,
                Value: (item: any) => item.Attributes?.LockState?.Reported !== undefined ?
                    (item.Attributes.LockState.Reported === "Locked"
                        ? <Tooltip title="Locked"><span className="cursor-defult"><RiIcons.RiDoorFill color="#28a745"></RiIcons.RiDoorFill></span></Tooltip>
                        : <Tooltip title="UnLocked"><span className="cursor-defult"><RiIcons.RiDoorOpenFill color="#dc3545"></RiIcons.RiDoorOpenFill></span></Tooltip>)
                    : "-"
            },
            {
                Name: `Actuator`,
                Value: (item: any) => item.Attributes?.ActuatorEnabled?.Reported?.toString() || "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Door Lock`}><span className="cursor-defult"><RiIcons.RiDoorOpenFill /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.DoorLock,
            title: 'Door Locks',
            path: '/doorlocks',
            icon: <RiIcons.RiDoorOpenFill />,
            cName: 'nav-text'
        } as NavbarItem
    } as ClusterViewOverride,
    OccupancySensing: {
        ViewTable: [
            {
                Name: `Type`,
                Value: (item: any) => item.Attributes?.OccupancySensorType?.Reported || "-"
            },
            {
                Name: `State`,
                Value: (item: any) => item.Attributes?.Occupancy?.Reported !== undefined ?
                    (item.Attributes?.Occupancy?.Reported?.SensedOccupancy === true
                        ? <Tooltip title="Occupied"><span className="cursor-defult"><MdIcons.MdLeakAdd color="#dc3545" /></span></Tooltip>
                        : <Tooltip title="UnOccupied"><span className="cursor-defult"><MdIcons.MdLeakAdd /></span></Tooltip>)
                    : "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Binary Sensors`}><span className="cursor-defult"><MdIcons.MdLeakAdd /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.OccupancySensing,
            title: 'Binary Sensors',
            path: '/binarysensors',
            icon: <MdIcons.MdLeakAdd />,
            cName: 'nav-text'
        } as NavbarItem
    } as ClusterViewOverride,
    Identify: {
        ViewTable: [
            {
                Name: `Identify Time`,
                Value: (item: any) => <><span className={item.Attributes?.IdentifyTime?.Reported === 0 ? '' : "dot-icon bg-warning blinking"}></span>{item.Attributes?.IdentifyTime?.Reported}</>
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Identify`}><span className="cursor-defult"><RiIcons.RiScan2Line /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.Identify,
            title: 'Identify',
            path: '/identify',
            icon: <RiIcons.RiScan2Line />,
            cName: 'nav-text'
        } as NavbarItem,
    } as ClusterViewOverride,
    Level: {
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Level`}><span className="cursor-defult"><AiIcons.AiOutlineControl /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.Level,
            title: 'Level',
            path: '/level',
            icon: <AiIcons.AiOutlineControl />,
            cName: 'nav-text'
        } as NavbarItem,
    } as ClusterViewOverride,
    ColorControl: {
        ViewTable: [
            {
                Name: ``,
                Value: (item: any, colorPicker: any) => item.SupportedCommands?.indexOf("MoveToHueAndSaturation") > -1 ?
                    colorPicker(item)
                    : ""
            },
            {
                Name: `Current Hue`,
                Value: (item: any) => item.Attributes?.CurrentHue?.Reported || "-"
            },
            {
                Name: `Current Saturation`,
                Value: (item: any) => item.Attributes?.CurrentSaturation?.Reported || "-"
            },
            {
                Name: `Remaining Time`,
                Value: (item: any) => item.Attributes?.RemainingTime?.Reported || "-"
            },
            {
                Name: `Drift Compensation`,
                Value: (item: any) => item.Attributes?.DriftCompensation?.Reported || "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Color Control`}><span className="cursor-defult"><MdIcons.MdInvertColors /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.ColorControl,
            title: 'ColorControl',
            path: '/colorcontrol',
            icon: <MdIcons.MdInvertColors />,
            cName: 'nav-text'
        } as NavbarItem,
    } as ClusterViewOverride,
    IASZone: {
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: IASZone`}><span className="cursor-defult"><MdIcons.MdPlace /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.IASZone,
            title: 'IASZone',
            path: '/iaszone',
            icon: <MdIcons.MdPlace />,
            cName: 'nav-text'
        } as NavbarItem,
    } as ClusterViewOverride,
    Basic: {
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Basic`}><span className="cursor-defult"><FiIcons.FiCircle /></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.Basic,
            title: 'Basic',
            path: '/basic',
            icon: <FiIcons.FiCircle />,
            cName: 'nav-text'
        } as NavbarItem,
    } as ClusterViewOverride,
    PowerConfiguration: {
        ViewTable: [
            {
                Name: `Battery`,
                Value: (item: any) => item?.Attributes?.BatteryPercentageRemaining?.Reported !== undefined ? item?.Attributes?.BatteryPercentageRemaining?.Reported : "-"
            },
            {
                Name: `Battery Low`,
                Value: (item: any) => item?.Attributes?.BatteryAlarmMask?.Reported?.BatteryVoltageTooLow !== undefined ? item?.Attributes?.BatteryAlarmMask?.Reported?.BatteryVoltageTooLow.toString() : "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => {
            if (attr && attr.BatteryPercentageRemaining !== undefined && attr.BatteryPercentageRemaining.Reported !== undefined) {
                let percentage = attr.BatteryPercentageRemaining.Reported;
                let icon: any;
                if (attr.BatteryAlarmMask && attr.BatteryAlarmMask.Reported && attr.BatteryAlarmMask.Reported.BatteryVoltageTooLow)
                    icon = <MdIcons.MdBatteryAlert color="#dc3545" />;
                else if (percentage < 20)
                    icon = <MdIcons.MdBattery20 />;
                else if (percentage < 50)
                    icon = <MdIcons.MdBattery50 />;
                else if (percentage < 80)
                    icon = <MdIcons.MdBattery80 />;
                else icon = <MdIcons.MdBatteryFull />;
                return <Tooltip title={`${endpoint}: battery ${percentage}%`}><span className="cursor-defult">{icon}</span></Tooltip>
            }
        },
        NavbarItem: {
            name: ClusterTypes.Basic,
            title: 'Battery',
            path: '/battery',
            icon: <MdIcons.MdBatteryStd />,
            cName: 'nav-text'
        } as NavbarItem
    } as ClusterViewOverride,
    RFTelemetry: {
        NavbarItem: {
            name: 'RF Telemetry',
            title: 'RF Telemetry',
            path: '/rftelemetry',
            icon: <MdIcons.MdOutlineTransform />,
            cName: 'nav-text'
        } as NavbarItem
    } as ClusterViewOverride,
    NameAndLocation: {
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title="NameAndLocation"><span className="cursor-defult"><MdIcons.MdBookmarks /></span></Tooltip>
    } as ClusterViewOverride,
    AoXLocator: {
        NodesTooltip: () => <Tooltip title="AoXLocator"><span className="cursor-defult"><IoIcons.IoIosLocate /></span></Tooltip>
    } as ClusterViewOverride,
    AoXPositionEstimation: {
        NodesTooltip: () => <Tooltip title="AoXPositionEstimation"><span className="cursor-defult"><IoIcons.IoMdLocate /></span></Tooltip>
    } as ClusterViewOverride
}