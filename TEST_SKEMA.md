# ESP32 Modbus Server - Komplet Test Skema

## Test Oversigt
**Firmware:** v1.0.0 Build #116
**Platform:** ESP32-WROOM-32
**Dato:** 2025-11-28

---

## 1. SHOW KOMMANDOER

| Test ID | Kommando | Forventet Output | Status | Kommentar |
|---------|----------|------------------|--------|-----------|
| SH-01 | `show config` | Version, slave ID, baud, counters, timers, GPIO | ⏳ | |
| SH-02 | `show counters` | Counter status (4 counters) | ⏳ | |
| SH-03 | `show timers` | Timer status (4 timers) | ⏳ | |
| SH-04 | `show registers` | Register dump (start 0, count default) | ⏳ | |
| SH-05 | `show registers 10 5` | Registers 10-14 | ⏳ | |
| SH-06 | `show coils` | Coil states | ⏳ | |
| SH-07 | `show inputs` | Discrete input states | ⏳ | |
| SH-08 | `show version` | Firmware version, build #, git hash | ⏳ | |
| SH-09 | `show gpio` | GPIO mappings (hardware + user) | ⏳ | |
| SH-10 | `show echo` | Remote echo status (on/off) | ⏳ | |
| SH-11 | `show reg` | Register configuration (STATIC/DYNAMIC) | ⏳ | |
| SH-12 | `show coil` | Coil configuration (STATIC/DYNAMIC) | ⏳ | |

---

## 2. READ/WRITE KOMMANDOER

| Test ID | Kommando | Forventet Output | Status | Kommentar |
|---------|----------|------------------|--------|-----------|
| RW-01 | `read reg 10 5` | Registers 10-14 værdier | ⏳ | |
| RW-02 | `read reg 100 1` | Register 100 værdi | ⏳ | |
| RW-03 | `read coil 0 10` | Coils 0-9 states | ⏳ | |
| RW-04 | `read coil 100 1` | Coil 100 state | ⏳ | |
| RW-05 | `read input 0 10` | Discrete inputs 0-9 | ⏳ | |
| RW-06 | `read input 32 1` | Discrete input 32 | ⏳ | |
| RW-07 | `write reg 100 value 12345` | Reg 100 = 12345 | ⏳ | |
| RW-08 | `write reg 100 value 0` | Reg 100 = 0 | ⏳ | |
| RW-09 | `write reg 100 value 65535` | Reg 100 = 65535 (max) | ⏳ | |
| RW-10 | `write coil 100 value on` | Coil 100 = ON | ⏳ | |
| RW-11 | `write coil 100 value off` | Coil 100 = OFF | ⏳ | |

---

## 3. COUNTER MODES - HARDWARE (PCNT)

| Test ID | Konfiguration | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|---------------|---------------|-------------------|--------|-----------|
| CNT-HW-01 | GPIO 13, RISING, 1kHz | Tæl i 10s | Count ≈ 10,000 | ✅ | PASSED - 10,001 counts |
| CNT-HW-02 | GPIO 13, RISING, 1kHz | Frekvens måling | Freq = 1000 Hz | ✅ | PASSED - 1000 Hz |
| CNT-HW-03 | GPIO 13, FALLING, 1kHz | Tæl falling edges | Count ≈ 10,000 | ⏳ | |
| CNT-HW-04 | GPIO 13, BOTH, 1kHz | Tæl begge edges | Count ≈ 20,000 | ⏳ | |
| CNT-HW-05 | GPIO 13, prescaler:4 | Division test | Count = raw/4 | ⏳ | |
| CNT-HW-06 | GPIO 13, scale:2.5 | Scaling test | Count = raw×2.5 | ⏳ | |
| CNT-HW-07 | GPIO 13, DIR:DOWN | Down counting | Count decreases | ⏳ | |
| CNT-HW-08 | GPIO 13, start:1000 | Start value | Initial = 1000 | ⏳ | |
| CNT-HW-09 | GPIO 13, bit-width:16 | Overflow test | Wraps at 65535 | ⏳ | |
| CNT-HW-10 | Control: reset bit | Reset via reg | Count = start_value | ⏳ | |
| CNT-HW-11 | Control: start bit | Start counting | Counter runs | ⏳ | |
| CNT-HW-12 | Control: stop bit | Stop counting | Counter pauses | ⏳ | |

**Hardware Fix Verification:**
- ✅ Division-med-2 bug FIXED (FIX P0.7)
- ✅ PCNT tæller korrekt single edges
- ✅ Frekvens måling præcis (1000 Hz)

---

## 4. COUNTER MODES - SOFTWARE ISR

