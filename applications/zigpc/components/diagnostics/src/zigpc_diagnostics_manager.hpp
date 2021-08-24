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

#ifndef ZIGPC_DIAGNOSTIC_MANAGER_H
#define ZIGPC_DIAGNOSTIC_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "zigpc_diagnostics_metric.hpp"
#include "zigpc_diagnostics_notification.hpp"

class zigpc_diagnostics_manager : public virtual zigpc_diagnostics_notification
{
  private:
  /* data */
  std::map<std::string, std::shared_ptr<zigpc_diagnostics_metric>> metric_map;
  zigpc_diagnostics_manager(/* args */);
  //unid of the device
  std::string unid;

  protected:
  

  public:
  void notify(std::string metric_id);

  void publish_metric_list();

  std::string get_unid()
  {
    return unid;
  }

  void set_unid(std::string unid)
  {
    this->unid = unid;
  }
  ~zigpc_diagnostics_manager();

  zigpc_diagnostics_manager(zigpc_diagnostics_manager &other) = delete;
  void operator=(const zigpc_diagnostics_manager &) = delete;

  static zigpc_diagnostics_manager& get_instance();

  /**
   * @brief Accept list of Metric IDs to trigger update.
   * 
   * @param metric_id_vector 
   */
  void handle_metricID_request(std::vector<std::string> metric_id_vector);

  /**
   * @brief add a metric to the diagnostic manager internal map
   *
   * @param metric_identifier metric_id identifier of the metric
   * @param metric the metric object to add
   */
  void add_metric(std::shared_ptr<zigpc_diagnostics_metric> metric)
  {
    metric_map[metric->get_metric_id()] = metric;
  };
};

#endif /* ZIGPC_DIAGNOSTIC_MANAGER_H */
