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
#include "unity.h"
#include "zpc_ncp_update_fixt.h"
#include "zpc_config_mock.h"
#include "zwapi_init_mock.h"
#include "zwapi_protocol_basis_mock.h"
#include "uic_gbl_interface_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

zpc_config_t my_cfg = {
  .serial_port = "MySerial",
  .ncp_version = false,
  /// If not zero length we should flash the NCP firmware and exit
  .ncp_update_filename = "",

};

const zpc_config_t *config_get(int n)
{
  return &my_cfg;
}

/// Called before each and every test
void setUp()
{
  zpc_get_config_Stub(config_get);
}

void test_zp_ncp_update_do_nothing_test()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zpc_ncp_update_fixt_setup());
}

void test_zp_ncp_update_get_version_test()
{
  zwapi_chip_data_t chip_data = {
    .chip_type     = 1,  ///< Represents the chip hardware.
    .chip_revision = 2,  ///< Represents the chip hardware revision
    .version_major = 3,
    .version_minor = 4,
  };

  my_cfg.ncp_version = true;

  zwapi_init_ExpectAndReturn(my_cfg.serial_port, 0, 0, SL_STATUS_OK);
  zwapi_init_IgnoreArg_serial_fd();

  zwapi_get_chip_data_Expect(0);
  zwapi_get_chip_data_IgnoreArg_user_chip_data();
  zwapi_get_chip_data_ReturnThruPtr_user_chip_data(&chip_data);

  zwapi_get_protocol_version_ExpectAndReturn(0, SL_STATUS_FAIL);
  zwapi_get_protocol_version_IgnoreArg_protocol_version();

  zwapi_destroy_Expect();

  TEST_ASSERT_EQUAL(SL_STATUS_ABORT, zpc_ncp_update_fixt_setup());
}

void test_zp_ncp_update_update_test()
{
  int serial_fd               = 42;
  zwapi_chip_data_t chip_data = {
    .chip_type     = 8,  ///< Represents the chip hardware.
    .chip_revision = 2,  ///< Represents the chip hardware revision
    .version_major = 8,
    .version_minor = 52,
  };

  my_cfg.ncp_version         = false;
  my_cfg.ncp_update_filename = "myfile.gbl";

  zwapi_init_ExpectAndReturn(my_cfg.serial_port, 0, 0, SL_STATUS_OK);
  zwapi_init_ReturnThruPtr_serial_fd(&serial_fd);
  zwapi_init_IgnoreArg_serial_fd();

  zwapi_get_chip_data_Expect(0);
  zwapi_get_chip_data_IgnoreArg_user_chip_data();
  zwapi_get_chip_data_ReturnThruPtr_user_chip_data(&chip_data);

  zwapi_get_protocol_version_ExpectAndReturn(0, SL_STATUS_OK);
  zwapi_get_protocol_version_IgnoreArg_protocol_version();

  zwapi_enable_auto_program_mode_ExpectAndReturn(SL_STATUS_OK);

  uic_gbl_interface_transfer_image_ExpectAndReturn(my_cfg.ncp_update_filename,
                                                   serial_fd,
                                                   0,
                                                   SL_STATUS_OK);
  uic_gbl_interface_transfer_image_IgnoreArg_progress_callback();
  zwapi_destroy_Expect();

  TEST_ASSERT_EQUAL(SL_STATUS_ABORT, zpc_ncp_update_fixt_setup());
}
