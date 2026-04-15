#include "control_package/pi_comms.h"

namespace control_package
{

PiMotorController::PiMotorController(int left_pwm_pin, int left_dir_pin, int right_pwm_pin, int right_dir_pin)
    : handle_(-1), is_connected_(false),
      left_pwm_pin_(left_pwm_pin), left_dir_pin_(left_dir_pin),
      right_pwm_pin_(right_pwm_pin), right_dir_pin_(right_dir_pin)
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

    // Set direction pins to output, defaulting to 1 (Forward)
    lgGpioClaimOutput(handle_, lgSET_PULL_NONE, left_dir_pin_, 1);
    lgGpioClaimOutput(handle_, lgSET_PULL_NONE, right_dir_pin_, 1);

    is_connected_ = true;
    return true;
}

void PiMotorController::set_motor_speeds(double left_speed, double right_speed)
{
    if (!is_connected_) return;

    // Handle Left Direction
    int left_dir = (left_speed >= 0) ? 1 : 0;
    lgGpioWrite(handle_, left_dir_pin_, left_dir);

    // Handle Right Direction
    int right_dir = (right_speed >= 0) ? 1 : 0;
    lgGpioWrite(handle_, right_dir_pin_, right_dir);

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
        lgGpioFree(handle_, left_dir_pin_);
        lgGpioFree(handle_, right_pwm_pin_);
        lgGpioFree(handle_, right_dir_pin_);

        // Close chip
        lgGpiochipClose(handle_);
        handle_ = -1;
        is_connected_ = false;
    }
}

} // namespace control_package

