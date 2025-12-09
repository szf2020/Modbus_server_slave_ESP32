# Modbus RTU Server (ESP32)

**Versionering:** v3.2.0 | **Status:** Production-Ready | **Platform:** ESP32-WROOM-32

A complete, modular **Modbus RTU Server** implementation for ESP32-WROOM-32 microcontroller with advanced features including ST Structured Text Logic programming, Wi-Fi networking, and telnet CLI interface.

---

## 🚀 Features

### Core Modbus RTU
- **Full Modbus RTU Protocol Support** (FC01-10)
- **Configurable Slave ID & Baudrate** (300-115200 bps)
- **160 Holding Registers + 160 Input Registers** (0-159)
- **Dynamic Coil/Discrete Input Management** (bit-addressable)
- **CRC16 Validation** on all frames
- **Hardware RS-485 Support** with GPIO15 direction control

### Counter & Timer Engines
- **4 Independent Counters** with three operating modes:
  - Software Polling (GPIO polling at main loop rate)
  - Software ISR (GPIO interrupt, INT0-5)
  - Hardware PCNT (Pulse Counter Unit with 32-bit precision)
- **4 Independent Timers** with four modes:
  - Mode 1: One-shot (3-phase sequence)
  - Mode 2: Monostable (retriggerable pulse)
  - Mode 3: Astable (oscillator/blink)
  - Mode 4: Input-triggered (edge detection)
- **Prescaler Support** (unified division across all modes)
- **Frequency Measurement** (0-20kHz, updated ~1sec)
- **Compare Feature** (threshold detection with auto-reset)

### ST Logic Programming
- **4 Independent Logic Programs** with Structured Text syntax
- **Real-time Bytecode Compilation** (no interpreter overhead)
- **Variable I/O Binding** to Modbus registers
- **Full ST Language Support** (VAR, BEGIN/END, loops, conditionals)
- **Automatic Register Mapping** (input/output/persistent)

### Networking (v3.0+)
- **Wi-Fi Client Mode** (DHCP or static IP)
- **Telnet Server** on port 23 (default)
- **Remote Authentication** (username/password)
- **MDNS Support** (service discovery)
- **Configuration Persistence** via NVS (non-volatile storage)

### CLI Interface
- **Serial Console** (115200 bps USB UART0)
- **Telnet Console** (port 23, arrow key support, history)
- **160+ CLI Commands** (set, show, read, write, save, load)
- **Command History** with arrow keys (↑↓)
- **Real-time Status** (counters, timers, registers, Wi-Fi)

### System Features
- **Persistent Configuration** (NVS storage with CRC validation)
- **Schema Migration** (automatic backward compatibility)
- **GPIO2 Heartbeat** (LED blink indicator, user-controllable)
- **Build System** (PlatformIO with auto-versioning)
- **Debug Flags** (selective debug output)

---

## 📋 Hardware Requirements

| Component | Specification |
|-----------|---|
| **MCU** | ESP32-WROOM-32 (dual-core 240MHz) |
| **RAM** | 320KB available (vs 8KB on Mega2560) |
| **Flash** | 4MB (vs 256KB on Mega2560) |
| **RS-485** | External MAX485 module (GPIO4/5/15) |
| **UART** | UART0 (USB), UART1 (Modbus), UART2 (available) |
| **Power** | 3.3V logic (USB power or external) |

### Pin Configuration
```
GPIO4   ← UART1 RX (Modbus)
GPIO5   ← UART1 TX (Modbus)
GPIO15  ← RS-485 DIR control (output)
GPIO16  ← Available (future)
GPIO17  ← Available (future)
GPIO18  ← Available (future)
GPIO19  ← PCNT unit0 input (HW counter mode)
GPIO21  ← SDA (future I2C)
GPIO22  ← SCL (future I2C)
GPIO2   ← Heartbeat LED (configurable)
```

---

## 🔧 Installation & Build

