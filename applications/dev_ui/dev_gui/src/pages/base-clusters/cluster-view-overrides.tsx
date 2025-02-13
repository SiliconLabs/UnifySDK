import React from 'react';
import * as IoIcons from 'react-icons/io';
import * as IoIcons5 from 'react-icons/io5';
import * as MdIcons from 'react-icons/md'
import * as RiIcons from 'react-icons/ri'
import * as AiIcons from 'react-icons/ai'
import * as FiIcons from 'react-icons/fi'
import { NavbarItem, SideMenu } from '../../components/navbar/navbar-types';
import { ClusterTypes } from '../../cluster-types/cluster-types';
import { ClusterViewOverride } from './base-cluster-types';
import { Link } from 'react-router-dom';
import { Tooltip } from '@mui/material';
import { Button } from 'react-bootstrap';

//Here you can find icons that can be used to customize you page: https://react-icons.github.io/react-icons/
//Don`t forgot to check licence if you use something that is not in Licence.txt

export let ClusterViewOverrides = {
    Thermostat: {
        ViewTable: [
            {
                Name: `Temperature`,
                Value: (item: any) => item.Attributes?.LocalTemperature?.Reported !== undefined ? `${((item.Attributes.LocalTemperature.Reported || 0) / 100).toFixed(2)}°C` : "-"
            },
            {
                Name: `Heating Setpoint`,
                Value: (item: any) => item.Attributes?.OccupiedHeatingSetpoint?.Reported !== undefined ? `${(item.Attributes.OccupiedHeatingSetpoint.Reported / 100).toFixed(2)}°C` : `-`
            },
            {
                Name: `Cooling Setpoint`,
                Value: (item: any) => item.Attributes?.OccupiedCoolingSetpoint?.Reported !== undefined ? `${(item.Attributes.OccupiedCoolingSetpoint.Reported / 100).toFixed(2)}°C` : `-`
            },
            {
                Name: `System Mode`,
                Value: (item: any) => item.Attributes?.SystemMode?.Reported || "-"
            },
        ],
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Thermostat`}>
                <span className="cursor-default">
                    <Link to={`/thermostats`}>
                        <IoIcons.IoMdThermometer color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: ClusterTypes.Thermostat,
            title: 'Thermostats',
            path: '/thermostats',
            icon: <IoIcons.IoMdThermometer />,
            cName: 'nav-text',
            subMenu: SideMenu.Actuators
        } as NavbarItem,
        IsExpandable: true
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
        NodesTooltip: (endpoint: string, attr: any) => {
            let state = (attr?.OnOff?.Reported !== undefined) ?
                ((attr?.OnOff?.Reported === true) ? "On" : "Off")
                : "Unknown";
            return (
                <Tooltip title={`Endpoint ${endpoint}: On/Off (${state})`}>
                    <span className="cursor-default">
                        <Link to={`/onoffs`}>
                            <IoIcons.IoIosSwitch color="#212529" />
                        </Link>
                    </span>
                </Tooltip>
            )
        },
        NavbarItem: {
            name: ClusterTypes.OnOff,
            title: 'On/Offs',
            path: '/onoffs',
            icon: <IoIcons.IoIosSwitch />,
            cName: 'nav-text',
            subMenu: SideMenu.Actuators
        } as NavbarItem,
        IsExpandable: true
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
        NodesTooltip: (endpoint: string, attr: any) => {
            let state = attr?.LockState?.Reported ?? "Unknown";
            return (
                <Tooltip title={`Endpoint ${endpoint}: Door Lock ${state}`}>
                    <span className="cursor-default">
                        <Link to={`/doorlocks`}>
                            <RiIcons.RiDoorOpenFill color="#212529" />
                        </Link>
                    </span>
                </Tooltip>
            )
        },
        NavbarItem: {
            name: ClusterTypes.DoorLock,
            title: 'Door Locks',
            path: '/doorlocks',
            icon: <RiIcons.RiDoorOpenFill />,
            cName: 'nav-text',
            subMenu: SideMenu.Actuators
        } as NavbarItem,
        IsExpandable: true
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
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Binary Sensors`}>
                <span className="cursor-default">
                    <Link to={`/binarysensors`}>
                        <MdIcons.MdLeakAdd color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: ClusterTypes.OccupancySensing,
            title: 'Binary Sensors',
            path: '/binarysensors',
            icon: <MdIcons.MdLeakAdd />,
            cName: 'nav-text',
            subMenu: SideMenu.Sensors
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    Identify: {
        ViewTable: [
            {
                Name: `Identify Time`,
                Value: (item: any) => <><span className={item.Attributes?.IdentifyTime?.Reported === 0 ? '' : "dot-icon bg-warning blinking"}></span>{item.Attributes?.IdentifyTime?.Reported}</>
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => {
            let time = attr?.IdentifyTime?.Reported ?? "0";
            return (
                <Tooltip title={`Endpoint ${endpoint}: Identify ${time}s`}>
                    <span className="cursor-default">
                        <Link to={`/identify`}>
                            <RiIcons.RiScan2Line color="#212529" />
                        </Link>
                    </span>
                </Tooltip>
            )
        },
        NavbarItem: {
            name: ClusterTypes.Identify,
            title: 'Identify',
            path: '/identify',
            icon: <RiIcons.RiScan2Line />,
            cName: 'nav-text',
            subMenu: SideMenu.NodeManagement
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    Level: {
        NodesTooltip: (endpoint: string, attr: any) => {
            let level = attr?.CurrentLevel?.Reported ?? "Unknown";
            return (
                <Tooltip title={`Endpoint ${endpoint}: Level ${level}`}>
                    <span className="cursor-default">
                        <Link to={`/level`}>
                            <AiIcons.AiOutlineControl color="#212529" />
                        </Link>
                    </span>
                </Tooltip>
            )
        },
        NavbarItem: {
            name: ClusterTypes.Level,
            title: 'Level',
            path: '/level',
            icon: <AiIcons.AiOutlineControl />,
            cName: 'nav-text',
            subMenu: SideMenu.Actuators
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    ColorControl: {
        ViewTable: [
            {
                Name: `Capabilities`,
                Value: (item: any) => {
                    let capabilities = item.Attributes?.ColorCapabilities?.Reported;
                    let capabilitiesKeys = capabilities && Object.keys(capabilities);
                    return capabilitiesKeys ? <ul className='padding-l-15'> {capabilitiesKeys.map((i: string, index: number) => {
                        return <Tooltip title={capabilities[i] ? "Supported" : "Not Supported"}><li key={index} className={capabilities[i] ? "supported" : "not-supported"}>{i}</li></Tooltip>
                    })}
                    </ul> : "-"
                }
            },
            {
                Name: `Color Mode`,
                Value: (item: any) => item.Attributes?.ColorMode?.Reported ?? item.Attributes?.EnhancedColorMode?.Reported ?? "-"
            },
            {
                Name: `Color`,
                Value: (item: any, colorPicker: any, clusters: any) => {
                    let hue = item.Attributes?.CurrentHue?.Reported === undefined ? 211 : item.Attributes.CurrentHue.Reported / 254 * 360;
                    let saturation = item.Attributes?.CurrentSaturation?.Reported === undefined ? 100 : item.Attributes.CurrentSaturation.Reported / 254 * 100;
                    let level = clusters?.Level?.Attributes?.CurrentLevel?.Reported ?? 50;
                    return item.SupportedCommands?.indexOf("MoveToHueAndSaturation") > -1 && colorPicker !== undefined ?
                        colorPicker(item)
                        : <MdIcons.MdInvertColors color={`hsl(${hue}deg ${saturation}% ${level > 90 ? 90 : level}%)`}></MdIcons.MdInvertColors>
                }
            },
            {
                Name: `Current Hue`,
                Value: (item: any) => item.Attributes?.CurrentHue?.Reported ?? "-"
            },
            {
                Name: `Current Saturation`,
                Value: (item: any) => item.Attributes?.CurrentSaturation?.Reported ?? "-"
            },
            {
                Name: `Temperature Mireds`,
                Value: (item: any) => item.Attributes?.ColorTemperatureMireds?.Reported ?? "-"
            },
            {
                Name: `MinMireds`,
                Value: (item: any) => item.Attributes?.ColorTempPhysicalMinMireds?.Reported ?? "-"
            },
            {
                Name: `MaxMireds`,
                Value: (item: any) => item.Attributes?.ColorTempPhysicalMaxMireds?.Reported ?? "-"
            },
            {
                Name: `Remaining Time`,
                Value: (item: any) => item.Attributes?.RemainingTime?.Reported ?? "-"
            }
        ],
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Color Control`}>
                <span className="cursor-default">
                    <Link to={`/colorcontrol`}>
                        <MdIcons.MdInvertColors color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: ClusterTypes.ColorControl,
            title: 'ColorControl',
            path: '/colorcontrol',
            icon: <MdIcons.MdInvertColors />,
            cName: 'nav-text',
            subMenu: SideMenu.Actuators
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    IASZone: {
        ViewTable: [
            {
                Name: `Alarm1`,
                Value: (item: any) => item.Attributes?.ZoneStatus?.Reported?.Alarm1 !== undefined ?
                    (item.Attributes.ZoneStatus.Reported.Alarm1 === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            },
            {
                Name: `Alarm2`,
                Value: (item: any) => item.Attributes?.ZoneStatus?.Reported?.Alarm2 !== undefined ?
                    (item.Attributes.ZoneStatus.Reported.Alarm2 === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            },
            {
                Name: `Tamper`,
                Value: (item: any) => item.Attributes?.ZoneStatus?.Reported?.Tamper !== undefined ?
                    (item.Attributes.ZoneStatus.Reported.Tamper === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            },
            {
                Name: `BatteryLow`,
                Value: (item: any) => item.Attributes?.ZoneStatus?.Reported?.BatteryLow !== undefined ?
                    (item.Attributes.ZoneStatus.Reported.BatteryLow === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            }
        ],
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: IASZone`}>
                <span className="cursor-default">
                    <Link to={`/iaszone`}>
                        <MdIcons.MdPlace color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: ClusterTypes.IASZone,
            title: 'IASZone',
            path: '/iaszone',
            icon: <MdIcons.MdPlace />,
            cName: 'nav-text',
            subMenu: SideMenu.Sensors
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    Basic: {
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Basic`}>
                <span className="cursor-default">
                    <Link to={`/basic`}>
                        <FiIcons.FiCircle color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: ClusterTypes.Basic,
            title: 'Basic',
            path: '/basic',
            icon: <FiIcons.FiCircle />,
            cName: 'nav-text',
            subMenu: SideMenu.NodeManagement
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
            let percentage = attr?.BatteryPercentageRemaining?.Reported !== undefined ? attr?.BatteryPercentageRemaining?.Reported : "Unknown";
            let icon = <MdIcons.MdBatteryUnknown color="#212529" />;
            if (attr && attr.BatteryPercentageRemaining !== undefined && attr.BatteryPercentageRemaining.Reported !== undefined) {
                if (attr.BatteryAlarmMask && attr.BatteryAlarmMask.Reported && attr.BatteryAlarmMask.Reported.BatteryVoltageTooLow)
                    icon = <MdIcons.MdBatteryAlert color="#dc3545" />;
                else if (percentage < 20)
                    icon = <MdIcons.MdBattery20 color="#212529" />;
                else if (percentage < 50)
                    icon = <MdIcons.MdBattery50 color="#212529" />;
                else if (percentage < 80)
                    icon = <MdIcons.MdBattery80 color="#212529" />;
                else icon = <MdIcons.MdBatteryFull color="#212529" />;
            }
            return (
                <Tooltip title={`Endpoint ${endpoint}: battery ${percentage}%`}>
                    <span className="cursor-default">
                        <Link to={`/battery`}>{icon}</Link>
                    </span>
                </Tooltip>
            )
        },
        NavbarItem: {
            name: ClusterTypes.PowerConfiguration,
            title: 'Battery',
            path: '/battery',
            icon: <MdIcons.MdBatteryStd />,
            cName: 'nav-text',
            subMenu: SideMenu.NodeManagement
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    RFTelemetry: {
        NavbarItem: {
            name: 'RF Telemetry',
            title: 'RF Telemetry',
            path: '/rftelemetry',
            icon: <MdIcons.MdOutlineTransform />,
            cName: 'nav-text',
            subMenu: SideMenu.Debugging
        } as NavbarItem
    } as ClusterViewOverride,
    NameAndLocation: {
        NodesTooltip: (endpoint: string, attr: any) =>
            <Tooltip title={`Endpoint ${endpoint}: NameAndLocation`}>
                <span className="cursor-default">
                    <MdIcons.MdBookmarks color="#212529" />
                </span>
            </Tooltip>
    } as ClusterViewOverride,
    ConfigurationParameters: {
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Configuration Parameters`}>
                <span className="cursor-default">
                    <Link to={`/configurationparameters`}>
                        <MdIcons.MdOutlineSettingsApplications color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: "Configuration Parameters",
            title: 'Configuration Parameters',
            path: '/configurationparameters',
            icon: <MdIcons.MdOutlineSettingsApplications />,
            cName: 'nav-text',
            subMenu: SideMenu.NodesConfiguration
        } as NavbarItem
    } as ClusterViewOverride,
    ElectricalMeasurement: {
        ViewTable: [
            {
                Name: `Active Power`,
                Value: (item: any) => item.Attributes?.ActivePower?.Reported ?? "-"
            },
            {
                Name: `Reactive Power`,
                Value: (item: any) => item.Attributes?.ReactivePower?.Reported ?? "-"
            },
            {
                Name: `RMS Voltage`,
                Value: (item: any) => item.Attributes?.RMSVoltage?.Reported ?? "-"
            },
            {
                Name: `RMS Current`,
                Value: (item: any) => item.Attributes?.RMSCurrent?.Reported ?? "-"
            },
            {
                Name: `Power Factor`,
                Value: (item: any) => item.Attributes?.PowerFactor?.Reported ?? "-"
            },
        ],
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Electrical Measurement`}>
                <span className="cursor-default">
                    <Link to={`/electricalmeasurement`}>
                        <MdIcons.MdPowerInput color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: ClusterTypes.ElectricalMeasurement,
            title: 'Electrical Measurement',
            path: '/electricalmeasurement',
            icon: <MdIcons.MdPowerInput />,
            cName: 'nav-text',
            subMenu: SideMenu.Sensors
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    Scenes: {
        ViewTable: [
            {
                Name: `Scene Count`,
                Value: (item: any) => item.Attributes?.SceneCount?.Reported ?? "-"
            },
            {
                Name: `Current Scene`,
                Value: (item: any) => item.Attributes?.CurrentScene?.Reported ?? "-"
            },
            {
                Name: `Scene Valid`,
                Value: (item: any) => item.Attributes?.SceneValid?.Reported !== undefined ?
                    (item.Attributes.SceneValid.Reported === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            },
            {
                Name: `Current Group`,
                Value: (item: any) => item.Attributes?.CurrentGroup?.Reported ?? "-"
            },
            {
                Name: `Names Supported`,
                Value: (item: any) => item.Attributes?.NameSupport?.Reported?.SceneNamesSupported !== undefined ?
                    (item.Attributes.NameSupport.Reported.SceneNamesSupported === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            },
        ],
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Scenes`}>
                <span className="cursor-default">
                    <Link to={`/scenes`}>
                        <MdIcons.MdOutlineDeveloperBoard color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        CustomActions: (unid: string, ep: string) => {
            return <Tooltip title="View/Edit Scenes">
                <span className="margin-h-5 icon">
                    <Link to={`/scenes/${unid}/${ep}`}><FiIcons.FiEdit /></Link>
                </span>
            </Tooltip>
        },
        NavbarItem: {
            name: 'Scenes',
            title: 'Scenes',
            path: '/scenes',
            icon: <MdIcons.MdOutlineDeveloperBoard />,
            cName: 'nav-text',
            subMenu: SideMenu.NodesConfiguration
        } as NavbarItem
    } as ClusterViewOverride,
    Binding: {
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Binding`}>
                <span className="cursor-default">
                    <Link to={`/binding`}>
                        <AiIcons.AiOutlineLink color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
    } as ClusterViewOverride,
    Metering: {
        ViewTable: [
            {
                Name: `Type`,
                Value: (item: any) => item.Attributes?.MeteringDeviceType?.Reported !== undefined
                    ? <span>
                        <Tooltip title="Electrical Services"><span hidden={!item.Attributes.MeteringDeviceType.Reported.ElectricMetering}><MdIcons.MdElectricalServices color="#212529" /></span></Tooltip>
                        <Tooltip title="Gas Metering"><span hidden={!item.Attributes.MeteringDeviceType.Reported.GasMetering}><MdIcons.MdAir color="#212529" /></span></Tooltip>
                        <Tooltip title="Water Metering"><span hidden={!item.Attributes.MeteringDeviceType.Reported.WaterMetering}><IoIcons5.IoWater color="#212529" /></span></Tooltip>
                        <Tooltip title="Thermal Metering"><span hidden={!item.Attributes.MeteringDeviceType.Reported.ThermalMetering}><IoIcons.IoMdThermometer color="#212529" /></span></Tooltip>
                        <Tooltip title="Pressure Metering"><span hidden={!item.Attributes.MeteringDeviceType.Reported.PressureMetering}><MdIcons.MdCompress color="#212529" /></span></Tooltip>
                        <Tooltip title="Heat Metering"><span hidden={!item.Attributes.MeteringDeviceType.Reported.HeatMetering}><MdIcons.MdOutlineWbSunny color="#212529" /></span></Tooltip>
                        <Tooltip title="Cooling Metering"><span hidden={!item.Attributes.MeteringDeviceType.Reported.CoolingMetering}><IoIcons5.IoSnow color="#212529" /></span></Tooltip>
                    </span>
                    : "-"
            },
            {
                Name: `Unit of Measure`,
                Value: (item: any) => item.Attributes?.UnitofMeasure?.Reported !== undefined ? item.Attributes.UnitofMeasure.Reported : `-`
            },
            {
                Name: `Summation Delivered`,
                Value: (item: any) => item.Attributes?.CurrentSummationDelivered?.Reported !== undefined ? item.Attributes.CurrentSummationDelivered.Reported : `-`
            },
            {
                Name: `Summation Received`,
                Value: (item: any) => item.Attributes?.CurrentSummationReceived?.Reported !== undefined ? item.Attributes.CurrentSummationReceived.Reported : `-`
            }
        ],
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Metering`}>
                <span className="cursor-default">
                    <Link to={`/metering`}>
                        <IoIcons5.IoSpeedometerOutline color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: ClusterTypes.Metering,
            title: 'Metering',
            path: '/metering',
            icon: <IoIcons5.IoSpeedometerOutline />,
            cName: 'nav-text',
            subMenu: SideMenu.Sensors
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    BarrierControl: {
        ViewTable: [
            {
                Name: `Moving State`,
                Value: (item: any) => item.Attributes?.MovingState?.Reported !== undefined ?
                    <Tooltip title={item.Attributes.MovingState.Reported}><span className='icon'>
                        {(item.Attributes.MovingState.Reported === "Opening"
                            ? <MdIcons.MdLockOpen color="#dc3545" />
                            : (item.Attributes.MovingState.Reported === "Closing"
                                ? <MdIcons.MdLockOutline color="#28a745" />
                                : <AiIcons.AiOutlineStop color="orange" />))}
                    </span>
                    </Tooltip>
                    : "-"
            },
            {
                Name: `Safety Status`,
                Value: (item: any) => {
                    let status = item.Attributes?.SafetyStatus?.Reported;
                    let statusKeys = status && Object.keys(status);
                    return statusKeys ? <ul className='padding-l-15'> {statusKeys.map((i: string, index: number) => {
                        return <li key={index}>{i}: {(status[i] === true
                            ? <FiIcons.FiCheck color="#28a745" />
                            : <FiIcons.FiXCircle color="#6c757d" />)}</li>
                    })}
                    </ul> : "-"
                }
            },
            {
                Name: `Partial Barrier`,
                Value: (item: any) => item.Attributes?.Capabilities?.Reported && item.Attributes.Capabilities.Reported.PartialBarrier !== undefined ?
                    (item.Attributes.Capabilities.Reported.PartialBarrier === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            },
            {
                Name: `Position`,
                Value: (item: any) => item.Attributes?.BarrierPosition?.Reported ?? "-"
            }
        ],
        NodesTooltip: (endpoint: string) =>
            <Tooltip title={`Endpoint ${endpoint}: Barrier Control`}>
                <span className="cursor-default">
                    <Link to={`/barriercontrol`}>
                        <MdIcons.MdLockOutline color="#212529" />
                    </Link>
                </span>
            </Tooltip>,
        NavbarItem: {
            name: ClusterTypes.BarrierControl,
            title: 'Barrier Control',
            path: '/barriercontrol',
            icon: <MdIcons.MdLockOutline />,
            cName: 'nav-text',
            subMenu: SideMenu.Actuators
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    WindowCovering: {
        ViewTable: [
            {
                Name: `Type`,
                Value: (item: any) => item.Attributes?.WindowCoveringType?.Reported ?? "-"
            },
            {
                Name: `Current Lift`,
                Value: (item: any) => item.Attributes?.CurrentPositionLift?.Reported ?? "-"
            },
            {
                Name: `Current Tilt`,
                Value: (item: any) => item.Attributes?.CurrentPositionTilt?.Reported ?? "-"
            },
            {
                Name: `Operational`,
                Value: (item: any) => item.Attributes?.ConfigOrStatus?.Reported?.Operational !== undefined ?
                    (item.Attributes.ConfigOrStatus.Reported.Operational === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            },
            {
                Name: `Online`,
                Value: (item: any) => item.Attributes?.ConfigOrStatus?.Reported?.Online !== undefined ?
                    (item.Attributes.ConfigOrStatus.Reported.Online === true
                        ? <FiIcons.FiCheck color="#28a745" />
                        : <FiIcons.FiXCircle color="#6c757d" />)
                    : "-"
            }
        ],
        NodesTooltip: (endpoint: string, attr: any) => {
            return (
                <Tooltip title={`Endpoint ${endpoint}: Window Covering`}>
                    <span className="cursor-default">
                        <Link to={`/windowcovering`}>
                            <MdIcons.MdWindow color="#212529" />
                        </Link>
                    </span>
                </Tooltip>
            )
        },
        NavbarItem: {
            name: ClusterTypes.WindowCovering,
            title: 'Window Covering',
            path: '/windowcovering',
            icon: <MdIcons.MdWindow />,
            cName: 'nav-text',
            subMenu: SideMenu.Actuators
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
    Descriptor: {
        ViewTable: [
          {
            Name: `Device Type List`,
            Value: (item: any) => {
              let deviceTypeList = item.Attributes?.DeviceTypeList?.Reported;
              return deviceTypeList ? (
                <ul className='padding-l-5'>
                  {deviceTypeList.map((device: any, index: number) => (
                      <li className='list-unstyled padding-t-5'>{device.DeviceType}</li>
                  ))}
                </ul>
              ) : "-";
            }
          },
          {
            Name: `Revision`,
            Value: (item: any) => {
              let deviceTypeList = item.Attributes?.DeviceTypeList?.Reported;
              return deviceTypeList ? (
                <ul className='padding-l-15'>
                  {deviceTypeList.map((device: any, index: number) => (
                      <li className='padding-t-5 list-unstyled'>{device.Revision}</li>
                  ))}
                </ul>
              ) : "-";
            }
          }
        ],
        NodesTooltip: (endpoint: string, attr: any) => {
            return (
                <Tooltip title={`Endpoint ${endpoint}: Descriptor`}>
                    <span className="cursor-default">
                        <Link to={`/descriptor`}>
                            <IoIcons.IoMdInformationCircleOutline color="#212529" />
                        </Link>
                    </span>
                </Tooltip>
            )
        },
        NavbarItem: {
            name: ClusterTypes.Descriptor,
            title: 'Descriptor',
            path: '/descriptor',
            icon: <IoIcons.IoMdInformationCircleOutline />,
            cName: 'nav-text',
            subMenu: SideMenu.NodeManagement
        } as NavbarItem,
        IsExpandable: true
    } as ClusterViewOverride,
}