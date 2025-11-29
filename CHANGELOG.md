# Changelog - ESP32 Modbus RTU Server

All notable changes to this project are documented in this file.

---

## [1.0.0] - 2025-11-29 🎉

### FEATURES

#### New: Timer Mode 4 (Input-Triggered) ✨
- **Rising edge detection** (trigger-edge:1): Responds to 0→1 transitions
- **Falling edge detection** (trigger-edge:0): Responds to 1→0 transitions
- **Configurable delay** (delay-ms): Wait before setting output
- **COIL-based input**: Use Modbus COILs as input source for testing
- **Output latch behavior**: Output stays HIGH once triggered
- **Proper state initialization**: No false edges on timer configuration
- **Code:** `src/timer_engine.cpp:152-216`, `src/cli_commands.cpp:391-395`
- **Test:** `test_mode4_comprehensive.py` - [PASS]

#### Timer Modes (Complete)
- **Mode 1:** One-shot (3-phase sequence)
- **Mode 2:** Monostable (retriggerable pulse)
- **Mode 3:** Astable (oscillator/blink) - Verified working
- **Mode 4:** Input-triggered (NEW - verified working)

#### Counter Modes (Complete)
- **Mode 1:** SW (Software Polling) ~10kHz
- **Mode 2:** SW-ISR (GPIO Interrupt) ~100kHz
- **Mode 3:** HW (PCNT Hardware) >1MHz

#### Advanced Features
- **Prescaler:** 1, 4, 8, 16, 64, 256, 1024
- **Scale factor:** Float multiplikator for units
- **Bit-width clamping:** 8, 16, 32, 64-bit support
- **Frequency measurement:** 0-20kHz with 1-sec window
- **Control register:** Start/stop/reset via Modbus
- **PCNT overflow handling:** 16-bit HW → 64-bit accumulator

#### Persistence
- **NVS Storage:** All configs persist across reboot
- **Schema versioning:** v2 with v1 migration support
- **CRC16 validation:** Data integrity check
- **Auto-save:** After counter/timer configuration

#### CLI Commands
- **70+ commands:** Set/show for timers, counters, registers, coils
- **GPIO mapping:** Static cortlægning (persistent)
- **Persistence:** save, load, defaults, reboot
- **System:** hostname, baud, id, echo

#### GPIO Features
- **Physical GPIO:** 0-39 (direct ESP32 pins)
- **Virtual GPIO:** 100-255 (read/write COIL mapping)
- **GPIO2 Heartbeat:** 500ms blink, toggleable
- **STATIC Mapping:** GPIO ↔ Discrete input/Coil

#### Register & Coil Management
- **STATIC registers:** 16 max (persistent)
- **DYNAMIC registers:** 16 max (auto-updated)
- **STATIC coils:** 16 max (persistent)
- **DYNAMIC coils:** 16 max (auto-updated)
- **Multi-word support:** 32/64-bit across registers

---

## [1.0.0-rc1] - 2025-11-28

### FIXED

#### P0.7: PCNT Division-by-2 Bug
- **Issue:** PCNT was dividing counter by 2 (incorrect)
- **Root cause:** Misunderstanding of PCNT hardware behavior
- **Fix:** Removed division - PCNT counts 100% correctly
- **Code:** `src/counter_hw.cpp`
- **Impact:** Counter Mode 3 now accurate

#### CNT-HW-08: Race Condition (Gate-based sync)
- **Issue:** PCNT polling task interfered with counter reads
- **Root cause:** Async poll updates during Modbus reads
- **Fix:** Implement pause/resume gates for atomic access
- **Code:** `src/counter_engine.cpp`, `src/pcnt_driver.cpp`
- **Impact:** Stable counter readings under load

#### Timer Mode 3 Test Suite Failure
- **Issue:** test_suite_extended reports [0,0,0,0,0] for Mode 3
- **Root cause:** Async serial reader race condition in test
- **Status:** NOT A FIRMWARE BUG - Manual tests prove Mode 3 works
- **Verification:** `test_timer_exact.py`, `test_coil_raw.py` - [PASS]

---

## [0.9.0] - 2025-11-15

### REFACTORING

#### Architecture Overhaul (Mega2560 → ESP32)
- **Modular structure:** 30+ focused .cpp files (100-250 lines each)
- **Layer-based design:** 8 clear abstraction layers
- **Hardware drivers:** Explicit abstraction (gpio, uart, pcnt, nvs)
- **Separation of concerns:** Config load/save/apply are separate

