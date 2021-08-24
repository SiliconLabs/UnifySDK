/***************************************************************************//**
 * @file
 * @brief Security code common to both the Trust Center and the normal node.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/security/af-security.h"

//------------------------------------------------------------------------------
bool emberAfClearLinkKeyTableUponFormingOrJoining = true;

const EmberAfSecurityProfileData emAfSecurityProfileData[] = {
  #include "security-profile-data.h"
};


void emAfClearLinkKeyTable(void)
{
#if EMBER_KEY_TABLE_SIZE
  if (emberAfClearLinkKeyTableUponFormingOrJoining) {
    emberClearKeyTable();
  }
  emberAfClearLinkKeyTableUponFormingOrJoining = true;
#endif
}

const EmberAfSecurityProfileData *emAfGetCurrentSecurityProfileData(void)
{
  uint8_t i;
  for (i = 0; i < COUNTOF(emAfSecurityProfileData); i++) {
    if (emAfProIsCurrentNetwork()
        && (emAfCurrentZigbeeProNetwork->securityProfile
            == emAfSecurityProfileData[i].securityProfile)) {
      return &emAfSecurityProfileData[i];
    }
  }
  return NULL;
}

bool emberAfIsCurrentSecurityProfileSmartEnergy(void)
{
#ifdef EMBER_AF_HAS_SECURITY_PROFILE_SE
  return (emAfProIsCurrentNetwork()
          && ((emAfCurrentZigbeeProNetwork->securityProfile
               == EMBER_AF_SECURITY_PROFILE_SE_TEST)
              || (emAfCurrentZigbeeProNetwork->securityProfile
                  == EMBER_AF_SECURITY_PROFILE_SE_FULL)));
#else
  return false;
#endif
}
