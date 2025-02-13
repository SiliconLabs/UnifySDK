import React from 'react';
import * as CgIcons from 'react-icons/cg';
import * as TbIcons from 'react-icons/tb';
import * as MdIcons from 'react-icons/md';
import * as IoIcons from 'react-icons/io';
import * as AiIcons from 'react-icons/ai';
import { ClusterViewOverrides } from '../../pages/base-clusters/cluster-view-overrides';
import { ClusterTypes } from '../../cluster-types/cluster-types';
import { NavbarItem, SideMenu } from './navbar-types';

export const NavbarItems: NavbarItem[] = [
  {
    name: 'Nodes',
    title: 'Nodes',
    path: '/nodes',
    icon: <CgIcons.CgEditUnmask />,
    cName: 'nav-text',
    subMenu: SideMenu.NetworkManagement
  },
  {
    name: 'Network Management',
    title: 'Network Management',
    path: '/networkmanagement',
    icon: <MdIcons.MdHistoryEdu />,
    cName: 'nav-text',
    subMenu: SideMenu.NetworkManagement
  },
  {
    name: 'SmartStart',
    title: 'SmartStart',
    path: '/smartstart',
    icon: <CgIcons.CgSmartphoneShake />,
    cName: 'nav-text',
    subMenu: SideMenu.NetworkManagement
  },
  {
    name: 'Commissionable Devices',
    title: 'Commissionable Devices',
    path: '/commissionabledevices',
    icon: <TbIcons.TbQrcode />,
    cName: 'nav-text',
    subMenu: SideMenu.NetworkManagement
  },
  {
    name: 'Application Monitoring',
    title: 'Application Monitoring',
    path: '/app-monitoring',
    icon: <TbIcons.TbHeartRateMonitor />,
    cName: 'nav-text',
    subMenu: SideMenu.Debugging
  },
  {
    name: 'RF Telemetry',
    title: 'RF Telemetry',
    path: '/rftelemetry',
    icon: <MdIcons.MdOutlineTransform />,
    cName: 'nav-text',
    subMenu: SideMenu.Debugging
  },
  {
    name: 'System Metrics',
    title: 'System Metrics',
    path: '/systemmetrics',
    icon: <MdIcons.MdQueryStats />,
    cName: 'nav-text',
    subMenu: SideMenu.Debugging
  },
  {
    name: 'UPTI',
    title: 'UPTI',
    path: '/upti',
    icon: <CgIcons.CgDebug />,
    cName: 'nav-text',
    subMenu: SideMenu.Debugging
  },
  {
    name: 'Groups',
    title: 'Groups',
    path: '/groups',
    icon: <CgIcons.CgDisplayGrid />,
    cName: 'nav-text',
    subMenu: SideMenu.NodesConfiguration
  },
  {
    name: 'Configuration Parameters',
    title: 'Configuration Parameters',
    path: '/configurationparameters',
    icon: <MdIcons.MdOutlineSettingsApplications />,
    cName: 'nav-text',
    subMenu: SideMenu.NodesConfiguration
  },
  {
    name: 'Binding',
    title: 'Binding',
    path: '/binding',
    icon: <AiIcons.AiOutlineLink />,
    cName: 'nav-text',
    subMenu: SideMenu.NodesConfiguration
  },
  {
    name: 'OTA',
    title: 'OTA',
    path: '/ota',
    icon: <CgIcons.CgSoftwareUpload />,
    cName: 'nav-text',
    subMenu: SideMenu.NodeManagement
  },
  {
    name: 'Measurements',
    title: 'Measurements',
    path: '/measurements',
    icon: <MdIcons.MdAutoGraph />,
    cName: 'nav-text',
    subMenu: SideMenu.Sensors
  },
  ...Object.keys(ClusterTypes).map((item) => (ClusterViewOverrides as any)[item]?.NavbarItem || {
    name: item,
    title: item,
    path: `/${item}`,
    icon: <CgIcons.CgEditBlackPoint />,
    cName: 'nav-text'
  })
];
