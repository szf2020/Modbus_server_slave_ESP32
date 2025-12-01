#!/usr/bin/env python3
import serial
import time

# Connect to ESP32
try:
    ser = serial.Serial('COM11', 115200, timeout=2)
except:
    print("ERROR: Cannot open COM11")
    exit(1)

time.sleep(2)

# ST program for blinking with variable intervals (condensed)
st_program = "VAR phase_counter: INT; blink_counter: INT; led: BOOL; blink_interval: INT; END_VAR phase_counter := phase_counter + 1; IF phase_counter >= 1000 THEN phase_counter := 0; END_IF; IF phase_counter < 250 THEN blink_interval := 10; ELSIF phase_counter < 500 THEN blink_interval := 25; ELSIF phase_counter < 750 THEN blink_interval := 50; ELSE blink_interval := 100; END_IF; IF blink_counter >= blink_interval THEN blink_counter := 0; led := NOT led; ELSE blink_counter := blink_counter + 1; END_IF;"

print("="*70)
print("LED BLINK TEST - Variable Interval Blinking on GPIO 2")
print("="*70)

# Step 1: Upload program to Logic1
print("\n[1/4] Uploading ST program to Logic1...")
cmd = f'set logic 1 upload "{st_program}"'
ser.write((cmd + '\r\n').encode())
time.sleep(1.5)

response = ser.read(ser.in_waiting).decode(errors='ignore')
print(response)

# Step 2: Bind led variable to HR#100
print("\n[2/4] Binding 'led' variable (index 2) to HR#100...")
cmd = 'set logic 1 bind 2 100 output'
ser.write((cmd + '\r\n').encode())
time.sleep(0.8)

response = ser.read(ser.in_waiting).decode(errors='ignore')
print(response)

# Step 3: Enable the program
print("\n[3/4] Enabling Logic1 program...")
cmd = 'set logic 1 enabled:true'
ser.write((cmd + '\r\n').encode())
time.sleep(0.8)

response = ser.read(ser.in_waiting).decode(errors='ignore')
print(response)

# Step 4: Show status and verify
print("\n[4/4] Checking program status...")
cmd = 'show logic 1'
ser.write((cmd + '\r\n').encode())
time.sleep(1.0)

response = ser.read(ser.in_waiting).decode(errors='ignore')
print(response)

print("\n" + "="*70)
print("PROGRAM RUNNING!")
print("="*70)
print("\nLED Blink Pattern (10-second cycle):")
print("  0-2.5 sec:  FAST   (~100ms interval)")
print("  2.5-5 sec:  MEDIUM-FAST (~250ms interval)")
print("  5-7.5 sec:  MEDIUM (~500ms interval)")
print("  7.5-10 sec: SLOW   (~1000ms interval)")
print("\nWatch GPIO 2 LED - it should cycle through these patterns!")
print("="*70)

ser.close()
