#include <iostream>
#include <lgpio.h>
#include <unistd.h> // for sleep

int main() {
    // ---- CONFIGURATION ----
    // Change these to the actual GPIO numbers you wired up on your Pi 5
    int PWM_PIN = 12;  // The pin connected to ENA or ENB (Speed)
    int DIR_PIN = 23;  // The pin connected to IN1 or IN2 (Direction)
    
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

    // Set the Direction Pin to Output Mode, and set it HIGH (1).
    // (If the motor goes backward, change the 1 to a 0)
    if (lgGpioClaimOutput(handle, lgSET_PULL_NONE, DIR_PIN, 1) < 0) {
        std::cerr << "Error claiming Direction Pin " << DIR_PIN << std::endl;
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
    lgGpioFree(handle, DIR_PIN);
    lgGpiochipClose(handle);

    std::cout << "Test Complete." << std::endl;
    return 0;
}

