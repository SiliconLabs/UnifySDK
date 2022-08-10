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
#include "unify_node_state_monitor.hpp"
#include "contiki_test_helper.h"
#include "mqtt_test_helper.h"
#include "zap-types.h"
#include "workaround.hpp"
using namespace unify::node_state_monitor;

extern "C" {
#include "unity.h"

/**
 * @brief Helper class for testing node_state_monitor
 *
 * This class implements the virtual "callback" functions defined in \ref
 * unify::node_state_monitor::node_state_monitor_interface. It has methods for
 * registering check functions for when each of the callbacks have been called.
 * The check function is pushed to a stack, whenever on_node_added is called, a
 * check function is popped from the stack and called. When the class is
 * destroyed it is checked if there all the check function stacks are empty, and
 * fails if they are not.
 *
 * In addition there are counters for how many times each of the functions from
 * \ref unify::node_state_monitor::node_state_monitor_interface is called.
 *
 */
class node_state_monitor_test_interface : public node_state_monitor_interface
{
  private:
  // Stacks used for check functions
  std::queue<std::function<void(const node &node)>> _on_unify_node_added_check;
  std::queue<std::function<void(const std::string &unid)>>
    _on_unify_node_removed_check;
  std::queue<std::function<void(const node &node)>>
    _on_unify_node_state_changed_check;

  public:
  /// Counter for number of calls to \ref on_unify_node_added
  size_t on_unify_node_added_calls = 0;
  /// Counter for number of calls to \ref on_unify_node_removed
  size_t on_unify_node_removed_calls = 0;
  /// Counter for number of calls to \ref on_unify_node_state_changed
  size_t on_unify_node_state_changed_calls = 0;

  virtual void on_unify_node_added(const node &node)
  {
    if (!_on_unify_node_added_check.empty()) {
      _on_unify_node_added_check.front()(node);
      _on_unify_node_added_check.pop();
    }
    on_unify_node_added_calls++;
  };

  virtual void on_unify_node_removed(const std::string &unid)
  {
    if (!_on_unify_node_removed_check.empty()) {
      _on_unify_node_removed_check.front()(unid);
      _on_unify_node_removed_check.pop();
    }
    on_unify_node_removed_calls++;
  };

  virtual void on_unify_node_state_changed(const node &node)
  {
    if (!_on_unify_node_state_changed_check.empty()) {
      _on_unify_node_state_changed_check.front()(node);
      _on_unify_node_state_changed_check.pop();
    }
    on_unify_node_state_changed_calls++;
  };

  /**
    * @brief Append a check function for when on_node_added is called.
    *
    * @param check
    */
  void append_on_node_added_check(std::function<void(const node &node)> check)
  {
    _on_unify_node_added_check.push(check);
  };
  /**
   * @brief Append a check function for when on_node_removed is called.
   *
   * @param check
   */
  void append_on_unify_node_removed_check(
    std::function<void(const std::string &unid)> check)
  {
    _on_unify_node_removed_check.push(check);
  };
  /**
   * @brief Append a check function for when on_node_node_state_changed is
   * called.
   *
   * @param check
   */
  void append_on_unify_node_state_changed_check(
    std::function<void(const node &node)> check)
  {
    _on_unify_node_state_changed_check.push(check);
  };

  void check_no_pending_checks()
  {
    // Check that all stacks of check functions are empty, otherwise they have
    // been called fewer times than expected.
    TEST_ASSERT_TRUE_MESSAGE(
      _on_unify_node_added_check.empty(),
      "on_unify_node_added() was called fewer times than expected");
    TEST_ASSERT_TRUE_MESSAGE(
      _on_unify_node_removed_check.empty(),
      "on_unify_node_removed() was called fewer times than expected");
    TEST_ASSERT_TRUE_MESSAGE(
      _on_unify_node_state_changed_check.empty(),
      "on_unify_node_added() was called fewer times than expected");
  }
  node_state_monitor_test_interface() {};
  ~node_state_monitor_test_interface()
  {
    check_no_pending_checks();
  };
};
void suiteSetUp()
{
  // Because the node_state_monitor registers a timer on creation, and it is a
  // singleton, only init contiki_test_helper_init once.
  contiki_test_helper_init();
  contiki_test_helper_run(0);
  mqtt_test_helper_init();
}

// Called before each and every test
void setUp() {}

void tearDown() {}

void test_node_added_and_removed()
{
  node_state_monitor_test_interface interface;
  node_state_monitor::get_instance().set_interface(&interface);

  // Online Interviewing
  interface.append_on_unify_node_state_changed_check([](const node &node) {
    TEST_ASSERT_EQUAL(ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING,
                      node.state);
    TEST_ASSERT_EQUAL(ZCL_NODE_STATE_SECURITY_NONE, node.security_info);
    TEST_ASSERT_EQUAL_STRING("ZW-1234", node.unid.c_str());
  });

  mqtt_test_helper_publish_str("ucl/by-unid/ZW-1234/State",
                               R"({
  "NetworkStatus": "Online interviewing",
  "Security": "None",
  "MaximumCommandDelay": 0
  })");
  contiki_test_helper_run(1001);
  interface.check_no_pending_checks();

  // Online Functional
  interface.append_on_unify_node_state_changed_check([](const node &node) {
    TEST_ASSERT_EQUAL(ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL,
                      node.state);
    TEST_ASSERT_EQUAL(ZCL_NODE_STATE_SECURITY_Z_WAVE_S2_ACCESS_CONTROL,
                      node.security_info);
  });
  interface.append_on_node_added_check([](const node &node) {
    TEST_ASSERT_EQUAL(ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL,
                      node.state);
  });
  mqtt_test_helper_publish_str("ucl/by-unid/ZW-1234/State",
                               R"({
  "NetworkStatus": "Online functional",
  "Security": "Z-Wave S2 Access Control",
  "MaximumCommandDelay": 0
  })");
  contiki_test_helper_run(1001);
  interface.check_no_pending_checks();

  // Remove Node
  interface.append_on_unify_node_removed_check([](const std::string &unid) {
    TEST_ASSERT_EQUAL_STRING("ZW-1234", unid.c_str());
  });
  mqtt_test_helper_publish("ucl/by-unid/ZW-1234/State", "", 0);
  interface.check_no_pending_checks();

  TEST_ASSERT_TRUE_MESSAGE(
    node_state_monitor::get_instance().get_nodes().empty(),
    "Expect nodes to be empty after removing the only node");
}

