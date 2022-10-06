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