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
#include "sl_log.h"
#include <stdarg.h>
#include <iomanip>
#include <vector>
#include <unordered_map>

// A whole bunch of includes from boost for logging
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>

#include <boost/phoenix/bind.hpp>

// Boost includes for string searching (split and tokenizer)
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "config.h"
#include "sl_status.h"

// Boost logging namespaces
namespace logging  = boost::log;
namespace src      = boost::log::sources;
namespace attrs    = boost::log::attributes;
namespace expr     = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace phoenix  = boost::phoenix;

#define LOG_TAG "sl_log"

// Map storing tag specific log levels
static std::unordered_map<std::string, sl_log_level_t> log_levels;
// Global log level
static sl_log_level_t log_level;

static std::locale
  loc(std::wcout.getloc(),
      new boost::posix_time::wtime_facet(L"%Y-%m-%d %H:%M:%S:%f"));

// Declare boost log attributes
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", sl_log_level_t)
BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)

const std::vector<std::string> log_level_short = {"d", "i", "W", "E", "C"};
const std::vector<std::string> log_level_long
  = {"debug", "info", "Warning", "Error", "CRITICAL"};
const std::vector<std::string> log_level_color
  = {"\033[34;1m", "\033[32;1m", "\033[33;1m", "\033[31;1m", "\033[37;41;1m"};

// sl_log_level_t to string representation used by boost::log
std::ostream &operator<<(std::ostream &strm, sl_log_level_t level)
{
  strm << log_level_short.at(level);
  return strm;
}

//
bool sl_log_filter(
  logging::value_ref<sl_log_level_t, tag::severity> const &level,
  logging::value_ref<std::string, tag::tag_attr> const &tag)
{
  // If log level is set for the tag, check against the tag level
  auto tag_level = log_levels.find(tag.get());
  if (tag_level != log_levels.end()) {
    return level >= tag_level->second;
  }
  // else check against the global level
  return level >= log_level;
}

void std_formatter(logging::record_view const &rec,
                   logging::formatting_ostream &strm)
{
  strm.imbue(loc);
  strm << logging::extract<boost::posix_time::ptime>("TimeStamp", rec);
  strm << " <" << rec[severity] << "> ";
  strm << "[" << rec[tag_attr] << "]";
  strm << " " << rec[expr::smessage];
}

void color_formatter(logging::record_view const &rec,
                     logging::formatting_ostream &strm)
{
  strm.imbue(loc);
  strm << logging::extract<boost::posix_time::ptime>("TimeStamp", rec);
  strm << log_level_color.at((rec[severity].get()));
  strm << " <" << rec[severity] << "> ";
  strm << "[" << rec[tag_attr] << "]";
  strm << " " << rec[expr::smessage];
  strm << "\033[0m";
}

// Initialisation of boost logging, wrapped in class to do a static initialisation
class sl_log_singleton
{
  public:
  sl_log_singleton()
  {
    bool in_terminal = isatty(fileno(stderr));

    log_level = SL_LOG_DEBUG;
    logging::add_common_attributes();
    logging::register_simple_formatter_factory<sl_log_level_t, char>(
      "Severity");

    auto sink = logging::add_console_log();
    if (in_terminal) {
      sink->set_formatter(&color_formatter);
    } else {
      sink->set_formatter(&std_formatter);
    }
    sink->set_filter(
      phoenix::bind(&sl_log_filter, severity.or_none(), tag_attr.or_none()));
  }
};
static sl_log_singleton singletron;

void sl_log_set_level(sl_log_level_t level)
{
  log_level = level;
  sl_log_debug(LOG_TAG,
               "Setting log level to %s\n",
               log_level_long.at(level).c_str());
}

void sl_log_set_tag_level(const char *tag, sl_log_level_t level)
{
  log_levels[tag] = level;
  sl_log_debug(LOG_TAG,
               "Setting log level for '%s' to %s",
               tag,
               log_level_long.at(level).c_str());
}

void sl_log_unset_tag_level(const char *tag)
{
  log_levels.erase(tag);
  sl_log_debug(LOG_TAG, "Unsetting log level for '%s'\n", tag);
}

sl_status_t sl_log_level_from_string(const char *level, sl_log_level_t *result)
{
  std::string level_lower(level);
  boost::algorithm::to_lower(level_lower);
  if (level_lower == "d" || level_lower == "debug") {
    *result = SL_LOG_DEBUG;
  } else if (level_lower == "i" || level_lower == "info") {
    *result = SL_LOG_INFO;
  } else if (level_lower == "w" || level_lower == "warning") {
    *result = SL_LOG_WARNING;
  } else if (level_lower == "e" || level_lower == "error") {
    *result = SL_LOG_ERROR;
  } else if (level_lower == "c" || level_lower == "critical") {
    *result = SL_LOG_CRITICAL;
  } else {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

void sl_log_read_config()
{
  // Read log level from config
  const char *log_level_str;
  config_get_as_string(CONFIG_KEY_LOG_LEVEL, &log_level_str);
  sl_log_level_t log_level;
  if (CONFIG_STATUS_OK == sl_log_level_from_string(log_level_str, &log_level)) {
    sl_log_set_level(log_level);
  }
  // Read tag_levels from config in format <tag>:<level>, <tag>:<level>, ...
  const char *tag_level_str;
  if (CONFIG_STATUS_OK
      == config_get_as_string(CONFIG_KEY_LOG_TAG_LEVEL, &tag_level_str)) {
    boost::char_separator<char> sep_list(",");
    std::string token_string(tag_level_str);
    boost::tokenizer<boost::char_separator<char>> tokenizer_list(token_string,
                                                                 sep_list);
    for (auto &list_entry: tokenizer_list) {
      std::vector<std::string> v;
      boost::algorithm::split(v, list_entry, [](char c) { return c == ':'; });
      if (v.size() == 2) {
        // Remove trailing and leading whitespaces
        boost::algorithm::trim(v[0]);
        boost::algorithm::trim(v[1]);
        if (CONFIG_STATUS_OK
            == sl_log_level_from_string(v[1].c_str(), &log_level)) {
          sl_log_set_tag_level(v[0].c_str(), log_level);
        }
      }
    }
  }
}

void sl_log(const char *const tag,
            sl_log_level_t level,
            const char *fmtstr,
            ...)
{
  std::string mystr;
  // Init variadic argument list
  va_list myargs;
  va_start(myargs, fmtstr);
  // Find size of string to print by printing it to nullptr
  // TODO: This cause the string parsed twice, is it better to allocate
  // a fixed size buffer instead?
  size_t size
    = std::vsnprintf(nullptr, 0, fmtstr, myargs) + 1;  // Extra space for '\0'
  // re initialize the argument list
  va_end(myargs);
  va_start(myargs, fmtstr);
  mystr.resize(size - 1);
  // Print parsed strint to buffer
  std::vsnprintf(mystr.data(), size, fmtstr, myargs);

  // remove trailing newline (as boost adds this later)
  if (mystr.back() == '\n') {
    mystr.pop_back();
  }

  // Write to the log
  src::severity_logger<sl_log_level_t> lg;
  //lg.add_attribute("Tag", my_tag);
  lg.add_attribute("Tag", attrs::constant<std::string>(tag));

  BOOST_LOG_SEV(lg, level) << mystr;

  va_end(myargs);
}
