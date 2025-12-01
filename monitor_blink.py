#!/usr/bin/env python3
"""Monitor LED blinking - show HR#100 value changing with different intervals"""
import serial
import time

# Connect to ESP32
try:
    ser = serial.Serial('COM11', 115200, timeout=2)
except Exception as e:
    print(f"ERROR: Cannot open COM11: {e}")
    exit(1)

time.sleep(1)

print("="*80)
print("LED BLINK MONITOR - Watching HR#100 (LED signal) with variable intervals")
print("="*80)
print("\nWatching for 30 seconds (3 complete 10-second cycles)...\n")
print("Phase       Time    Expected Pattern           LED Values Observed")
print("-" * 80)

start_time = time.time()
last_value = None
toggle_count = 0
phase_start = start_time

for i in range(30):
    elapsed = time.time() - start_time

    # Determine phase
    phase_in_cycle = elapsed % 10
    if phase_in_cycle < 2.5:
        phase = "FAST (100ms)"
        expected_toggles = 5  # 2.5s / 100ms = 25 toggles, every ~0.5s we see one
    elif phase_in_cycle < 5:
        phase = "MED-FAST (250ms)"
        expected_toggles = 2
    elif phase_in_cycle < 7.5:
        phase = "MEDIUM (500ms)"
        expected_toggles = 1
    else:
        phase = "SLOW (1000ms)"
        expected_toggles = 0

    # Get current register value
    cmd = 'read_holding_register 100'
    ser.write((cmd + '\r\n').encode())
    time.sleep(0.05)

    response = ser.read(ser.in_waiting).decode(errors='replace')

    # Extract value (simplistic parsing)
    value_str = "?"
    if "0" in response or "1" in response:
        # Try to find the actual value
        lines = response.split('\n')
        for line in lines:
            if 'register' in line.lower() or '0' in line or '1' in line:
                if '100' in line and ':' in line:
                    parts = line.split(':')
                    if len(parts) > 1:
                        val = parts[-1].strip()[:1]
                        if val in ['0', '1']:
                            value_str = val + (" [OFF]" if val == "0" else " [ON]")
                            break

    # Detect toggle
    if value_str[0] in ['0', '1']:
        current_value = int(value_str[0])
        if last_value is not None and current_value != last_value:
            toggle_count += 1
        last_value = current_value

    print(f"{phase:20s}  {elapsed:5.1f}s   Every ~{100 if 'FAST' in phase else 250 if 'MED-FAST' in phase else 500 if 'MEDIUM' in phase else 1000}ms    {value_str:20s}")

    time.sleep(1)

print("\n" + "="*80)
print(f"SUMMARY: Observed {toggle_count} LED toggles over 30 seconds")
print("="*80)
print("\nExpected behavior:")
print("  [FAST]      - LED toggles every ~100ms (many changes per second)")
print("  [MED-FAST]  - LED toggles every ~250ms (changes every quarter second)")
print("  [MEDIUM]    - LED toggles every ~500ms (changes every half second)")
print("  [SLOW]      - LED toggles every ~1000ms (changes every second)")
print("\nPattern repeats every 10 seconds in this order:")
print("  0-2.5s:  FAST")
print("  2.5-5s:  MEDIUM-FAST")
print("  5-7.5s:  MEDIUM")
print("  7.5-10s: SLOW")
print("="*80)

ser.close()
