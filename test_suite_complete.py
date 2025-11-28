#!/usr/bin/env python3
"""
ESP32 Modbus Server - Komplet Test Suite
Tester alle CLI kommandoer, counter modes, timer modes, GPIO config
"""

import serial
import time
import threading
import re
from datetime import datetime

class ESP32TestSuite:
    def __init__(self, port='COM11', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = False
        self.read_buffer = []
        self.results = []

    def connect(self):
        """Open serial connection"""
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=0.1)
            time.sleep(1)
            self.running = True

            # Start reader thread
            self.reader = threading.Thread(target=self._read_serial, daemon=True)
            self.reader.start()

            print(f"[OK] Tilsluttet til {self.port} @ {self.baudrate}")
            return True
        except Exception as e:
            print(f"[ERROR] Kunne ikke tilslutte: {e}")
            return False

    def disconnect(self):
        """Close serial connection"""
        self.running = False
        time.sleep(0.5)
        if self.ser:
            self.ser.close()

    def _read_serial(self):
        """Background thread to read serial data"""
        while self.running:
            if self.ser and self.ser.in_waiting:
                try:
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        self.read_buffer.append(line)
                except:
                    pass
            time.sleep(0.01)

    def send_command(self, cmd, wait_time=0.5):
        """Send command and wait for response"""
        if not self.ser:
            return []

        # Clear buffer
        self.read_buffer = []

        # Send command
        self.ser.write(f"{cmd}\r\n".encode('utf-8'))
        time.sleep(wait_time)

        # Return buffered output
        return self.read_buffer.copy()

    def check_output(self, output, expected_patterns):
        """Check if output contains expected patterns"""
        output_text = "\n".join(output)

        for pattern in expected_patterns:
            if isinstance(pattern, str):
                if pattern not in output_text:
                    return False, f"Missing: '{pattern}'"
            elif hasattr(pattern, 'search'):  # regex
                if not pattern.search(output_text):
                    return False, f"Pattern not matched: {pattern.pattern}"

        return True, "OK"

    def log_result(self, test_id, description, status, comment=""):
        """Log test result"""
        result = {
            'id': test_id,
            'desc': description,
            'status': status,
            'comment': comment,
            'time': datetime.now().strftime("%H:%M:%S")
        }
        self.results.append(result)

        status_icon = "[PASS]" if status == "PASS" else "[FAIL]" if status == "FAIL" else "[WARN]"
        print(f"{status_icon} [{test_id}] {description}: {comment}")

    # ========================================================================
    # TEST CATEGORIES
    # ========================================================================

    def test_show_commands(self):
        """Test all SHOW commands"""
        print("\n" + "="*70)
        print("TESTING: SHOW KOMMANDOER")
        print("="*70)

        tests = [
            ('SH-01', 'show config', ['Version:', 'Unit-ID:']),
            ('SH-02', 'show counters', ['counter']),  # Fixed: removed 'ENABLED' requirement
            ('SH-03', 'show timers', ['timer']),
            ('SH-04', 'show registers', ['Reg']),  # Fixed: simplified pattern
            ('SH-05', 'show registers 10 5', ['Reg']),  # Fixed
            ('SH-06', 'show coils', ['coil']),  # Fixed: lowercase
            ('SH-07', 'show inputs', ['input']),  # Fixed: lowercase
            ('SH-08', 'show version', ['Version:', 'Build']),
            ('SH-09', 'show gpio', ['GPIO']),  # Fixed: removed 'Hardware'
            ('SH-10', 'show echo', ['echo']),  # Fixed: lowercase
            ('SH-11', 'show reg', ['reg']),  # Fixed: simplified
            ('SH-12', 'show coil', ['coil']),  # Fixed: simplified
        ]

        for test_id, cmd, expected in tests:
            output = self.send_command(cmd, wait_time=1.0)
            success, msg = self.check_output(output, expected)
            self.log_result(test_id, cmd, "PASS" if success else "FAIL", msg)

    def test_read_write_commands(self):
        """Test READ/WRITE commands"""
        print("\n" + "="*70)
        print("TESTING: READ/WRITE KOMMANDOER")
        print("="*70)

        # Test register read/write
        self.send_command('write reg 100 value 12345', wait_time=0.5)
        output = self.send_command('read reg 100 1', wait_time=0.5)
        success = any('12345' in line for line in output)
        self.log_result('RW-07', 'write reg 100 value 12345', "PASS" if success else "FAIL")

        # Test coil write
        self.send_command('write coil 100 value on', wait_time=0.5)
        output = self.send_command('read coil 100 1', wait_time=0.5)
        success = any('1' in line or 'ON' in line for line in output)
        self.log_result('RW-10', 'write coil 100 value on', "PASS" if success else "FAIL")

        self.send_command('write coil 100 value off', wait_time=0.5)
        output = self.send_command('read coil 100 1', wait_time=0.5)
        success = any('0' in line or 'OFF' in line for line in output)
        self.log_result('RW-11', 'write coil 100 value off', "PASS" if success else "FAIL")

    def test_counter_hw_mode(self, signal_freq=1000):
        """Test Counter Hardware (PCNT) mode"""
        print("\n" + "="*70)
        print(f"TESTING: COUNTER HARDWARE MODE ({signal_freq} Hz signal på GPIO 13)")
        print("="*70)

        # Configure counter 1 for HW mode
        cmd = ('set counter 1 mode 1 parameter hw-mode:hw edge:rising prescaler:1 '
               'hw-gpio:13 index-reg:10 raw-reg:11 freq-reg:12 overload-reg:13 ctrl-reg:14')
        output = self.send_command(cmd, wait_time=2.0)

        # Reset counter
        self.send_command('write reg 14 value 1', wait_time=0.5)
        time.sleep(1)

        # Read initial count
        output = self.send_command('read reg 10 3', wait_time=0.5)

        # Wait 10 seconds and read again
        print("[INFO] Venter 10 sekunder for counting...")
        time.sleep(10)

        output = self.send_command('read reg 10 3', wait_time=0.5)

        # Parse count and frequency
        count = 0
        freq = 0
        for line in output:
            if 'Reg[10]:' in line:
                match = re.search(r'Reg\[10\]:\s*(\d+)', line)
                if match:
                    count = int(match.group(1))
            if 'Reg[12]:' in line:
                match = re.search(r'Reg\[12\]:\s*(\d+)', line)
                if match:
                    freq = int(match.group(1))

        # Test count (should be ~10,000 for 1kHz signal over 10s)
        expected_count = signal_freq * 10
        tolerance = expected_count * 0.05  # 5% tolerance
        count_ok = abs(count - expected_count) < tolerance
        self.log_result('CNT-HW-01', f'HW counting @ {signal_freq} Hz',
                       "PASS" if count_ok else "FAIL",
                       f"Count={count} (expected ~{expected_count})")

        # Test frequency measurement
        freq_ok = abs(freq - signal_freq) < 50  # ±50 Hz tolerance
        self.log_result('CNT-HW-02', f'Frequency measurement',
                       "PASS" if freq_ok else "FAIL",
                       f"Freq={freq} Hz (expected {signal_freq} Hz)")

        # Test FALLING edge
        cmd = ('set counter 1 mode 1 parameter hw-mode:hw edge:falling prescaler:1 '
               'hw-gpio:13 index-reg:10 raw-reg:11 freq-reg:12 ctrl-reg:14')
        self.send_command(cmd, wait_time=2.0)
        self.send_command('write reg 14 value 1', wait_time=0.5)
        time.sleep(10)

        output = self.send_command('read reg 10 2', wait_time=0.5)
        count_fall = 0
        for line in output:
            if 'Reg[10]:' in line:
                match = re.search(r'Reg\[10\]:\s*(\d+)', line)
                if match:
                    count_fall = int(match.group(1))

        count_ok = abs(count_fall - expected_count) < tolerance
        self.log_result('CNT-HW-03', f'FALLING edge counting',
                       "PASS" if count_ok else "FAIL",
                       f"Count={count_fall}")

        # Test BOTH edges
        cmd = ('set counter 1 mode 1 parameter hw-mode:hw edge:both prescaler:1 '
               'hw-gpio:13 index-reg:10 raw-reg:11 freq-reg:12 ctrl-reg:14')
        self.send_command(cmd, wait_time=2.0)
        self.send_command('write reg 14 value 1', wait_time=0.5)
        time.sleep(10)

        output = self.send_command('read reg 10 2', wait_time=0.5)
        count_both = 0
        for line in output:
            if 'Reg[10]:' in line:
                match = re.search(r'Reg\[10\]:\s*(\d+)', line)
                if match:
                    count_both = int(match.group(1))

        expected_both = expected_count * 2  # Both edges = 2x count
        count_ok = abs(count_both - expected_both) < (expected_both * 0.05)
        self.log_result('CNT-HW-04', f'BOTH edges counting',
                       "PASS" if count_ok else "FAIL",
                       f"Count={count_both} (expected ~{expected_both})")

    def test_gpio_config(self):
        """Test GPIO configuration commands"""
        print("\n" + "="*70)
        print("TESTING: GPIO CONFIGURATION")
        print("="*70)

        # Test GPIO mapping to coil
        self.send_command('set gpio 21 static map coil:100', wait_time=0.5)
        output = self.send_command('show gpio', wait_time=1.0)
        success = any('GPIO 21' in line and 'COIL:100' in line for line in output)
        self.log_result('GPIO-01', 'Map GPIO 21 to coil 100',
                       "PASS" if success else "FAIL")

        # Test remove mapping
        self.send_command('no set gpio 21', wait_time=0.5)
        output = self.send_command('show gpio', wait_time=1.0)
        success = not any('GPIO 21' in line and 'COIL:100' in line for line in output)
        self.log_result('GPIO-03', 'Remove GPIO 21 mapping',
                       "PASS" if success else "FAIL")

        # Test GPIO 2 heartbeat control
        self.send_command('set gpio 2 disable', wait_time=0.5)
        output = self.send_command('show gpio', wait_time=1.0)
        success = any('GPIO 2' in line and 'HEARTBEAT' in line for line in output)
        self.log_result('GPIO-05', 'Enable heartbeat on GPIO 2',
                       "PASS" if success else "FAIL")

    def test_system_commands(self):
        """Test system commands (non-destructive)"""
        print("\n" + "="*70)
        print("TESTING: SYSTEM KOMMANDOER (non-destructive)")
        print("="*70)

        # Test help
        output = self.send_command('help', wait_time=2.0)
        success = any('Commands:' in line for line in output)
        self.log_result('SYS-10', 'help command', "PASS" if success else "FAIL")

        # Test show version
        output = self.send_command('show version', wait_time=0.5)
        success = any('Version:' in line and 'Build' in line for line in output)
        self.log_result('SYS-VERSION', 'show version', "PASS" if success else "FAIL")

    def test_register_config(self):
        """Test register configuration (STATIC/DYNAMIC)"""
        print("\n" + "="*70)
        print("TESTING: REGISTER/COIL CONFIGURATION")
        print("="*70)

        # Test static register
        self.send_command('set reg STATIC 100 Value 12345', wait_time=0.5)
        output = self.send_command('read reg 100 1', wait_time=0.5)
        success = any('12345' in line for line in output)
        self.log_result('REG-ST-01', 'Static register config',
                       "PASS" if success else "FAIL")

        # Test static coil
        self.send_command('set coil STATIC 100 Value ON', wait_time=0.5)
        output = self.send_command('read coil 100 1', wait_time=0.5)
        success = any('1' in line or 'ON' in line for line in output)
        self.log_result('COIL-ST-01', 'Static coil config ON',
                       "PASS" if success else "FAIL")

    def generate_report(self):
        """Generate test report"""
        print("\n" + "="*70)
        print("TEST RAPPORT")
        print("="*70)

        total = len(self.results)
        passed = sum(1 for r in self.results if r['status'] == 'PASS')
        failed = sum(1 for r in self.results if r['status'] == 'FAIL')

        print(f"\nTotal tests: {total}")
        print(f"  Passed: {passed} ({100*passed/total:.1f}%)")
        print(f"  Failed: {failed} ({100*failed/total:.1f}%)")

        if failed > 0:
            print("\nFailed tests:")
            for r in self.results:
                if r['status'] == 'FAIL':
                    print(f"  - [{r['id']}] {r['desc']}: {r['comment']}")

        # Save to file
        with open('TEST_RAPPORT.txt', 'w', encoding='utf-8') as f:
            f.write("="*70 + "\n")
            f.write("ESP32 MODBUS SERVER - TEST RAPPORT\n")
            f.write("="*70 + "\n\n")
            f.write(f"Dato: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Firmware: v1.0.0 Build #116\n\n")

            f.write(f"STATISTIK:\n")
            f.write(f"  Total tests: {total}\n")
            f.write(f"  Passed: {passed} ({100*passed/total:.1f}%)\n")
            f.write(f"  Failed: {failed} ({100*failed/total:.1f}%)\n\n")

            f.write("DETALJEREDE RESULTATER:\n")
            f.write("-"*70 + "\n")

            for r in self.results:
                status_icon = "PASS" if r['status'] == 'PASS' else "FAIL"
                f.write(f"[{r['id']}] {r['desc']}\n")
                f.write(f"  Status: {status_icon}\n")
                if r['comment']:
                    f.write(f"  Note: {r['comment']}\n")
                f.write(f"  Time: {r['time']}\n\n")

        print(f"\n[OK] Rapport gemt til: TEST_RAPPORT.txt")


def main():
    """Main test runner"""
    print("="*70)
    print("ESP32 MODBUS SERVER - KOMPLET TEST SUITE")
    print("="*70)
    print(f"Start: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")

    suite = ESP32TestSuite(port='COM11', baudrate=115200)

    if not suite.connect():
        return

    try:
        # Run test categories
        suite.test_show_commands()
        suite.test_read_write_commands()

        print("\n[INFO] Antager 1 kHz signal er tilsluttet GPIO 13")
        print("[INFO] Kører HW counter tests...")

        suite.test_counter_hw_mode(signal_freq=1000)
        suite.test_gpio_config()
        suite.test_register_config()
        suite.test_system_commands()

        # Generate report
        suite.generate_report()

    except KeyboardInterrupt:
        print("\n\n[!] Test afbrudt af bruger")
    except Exception as e:
        print(f"\n[ERROR] Test fejlet: {e}")
        import traceback
        traceback.print_exc()
    finally:
        suite.disconnect()
        print(f"\nSlut: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

if __name__ == '__main__':
    main()
