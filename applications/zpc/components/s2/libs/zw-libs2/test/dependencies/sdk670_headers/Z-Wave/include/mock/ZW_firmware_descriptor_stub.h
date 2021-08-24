/*
 * ZW_firmware_descriptor_stub.c
 *
 *  Created on: Nov 24 2015
 *      Author: trasmussen
 */

/**@file This file contains a single control function so that the firmware descriptor data can 
 *       easily be set according to the needs of the test.
 */
#ifndef _FIRMWARE_DESCRIPTOR_STUB_H_
#define _FIRMWARE_DESCRIPTOR_STUB_H_

#include <ZW_typedefs.h>
#include "ZW_firmware_descriptor.h"

void firmwareDescriptor_set(t_firmwareDescriptor * firmwareDescriptor);

#endif /* _FIRMWARE_DESCRIPTOR_STUB_H_ */