### Prerequisites
- [PlatformIO Core](https://platformio.org/install/cli)
- Python 3.x
- Git

### Clone & Build
```bash
git clone https://github.com/Jangreenlarsen/Modbus_server_slave_ESP32.git
cd Modbus_server_slave_ESP32

# Build firmware
pio run

# Upload to ESP32
pio run -t upload

# Monitor serial output
pio device monitor -b 115200
```

### Configuration
Edit `platformio.ini`:
```ini
[env:esp32]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
```

---

## 📖 Usage

### Serial CLI (USB)
Connect via serial terminal (115200 bps):
```bash
> show config
> set id 14
> set baud 19200
> set hostname my-esp32
> save
```

### Telnet CLI
```bash
telnet 192.168.1.100 23
Username: (default: empty)
Password: (default: empty)
> show counters
> set timer 1 mode 3 on-ms:1000 off-ms:500
> show timer 1
```

### Essential Commands
```bash
# Configuration
show config              # View all settings
show version            # Show firmware version
save                    # Persist config to NVS

# Monitoring
show counters           # All counters status
show counter <id>       # Specific counter (1-4)
show timers             # All timers status
show timer <id>         # Specific timer (1-4)
show registers          # Holding registers (0-159)
show inputs             # Input registers (0-159)
show coils              # Coil status
show gpio               # GPIO mappings
show wifi               # Wi-Fi connection status

# Configuration
set id <slave_id>       # Modbus slave ID
set baud <rate>         # Baudrate (300-115200)
set hostname <name>     # System hostname
set echo <on|off>       # Remote echo toggle
set counter 1 mode 1    # Configure counter 1 (hardware mode)
set timer 1 mode 3      # Configure timer 1 (astable/blink)
set wifi ssid mynet     # Set Wi-Fi SSID
set wifi password pass  # Set Wi-Fi password
set wifi ip 192.168.1.100 # Static IP
```

### ST Logic Programming
```bash
# Upload program
set logic 1 upload
VAR
  counter : INT;
  output : INT;
END_VAR

BEGIN
  counter := counter + 1;
  IF counter > 100 THEN
    output := 1;
  END_IF;
END
<blank line to finish>

# Compile & run
set logic 1 enable on
show logic 1
show logic 1 code    # Show compiled bytecode
```

---

## 📁 Project Structure

```
Modbus_server_slave_ESP32/
├── README.md                      # This file
├── CHANGELOG.md                   # Version history
├── CLAUDE.md                      # Developer guidelines
├── platformio.ini                 # Build configuration
├── build_number.txt               # Auto-generated build number
│
├── include/                       # Header files
│   ├── types.h                   # Struct definitions
│   ├── constants.h               # All #defines and enums
│   ├── config_struct.h           # Configuration persistence
│   ├── modbus_*.h                # Modbus protocol layer
│   ├── counter_*.h               # Counter engine
│   ├── timer_*.h                 # Timer engine
│   ├── cli_*.h                   # CLI interface
│   ├── st_*.h                    # ST Logic compiler
│   ├── gpio_driver.h             # GPIO abstraction
│   ├── uart_driver.h             # UART abstraction
│   └── nvs_driver.h              # NVS persistence
│
├── src/                           # Implementation files
│   ├── main.cpp                  # Entry point
│   ├── modbus_*.cpp              # Modbus protocol
│   ├── counter_*.cpp             # Counter implementation
│   ├── timer_*.cpp               # Timer implementation
│   ├── cli_*.cpp                 # CLI commands & parser
│   ├── st_*.cpp                  # ST Logic engine
│   ├── config_*.cpp              # Configuration management
│   └── *.cpp                     # Other subsystems
│
├── docs/                          # Documentation
│   ├── README_ST_LOGIC.md        # ST Logic guide
│   └── ARCHIVED/                 # Old reports & analysis
│
├── TEST/                          # Test scripts & utilities
│   ├── *.py                      # Python test scripts
│   └── *.txt                     # Test outputs
│
└── .platformio/                   # Build artifacts (ignored)
```

---

## 🔍 Architecture

### Layered Design (8 Layers)

**Layer 0: Drivers** - Hardware abstraction (GPIO, UART, PCNT, NVS)
**Layer 1: Protocol** - Modbus frame parsing & serialization
**Layer 2: Function Codes** - FC01-10 implementations
**Layer 3: Server** - Main Modbus state machine
**Layer 4: Storage** - Register/coil arrays with access functions
**Layer 5: Engines** - Counter & timer logic
**Layer 6: Persistence** - Config save/load/apply
**Layer 7: UI** - CLI & telnet interface

### Key Design Principles
- ✅ **Modular**: 30+ files, each with single responsibility
- ✅ **No circular dependencies**: Each module independently testable
- ✅ **CRC-protected persistence**: Data integrity guaranteed
- ✅ **Schema versioning**: Backward-compatible migrations
- ✅ **Dual-core friendly**: Core0 = FreeRTOS, Core1 = application

---

## 🧪 Testing

### Run Unit Tests (local)
```bash
pio test
```

### Run Integration Tests (on hardware)
1. Upload firmware
2. Connect serial monitor
3. Run CLI commands
4. Verify via Modbus master

### Test Scripts (Python)
Located in `TEST/` folder:
```bash
# Examples
python TEST/test_counter.py        # Test counter functionality
python TEST/test_timer_debug.py    # Test timer modes
python TEST/test_logic_mode.py     # Test ST Logic
python TEST/test_suite_complete.py # Full integration test
```

---

## 📊 Specification

| Feature | Value |
|---------|-------|
| **Modbus Function Codes** | FC01-06, FC0F-10 (full suite) |
| **Holding Registers** | 160 (0-159) |
| **Input Registers** | 160 (0-159) |
| **Coils** | 256 (0-255, bit-addressable) |
| **Discrete Inputs** | 256 (0-255, bit-addressable) |
| **Counters** | 4 (3 modes each) |
| **Timers** | 4 (4 modes each) |
| **ST Logic Programs** | 4 (independent, ~2KB each) |
| **GPIO Mappings** | 64 (GPIO + ST variables) |
| **Max Baudrate** | 115200 bps |
| **Response Time** | <100ms (typical) |
| **RAM Usage** | ~120KB / 327KB available |
| **Flash Usage** | ~835KB / 1310KB available |

---

## 🔐 Security Considerations

- **Authentication**: Telnet username/password support
- **Persistence**: CRC16 validates all saved data
- **Schema Migration**: Automatic data corruption detection
- **Default Config**: Safe defaults applied on schema mismatch
- **UART Isolation**: Modbus on UART1, debug on UART0

⚠️ **Not suitable for:** Internet-exposed devices (no encryption), critical infrastructure (add watchdog)

---

## 📝 Version History

- **v3.2.0** (2025-12-09) - CLI Commands Complete + Persistent Settings
  - `show counter <id>` and `show timer <id>` detail views
  - `set hostname` and `set echo` now persistent
  - Schema v7 with auto-migration

- **v3.1.1** (2025-12-08) - Telnet Insert Mode & ST Upload Copy/Paste
  - Telnet text editing in insert mode
  - Fixed multi-line copy/paste into ST Logic upload

- **v3.1.0** (2025-12-05) - Wi-Fi Display & Telnet Auth
  - Enhanced `show config` with Wi-Fi section
  - Improved telnet authentication

- **v3.0.0** (2025-12-02) - Telnet Server & Console Layer
  - Telnet CLI on port 23
  - Console abstraction (Serial/Telnet unified)

See [CHANGELOG.md](CHANGELOG.md) for full history.

---

## 🤝 Contributing

1. **Code Standards**
   - Use SemVer for version bumps (MAJOR.MINOR.PATCH)
   - Follow existing code style (K&R with 2-space indents)
   - Document schema changes in CHANGELOG.md

2. **Adding Features**
   - Create new .cpp/.h files in appropriate layer
   - No circular dependencies (review architecture)
   - Add CLI commands in `src/cli_commands.cpp`
   - Update version number in `include/constants.h`

3. **Testing**
   - Run `pio run` to build
   - Test on hardware if adding driver changes
   - Update CHANGELOG.md
   - Commit with `git tag -a vX.Y.Z`

---

## 📞 Support & Issues

- **GitHub Issues**: Report bugs & feature requests
- **Documentation**: See `docs/` folder and CLAUDE.md
- **Serial Console**: Use `show debug` for diagnostics

---

## 📄 License

[Specify your license here, e.g., MIT, GPL-3.0, proprietary]

---

## 🙏 Acknowledgments

- Original Mega2560 v3.6.5 architecture (reference implementation)
- ESP32-WROOM-32 RTOS & hardware documentation
- PlatformIO framework & toolchain
- Modbus RTU specification (IEC 61131-3)

---

**Last Updated:** 2025-12-09 | **Maintained by:** Jan Green Larsen