| Test ID | Konfiguration | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|---------------|---------------|-------------------|--------|-----------|
| CNT-ISR-01 | GPIO 13, RISING, ISR | Tæl via interrupt | Count stiger | ⏳ | |
| CNT-ISR-02 | GPIO 13, FALLING, ISR | Falling edge ISR | Count stiger | ⏳ | |
| CNT-ISR-03 | GPIO 13, debounce:on | Debounce test | Stabilt ved bounce | ⏳ | |
| CNT-ISR-04 | GPIO 13, debounce:off | No debounce | Tæller alle edges | ⏳ | |
| CNT-ISR-05 | GPIO 13, debounce-ms:50 | 50ms debounce | Ignorer <50ms | ⏳ | |
| CNT-ISR-06 | Start/stop via ctrl | Control test | ISR enable/disable | ⏳ | |

---

## 5. COUNTER MODES - SOFTWARE POLLING

| Test ID | Konfiguration | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|---------------|---------------|-------------------|--------|-----------|
| CNT-SW-01 | input-dis:32, RISING | SW polling | Count stiger | ⏳ | |
| CNT-SW-02 | input-dis:32, FALLING | Falling polling | Count stiger | ⏳ | |
| CNT-SW-03 | Low freq (<10 Hz) | Slow signal | Korrekt count | ⏳ | |
| CNT-SW-04 | Debounce test | Bounce handling | Stabilt count | ⏳ | |

---

## 6. TIMER MODES

### Mode 1: One-Shot (3-fase timing)

| Test ID | Konfiguration | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|---------------|---------------|-------------------|--------|-----------|
| TIM-01-01 | Phase1: 1s HIGH | Trigger → fase 1 | Output HIGH i 1s | ⏳ | |
| TIM-01-02 | Phase2: 2s LOW | Fase 1 → fase 2 | Output LOW i 2s | ⏳ | |
| TIM-01-03 | Phase3: 1s HIGH | Fase 2 → fase 3 | Output HIGH i 1s | ⏳ | |
| TIM-01-04 | Complete cycle | 3 faser komplet | Return to idle | ⏳ | |

### Mode 2: Monostable (retriggerable pulse)

| Test ID | Konfiguration | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|---------------|---------------|-------------------|--------|-----------|
| TIM-02-01 | Pulse: 5s | Single trigger | 5s pulse | ⏳ | |
| TIM-02-02 | Pulse: 5s | Retrigger under pulse | Restart 5s | ⏳ | |
| TIM-02-03 | Trigger level | Level vs edge | Correct trigger | ⏳ | |

### Mode 3: Astable (blink/toggle)

| Test ID | Konfiguration | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|---------------|---------------|-------------------|--------|-----------|
| TIM-03-01 | ON:1s, OFF:1s | Continuous run | 1 Hz toggle | ⏳ | |
| TIM-03-02 | ON:500ms, OFF:500ms | Fast blink | 1 Hz toggle | ⏳ | |
| TIM-03-03 | ON:2s, OFF:1s | Asymmetric | 66% duty cycle | ⏳ | |

### Mode 4: Input-Triggered

| Test ID | Konfiguration | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|---------------|---------------|-------------------|--------|-----------|
| TIM-04-01 | input-dis:32, delay:2s | Input → delay → output | 2s delay | ⏳ | |
| TIM-04-02 | Edge: rising | Trigger on rising | Correct edge | ⏳ | |
| TIM-04-03 | Edge: falling | Trigger on falling | Correct edge | ⏳ | |

---

## 7. GPIO CONFIGURATION

| Test ID | Kommando | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|----------|---------------|-------------------|--------|-----------|
| GPIO-01 | `set gpio 21 static map coil:100` | Map GPIO21 → coil 100 | GPIO follows coil | ⏳ | |
| GPIO-02 | `set gpio 4 static map input:32` | Map GPIO4 → input 32 | Input reads GPIO | ⏳ | |
| GPIO-03 | `no set gpio 21` | Remove mapping | GPIO unmapped | ⏳ | |
| GPIO-04 | `set gpio 2 enable` | Release heartbeat | GPIO2 available | ⏳ | |
| GPIO-05 | `set gpio 2 disable` | Enable heartbeat | LED blinks | ⏳ | |
| GPIO-06 | `show gpio` | Display mappings | All mappings shown | ⏳ | |

---

## 8. REGISTER/COIL CONFIGURATION

### Static Registers

| Test ID | Kommando | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|----------|---------------|-------------------|--------|-----------|
| REG-ST-01 | `set reg STATIC 100 Value 12345` | Set static reg | Reg 100 = 12345 | ⏳ | |
| REG-ST-02 | `read reg 100 1` | Read static | Value = 12345 | ⏳ | |
| REG-ST-03 | `show reg` | Show config | Static reg listed | ⏳ | |

### Dynamic Registers

| Test ID | Kommando | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|----------|---------------|-------------------|--------|-----------|
| REG-DY-01 | `set reg DYNAMIC 100 counter1:index` | Counter value → reg | Reg tracks counter | ⏳ | |
| REG-DY-02 | `set reg DYNAMIC 101 counter1:freq` | Counter freq → reg | Reg = frequency | ⏳ | |
| REG-DY-03 | `set reg DYNAMIC 102 counter1:overflow` | Overflow flag → reg | Reg = 0/1 | ⏳ | |
| REG-DY-04 | `set reg DYNAMIC 103 timer1:output` | Timer output → reg | Reg = timer state | ⏳ | |