void test_node_interview()
{
  node_state_monitor_test_interface interface;
  node_state_monitor::get_instance().set_interface(&interface);
  // Add some ep0 clusters
  // Basic cluster
  mqtt_test_helper_publish_str("ucl/by-unid/ZW-42/ep0/Basic/SupportedCommands",
                               R"({"value": []})");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep0/Basic/SupportedGeneratedCommands",
    R"({"value": []})");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep0/Basic/Attributes/ClusterRevision/Reported",
    R"({"value": 1})");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep0/Basic/Attributes/PowerSource/Reported",
    R"({"value": "Battery"})");
  // ConfigurationParameters Cluster
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep0/ConfigurationParameters/SupportedCommands",
    R"({"value": ["DiscoverParameter", "SetParameter"]})");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep0/ConfigurationParameters/SupportedGeneratedCommands",
    R"({"value": []})");
  mqtt_test_helper_publish_str("ucl/by-unid/ZW-42/ep0/ConfigurationParameters/"
                               "Attributes/ClusterRevision/Reported",
                               R"({"value": 2})");
  // Scenes Cluster
  mqtt_test_helper_publish_str("ucl/by-unid/ZW-42/ep0/Scenes/SupportedCommands",
                               R"({"value": []})");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep0/Scenes/SupportedGeneratedCommands",
    R"({"value": ["RecallScene"]})");
  mqtt_test_helper_publish_str("ucl/by-unid/ZW-42/ep0/Scenes/"
                               "Attributes/ClusterRevision/Reported",
                               R"({"value": 2})");

  // Add some ep1 Clusters
  // NameAndLocation cluster
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep1/NameAndLocation/SupportedCommands",
    R"({"value": ["WriteAttributes"]})");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep1/NameAndLocation/SupportedGeneratedCommands",
    R"({"value": []})");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep1/NameAndLocation/Attributes/Location/Reported",
    R"({"value": ""})");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep1/NameAndLocation/Attributes/Name/Reported",
    R"({"value": ""})");
  contiki_test_helper_run(1001);

  mqtt_test_helper_publish_str("ucl/by-unid/ZW-42/State",
                               R"({
  "NetworkStatus": "Online functional",
  "Security": "None",
  "MaximumCommandDelay": 0
  })");

  interface.append_on_node_added_check([](const node &node) {
    TEST_ASSERT_NOT_EMPTY(node.to_string().c_str());
    TEST_ASSERT_EQUAL_STRING("ZW-42", node.unid.c_str());
    TEST_ASSERT_EQUAL(2, node.endpoints.size());
    // ep0
    {
      auto &ep = node.endpoints.at(0);
      {
        TEST_ASSERT_EQUAL(0, ep.endpoint_id);
        TEST_ASSERT_EQUAL(3, ep.clusters.size());
        // Basic Cluster
        auto &cluster = ep.clusters.at("Basic");
        TEST_ASSERT_EQUAL(0, cluster.supported_commands.size());
        TEST_ASSERT_EQUAL(0, cluster.generated_commands.size());
        auto &attributes = cluster.attributes;
        TEST_ASSERT_EQUAL(2, attributes.size());
        TEST_ASSERT_TRUE(attributes.find("ClusterRevision")
                         != attributes.end());
        TEST_ASSERT_TRUE(attributes.find("PowerSource") != attributes.end());
      }
      // ConfigurationParameters Cluster
      {
        auto &cluster            = ep.clusters.at("ConfigurationParameters");
        auto &supported_commands = cluster.supported_commands;
        TEST_ASSERT_EQUAL(2, supported_commands.size());
        TEST_ASSERT_TRUE(supported_commands.find("DiscoverParameter")
                         != supported_commands.end());
        TEST_ASSERT_TRUE(supported_commands.find("SetParameter")
                         != supported_commands.end());

        TEST_ASSERT_EQUAL(0, cluster.generated_commands.size());
        auto &attributes = cluster.attributes;
        TEST_ASSERT_EQUAL(1, attributes.size());
        TEST_ASSERT_TRUE(attributes.find("ClusterRevision")
                         != attributes.end());
      }
      // Scenes Cluster
      {
        auto &cluster            = ep.clusters.at("Scenes");
        auto &generated_commands = cluster.generated_commands;
        TEST_ASSERT_EQUAL(0, cluster.supported_commands.size());
        TEST_ASSERT_EQUAL(1, generated_commands.size());
        TEST_ASSERT_TRUE(generated_commands.find("RecallScene")
                         != generated_commands.end());
        auto &attributes = cluster.attributes;
        TEST_ASSERT_EQUAL(1, attributes.size());
        TEST_ASSERT_TRUE(attributes.find("ClusterRevision")
                         != attributes.end());
      }
    }
    // ep1
    {
      auto &ep = node.endpoints.at(1);
      {
        TEST_ASSERT_EQUAL(1, ep.endpoint_id);
        TEST_ASSERT_EQUAL(1, ep.clusters.size());
        // Basic Cluster
        auto &cluster            = ep.clusters.at("NameAndLocation");
        auto &supported_commands = cluster.supported_commands;
        TEST_ASSERT_EQUAL(1, supported_commands.size());
        TEST_ASSERT_TRUE(supported_commands.find("WriteAttributes")
                         != supported_commands.end());
        TEST_ASSERT_EQUAL(0, cluster.generated_commands.size());
        auto &attributes = cluster.attributes;
        TEST_ASSERT_EQUAL(2, attributes.size());
        TEST_ASSERT_TRUE(attributes.find("Name") != attributes.end());
        TEST_ASSERT_TRUE(attributes.find("Location") != attributes.end());
      }
    }

    TEST_ASSERT_EQUAL(1, node.endpoints.at(1).endpoint_id);
  });

  // Set another time threshold
  node_state_monitor::get_instance().set_node_state_functional_threshold(500);
  // Set another timeout
  node_state_monitor::get_instance().set_node_state_functional_timeout_time(
    100);
  // Let time pass to trigger timeout.
  contiki_test_helper_run(501);
  interface.check_no_pending_checks();

  // Remove SupportedCommands, SupportedGeneratedCommands and Attributes for NameAndLocation at ep1
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep1/NameAndLocation/SupportedCommands",
    "");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep1/NameAndLocation/SupportedGeneratedCommands",
    "");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep1/NameAndLocation/Attributes/Location/Reported",
    "");
  mqtt_test_helper_publish_str(
    "ucl/by-unid/ZW-42/ep1/NameAndLocation/Attributes/Name/Reported",
    "");

  TEST_ASSERT_TRUE(node_state_monitor::get_instance()
                     .get_nodes()
                     .at("ZW-42")
                     .endpoints.at(1)
                     .clusters.at("NameAndLocation")
                     .supported_commands.empty());
  TEST_ASSERT_TRUE(node_state_monitor::get_instance()
                     .get_nodes()
                     .at("ZW-42")
                     .endpoints.at(1)
                     .clusters.at("NameAndLocation")
                     .generated_commands.empty());
  TEST_ASSERT_TRUE(node_state_monitor::get_instance()
                     .get_nodes()
                     .at("ZW-42")
                     .endpoints.at(1)
                     .clusters.at("NameAndLocation")
                     .attributes.empty());
}
}
