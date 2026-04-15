#include <iostream>
#include <lgpio.h>
#include <unistd.h> // for sleep

int main() {
    // ---- CONFIGURATION ----
    // Change these to the actual GPIO numbers you wired up on your Pi 5
    int PWM_PIN = 12;  // The pin connected to PWMA or PWMB (Speed)
    int IN1_PIN = 23;  // The pin connected to AIN1 or BIN1 (Direction 1)
    int IN2_PIN = 24;  // The pin connected to AIN2 or BIN2 (Direction 2)
    
    std::cout << "Initializing lgpio for Raspberry Pi 5..." << std::endl;

    // Pi 5 Specifics: The main 40-pin header is handled by the RP1 chip, 
    // which Linux usually maps to GPIO chip 4. 
    int handle = lgGpiochipOpen(4);
    
    if (handle < 0) {
        // If chip 4 fails, try 0 just in case.
        handle = lgGpiochipOpen(0);
        if (handle < 0) {
            std::cerr << "CRITICAL ERROR: Failed to open GPIO chip." << std::endl;
            std::cerr << "Make sure you installed liblgpio-dev and are using sudo!" << std::endl;
            return 1;
        }
    }

    std::cout << "GPIO Chip Opened successfully." << std::endl;

    // Set the Direction Pins to Output Mode. For Forward: IN1 = 1, IN2 = 0
    if (lgGpioClaimOutput(handle, LG_SET_PULL_NONE, IN1_PIN, 1) < 0) {
        std::cerr << "Error claiming IN1 Pin " << IN1_PIN << std::endl;
        return 1;
    }
    if (lgGpioClaimOutput(handle, LG_SET_PULL_NONE, IN2_PIN, 0) < 0) {
        std::cerr << "Error claiming IN2 Pin " << IN2_PIN << std::endl;
        return 1;
    }

    std::cout << "Spinning Motor at 50% Speed for 3 seconds..." << std::endl;

    // Start PWM on the PWM Pin.
    // Arguments: Handle, Pin, Frequency (Hz), Duty Cycle (0.0 to 100.0)
    if (lgTxPwm(handle, PWM_PIN, 1000, 50.0, 0, 0) < 0) {
         std::cerr << "Error starting PWM on Pin " << PWM_PIN << std::endl;
    }

    // Let the motor run for 3 seconds
    sleep(3);

    std::cout << "Stopping Motor..." << std::endl;
    
    // Stop the motor by setting Duty Cycle to 0.0
    lgTxPwm(handle, PWM_PIN, 1000, 0.0, 0, 0);

    // Free the pins and close the chip safely to prevent locking them out
    lgGpioFree(handle, PWM_PIN);
    lgGpioFree(handle, IN1_PIN);
    lgGpioFree(handle, IN2_PIN);
    lgGpiochipClose(handle);

    std::cout << "Test Complete." << std::endl;
    return 0;
}

