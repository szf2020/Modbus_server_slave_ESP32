#!/usr/bin/env python3
"""
Debug timer mode 3 configuration values
"""
import serial
import time

def main():
    try:
        ser = serial.Serial('COM11', 115200, timeout=0.5)
        time.sleep(2)

        print("=" * 70)
        print("DEBUG: TIMER MODE 3 CONFIG")
        print("=" * 70)

        # Set timer 1 mode 3 with on:1000 off:1000
        print("\n[TEST] Configure timer 1 mode 3 on:1000 off:1000")
        cmd = 'set timer 1 mode 3 on:1000 off:1000 output-coil:200'
        ser.write(f"{cmd}\r\n".encode())
        time.sleep(2)
        output = ser.read(2000).decode('utf-8', errors='ignore')
        print("Config output:")
        print(output)

        # Try to read timer config (if show command exists)
        print("\n[TEST] Try to show timer 1 config")
        ser.write(b"show timer 1\r\n")
        time.sleep(0.5)
        output = ser.read(2000).decode('utf-8', errors='ignore')
        print(output)

        # Check if we can read registers that timer uses
        print("\n[TEST] Read registers to see timer state")
        ser.write(b"read reg 0 10\r\n")
        time.sleep(0.5)
        output = ser.read(2000).decode('utf-8', errors='ignore')
        print(output[:500])

        ser.close()

    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    main()
