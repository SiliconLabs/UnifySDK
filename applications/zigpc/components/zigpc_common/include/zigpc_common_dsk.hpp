/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <string>

#include "sl_status.h"
#include "zigpc_common_zigbee.h"

namespace zigpc_common
{
    /**
    * @brief Content of DSKs that have been parsed for install code based joins.
    *
    */
    typedef struct {
        std::string dsk;
        uint8_t install_code_length;
        zigbee_eui64_t eui64;
        zigbee_install_code_t install_code;
    } dsk_install_code_t;

    bool operator==(
            const dsk_install_code_t &lhs, 
            const dsk_install_code_t &rhs);

    sl_status_t deserialize_dsk_install_code(
                    const std::string &dsk, 
                    dsk_install_code_t &dsk_content);
}
