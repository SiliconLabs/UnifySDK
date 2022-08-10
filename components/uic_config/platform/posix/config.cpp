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
#include "config.h"
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <unistd.h>
#include <libgen.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include "yaml_parser.hpp"
namespace po = boost::program_options;

std::string mapper(std::string env_var)
{
  // ensure the env_var is all caps
  std::transform(env_var.begin(), env_var.end(), env_var.begin(), ::toupper);

  if (env_var == "UIC_CONF") {
    std::cout << "Enviornment variable UIC_CONF is found: \n";
    return "conf";
  }
  return "";
}

/**
 * @brief Config class supporting adding, removing and parsing configurations
 *
 * Configurations are parsed from both command line and config file.
 * If the same option is given in both command line and config file,
 * the command line option takes precendence
 *
 */
class Config
{
  private:
  // Local variables
  // Options that are supported both by command line and config files
  po::options_description general;
  po::variables_map vm;

  public:
  Config() :
    general("Following options can also be in a Config file. "
            "\n Options and values passed on command line here take "
            "precedence over the options and values in config file")
  {
    this->config_add(CONFIG_KEY_LOG_LEVEL,
                     "Log Level (d,i,w,e,c)",
                     std::string("i"));
    this->config_add(
      CONFIG_KEY_LOG_TAG_LEVEL,
      "Tag based log level\nFormat: <tag>:<severity>, <tag>:<severity>, ...",
      std::string(""));
  }

  template<typename T> config_status_t
    config_add(const char *name, const char *help, const T &default_value)
  {
    general.add_options()(name,
                          po::value<T>()->default_value(default_value),
                          help);
    return CONFIG_STATUS_OK;
  }

  config_status_t config_add(const char *name, const char *help)
  {
    general.add_options()(name, help);
    return CONFIG_STATUS_OK;
  }

  void add_mqtt_config_default(char **argv)
  {
    this->config_add(CONFIG_KEY_MQTT_HOST,
                     "MQTT broker hostname or IP",
                     std::string("localhost"));
    this->config_add(CONFIG_KEY_MQTT_PORT, "MQTT broker port", 1883);
    this->config_add(CONFIG_KEY_MQTT_CAFILE,
                     "Path to file containing the PEM encoded CA certificate "
                     "to connect to Mosquitto MQTT broker for TLS "
                     "encryption",
                     std::string(""));
    this->config_add(CONFIG_KEY_MQTT_CERTFILE,
                     "Path to file containing the PEM encoded client "
                     "certificate to connect to Mosquitto "
                     "MQTT broker for TLS "
                     "encryption",
                     std::string(""));
    this->config_add(CONFIG_KEY_MQTT_KEYFILE,
                     "Path to a file containing the PEM encoded unencrypted "
                     "private key for this client",
                     std::string(""));

    char mqtt_client_id[64];
    snprintf(mqtt_client_id,
             sizeof(mqtt_client_id),
             "%s_%i",
             basename(argv[0]),
             getpid());
    this->config_add(CONFIG_KEY_MQTT_CLIENT_ID,
                     "Set the MQTT client ID of the application.",
                     std::string(mqtt_client_id));
  }

