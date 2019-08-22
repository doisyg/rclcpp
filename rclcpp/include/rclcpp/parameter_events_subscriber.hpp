// Copyright (c) 2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RCLCPP__PARAMETER_EVENTS_SUBSCRIBER_HPP_
#define RCLCPP__PARAMETER_EVENTS_SUBSCRIBER_HPP_

#include <string>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/parameter_events_filter.hpp"

namespace rclcpp
{

class ParameterEventsSubscriber
{
public:
  ParameterEventsSubscriber(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging,
    const rclcpp::QoS & qos =
    rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_parameter_events)));

  template <typename NodeT>
  ParameterEventsSubscriber(
    NodeT node,
    const rclcpp::QoS & qos =
    rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(rmw_qos_profile_parameter_events)))
  : ParameterEventsSubscriber(
      node->get_node_base_interface(),
      node->get_node_topics_interface(),
      node->get_node_logging_interface(),
      qos)
  {}

  /// Sets a custom callback for parameter events
  /**
   * If no namespace is provided, a subscription will be created for the current namespace
   * Repeated calls to this function will overwrite the callback
   * If more than one namespace already has a subscription to its parameter events topic, then the
   * provided callback will be applied to all of them
   * 
   * \param[in] callback Function callback to be evaluated on event
   * \param[in] Name of namespace for which a subscription will be created
   */
  void set_event_callback(
    std::function<void(const rcl_interfaces::msg::ParameterEvent::SharedPtr &)> callback,
    const std::string & node_namespace = "");

  /// adds a custom callback for a specified parameter
  /**
   * If a node_name is not provided, defaults to the current node
   * 
   * \param[in] parameter_name Name of parameter
   * \param[in] callback Function callback to be evaluated upon parameter event
   * \param[in] node_name Name of node which hosts the parameter
   */
  void register_param_callback(
    const std::string & parameter_name,
    std::function<void()> callback,
    const std::string & node_name = "");

  /// adds a callback to assign the value of a changed parameter to a reference variable
  /**
   * If a node_name is not provided, defaults to the current node
   * 
   * \param[in] parameter_name Name of parameter
   * \param[in] value Reference to variable receiving update
   * \param[in] node_name Name of node which hosts the parameter
   */
  template<typename ParameterT>
  void register_param_update(
    const std::string & parameter_name, ParameterT & value, const std::string & node_name = "")
  {
    auto callback =
      [parameter_name, &value, this]() {
        get_param_update<ParameterT>(parameter_name, value);
      };

    register_param_callback(parameter_name, callback, node_name);
  }

  /// Gets value of specified parameter from an event
  /**
   * If the parameter does not appear in the event, no value will be assigned
   * \param[in] parameter_name Name of parameter
   * \param[in] value Reference to variable receiving updates
   */
  template<typename ParameterT>
  void get_param_update(const std::string & parameter_name, ParameterT & value)
  {
    rclcpp::ParameterEventsFilter filter(last_event_, {parameter_name},
      {rclcpp::ParameterEventsFilter::EventType::NEW,
        rclcpp::ParameterEventsFilter::EventType::CHANGED});
    if (!filter.get_events().empty()) {
      RCLCPP_DEBUG(node_logging_->get_logger(), "Updating parameter: %s", parameter_name.c_str());
      auto param_msg = filter.get_events()[0].second;
      auto param = rclcpp::Parameter::from_parameter_msg(*param_msg);
      value = param.get_value<ParameterT>();
    }
  }

protected:
  /// Adds a subscription (if unique) to a namespace parameter events topic
  void add_namespace_event_subscriber(const std::string & node_namespace);

  /// Callback for parameter events subscriptions
  void event_callback(const rcl_interfaces::msg::ParameterEvent::SharedPtr event);

  // Utility functions for string and path name operations
  std::string resolve_path(const std::string & path);
  std::pair<std::string, std::string> split_path(const std::string & str);
  std::string join_path(std::string path, std::string name);

  // Node Interfaces used for logging and creating subscribers
  rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_;
  rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics_;
  rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging_;

  rclcpp::QoS qos_;

  // Map containers for registered parameters
  std::map<std::string, std::string> parameter_node_map_;
  std::map<std::string, std::function<void()>> parameter_callbacks_;

  // Vector of unique namespaces added
  std::vector<std::string> node_namespaces_;

  // vector of event subscriptions for each namespace
  std::vector<rclcpp::Subscription
    <rcl_interfaces::msg::ParameterEvent>::SharedPtr> event_subscriptions_;

  std::function<void(const rcl_interfaces::msg::ParameterEvent::SharedPtr &)> user_callback_;

  // Pointer to latest event message
  rcl_interfaces::msg::ParameterEvent::SharedPtr last_event_;
};

}  // namespace rclcpp

#endif  // RCLCPP__PARAMETER_EVENTS_SUBSCRIBER_HPP_
