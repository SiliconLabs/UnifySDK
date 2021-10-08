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

/**
 * @file attribute_mapper_engine.hpp
 * @brief Mapper engine which defines relations between attributes in the atttribute store.
 *
 *
 * @{
 */

#ifndef ATTRIBUTE_MAPPER_ENGINE_H
#define ATTRIBUTE_MAPPER_ENGINE_H

#include "attribute.hpp"
#include "attribute_mapper_ast.hpp"
#include "attribute_mapper_ast_dep_eval.hpp"
#include <map>
#include <memory>
/**
 * @brief Mapper Engine
 *
 * The purpose of the mapping engine define relations between attributes in the
 * attribute store, in general the relation between the Z-Wave attribute and the
 * Zigbee attributes. The actions performed by the mapping engine is quite
 * similar to the actions performed by a compiler. A compiler parses a text
 * document and builds an Abstract Syntax Tree(AST). The AST is then optimized
 * and the AST nodes are translated into machine op codes. In the attribute
 * mapper we are not translating AST nodes to opcodes but instead we are keeping
 * the AST in memory.

* The mapping engine only interacts with the attribute store. An example of a
* simple mapping is the relation between the zigbee attribute OnOf.value and the
* Z-Wave attribute binary_switch.value, if desired of OnOf.value is update the
* binary_switch.value should be update. The the same applies to the reported
* value in the reverse direction. So in a meta description we could describe this
* behaviour with the following:
*
* r'OnOff_Value = r'BinarySwitch_Value
* d'BinarySwitch_Value = d'OnOff_Value
*
* This means when the reported value of BinarySwitch_Value changes we would like to
* update the repported value of OnOff_Value. Actually we will also create the OnOff_Value
* if it does not exists (this will only happen with reported values).
* For the desired value we update BinarySwitch_Value if OnOff_Value changes.
*
* For simplicity the attribute mapper evaluates every thing as int32. This mean that
* All attributes the mapper deals with must be int32.
*
* The AST of the rule set is stored in memory because when an attribute changes, we
* should locate dependencies using the AST. The value of each AST node can be
* calculated in the context of an attribute node. Let's look at the example where
* an attribute is updated and the node type of that attribute is a leaf nodes in
* the AST. Then we need to try to re-calculate the value of all AST nodes that
* depends on the updated node. The update may lead to a write of another
* attribute. In the example above an update leads to an update of the OnOff value.
* There could be cases where some node in the AST cannot be evaluated in a given
* context, for example if one attribute depends on two other attributes but only
* one of them is defined. Then the AST will not be fully evaluated in the given
* context (the node does not support all the required command classes). In this
* case the update will not lead to an attribute write.
*
* In some cases there could be multiple ways to calculate a mapping. Let's take a
* fictive example of some ZigBee LED color command class. On the Zigbee
* side colors are defined as RGBW on the Z-Wave side the color may be in RGBW or
* may be in just RGB.
*
* A mapping could look like this
*
* zbColorW = zwColor[W] or ((zwColor[R] + zwColor[G] + zwColor[B])*1000 ) / 3333
*
* Here we would persue the relation using zwColor[W] first, if that is not
* possible we would fall back to ((zwColor[R] + zwColor[G] + zwColor[B])*1000) /3333
*
* The mapping engine uses a number of C++ classes to perform its tasks.
*
* - The ast::UAMGrammar class defnies how we translate a text document into our
* ast using the boost spirit framework.
* - The ast::dep_evel class is used to build a graph of relations between attributes.
* - The ast::eval class is used to evaluate expressions when a change in a attribute value occurs
*
* @startuml
* title Relationships - Class Diagram
*
* title Mapping engine
*
* class ast::MapperEngine
* class ast::UAMGrammer
* class ast::eval
* class ast::dep_eval
* class attribute_store
*
* ast::MapperEngine <|-down- ast::UAMGrammer : AST
* ast::MapperEngine <|-down- ast::eval : optional value
* ast::MapperEngine <|-down- ast::dep_eval : dependencies
* ast::MapperEngine <|-up-   attribute_store:  on_attribute_updated()
* @enduml
*/
class MapperEngine
{
  public:
  /**
   * @brief Reset the mapper engine and clear all releations
   *
   */
  void reset();

  /**
   * @brief Load engine with all uam file in a given directory
   *
   * @param uam_path
   * @return true If load succeed
   * @return false Some error occurred in parsing of AST validation
   */
  bool load_path(const std::string &uam_path);

  /**
   * @brief Load engine with single uam file
   *
   * @param uam_file_name
   * @return true
   * @return false
   */
  bool load_file(std::string uam_file_name);

  /**
   * @brief Add uam expression to the ast

   * @param expression Expression given as string
   * @return true If load succeed
   * @return false Some error occurred in parsing of AST validation
   */
  bool add_expression(const std::string expression);

  /**
   * @brief Re-calculate dependencies of a the ast,
   *
   * this should be called if the AST is updated.
   *
   * @return true If load succeed
   * @return false Some error occurred in parsing of AST validation
   */
  bool update_dependencies(const ast::ast_tree &ast);

  /**
   * @brief Set the endpoint type which should be used as start context for all
   * attribute evaluations
   *
   * @param t Type to use as start context
   */
  void set_ep_type(attribute_store_type_t t)
  {
    ep_type = t;
  }

  /**
   * @brief Get the singletron
   *
   * @return The singletron
   */
  static MapperEngine &get_instance();

  private:
  /**
   * @brief Get the destination for attribute
   *
   * Try to resolve the attribute node from the AST attribute path.
   *
   * @param endpoint          endpoint to start from
   * @param attribute        Attribute (path) to resolve
   * @param create_if_missing If true the destination attribute is created if it does not exists
   * @return destination attribute, if no destination is found we return INVALID_ATTRIBUTE
   */
  attribute_store::attribute
    get_destination_for_attribute(const attribute_store::attribute &endpoint,
                                  const ast::attribute &attribute,
                                  bool create_if_missing) const;

  /**
   * @brief Called when an attribute is updated.
   *
   * This function will do a look up the updated attribute in the relation
   * table and evaluate the expressios which depends on the attribute.
   *
   * @param original_node   Node that has been updated
   * @param state           What updated desired or reported
   * @param change          Type of update, create, delete, value change
   */
  void on_attribute_updated(attribute_store::attribute const &original_node,
                            attribute_store_node_value_state_t state,
                            attribute_store_change_t change);

  /**
   * @brief Adapter function for the attribute store
   */
  static void on_attribute_updated_reported(attribute_store_node_t n,
                                            attribute_store_change_t change);

  /**
   * @brief Adapter function for the attribute store
   */
  static void on_attribute_updated_desired(attribute_store_node_t n,
                                           attribute_store_change_t change);

  //Lookup table for which assignments depend on which attributes
  std::multimap<ast::attribute_dependency_t, std::shared_ptr<ast::assignment>>
    relations;
  attribute_store_type_t ep_type;
};

#endif  //ATTRIBUTE_MAPPER_ENGINE_H

/**
 * @}
 */
