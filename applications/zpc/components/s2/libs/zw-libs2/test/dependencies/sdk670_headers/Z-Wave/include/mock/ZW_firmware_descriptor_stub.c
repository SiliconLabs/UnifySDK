/*
 * ZW_firmware_descriptor_stub.c
 *
 *  Created on: Nov 24 2015
 *      Author: trasmussen
 */

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include "ZW_firmware_descriptor_stub.h"
#include <ZW_firmware_descriptor.h>
#include <string.h>


/* Firmware descriptor for OTA firmware update */
t_firmwareDescriptor firmwareDescriptor;

void firmwareDescriptor_set(t_firmwareDescriptor * p_firmwareDescriptor)
{
  memcpy(&firmwareDescriptor, p_firmwareDescriptor, sizeof(t_firmwareDescriptor));
}
