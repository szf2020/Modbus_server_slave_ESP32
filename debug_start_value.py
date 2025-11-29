#!/usr/bin/env python3
"""
Debug CNT-HW-08: Counter start value race condition
Trace the exact sequence and timings
"""
import serial
import time

def main():
    try:
        ser = serial.Serial('COM11', 115200, timeout=0.5)
        time.sleep(2)

        print("=" * 70)
        print("DEBUG: CNT-HW-08 START VALUE TEST")
        print("=" * 70)

        # Step 1: Configure counter
        print("\n[STEP 1] Configure counter 1 with start-value:5000")
        t0 = time.time()
        cmd = ('set counter 1 mode 1 hw-mode:hw edge:rising prescaler:1 '
               'start-value:5000 hw-gpio:13 index-reg:10 ctrl-reg:14')
        ser.write(f"{cmd}\r\n".encode())
        time.sleep(2)
        output = ser.read(2000).decode('utf-8', errors='ignore')
        print(f"Config done at T+{time.time()-t0:.2f}s")
        print("Output:", output[:300])

        # Step 2: Read before reset
        print("\n[STEP 2] Read register 10 BEFORE reset")
        t_before = time.time()
        ser.write(b"read reg 10 1\r\n")
        time.sleep(0.5)
        output = ser.read(1000).decode('utf-8', errors='ignore')
        for line in output.split('\n'):
            if 'Reg[10]' in line:
                print(f"  Before reset: {line.strip()}")
                break

        # Step 3: Write reset command
        print("\n[STEP 3] Write reset command (ctrl-reg = 1)")
        t_reset = time.time()
        ser.write(b"write reg 14 value 1\r\n")
        time.sleep(0.5)
        output = ser.read(1000).decode('utf-8', errors='ignore')
        print(f"Reset sent at T+{t_reset-t0:.2f}s, response:")
        print("Output:", output[:200])

        # Step 4: Read immediately after reset (no sleep)
        print("\n[STEP 4] Read register 10 immediately after reset")
        t_immed = time.time()
        ser.write(b"read reg 10 1\r\n")
        time.sleep(0.5)
        output = ser.read(1000).decode('utf-8', errors='ignore')
        for line in output.split('\n'):
            if 'Reg[10]' in line:
                print(f"  Immediate: {line.strip()}")
                break

        # Step 5: Wait and read again
        print("\n[STEP 5] Wait 0.5s and read register 10 again")
        time.sleep(0.5)
        t_later = time.time()
        ser.write(b"read reg 10 1\r\n")
        time.sleep(0.5)
        output = ser.read(1000).decode('utf-8', errors='ignore')
        for line in output.split('\n'):
            if 'Reg[10]' in line:
                print(f"  After 0.5s: {line.strip()}")
                break

        # Summary
        print("\n[SUMMARY]")
        print(f"Total time: {time.time()-t0:.2f}s")
        print(f"Expected: 5000 (or close)")
        print(f"Test tolerance: ±100")

        ser.close()

    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    main()
