#ifndef CONTROL_PACKAGE__PI_SYSTEM_HPP
#define CONTROL_PACKAGE__PI_SYSTEM_HPP

#include <vector>
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp_lifecycle/state.hpp"

// Include your offline hardware class
#include "control_package/pi_comms.h"

namespace control_package
{

class PiSystemHardware : public hardware_interface::SystemInterface
{
public:
  hardware_interface::CallbackReturn on_init(
    const hardware_interface::HardwareInfo & info) override;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::return_type read(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

  hardware_interface::return_type write(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

private:
  // Your custom offline class
  PiMotorController comms_ = PiMotorController(12, 23, 13, 24); // Replace with your exact Pins

  // ROS 2 memory spaces for the left and right wheels
  std::vector<double> hw_commands_;
  std::vector<double> hw_positions_;
  std::vector<double> hw_velocities_;
};

} // namespace control_package

#endif // CONTROL_PACKAGE__PI_SYSTEM_HPP

