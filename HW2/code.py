import board
import digitalio
import pwmio
import time

print("Hello world!")

servo = pwmio.PWMOut(board.GP16, variable_frequency=True)
servo.frequency = 50 # in hertz
servo.duty_cycle = 0 # initally off, a 16bit number so max on is 65535

def set_servo_angle(angle):
    # 0 degrees is 0.5ms pulse, 180 degrees is 2.5ms pulse
    # at 50Hz, the period is 20ms, so duty cycle is pulse width / period
    pulse_width = (angle / 180) * (2.5 - 0.5) + 0.5 # in ms
    duty_cycle = int((pulse_width / 20) * 65535)
    servo.duty_cycle = duty_cycle

while True:
    set_servo_angle(0)
    time.sleep(1)
    set_servo_angle(180)
    time.sleep(1)