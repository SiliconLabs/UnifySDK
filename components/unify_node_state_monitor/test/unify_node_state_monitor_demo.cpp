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

#include <sstream>
#include "unify_node_state_monitor.hpp"

#include "uic_main.h"
#include "process.h"
#include "uic_mqtt.h"
#include "sl_status.h"
#include "sl_log.h"

using namespace unify::node_state_monitor;

static constexpr const char LOG_TAG[] = "DEMO";
class my_node_state_interface : public node_state_monitor_interface
{
  public:
  virtual void on_unify_node_added(const node &node)
  {
    std::stringstream ss;
    ss << "on_unify_node_added UNID: " << node.to_string() << std::endl;
    sl_log_warning(LOG_TAG, ss.str().c_str());
  };

  virtual void on_unify_node_removed(const std::string &unid)
  {
    std::stringstream ss;
    ss << "on_unify_node_removed UNID: " << unid;
    sl_log_warning(LOG_TAG, ss.str().c_str());
  };

  virtual void on_unify_node_state_changed(const node &node)
  {
    std::stringstream ss;
    ss << "on_unify_node_state_changed UNID: " << node.unid
       << " State: " << node.state << std::endl;
    sl_log_warning(LOG_TAG, ss.str().c_str());
  };
};

sl_status_t setup_fixt()
{
  static my_node_state_interface interface;
  node_state_monitor::get_instance().set_interface(&interface);
  sl_log_warning(LOG_TAG, "Hello World");
  return SL_STATUS_OK;
}

int main(int argc, char *argv[])
{
  // Setup fixtures
  uic_fixt_setup_step_t uic_fixt_setup_steps_list[]
    = {{&setup_fixt, "demo setup fixture"}, {NULL, "Terminator"}};
  uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[]
    = {{NULL, "Terminator"}};

  return uic_main(uic_fixt_setup_steps_list,
                  uic_fixt_shutdown_steps_list,
                  argc,
                  argv,
                  "1.1.1");
}
