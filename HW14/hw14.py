import serial
import time
import numpy as np
import matplotlib.pyplot as plt

# Configuration
NUM_SAMPLES = 500
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 115200

def main():
    print(f"Connecting to {SERIAL_PORT}...")
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return

    # Wait for the pico to be ready
    time.sleep(1)
    
    print(f"Requesting {NUM_SAMPLES} samples...")
    # Send the number of samples followed by a newline
    ser.write(f"{NUM_SAMPLES}\r\n".encode('utf-8'))
    
    times = []
    raws = []
    filtereds = []
    
    print("Collecting data...")
    count = 0
    # Increase timeout for blocking read loop
    ser.timeout = 0.5
    
    empty_reads = 0
    while count < NUM_SAMPLES:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            empty_reads += 1
            if empty_reads > 20: # 10 seconds of no data
                print("Error: Timed out waiting for data from Pico.")
                break
            continue
            
        empty_reads = 0
        try:
            parts = line.split()
            if len(parts) == 3:
                t = int(parts[0])
                r = int(parts[1])
                f = int(parts[2])
                times.append(t)
                raws.append(r)
                filtereds.append(f)
                count += 1
                if count % 100 == 0:
                    print(f"Collected {count}/{NUM_SAMPLES} samples")
        except ValueError:
            pass
            
    ser.close()
    
    if count == 0:
        print("No data collected. Exiting.")
        return
        
    print("Data collection complete. Generating plots...")
    
    times = np.array(times)
    raws = np.array(raws)
    filtereds = np.array(filtereds)
    
    # Calculate relative time in seconds for plotting
    t_sec = (times - times[0]) / 1000.0
    
    # Plotting Data
    plt.figure(figsize=(12, 6))
    
    plt.subplot(1, 2, 1)
    plt.plot(t_sec, raws, label="Raw Data", color='blue', alpha=0.5)
    plt.plot(t_sec, filtereds, label="Filtered Data", color='red', linewidth=2)
    plt.title("Sensor Data vs Time")
    plt.xlabel("Time (s)")
    plt.ylabel("Sensor Value")
    plt.legend()
    plt.grid(True)
    
    # FFT
    # Approximate sampling period
    T = np.mean(np.diff(t_sec))
    Fs = 1.0 / T
    print(f"Approximate Sampling Frequency: {Fs:.2f} Hz")
    
    # Compute FFT for Raw (remove DC offset first)
    N = len(raws)
    yf_raw = np.fft.fft(raws - np.mean(raws))
    xf_raw = np.fft.fftfreq(N, T)[:N//2]
    magnitude_raw = 2.0/N * np.abs(yf_raw[0:N//2])
    
    # Compute FFT for Filtered (remove DC offset first)
    yf_filt = np.fft.fft(filtereds - np.mean(filtereds))
    magnitude_filt = 2.0/N * np.abs(yf_filt[0:N//2])
    
    plt.subplot(1, 2, 2)
    plt.plot(xf_raw, magnitude_raw, label="Raw FFT", color='blue', alpha=0.5)
    plt.plot(xf_raw, magnitude_filt, label="Filtered FFT", color='red', linewidth=2)
    plt.title("FFT of Sensor Data")
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.legend()
    plt.grid(True)
    
    plt.tight_layout()
    
    # Save the figure
    plt.savefig('hw14_plot.png')
    print("Plot saved to hw14_plot.png")
    
    plt.show()

if __name__ == "__main__":
    main()
