import pgzrun
from pygame import Rect
import serial
import threading
import random
import time
import os
import wave
import struct
import math

# Sound generation
os.makedirs("sounds", exist_ok=True)
def create_sound(filename, start_freq, end_freq, duration_ms):
    if not os.path.exists(filename):
        with wave.open(filename, 'w') as f:
            f.setnchannels(1)
            f.setsampwidth(2)
            f.setframerate(44100)
            num_samples = int(44100 * (duration_ms / 1000.0))
            for i in range(num_samples):
                progress = i / float(num_samples)
                freq = start_freq + (end_freq - start_freq) * progress
                value = int(32767.0 * 0.3 * math.sin(2 * math.pi * freq * (i / 44100.0)))
                data = struct.pack('<h', value)
                f.writeframesraw(data)

create_sound("sounds/jump.wav", 300, 600, 150)
create_sound("sounds/shoot.wav", 800, 200, 100)
create_sound("sounds/lose.wav", 200, 50, 500)

WIDTH = 400
HEIGHT = 600

# Game variables
player = Actor('player', center=(WIDTH // 2, HEIGHT // 2))
player_dy = 0
score = 0
game_over = False

platforms = []
bullets = []
enemies = []

# Controller state
ctrl_tilt_x = 0
ctrl_button = 0
prev_button = 0

# Setup Serial Communication
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

def serial_thread():
    global ctrl_tilt_x, ctrl_button
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Connected to Pico on {SERIAL_PORT}")
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()
                parts = line.split(',')
                if len(parts) == 3:
                    try:
                        accel_x = int(parts[0])
                        accel_y = int(parts[1])
                        button = int(parts[2])
                        
                        # MPU6050 raw accel range is roughly +/- 16384 for 2g
                        # We map accel_x to tilt. You might need to swap with accel_y
                        # depending on the orientation of your breadboard.
                        ctrl_tilt_x = accel_x
                        ctrl_button = button
                    except ValueError:
                        pass
    except Exception as e:
        print(f"Serial connection failed or disconnected: {e}")
        print("Falling back to Keyboard controls (Left/Right to steer, Space to shoot)")

# Start serial reader in the background
threading.Thread(target=serial_thread, daemon=True).start()

def spawn_platform(y_pos):
    x_pos = random.randint(0, WIDTH - 60)
    platforms.append(Rect((x_pos, y_pos), (60, 15)))
    
    # 15% chance to spawn an enemy on this platform, max 3 on screen
    if len(enemies) < 3 and random.random() < 0.15:
        # Enemy is an actor resting on the platform
        enemies.append(Actor('enemy', bottomleft=(x_pos + 15, y_pos)))

def reset_game():
    global player_dy, score, game_over, platforms, bullets, enemies
    player.center = (WIDTH // 2, HEIGHT // 2)
    player_dy = -10
    score = 0
    game_over = False
    platforms = []
    bullets = []
    enemies = []
    
    # Initial platforms
    spawn_platform(HEIGHT - 50)
    for i in range(1, 10):
        spawn_platform(HEIGHT - 50 - i * 60)

reset_game()

def get_input():
    global prev_button
    
    # IMU Mapping
    # Raw value ranges from -16000 to +16000 typically.
    # Map it to a velocity of -8 to +8.
    vx = ctrl_tilt_x / 2000.0
    
    # Cap speed
    vx = max(-8, min(8, vx))
    
    # Keyboard fallback
    if keyboard.left:
        vx = 5 # inverted because x -= vx in update
    if keyboard.right:
        vx = -5
        
    shoot = False
    if ctrl_button == 1 and prev_button == 0:
        shoot = True
    if keyboard.space:
        shoot = True
        
    prev_button = ctrl_button
    
    return vx, shoot

def update():
    global player_dy, score, game_over

    if game_over:
        if keyboard.r:
            reset_game()
        return

    vx, shoot = get_input()

    # Apply steering
    player.x -= vx # Subtract because positive X tilt might mean tilt left depending on IMU orientation

    # Screen wrap
    if player.right < 0:
        player.left = WIDTH
    elif player.left > WIDTH:
        player.right = 0

    # Physics
    player_dy += 0.4 # Gravity
    player.y += player_dy

    # Collisions with platforms (only when falling)
    if player_dy > 0:
        for p in platforms:
            if player.colliderect(p) and player.bottom - player_dy <= p.top + 10:
                # Bounce
                player.bottom = p.top
                player_dy = -12
                try: sounds.jump.play()
                except: pass
                break
                
    # Shooting
    if shoot:
        bullets.append(Rect((player.centerx - 2, player.top), (4, 10)))
        try: sounds.shoot.play()
        except: pass

    # Bullet update
    for b in bullets[:]:
        b.y -= 10
        if b.bottom < 0:
            bullets.remove(b)
            continue
            
        # Check bullet collision with enemies
        for e in enemies[:]:
            if e.colliderect(b):
                if b in bullets: bullets.remove(b)
                enemies.remove(e)
                score += 50
                break

    # Player collision with enemies
    for e in enemies:
        if player.colliderect(e):
            if not game_over:
                game_over = True
                try: sounds.lose.play()
                except: pass

    # Camera scroll logic
    scroll_thresh = HEIGHT // 2
    if player.y < scroll_thresh:
        diff = scroll_thresh - player.y
        player.y = scroll_thresh
        score += int(diff)
        
        # Move platforms down
        for p in platforms:
            p.y += diff
            
        # Move bullets down
        for b in bullets:
            b.y += diff
            
        # Move enemies down
        for e in enemies:
            e.y += diff

    # Clean up off-screen platforms and spawn new ones
    for p in platforms[:]:
        if p.top > HEIGHT:
            platforms.remove(p)
            spawn_platform(platforms[-1].top - random.randint(50, 80))
            
    # Clean up off-screen enemies
    for e in enemies[:]:
        if e.top > HEIGHT:
            enemies.remove(e)

    # Game Over condition
    if player.top > HEIGHT:
        if not game_over:
            game_over = True
            try: sounds.lose.play()
            except: pass

def draw():
    screen.clear()
    screen.fill((200, 230, 255)) # Sky blue background
    
    # Draw platforms
    for p in platforms:
        screen.draw.filled_rect(p, (50, 200, 50))
        
    # Draw bullets
    for b in bullets:
        screen.draw.filled_rect(b, (255, 0, 0))
        
    # Draw enemies
    for e in enemies:
        e.draw()
        
    # Draw player
    player.draw()
    
    # UI
    screen.draw.text(f"Score: {score}", (10, 10), color="black", fontsize=30)
    
    if game_over:
        screen.draw.text("GAME OVER", center=(WIDTH//2, HEIGHT//2), color="red", fontsize=60)
        screen.draw.text("Press R to restart", center=(WIDTH//2, HEIGHT//2 + 50), color="black", fontsize=30)

pgzrun.go()
