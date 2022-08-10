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

// Shared Unify includes
#include <sl_log.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_config.h>

// Component includes
#include "zigpc_gateway_request_queue.hpp"
#include "zigpc_gateway_int.h"

/**
 * @brief Setup NCP EZSP policies based on configuration passed in.
 *
 */
static inline void zigpc_gateway_ncp_set_ezsp_policies(void)
{
  const zigpc_config_t *conf = zigpc_get_config();
  if ((conf != nullptr) && (conf->tc_use_well_known_key == false)) {
    EzspStatus status = zigbeeHostSetEzspPolicy(
      EZSP_TRUST_CENTER_POLICY,
      (EZSP_DECISION_ALLOW_JOINS | EZSP_DECISION_JOINS_USE_INSTALL_CODE_KEY),
      "Trust Center Policy",
      "Joins using install code only");

    sl_log_info(LOG_TAG,
                "Status for setting EZSP policy(allow joins using install code "
                "only): 0x%X",
                status);
  }
}

void zigpc_gateway_on_ncp_pre_reset(EzspStatus resetStatus)
{
  sl_log_warning(LOG_TAG,
                 "NCP Resetting due to EZSP error detected: 0x%X",
                 resetStatus);
  zigpc::gateway::RequestQueue::getInstance().stopDispatching();
}
void zigpc_gateway_on_ncp_post_reset(bool ncpMemConfigureStage)
{
  if (ncpMemConfigureStage == false) {
    zigpc_gateway_ncp_set_ezsp_policies();

    zigpc::gateway::RequestQueue::getInstance().startDispatching();
  }
}