  /**
   * @brief Dump a YAML key/value with indentation to an ostream
   *
   * This is a recursive function that handles printing key/values as YAML. This
   * serves as a helper function for \ref dump_config.
   *
   * @param out output stream
   * @param key_parts The config key split on every '.' character in reverse
   * order, e.g. "log.level" key shall be ["level", "log"]
   * @param last_key_parts Previous key (last time this function was called) in
   * reverse order, same as key_parts
   * @param value Value to store for the key.
   * @param indent Indentation level, when calling first time this is usualy 0
   */
  static void dump_yaml_key_value(std::ostream &out,
                                  std::vector<std::string> key_parts,
                                  std::vector<std::string> last_key_parts,
                                  boost::any value,
                                  size_t indent)
  {
    // If there is only one keypart, write it and print the value
    if (key_parts.size() == 1) {
      if (indent > 0) {  // print indentation if any
        out << std::setfill(' ') << std::setw(indent * 2) << ' ';
      }
      out << key_parts.at(0) << ": ";
      if (value.type() == typeid(int)) {
        out << boost::any_cast<int>(value);
      } else if (value.type() == typeid(std::string)) {
        out << "'" << boost::any_cast<std::string>(value) << "'";
      } else if (value.type() == typeid(float)) {
        out << boost::any_cast<float>(value);
      } else if (value.type() == typeid(bool)) {
        out << boost::any_cast<bool>(value);
      } else {
        assert(0);
      }
      out << std::endl;
    } else {  // There are multiple keyparts
      // Test if current keypart and last_keypart share a level
      if (last_key_parts.back() == key_parts.back()) {
        // In this case all that is done is popping the key_parts and increasing
        // indentation before calling the recursive function again.
        // Example:
        //   if last key was "log.level" the last_keyparts was ["level", "log"],
        //   and the new key is "log.tag_level" (i.e. ["tag_level", "log"]),
        //   then  this would pop "log" of both last_keyparts and key_parts and
        //   increase indentation.
        last_key_parts.pop_back();
      } else {
        // In this case the last_key_part doesn't match the key_part, in which
        // case we print the part
        if (indent > 0) {  // print indentation if any
          out << std::setfill(' ') << std::setw(indent * 2) << ' ';
        }
        out << key_parts.back() << ":" << std::endl;
      }
      key_parts.pop_back();
      indent++;
      dump_yaml_key_value(out, key_parts, last_key_parts, value, indent);
    }
  }
  /**
   * @brief Dump the current configuration in YAML format
   *
   * Example following internal config:
   * vm["zpc.serial"] = "/dev/ttyUSB0"
   * vm["zpc.poll.backoff"] = 42
   * vm["zpc.poll.default_interval"] = 500
   * vm["zpc.qos"] = 2
   * Output following YAML config:
   * zpc:
   *   poll:
   *     backoff: 42
   *     default_interval: 500
   *   qos: 2
   *   serial: "/dev/ttyUSB0"
   * @param out Stream to dump config to
   */
  void dump_config(std::ostream &out) const
  {
    std::vector<std::string> last_key_parts;
    out << "# Unify sample conf file" << std::endl << std::endl;
    YAML::Emitter emitter(out);
    YAML::Node yaml_config;

    for (const auto &[key, var]: vm) {
      if (var.empty() || (key == "dump-config")) {
        continue;
      }
      auto value = var.value();

      std::vector<std::string> key_parts;  // ["zpc", "poll", "backoff"]
      boost::split(key_parts, key, boost::is_any_of("."));
      std::reverse(key_parts.begin(), key_parts.end());
      dump_yaml_key_value(out, key_parts, last_key_parts, value, 0);
      last_key_parts = key_parts;
    }
  }

  config_status_t
    config_parse(int argc, char **argv, const char *version, std::ostream &out)
  {
    bool dump_requested = false;
    vm.clear();
    // Options that are only supported from command line
    po::options_description cmdline("Options");
    // clang-format off
    cmdline.add_options()
      ("conf",
        po::value<std::string>()->default_value(DEFAULT_CONFIG_PATH),
        "Config file in YAML format. UIC_CONF env variable can be set to override the default config file path")
      ("help", "Print this help message and quit")
      ("dump-config", "Dumps the current configuration on a YAML config file format that can be passed to --conf option")
      ("version", "Print version information and quit");

    // MQTT Default Config
    add_mqtt_config_default(argv);

    // clang-format on
    cmdline.add(general);

    store(
      po::parse_environment(cmdline,
                            boost::function1<std::string, std::string>(mapper)),
      vm);

    if (vm["conf"].as<std::string>().length() == 0) {
      std::cout << "Empty UIC_CONF env found. Consider unsetting "
                   "the env variable\n";
    }

    std::string exe_file(argv[0]);
    try {
      // Parse the command line arguments
      po::store(po::parse_command_line(argc, argv, cmdline), vm);
      if (vm.count("help")) {
        out << "\nUsage: " << argv[0] << " [Options]\n" << std::endl;
        out << cmdline << std::endl;
        // return CONFIG_STATUS_INFO_MESSAGE to not try to start Contiki
        return CONFIG_STATUS_INFO_MESSAGE;
      } else if (vm.count("version")) {
        out << "Version: " << version << std::endl;
        // return CONFIG_STATUS_INFO_MESSAGE to not try to start Contiki
        return CONFIG_STATUS_INFO_MESSAGE;
      } else if (vm.count("dump-config")) {
        dump_requested = true;
      } else if (vm.count("conf")) {
        std::cout << "Using confg file: " << vm["conf"].as<std::string>()
                  << std::endl;
        const std::string config_file(vm["conf"].as<std::string>());
        std::ifstream istrm(config_file);
        if (istrm) {
          po::store(yaml_parser::parse(istrm, general), vm);
        }
      }
      // Parse config file
    } catch (std::exception &e) {
      std::cerr << "error: " << e.what() << std::endl;
      return CONFIG_STATUS_ERROR;
    } catch (...) {
      std::cerr << "Exception of unknown type!" << std::endl;
      return CONFIG_STATUS_ERROR;
    }
    if (dump_requested) {
      dump_config(out);
      // return CONFIG_STATUS_INFO_MESSAGE to not try to start Contiki
      return CONFIG_STATUS_INFO_MESSAGE;
    }
    return CONFIG_STATUS_OK;
  }

