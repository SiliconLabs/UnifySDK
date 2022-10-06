/***************************************************************************//**
 * @file zigbee_af_token_headers.h
 * @brief ZCL callback dispatcher declarations.
 *******************************************************************************
 * # License
<<<<<<< HEAD:applications/zigbeed/project_files/autogen/hello_gmock.cpp
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
=======
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
>>>>>>> origin/release/22q2_FC:applications/zigpc/components/zigpc_gateway/libs/zigbee_host/gen/autogen/zigbee_af_token_headers.h
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
<<<<<<< HEAD:applications/zigbeed/project_files/autogen/hello_gmock.cpp
 *****************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class Foo
{
  public:
  virtual ~Foo() {};
  void bar();
};

class FooMock : public Foo
{
  public:
  MOCK_METHOD(void, bar, ());
};

TEST(SquareRootTest, PositiveNos)
{
  FooMock foo_mock;
  EXPECT_CALL(foo_mock, bar()).Times(1);
  foo_mock.bar();
}

TEST(SquareRootTest, ZeroAndNegativeNos)
{
  EXPECT_TRUE(true);
}
=======
 ******************************************************************************/
/*
 * This file includes token headers for the components selected in the project
 */
//
// *** Generated file. Do not edit! ***
//

#include "app/framework/plugin/reporting/reporting-tokens.h"

>>>>>>> origin/release/22q2_FC:applications/zigpc/components/zigpc_gateway/libs/zigbee_host/gen/autogen/zigbee_af_token_headers.h
