#include "control_package/pi_comms.h"

namespace control_package
{

PiMotorController::PiMotorController(int left_pwm_pin, int left_in1_pin, int left_in2_pin, 
                                     int right_pwm_pin, int right_in1_pin, int right_in2_pin)
    : handle_(-1), is_connected_(false),
      left_pwm_pin_(left_pwm_pin), left_in1_pin_(left_in1_pin), left_in2_pin_(left_in2_pin),
      right_pwm_pin_(right_pwm_pin), right_in1_pin_(right_in1_pin), right_in2_pin_(right_in2_pin)
{
}

PiMotorController::~PiMotorController()
{
    cleanup();
}

bool PiMotorController::setup_gpio()
{
    // Try to open Raspberry Pi 5 GPIO chip
    handle_ = lgGpiochipOpen(4);
    if (handle_ < 0) {
        handle_ = lgGpiochipOpen(0); // Fallback
    }

    if (handle_ < 0) {
        std::cerr << "Failed to open GPIO chip. Run with sudo!" << std::endl;
        return false;
    }

    // Set direction pins to output, defaulting to Stop (LOW)
    lgGpioClaimOutput(handle_, LG_SET_PULL_NONE, left_in1_pin_, 0);
    lgGpioClaimOutput(handle_, LG_SET_PULL_NONE, left_in2_pin_, 0);
    lgGpioClaimOutput(handle_, LG_SET_PULL_NONE, right_in1_pin_, 0);
    lgGpioClaimOutput(handle_, LG_SET_PULL_NONE, right_in2_pin_, 0);

    is_connected_ = true;
    return true;
}

void PiMotorController::set_motor_speeds(double left_speed, double right_speed)
{
    if (!is_connected_) return;

    // Handle Left Direction (TB6612FNG truth table)
    if (left_speed > 0.0) {        // Forward
        lgGpioWrite(handle_, left_in1_pin_, 1);
        lgGpioWrite(handle_, left_in2_pin_, 0);
    } else if (left_speed < 0.0) { // Reverse
        lgGpioWrite(handle_, left_in1_pin_, 0);
        lgGpioWrite(handle_, left_in2_pin_, 1);
    } else {                       // Stop
        lgGpioWrite(handle_, left_in1_pin_, 0);
        lgGpioWrite(handle_, left_in2_pin_, 0);
    }

    // Handle Right Direction (TB6612FNG truth table)
    if (right_speed > 0.0) {        // Forward
        lgGpioWrite(handle_, right_in1_pin_, 1);
        lgGpioWrite(handle_, right_in2_pin_, 0);
    } else if (right_speed < 0.0) { // Reverse
        lgGpioWrite(handle_, right_in1_pin_, 0);
        lgGpioWrite(handle_, right_in2_pin_, 1);
    } else {                        // Stop
        lgGpioWrite(handle_, right_in1_pin_, 0);
        lgGpioWrite(handle_, right_in2_pin_, 0);
    }

    // Convert speed to positive duty cycle (0 to 100)
    double left_pwm = std::abs(left_speed);
    double right_pwm = std::abs(right_speed);

    // Cap at 100%
    if (left_pwm > 100.0) left_pwm = 100.0;
    if (right_pwm > 100.0) right_pwm = 100.0;

    // Output to PWM pins at 1000 Hz
    lgTxPwm(handle_, left_pwm_pin_, 1000, left_pwm, 0, 0);
    lgTxPwm(handle_, right_pwm_pin_, 1000, right_pwm, 0, 0);
}

void PiMotorController::cleanup()
{
    if (handle_ >= 0) {
        // Stop motors
        lgTxPwm(handle_, left_pwm_pin_, 1000, 0.0, 0, 0);
        lgTxPwm(handle_, right_pwm_pin_, 1000, 0.0, 0, 0);

        // Free pins
        lgGpioFree(handle_, left_pwm_pin_);
        lgGpioFree(handle_, left_in1_pin_);
        lgGpioFree(handle_, left_in2_pin_);
        lgGpioFree(handle_, right_pwm_pin_);
        lgGpioFree(handle_, right_in1_pin_);
        lgGpioFree(handle_, right_in2_pin_);

        // Close chip
        lgGpiochipClose(handle_);
        handle_ = -1;
        is_connected_ = false;
    }
}

} // namespace control_package

