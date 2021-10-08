import React from 'react';
import * as CgIcons from 'react-icons/cg';
import { ClusterViewOverrides } from '../../pages/base-clusters/cluster-view-overrides';
import { ClusterTypes } from '../../cluster-types/cluster-types';
import { NavbarItem } from './navbar-types';


export const NavbarItems: NavbarItem[] = [
  {
    name: 'Nodes',
    title: 'Nodes',
    path: '/nodes',
    icon: <CgIcons.CgEditUnmask />,
    cName: 'nav-text'
  },
  {
    name: 'Groups',
    title: 'Groups',
    path: '/groups',
    icon: <CgIcons.CgDisplayGrid />,
    cName: 'nav-text'
  },
  {
    name: 'SmartStart',
    title: 'SmartStart',
    path: '/smartstart',
    icon: <CgIcons.CgSmartphoneShake />,
    cName: 'nav-text'
  },
  {
    name: 'OTA',
    title: 'OTA',
    path: '/ota',
    icon: <CgIcons.CgSoftwareUpload />,
    cName: 'nav-text'
  },
  ...Object.keys(ClusterTypes).map((item) => (ClusterViewOverrides as any)[item]?.NavbarItem || {
    name: item,
    title: item,
    path: `/${item}`,
    icon: <CgIcons.CgEditBlackPoint />,
    cName: 'nav-text'
  })
];