  config_status_t has_flag(const char *name)
  {
    return vm.count(name) ? CONFIG_STATUS_OK : CONFIG_STATUS_ERROR;
  }

  template<typename T> config_status_t config_get(const char *name, T &result)
  {
    try {
      const boost::program_options::variable_value &vm_result = vm[name];
      if (vm_result.empty()) {
        return CONFIG_STATUS_DOES_NOT_EXIST;
      }
      result = vm_result.as<T>();
    } catch (boost::bad_any_cast &e) {
      return CONFIG_STATUS_INVALID_TYPE;
    }
    return CONFIG_STATUS_OK;
  }

  config_status_t config_get_as_string(const char *name, const char **result)
  {
    //TODO: Find out if this can use the template config_get function
    // It is an issue, because the result shall be a string, that points to the
    // char array within the vm_result, and it is not possible to copy a c++
    // std::string reference
    try {
      const boost::program_options::variable_value &vm_result = vm[name];
      if (vm_result.empty()) {
        return CONFIG_STATUS_DOES_NOT_EXIST;
      }
      *result = vm_result.as<std::string>().c_str();
    } catch (boost::bad_any_cast &e) {
      return CONFIG_STATUS_INVALID_TYPE;
    }
    return CONFIG_STATUS_OK;
    std::string str;
    auto ret = config_get(name, str);

    return ret;
  }
};

static Config *config_object;

class Config_singleton
{
  public:
  Config_singleton()
  {
    config_object = new Config();
  }
  ~Config_singleton()
  {
    delete config_object;
    config_object = nullptr;
  }
};
static Config_singleton config_singleton;

void config_reset()
{
  delete config_object;
  config_object = new Config();
}

config_status_t config_add_string(const char *name,
                                  const char *help,
                                  const char *default_value)
{
  return config_object->config_add(name, help, std::string(default_value));
}

config_status_t
  config_add_int(const char *name, const char *help, int default_value)
{
  return config_object->config_add(name, help, default_value);
}

config_status_t
  config_add_double(const char *name, const char *help, double default_value)
{
  return config_object->config_add(name, help, default_value);
}

config_status_t
  config_add_bool(const char *name, const char *help, bool default_value)
{
  return config_object->config_add(name, help, default_value);
}

config_status_t config_add_flag(const char *name, const char *help)
{
  return config_object->config_add(name, help);
}

config_status_t config_parse_with_output(int argc,
                                         char **argv,
                                         const char *version,
                                         std::ostream &out)
{
  return config_object->config_parse(argc, argv, version, out);
}

config_status_t config_parse(int argc, char **argv, const char *version)
{
  return config_parse_with_output(argc, argv, version, std::cout);
}

config_status_t config_get_as_string(const char *name, const char **result)
{
  return config_object->config_get_as_string(name, result);
}

config_status_t config_get_as_int(const char *name, int *result)
{
  return config_object->config_get(name, *result);
}

config_status_t config_get_as_double(const char *name, double *result)
{
  return config_object->config_get(name, *result);
}

config_status_t config_get_as_bool(const char *name, bool *result)
{
  return config_object->config_get(name, *result);
}

config_status_t config_has_flag(const char *name)
{
  return config_object->has_flag(name);
}