#### Module Structure
```
Layer 0: Hardware Abstraction (gpio_driver, uart_driver, pcnt_driver, nvs_driver)
Layer 1: Protocol Core (modbus_frame, modbus_parser, modbus_serializer)
Layer 2: Function Codes (modbus_fc_read, modbus_fc_write, modbus_fc_dispatch)
Layer 3: Modbus Runtime (modbus_rx, modbus_tx, modbus_server)
Layer 4: Storage (registers, coils, gpio_mapping)
Layer 5: Feature Engines (counter_*, timer_*)
Layer 6: Persistence (config_load, config_save, config_apply)
Layer 7: CLI (cli_parser, cli_commands, cli_show, cli_shell)
Layer 8: System (heartbeat, version, debug, main)
```

---

## [0.8.5] - Previous Mega2560 v3.6.5

### ORIGINAL FEATURES (Ported to ESP32)
- Modbus RTU (FC01-10)
- 4 Timers (Modes 1-3)
- 4 Counters (SW, SW-ISR, HW)
- EEPROM persistence
- CLI shell
- RS-485 support

---

## Development Notes

### Testing Status
- ✅ Mode 3: Astable/Blink verified working
- ✅ Mode 4: Input-triggered [PASS] both rise/fall
- ✅ Counter: All modes operational
- ✅ Persistence: Save/load/defaults working
- ✅ Extended suite: 10/11 pass (1 known async race)

### Known Limitations
1. **Async test suite race:** test_suite_extended shows false negatives for Mode 3
   - Workaround: Run manual timing tests for verification
   - Root cause: Unbuffered serial reader in test framework

2. **GPIO signal requirement:** Counter tests require actual signal on GPIO
   - Workaround: Use test simulation or COIL-based input

3. **UART1 only:** Modbus RTU on UART1 (GPIO4/5)
   - Design choice: GPIO0, GPIO2, GPIO15 are strapping pins

---

## Build Information

### RAM Usage
- Typical: ~21KB / 327KB (6.5%)
- Headroom for future features: ~300KB available

### Flash Usage
- Firmware: ~262KB / 1.3MB usable
- Headroom: ~1MB available for features/data

### Dependencies
- ESP32-Arduino core (v2.0.0+)
- PlatformIO (v6.0+)
- No external libraries required (zero dependencies)

---

## Documentation Updates (v1.0.0)

### NEW
- **FEATURE_GUIDE.md:** Complete reference for all features
  - All Timer Modes (1-4) with examples
  - All Counter modes with detailed specs
  - 70+ CLI commands documented
  - GPIO mapping, persistence, system commands
  - Comprehensive examples section

### UPDATED
- **SETUP_GUIDE.md:** Added Quick Start Examples
  - Timer Mode 3 blink test
  - Timer Mode 4 input-triggered test
  - Configuration save/load
  - Hardware counter setup

### EXISTING
- **CLAUDE.md:** Architecture & development guide (maintained)
- **ESP32_Module_Architecture.md:** Module structure (up-to-date)
- **ESP32_Migration_Analysis.md:** Mega2560→ESP32 notes

---

## Future Roadmap

### Planned Features
- [ ] Ethernet (W5500) integration
- [ ] Analog input support (ADC)
- [ ] PWM output (Mode 5 timer)
- [ ] More counter modes (differential, encoder)
- [ ] Real-time clock (RTC)
- [ ] Advanced filtering (moving average, etc)

### Research Phase
- [ ] I2C slave mode
- [ ] Multi-UART support (UART2)
- [ ] SD card logging
- [ ] JSON configuration import/export

---

## Contributors

- **Primary Developer:** Jan Grønlund Larsen
- **Architecture:** Modular redesign of Mega2560 project
- **Testing:** Comprehensive test suite with Python automation

---

## License

[Your License Here]

---

## Support & Resources

- **GitHub:** https://github.com/Jangreenlarsen/Modbus_server_slave_ESP32
- **Documentation:** `docs/` folder (FEATURE_GUIDE.md, SETUP_GUIDE.md)
- **Architecture:** CLAUDE.md (project development guide)
- **Tests:** Test scripts in root (`test_*.py`)

---

## Version History Timeline

```
v1.0.0      (2025-11-29) Release - Timer Mode 4 complete
v1.0.0-rc1  (2025-11-28) Release Candidate 1
v0.9.0      (2025-11-15) Major refactor (30+ modules)
v0.8.5      (2025-10-xx) Ported from Mega2560 v3.6.5
```

---

**Last Updated:** 2025-11-29 | **Status:** Production Ready ✅
