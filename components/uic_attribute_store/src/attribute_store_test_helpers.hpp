
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
#ifndef ATTRIBUTE_STORE_TEST_HELPERS_H
#define ATTRIBUTE_STORE_TEST_HELPERS_H

#include <string>
#include <iostream>
#include <vector>
#include <regex>

#include "attribute.hpp"
/**
 * @brief Attribute Store Test Helpers
 * 
 * 
 * The purpose of the attribute store test helpers - to simplify attributes creation and validation
 * in the attribute store
 *
 * For simplicity the attribute mapper test helpers evaluates parent nodes value as uint8. 
 *
 * A branch descriptor look like:
 *
 * "ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_MULTILEVEL_SCALE" or
 * "0x3102[1].0x3105" or
 * "0x3102[0x01].0x3105"
 * 
 *
 * Tree creation example:
 * create_branch("DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE").set_reported<uint32_t>(100);
 * 
 * create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_MULTILEVEL_SCALE").set_reported<uint32_t>(0);
 * create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_MULTILEVEL_PRECISION").set_reported<uint32_t>(2);
 * create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_VALUE").set_reported<uint32_t>(10);
 *
 * create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_MULTILEVEL_SCALE").set_reported<uint32_t>(1);
 * create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_MULTILEVEL_PRECISION").set_reported<uint32_t>(2);
 * create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_VALUE").set_reported<uint32_t>(10);
 * 
 *
 * It will create attribute tree with topology (in uml):
 *
  @startuml
 
  ' Style for the diagram
  skinparam classFontColor black
  skinparam classFontSize 10
  skinparam classFontName Helvetica
  skinparam shadowing false
  skinparam ArrowColor #000000
 
  skinparam ObjectBackgroundColor #DEDEDE
  skinparam ObjectBorderColor #480509
  skinparam ObjectBorderThickness 2
  skinparam ObjectFontColor #000000
 
  skinparam NoteBackgroundColor #FFFFFF
  skinparam NoteBorderColor #000000
 
  title Attribute Creation Example
 
  object "Root" as root #e6fff7
  root : ID = ROOT
 
  object "Attribute" as attribute_1 #FFFFFF
  attribute_1 : ID = DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE
  attribute_1 : value = Desired: [], Reported: [100]
 
  object "Attribute" as attribute_2_1 #FFFFFF
  attribute_2_1 : ID = ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE
  attribute_2_1 : value = Desired: [], Reported: [01]
 
  object "Attribute" as attribute_2_2 #FFFFFF
  attribute_2_2 : ID = ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE
  attribute_2_2 : value = Desired: [], Reported: [02]
 
  object "Attribute" as attribute_2_1_1 #FFFFFF
  attribute_2_1_1 : ID = ATTRIBUTE_SENSOR_MULTILEVEL_SCALE
  attribute_2_1_1 : value = Desired: [], Reported: [00]
 
  object "Attribute" as attribute_2_1_2 #FFFFFF
  attribute_2_1_2 : ID = ATTRIBUTE_SENSOR_MULTILEVEL_PRECISION
  attribute_2_1_2 : value = Desired: [], Reported: [02]
 
  object "Attribute" as attribute_2_1_3 #FFFFFF
  attribute_2_1_3 : ID =   ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_VALUE
  attribute_2_1_3 : value = Desired: [], Reported: [10]
 
  object "Attribute" as attribute_2_2_1 #FFFFFF
  attribute_2_2_1 : ID =   ATTRIBUTE_SENSOR_MULTILEVEL_SCALE
  attribute_2_2_1 : value = Desired: [], Reported: [01]
 
  object "Attribute" as attribute_2_2_2 #FFFFFF
  attribute_2_2_2 : ID =   ATTRIBUTE_SENSOR_MULTILEVEL_PRECISION
  attribute_2_2_2 : value = Desired: [], Reported: [02]
 
  object "Attribute" as attribute_2_2_3 #FFFFFF
  attribute_2_2_3 : ID =   ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_VALUE
  attribute_2_2_3 : value = Desired: [], Reported: [10]
  
  root *-- attribute_1
  root *-- attribute_2_1
  root *-- attribute_2_2
 
  attribute_2_1 *-- attribute_2_1_1
  attribute_2_1 *-- attribute_2_1_2
  attribute_2_1 *-- attribute_2_1_3
 
  attribute_2_2 *-- attribute_2_2_1
  attribute_2_2 *-- attribute_2_2_2
  attribute_2_2 *-- attribute_2_2_3
 
  @enduml
 *
 */

using namespace attribute_store;

/**
 * @brief Parse branch descriptor and create branch starting from branch root node.
 * 
 * @param branch_descriptor String to describe attribute branch.
 * @param branch_root Root node for branch (optional param, attribute::root() by default).
 * @return node_id of last attribute in branch if parse branch_descriptor was successfull.
 * @return ATTRIBUTE_STORE_INVALID_NODE if parse did not succeed.
 */
attribute create_branch(std::string branch_descriptor, attribute branch_root = 0);

/**
 * @brief Parse branch descriptor and check branch starting from branch root node.
 * 
 * @param branch_descriptor String to describe attribute branch.
 * @param branch_root Root node for branch (optional param, attribute::root() by default).
 * @return node_id of last attribute in branch if parse branch_descriptor was successfull.
 * @return ATTRIBUTE_STORE_INVALID_NODE if parse did not succeed or nodes in branch not exists.
 */
attribute check_branch(std::string branch_descriptor, attribute branch_root = 0);

#endif //ATTRIBUTE_MAPPER_TEST_HELPERS_H
