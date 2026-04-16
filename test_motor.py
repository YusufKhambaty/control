import lgpio
import time

# --- CONFIGURATION (TB6612FNG) ---
# Left Motor
PWM_A = 12
AIN1 = 23
AIN2 = 24

# Right Motor
PWM_B = 13
BIN1 = 25
BIN2 = 26

def setup():
    print("Opening GPIO chip...")
    # Open Pi 5's main GPIO chip (usually chip 4)
    try:
        h = lgpio.gpiochip_open(4)
    except:
        h = lgpio.gpiochip_open(0) # Fallback

    # Set as outputs
    for pin in [AIN1, AIN2, BIN1, BIN2]:
        lgpio.gpio_claim_output(h, pin)

    return h

def set_motors(h, left_speed, right_speed):
    # left_speed and right_speed from -100 to 100

    # Left Direction
    if left_speed > 0:
        lgpio.gpio_write(h, AIN1, 1)
        lgpio.gpio_write(h, AIN2, 0)
    elif left_speed < 0:
        lgpio.gpio_write(h, AIN1, 0)
        lgpio.gpio_write(h, AIN2, 1)
    else:
        lgpio.gpio_write(h, AIN1, 0)
        lgpio.gpio_write(h, AIN2, 0)

    # Right Direction
    if right_speed > 0:
        lgpio.gpio_write(h, BIN1, 1)
        lgpio.gpio_write(h, BIN2, 0)
    elif right_speed < 0:
        lgpio.gpio_write(h, BIN1, 0)
        lgpio.gpio_write(h, BIN2, 1)
    else:
        lgpio.gpio_write(h, BIN1, 0)
        lgpio.gpio_write(h, BIN2, 0)

    # Convert to pure positive PWM Duty Cycle
    lpwm = abs(left_speed)
    rpwm = abs(right_speed)

    # Start PWM at 1000 Hz
    lgpio.tx_pwm(h, PWM_A, 1000, lpwm)
    lgpio.tx_pwm(h, PWM_B, 1000, rpwm)

def cleanup(h):
    lgpio.tx_pwm(h, PWM_A, 1000, 0)
    lgpio.tx_pwm(h, PWM_B, 1000, 0)
    for pin in [PWM_A, AIN1, AIN2, PWM_B, BIN1, BIN2]:
        lgpio.gpio_free(h, pin)
    lgpio.gpiochip_close(h)

if __name__ == "__main__":
    h = setup()
    try:
        print("Starting Forward Loop...")
        # Ramp up speed
        for speed in range(0, 101, 10):
            print(f"Speed: {speed}%")
            set_motors(h, speed, speed)
            time.sleep(0.5)

        # Ramp down
        for speed in range(100, -1, -10):
            print(f"Speed: {speed}%")
            set_motors(h, speed, speed)
            time.sleep(0.5)

        print("Starting Reverse Loop...")
        # Ramp up reverse
        for speed in range(0, -101, -10):
            print(f"Speed: {speed}%")
            set_motors(h, speed, speed)
            time.sleep(0.5)

        # Ramp down reverse
        for speed in range(-100, 1, 10):
            print(f"Speed: {speed}%")
            set_motors(h, speed, speed)
            time.sleep(0.5)

        print("Test Complete!")

    except KeyboardInterrupt:
        print("\nManually stopped.")
    finally:
        cleanup(h)
