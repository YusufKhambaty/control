#include "control_package/pi_system.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "pluginlib/class_list_macros.hpp"

namespace control_package
{

hardware_interface::CallbackReturn PiSystemHardware::on_init(
  const hardware_interface::HardwareInfo & info)
{
  if (hardware_interface::SystemInterface::on_init(info) !=
      hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Allocate memory for 2 wheels (left and right)
  hw_positions_.resize(info_.joints.size(), 0.0);
  hw_velocities_.resize(info_.joints.size(), 0.0);
  hw_commands_.resize(info_.joints.size(), 0.0);

  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> PiSystemHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
  for (auto i = 0u; i < info_.joints.size(); i++) {
    state_interfaces.emplace_back(hardware_interface::StateInterface(
      info_.joints[i].name, hardware_interface::HW_IF_POSITION, &hw_positions_[i]));
    state_interfaces.emplace_back(hardware_interface::StateInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_velocities_[i]));
  }
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> PiSystemHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  for (auto i = 0u; i < info_.joints.size(); i++) {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_commands_[i]));
  }
  return command_interfaces;
}

hardware_interface::CallbackReturn PiSystemHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  // When ROS 2 starts the controller, physically open the Pi GPIOs
  if (!comms_.setup_gpio()) {
    return hardware_interface::CallbackReturn::ERROR;
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn PiSystemHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  // When ROS 2 stops, safely shut off motors
  comms_.cleanup();
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type PiSystemHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // Because you don't have encoders right now, we "fake" the read by just applying the commands directly to the state
  // so the controller thinks the wheels moved perfectly.
  for (auto i = 0u; i < hw_commands_.size(); i++) {
    hw_velocities_[i] = hw_commands_[i];
    hw_positions_[i] += hw_velocities_[i] * 0.01; // Fake positional math
  }
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type PiSystemHardware::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  // Grab the velocity commanded by ROS 2 geometry math
  double left_cmd_rads_per_sec = hw_commands_[0];
  double right_cmd_rads_per_sec = hw_commands_[1];

  // Map the radians/sec to a PWM completely arbitrary value (0 to 100).
  // E.g., assume 10 radians/sec = 100% PWM speed. You can tune this math later!
  double left_pwm = (left_cmd_rads_per_sec / 10.0) * 100.0;
  double right_pwm = (right_cmd_rads_per_sec / 10.0) * 100.0;

  // Send to the hardware!
  comms_.set_motor_speeds(left_pwm, right_pwm);

  return hardware_interface::return_type::OK;
}

} // namespace control_package

// VERY IMPORTANT: This macro allows ROS 2 to "see" your class as a plugin!
PLUGINLIB_EXPORT_CLASS(
  control_package::PiSystemHardware, hardware_interface::SystemInterface)

