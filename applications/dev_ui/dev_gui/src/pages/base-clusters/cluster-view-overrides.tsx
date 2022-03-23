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
import { Link } from 'react-router-dom';

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
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Thermostat`}><span className="cursor-default"><Link to={`/thermostats`}><IoIcons.IoMdThermometer color="#212529" /></Link></span></Tooltip>,
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
                        ? <Tooltip title="On"><span className="cursor-default"><RiIcons.RiLightbulbFill color="#28a745" /></span></Tooltip>
                        : <Tooltip title="Off"><span className="cursor-default"><RiIcons.RiLightbulbLine color="grey" /></span></Tooltip>)
                    : "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: On/Off`}><span className="cursor-default"><Link to={`/onoffs`}><IoIcons.IoIosSwitch color="#212529" /></Link></span></Tooltip>,
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
                        ? <Tooltip title="Locked"><span className="cursor-default"><RiIcons.RiDoorFill color="#28a745"></RiIcons.RiDoorFill></span></Tooltip>
                        : <Tooltip title="UnLocked"><span className="cursor-default"><RiIcons.RiDoorOpenFill color="#dc3545"></RiIcons.RiDoorOpenFill></span></Tooltip>)
                    : "-"
            },
            {
                Name: `Actuator`,
                Value: (item: any) => item.Attributes?.ActuatorEnabled?.Reported?.toString() || "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Door Lock`}><span className="cursor-default"><Link to={`/doorlocks`}><RiIcons.RiDoorOpenFill color="#212529" /></Link></span></Tooltip>,
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
                        ? <Tooltip title="Occupied"><span className="cursor-default"><MdIcons.MdLeakAdd color="#dc3545" /></span></Tooltip>
                        : <Tooltip title="UnOccupied"><span className="cursor-default"><MdIcons.MdLeakAdd /></span></Tooltip>)
                    : "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Binary Sensors`}><span className="cursor-default"><Link to={`/binarysensors`}><MdIcons.MdLeakAdd color="#212529" /></Link></span></Tooltip>,
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
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Identify`}><span className="cursor-default"><Link to={`/identify`}><RiIcons.RiScan2Line color="#212529" /></Link></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.Identify,
            title: 'Identify',
            path: '/identify',
            icon: <RiIcons.RiScan2Line />,
            cName: 'nav-text'
        } as NavbarItem,
    } as ClusterViewOverride,
    Level: {
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Level`}><span className="cursor-default"><Link to={`/level`}><AiIcons.AiOutlineControl color="#212529" /></Link></span></Tooltip>,
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
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Color Control`}><span className="cursor-default"><Link to={`/colorcontrol`}><MdIcons.MdInvertColors color="#212529" /></Link></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.ColorControl,
            title: 'ColorControl',
            path: '/colorcontrol',
            icon: <MdIcons.MdInvertColors />,
            cName: 'nav-text'
        } as NavbarItem,
    } as ClusterViewOverride,
    IASZone: {
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: IASZone`}><span className="cursor-default"><Link to={`/iaszone`}><MdIcons.MdPlace color="#212529" /></Link></span></Tooltip>,
        NavbarItem: {
            name: ClusterTypes.IASZone,
            title: 'IASZone',
            path: '/iaszone',
            icon: <MdIcons.MdPlace />,
            cName: 'nav-text'
        } as NavbarItem,
    } as ClusterViewOverride,
    Basic: {
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title={`${endpoint}: Basic`}><span className="cursor-default"><Link to={`/basic`}><FiIcons.FiCircle color="#212529" /></Link></span></Tooltip>,
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
                    icon = <MdIcons.MdBattery20 color="#212529" />;
                else if (percentage < 50)
                    icon = <MdIcons.MdBattery50 color="#212529" />;
                else if (percentage < 80)
                    icon = <MdIcons.MdBattery80 color="#212529" />;
                else icon = <MdIcons.MdBatteryFull color="#212529" />;
                return <Tooltip title={`${endpoint}: battery ${percentage}%`}><span className="cursor-default"><Link to={`/battery`}>{icon}</Link></span></Tooltip>
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
        NodesTooltip: (endpoint: string, attr: any) => <Tooltip title="NameAndLocation"><span className="cursor-default"><MdIcons.MdBookmarks color="#212529" /></span></Tooltip>
    } as ClusterViewOverride,
    AoXLocator: {
        NodesTooltip: () => <Tooltip title="AoXLocator"><span className="cursor-default"><Link to={`/locators`}><IoIcons.IoIosLocate color="#212529" /></Link></span></Tooltip>
    } as ClusterViewOverride,
    AoXPositionEstimation: {
        NodesTooltip: () => <Tooltip title="AoXPositionEstimation"><span className="cursor-default"><Link to={`/locators`}><IoIcons.IoMdLocate color="#212529" /></Link></span></Tooltip>
    } as ClusterViewOverride,
    ConfigurationParameters: {
        NodesTooltip: () => <Tooltip title="Configuration Parameters"><span className="cursor-defult"><Link to={`/configurationparameters`}><MdIcons.MdOutlineSettingsApplications color="#212529"/></Link></span></Tooltip>,
        NavbarItem: {
            name: "Configuration Parameters",
            title: 'Configuration Parameters',
            path: '/configurationparameters',
            icon: <MdIcons.MdOutlineSettingsApplications />,
            cName: 'nav-text'
        } as NavbarItem
    } as ClusterViewOverride,
    Scenes: {
        ViewTable: [
            {
                Name: `Number of Scenes`,
                Value: (item: any) => item.Attributes?.SceneCount?.Reported || "-"
            },
            {
                Name: `Current Scene`,
                Value: (item: any) =>
                    item.Attributes?.CurrentScene?.Reported !== undefined ?
                        item.Attributes?.CurrentScene?.Reported : "-"
            },
            {
                Name: `Current Scene valid`,
                Value: (item: any) =>
                    item.Attributes?.SceneValid?.Reported !== undefined ?
                        (item.Attributes.SceneValid.Reported === true
                            ? <FiIcons.FiCheck color="#28a745" />
                            : <FiIcons.FiXCircle color="#6c757d" /> )
                        : "-"
            }
        ],
        NodesTooltip: () => <Tooltip title="Scenes"><span className="cursor-defult"><Link to={`/Scenes`}><MdIcons.MdOutlineDeveloperBoard color="#212529"/></Link></span></Tooltip>,
        NavbarItem: {
            name: "Scenes",
            title: 'Scenes',
            path: '/Scenes',
            icon: <MdIcons.MdOutlineDeveloperBoard />,
            cName: 'nav-text'
        } as NavbarItem
    } as ClusterViewOverride
}