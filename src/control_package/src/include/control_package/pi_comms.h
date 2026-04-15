#ifndef CONTROL_PACKAGE__PI_COMMS_H
#define CONTROL_PACKAGE__PI_COMMS_H

#include <lgpio.h>
#include <iostream>

namespace control_package
{

class PiMotorController
{
public:
    // Constructor & Destructor
    PiMotorController(int left_pwm_pin, int left_dir_pin, int right_pwm_pin, int right_dir_pin);
    ~PiMotorController();

    // Core functionality
    bool setup_gpio();
    void set_motor_speeds(double left_speed, double right_speed); // -100.0 to 100.0
    void cleanup();

private:
    // State variables
    int handle_;
    bool is_connected_;

    // Pin configurations
    int left_pwm_pin_;
    int left_dir_pin_;
    int right_pwm_pin_;
    int right_dir_pin_;
};

} // namespace control_package

#endif // CONTROL_PACKAGE__PI_COMMS_H