### Static Coils

| Test ID | Kommando | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|----------|---------------|-------------------|--------|-----------|
| COIL-ST-01 | `set coil STATIC 100 Value ON` | Set static coil ON | Coil 100 = 1 | ⏳ | |
| COIL-ST-02 | `set coil STATIC 100 Value OFF` | Set static coil OFF | Coil 100 = 0 | ⏳ | |
| COIL-ST-03 | `show coil` | Show config | Static coil listed | ⏳ | |

### Dynamic Coils

| Test ID | Kommando | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|----------|---------------|-------------------|--------|-----------|
| COIL-DY-01 | `set coil DYNAMIC 100 counter1:overflow` | Overflow → coil | Coil = overflow | ⏳ | |
| COIL-DY-02 | `set coil DYNAMIC 101 timer1:output` | Timer → coil | Coil = timer state | ⏳ | |

---

## 9. SYSTEM KOMMANDOER

| Test ID | Kommando | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|----------|---------------|-------------------|--------|-----------|
| SYS-01 | `set hostname esp32-test` | Change hostname | Hostname updated | ⏳ | |
| SYS-02 | `set baud 115200` | Change baudrate | Baud updated (reboot) | ⏳ | |
| SYS-03 | `set id 42` | Change slave ID | Slave ID = 42 | ⏳ | |
| SYS-04 | `set echo on` | Enable echo | Echo enabled | ⏳ | |
| SYS-05 | `set echo off` | Disable echo | Echo disabled | ⏳ | |
| SYS-06 | `save` | Save config to NVS | Config saved | ⏳ | |
| SYS-07 | `load` | Load config from NVS | Config loaded | ⏳ | |
| SYS-08 | `defaults` | Reset to defaults | Factory reset | ⏳ | |
| SYS-09 | `reboot` | Restart ESP32 | System reboots | ⏳ | |
| SYS-10 | `help` | Show help text | Help displayed | ⏳ | |

---

## 10. COUNTER CONTROL

| Test ID | Kommando | Test Scenario | Forventet Resultat | Status | Kommentar |
|---------|----------|---------------|-------------------|--------|-----------|
| CTL-01 | `reset counter 1` | Reset single counter | Counter = start_value | ⏳ | |
| CTL-02 | `clear counters` | Reset all counters | All counters reset | ⏳ | |
| CTL-03 | `set counter 1 control reset-on-read:on` | Enable reset-on-read | Reset after Modbus read | ⏳ | |
| CTL-04 | `set counter 1 control auto-start:on` | Enable auto-start | Counter starts at boot | ⏳ | |
| CTL-05 | `set counter 1 control running:on` | Start counter | Counter runs | ⏳ | |
| CTL-06 | `set counter 1 control running:off` | Stop counter | Counter pauses | ⏳ | |

---

## TEST STATUS LEGEND

- ✅ **PASSED** - Test udført og bestået
- ❌ **FAILED** - Test fejlet
- ⚠️ **PARTIAL** - Delvist bestået / advarsler
- ⏳ **PENDING** - Afventer test
- 🚫 **SKIPPED** - Ikke relevant / sprunget over

---

## SAMLET STATISTIK

| Kategori | Total Tests | Passed | Failed | Pending |
|----------|-------------|--------|--------|---------|
| SHOW Kommandoer | 12 | 0 | 0 | 12 |
| READ/WRITE | 11 | 0 | 0 | 11 |
| Counter HW Mode | 12 | 2 | 0 | 10 |
| Counter ISR Mode | 6 | 0 | 0 | 6 |
| Counter SW Mode | 4 | 0 | 0 | 4 |
| Timer Mode 1 | 4 | 0 | 0 | 4 |
| Timer Mode 2 | 3 | 0 | 0 | 3 |
| Timer Mode 3 | 3 | 0 | 0 | 3 |
| Timer Mode 4 | 3 | 0 | 0 | 3 |
| GPIO Config | 6 | 0 | 0 | 6 |
| Register/Coil Config | 11 | 0 | 0 | 11 |
| System Commands | 10 | 0 | 0 | 10 |
| Counter Control | 6 | 0 | 0 | 6 |
| **TOTAL** | **91** | **2** | **0** | **89** |

**Test Coverage:** 2.2% (2/91)

---

## NOTER

1. **PCNT Hardware Fix (FIX P0.7)** er verified og working
2. Counter HW mode tests CNT-HW-01 og CNT-HW-02 er PASSED
3. Resterende tests skal køres systematisk

---

**Genereret:** 2025-11-28
**Test Engineer:** Claude Code
**Firmware:** v1.0.0 Build #116
