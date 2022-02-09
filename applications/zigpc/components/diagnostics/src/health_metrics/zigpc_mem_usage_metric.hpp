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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>

#include "zigpc_diagnostics_metric.hpp"

namespace zigpc_diagnostics::util
{
static constexpr char LOG_TAG[] = "zigpc_metric_ram_usage";

static constexpr char PATH_PROC_MEMINFO[] = "/proc/meminfo";
static constexpr char PATH_PROC_STATM[]   = "/proc/self/statm";

/**
   * @brief Get system page size in KiloBytes.
   *
   * @return long int system page size in KB, or -1 on error.
   */
static long int get_pagesize_kb(void)
{
  long int page_size = sysconf(_SC_PAGESIZE);
  if (page_size == -1) {
    sl_log_error(LOG_TAG, "Failed to get pagesize");
    return page_size;
  }

  page_size /= 1024UL;

  return page_size;
}

/**
   * @brief Get system total memory usage in KiloBytes.
   *
   * @return unsigned long  System total used memory in KB, 0 otherwise.
   */
static unsigned long get_mem_total(void)
{
  unsigned long mem_total_kb = 0UL;

  std::ifstream meminfo_f(PATH_PROC_MEMINFO, std::ifstream::in);
  if (!meminfo_f.is_open()) {
    sl_log_error(LOG_TAG, "Failed to read %s", PATH_PROC_MEMINFO);
    return mem_total_kb;
  }

  std::string meminfo_field;
  meminfo_f >> meminfo_field;
  if (meminfo_field != "MemTotal:") {
    sl_log_error(LOG_TAG, "Failed to find MemTotal from %s", PATH_PROC_MEMINFO);
    return mem_total_kb;
  }

  meminfo_f >> mem_total_kb;
  if (meminfo_f.fail()) {
    sl_log_error(LOG_TAG, "Failed to read MemTotal from %s", PATH_PROC_MEMINFO);
    return mem_total_kb;
  }

  return mem_total_kb;
}

/**
   * @brief Get the resident memory pages used by program. This number is in
   * KiloBytes.
   *
   * @return unsigned long The resulting resident set retrieved, or 0 on error.
   */
static unsigned long get_resident_set_kb(void)
{
  unsigned long rss_kb = 0UL;

  std::ifstream statm_file(PATH_PROC_STATM, std::ifstream::in);
  if (!statm_file.is_open()) {
    sl_log_error(LOG_TAG, "Failed to read %s", PATH_PROC_STATM);
    return rss_kb;
  }

  unsigned long stat_m_field;
  statm_file >> stat_m_field;  // total_program_size: Ignore
  statm_file >> stat_m_field;  // resident_set_size: Used

  if (statm_file.fail()) {
    sl_log_error(LOG_TAG,
                 "Failed to read resident set from %s",
                 PATH_PROC_STATM);
    return rss_kb;
  }

  rss_kb = stat_m_field;

  return rss_kb;
}

}  // namespace zigpc_diagnostics::util

class zigpc_mem_usage_metric : public zigpc_diagnostics_metric
{
  private:
  static constexpr char LOG_TAG[] = "zigpc_metric_ram_usage";

  long int page_size_kb      = zigpc_diagnostics::util::get_pagesize_kb();
  unsigned long total_mem_kb = zigpc_diagnostics::util::get_mem_total();
  float mem_usage            = 0.f;

  public:
  zigpc_mem_usage_metric(zigpc_diagnostics_notification &notif,
                         std::string const &metric_id) :
    zigpc_diagnostics_metric(notif, metric_id)
  {}

  /**
   * @brief Get the memory usage.
   *
   * @return float  memory use by process.
   */
  float get_value(void) const
  {
    return mem_usage;
  }

  /**
   * @brief Update the memory usage by process.
   *
   */
  void update_value(void) final
  {
    if ((page_size_kb == -1) || (total_mem_kb == 0)) {
      sl_log_error(LOG_TAG, "Failed to get valid page_size and total memory");
      return;
    }

    unsigned long resident_set_kb
      = zigpc_diagnostics::util::get_resident_set_kb();
    if (resident_set_kb == 0UL) {
      sl_log_error(LOG_TAG, "Failed to get valid resident_set");
      return;
    }

    // RAM Usage representative of what's seen from htop
    mem_usage
      = (static_cast<float>(resident_set_kb * page_size_kb) / total_mem_kb)
        * 100.f;

    // Format value representation
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << mem_usage;
    serialized_value = std::move(ss.str());

    // Update Manager on value update complete
    on_update_complete.notify(metric_id);
  }
};
