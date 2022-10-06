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
#ifndef MATTER_HEADER
#define MATTER_HEADER
// This is a generic include headers which includes all matter headers. order
// matters for the first include headers!
#include <system/SystemBuildConfig.h>
#include <ble/BleLayer.h>

#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <app/ConcreteAttributePath.h>
#include <app/CommandHandlerInterface.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <app/util/af.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include "app/util/attribute-metadata.h"
#include <app/util/attribute-table.h>
#include <app/util/util.h>
#include <app/util/basic-types.h>
#include <app/EventLogging.h>
#include <app/chip-zcl-zpro-codec.h>
#include <app/reporting/reporting.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>
#include <app/util/af-types.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <zap-generated/endpoint_config.h>
//#include "LinuxCommissionableDataProvider.h"
#include <credentials/DeviceAttestationCredsProvider.h>
#include <platform/CommissionableDataProvider.h>
//#include <platform/LinuxCommissionableDataProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
//#include <lib/support/RandUtils.h>
#include <lib/support/ZclString.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <AppMain.h>
#endif