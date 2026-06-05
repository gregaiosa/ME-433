import pgzrun
import serial
import re

# Set window size
WIDTH = 800
HEIGHT = 600

# Serial port configuration
# Note: You may need to change this port to match your Pico's port. 
# Typical ports: '/dev/ttyACM0' (Linux), 'COM3' (Windows), '/dev/cu.usbmodem...' (Mac)
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0)
    print(f"Connected to {SERIAL_PORT}")
except Exception as e:
    ser = None
    print(f"Warning: Could not open serial port {SERIAL_PORT}: {e}")

# Data variables
position = 0
force = 0

# Variables for visualization tuning
force_offset = 0
# You might need to tune this scale depending on the variance of your load cell readings
force_scale = 0.005 

def update():
    global position, force
    if ser is not None and ser.in_waiting > 0:
        try:
            # Read all available lines and keep the last complete one to avoid lag
            lines = ser.readlines()
            if lines:
                # Use the last line to ensure we have the most recent data
                line = lines[-1].decode('utf-8', errors='ignore').strip()
                # Expecting format: "Angle: <pos>\t Force: <force>" based on HW17.c
                match = re.search(r"Angle:\s*(\d+)\s*Force:\s*(-?\d+)", line)
                if match:
                    position = int(match.group(1))
                    force = int(match.group(2))
        except Exception as e:
            pass

def on_key_down(key):
    global force_offset
    if key == keys.T:
        # Tare the force reading (set current reading to 0 in viz)
        force_offset = force
        print("Tared force!")

def draw():
    screen.clear()
    screen.fill((30, 30, 40)) # Dark blue-gray background
    
    # Title
    screen.draw.text("Pico 2W Sensor Data", center=(WIDTH//2, 50), color="white", fontsize=50)
    screen.draw.text("Press 'T' to Tare Force", center=(WIDTH//2, 90), color="gray", fontsize=25)
    
    # --- Position Visualization ---
    screen.draw.text(f"Position (Angle): {position}°", (100, 150), color="cyan", fontsize=40)
    
    max_pos_width = 600
    # Allow wrapping if it goes above 360, but limit display width visually
    display_pos = position % 360
    pos_width = (display_pos / 360.0) * max_pos_width
    
    # Draw background bar
    screen.draw.filled_rect(Rect((100, 200), (max_pos_width, 40)), (50, 50, 60))
    # Draw filled bar
    screen.draw.filled_rect(Rect((100, 200), (pos_width, 40)), "cyan")
    # Draw outline
    screen.draw.rect(Rect((100, 200), (max_pos_width, 40)), "white")
    
    # --- Force Visualization ---
    adjusted_force = force - force_offset
    screen.draw.text(f"Force (Raw): {force}", (100, 300), color="orange", fontsize=30)
    screen.draw.text(f"Force (Adjusted): {adjusted_force}", (100, 340), color="red", fontsize=40)
    
    center_x = WIDTH // 2
    force_width = adjusted_force * force_scale
    
    # Draw background track
    screen.draw.filled_rect(Rect((100, 400), (600, 40)), (50, 50, 60))
    
    # Draw filled bar from center
    # Clamp width to avoid drawing outside bounds
    draw_width = force_width
    if draw_width > 300: draw_width = 300
    if draw_width < -300: draw_width = -300
    
    if draw_width >= 0:
        screen.draw.filled_rect(Rect((center_x, 400), (draw_width, 40)), "red")
    else:
        screen.draw.filled_rect(Rect((center_x + draw_width, 400), (-draw_width, 40)), "red")
        
    # Draw outline and center line
    screen.draw.rect(Rect((100, 400), (600, 40)), "white")
    screen.draw.line((center_x, 390), (center_x, 450), "white")

pgzrun.go()
