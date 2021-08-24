/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file yaml_parser.hpp
 * @brief YAML Parser for Boost Program Options Config file
 *
 * Parse YAML configuration file for UIC.
 * Each configuration name is prepended with its parent, e.g. for following YAML
 * mqtt:
 *     host: 127.0.0.1
 *     port: 1883
 * zpc:
 *     serial_port: /dev/ttyUSB0
 * will parse to following configurations: "mqtt.host", "mqtt.port",
 * and "zpc.serial_port".
 *
 *
 * @{
 */

#ifndef BOOST_PROGRAM_OPTIONS_YAML_PARSER_HPP
#define BOOST_PROGRAM_OPTIONS_YAML_PARSER_HPP

#include <algorithm>
#include <set>
#include <string>
#include <stdexcept>

#include <yaml-cpp/yaml.h>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>

class yaml_parser
{
  public:
  /**
   * @brief Parse YAML configuration from istream
   *
   * @tparam char
   * @param istrm Istream containing YAML configuration
   * @param desc Program options description, only options described here
   *             will be parsed from the YAML configuraion, all other options
   *             will be ignored from the file
   * @return boost::program_options::basic_parsed_options<char_T> Parsed options
   */
  template<typename char_T = char>
  static boost::program_options::basic_parsed_options<char_T>
    parse(std::istream &istrm,
          const boost::program_options::options_description &desc)
  {
    YAML::Node config = YAML::Load(istrm);
    return parse(config, desc);
  }

  /**
   * @brief Pasre YAML configuration from YAML::Node
   *
   * @tparam char
   * @param node YAML::Node containing YAML configuration
   * @param desc Program options description, only options described here
   *             will be parsed from the YAML configuraion, all other options
   *             will be ignored from the file
   * @return boost::program_options::basic_parsed_options<char_T> Parsed options
   */
  template<typename char_T = char>
  static boost::program_options::basic_parsed_options<char_T>
    parse(const YAML::Node &node,
          const boost::program_options::options_description &desc)
  {
    boost::program_options::basic_parsed_options<char_T> result(&desc);
    parse_subnode(node, "", result);
    return result;
  }

  protected:
  template<typename char_T> void static parse_subnode(
    const YAML::Node &node,
    const std::string &key,
    boost::program_options::basic_parsed_options<char_T> &result)
  {
    switch (node.Type()) {
      case YAML::NodeType::Scalar:
        add_option(key, node.as<std::basic_string<char_T>>(), result);
        break;
      case YAML::NodeType::Sequence:
        parse_subnode_sequence(node, key, result);
        break;
      case YAML::NodeType::Map:
        parse_subnode_map(node, key, result);
        break;
      default:
        throw std::invalid_argument("Unsupported node type");
        break;
    }
  }

  template<typename char_T> static void parse_subnode_sequence(
    const YAML::Node &node,
    const std::string &key,
    boost::program_options::basic_parsed_options<char_T> &result)
  {
    for (const auto &subnode: node) {
      parse_subnode(subnode, key, result);
    }
  }

  template<typename char_T> static void parse_subnode_map(
    const YAML::Node &node,
    const std::string &key,
    boost::program_options::basic_parsed_options<char_T> &result)
  {
    for (const auto &pair: node) {
      std::string node_key = pair.first.as<std::string>();
      try {
        std::stoul(node_key);  // check if we have an integer
        node_key = key;        // treat this as if it were a Sequence
      } catch (std::invalid_argument &e) {
        // not an integer
        if (!key.empty()) {
          node_key = key + '.' + node_key;
        }
      }
      parse_subnode(pair.second, node_key, result);
    }
  }

  template<typename char_T> static void
    add_option(const std::string &key,
               const std::basic_string<char_T> &value,
               boost::program_options::basic_parsed_options<char_T> &result)
  {
    if (key.empty()) {
      throw std::logic_error("Empty key - malformed Config file");
    }
    if (result.description->find_nothrow(key, false) == NULL) {
      // If key is not defined in description, we ignore/skip it.
      // Since the configuration file may be shared among multiple UIC modules,
      // it is likely there are configuration options not for us.
      return;
    }
    auto option_iter = std::find_if(
      result.options.begin(),
      result.options.end(),
      [&key](const boost::program_options::basic_option<char_T> &test) {
        return test.string_key == key;
      });

    if (option_iter == result.options.end()) {
      result.options.emplace_back();
      option_iter             = result.options.end() - 1;
      option_iter->string_key = key;
      option_iter->unregistered
        = (result.description->find_nothrow(key, false) == NULL);
    }

    option_iter->value.push_back(value);
  }
};

#endif  //BOOST_PROGRAM_OPTIONS_YAML_PARSER_HPP
/** @} end boost_program_options_yaml_parser */
