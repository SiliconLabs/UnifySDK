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
#include "config.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "yaml_parser.hpp"
namespace po = boost::program_options;

// Default Settings
#define DEFAULT_CONFIG_PATH    "/etc/uic/uic.cfg"
#define DEFAULT_DATASTORE_FILE "/var/lib/uic/database.db"

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
                     "private key for this client", std::string(""));
   this->config_add(CONFIG_KEY_LOG_LEVEL,
                     "Log Level (d,i,w,e,c)",
                     std::string("d"));
    this->config_add(
      CONFIG_KEY_LOG_TAG_LEVEL,
      "Tag based log level\nFormat: <tag>:<severity>, <tag>:<severity>, ...",
      std::string(""));
    this->config_add(CONFIG_KEY_DATASTORE_FILE,
                     "Datastore database file",
                     std::string(DEFAULT_DATASTORE_FILE));
  }

  template<typename T> config_status_t
    config_add(const char *name, const char *help, const T &default_value)
  {
    general.add_options()(name,
                          po::value<T>()->default_value(default_value),
                          help);
    return CONFIG_STATUS_OK;
  }

  /**
   * @brief Dump the current configuration in YAML format
   * 
   * @param out Stream to dump config to
   */
  void dump_config(std::ostream &out) const
  {
    std::string last_prefix;

    out << "# UIC sample conf file\n" << std::endl;
    for (const auto &[key, var]: vm) {
      if (var.empty() || (key == "dump-config")) {
        continue;
      }
      auto value = var.value();

      size_t last_dot
        = key.find_last_of('.');  //Location of the last dot in the string
      if (last_dot != std::string::npos) {
        std::string prefix = key.substr(0, last_dot);
        if (prefix != last_prefix) {
          out << key.substr(0, last_dot) << ": " << std::endl;
        }
        last_prefix = prefix;
        out << "  " << key.substr(last_dot + 1, key.size()) << ": ";
      } else {
        out << key << ": ";
      }

      if (value.type() == typeid(int)) {
        out << boost::any_cast<int>(value);
      } else if (value.type() == typeid(std::string)) {
        out << "'" << boost::any_cast<std::string>(value) << "'";
      } else if (value.type() == typeid(float)) {
        out << boost::any_cast<float>(value);
      } else if (value.type() == typeid(bool)) {
        out << boost::any_cast<bool>(value);
      } else {
        //Implement me
        assert(0);
      }
      out << std::endl;
    }
  }

  config_status_t config_parse(int argc, char **argv, const char *version)
  {
    bool dump_requested = false;
    vm.clear();
    // Options that are only supported from command line
    po::options_description cmdline("Options");
    // clang-format off
    cmdline.add_options()
      ("conf",
        po::value<std::string>()->default_value(DEFAULT_CONFIG_PATH),
        "Config file in YAML format")
      ("help", "Print this help message and quit")
      ("dump-config", "Dumps the current configuration on a YAML config file format that can be passed to --conf option")
      ("version", "Print version information and quit");
    // clang-format on
    cmdline.add(general);
    std::string exe_file(argv[0]);
    try {
      // Parse the command line arguments
      po::store(po::parse_command_line(argc, argv, cmdline), vm);
      if (vm.count("help")) {
        std::cout << "\nUsage: " << argv[0] << " [Options]\n" << std::endl;
        std::cout << cmdline << std::endl;
        // When we print help, we don't parse the other options,
        // thus we return NOK
        return CONFIG_STATUS_NOK;
      } else if (vm.count("version")) {
        std::cout << "Version: " << version << std::endl;
        // Same as with "help", we want to return NOK after printing version.
        return CONFIG_STATUS_NOK;
      } else if (vm.count("dump-config")) {
        dump_requested = true;
      } else if (vm.count("conf")) {
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
      dump_config(std::cout);
      return CONFIG_STATUS_NOK;
    }
    return CONFIG_STATUS_OK;
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

config_status_t config_parse(int argc, char **argv, const char *version)
{
  return config_object->config_parse(argc, argv, version);
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
