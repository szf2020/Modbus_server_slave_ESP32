# ESP32 Counter System - Bug Fix & Test Rapport

**Projekt:** Modbus RTU Server ESP32
**Version:** 1.0.0 Build #80
**Dato:** 2025-11-21
**Status:** 🟡 AFVENTER GODKENDELSE

---

## 📋 EXECUTIVE SUMMARY

Denne rapport dokumenterer systematisk fix af **12 kritiske bugs** i ESP32 counter systemet samt komplet testplan for verifikation med 5 kHz test signaler på GPIO 13 og 19.

**Hovedresultater:**
- ✅ Alle 12 bugs identificeret og fixet
- ✅ 79 builds kompileret succesfuldt
- ✅ Firmware uploadet til ESP32 (Build #80)
- 🟡 Afventer manuel test og godkendelse

---

## 🐛 BUGS FIXET (1.1 - 3.1)

### Kategori 1: Funktionalitet Mangler

| Bug | Beskrivelse | Status | Build | Severity |
|-----|-------------|--------|-------|----------|
| **1.1** | Overflow tracking mangler i SW mode | ✅ Fixed | #69 | HIGH |
| **1.2** | Overflow tracking mangler i SW-ISR mode | ✅ Fixed | #70 | HIGH |
| **1.3** | PCNT int16_t limitation ikke håndteret | ✅ Fixed | #71 | CRITICAL |
| **1.4** | HW mode delta-beregning wrap handling | ✅ Fixed | #71 | CRITICAL |
| **1.5** | Direction (UP/DOWN) ikke implementeret | ✅ Fixed | #72 | HIGH |
| **1.6** | Start value ikke anvendt ved init | ✅ Fixed | #73 | MEDIUM |
| **1.7** | Debounce logic forkert i SW mode | ✅ Fixed | #74 | MEDIUM |
| **1.8** | Frequency 64-bit overflow | ✅ Fixed | #75 | HIGH |
| **1.9** | HW mode GPIO hardcoding | ✅ Fixed | #76 | HIGH |

### Kategori 2: Validation Mangler

| Bug | Beskrivelse | Status | Build | Severity |
|-----|-------------|--------|-------|----------|
| **2.3** | Bit width validation mangler upper bound | ✅ Fixed | #77 | MEDIUM |
| **2.1** | Control register bits ikke implementeret | ✅ Fixed | #79 | HIGH |

### Kategori 3: Race Conditions

| Bug | Beskrivelse | Status | Build | Severity |
|-----|-------------|--------|-------|----------|
| **3.1** | ISR counter state ikke volatile | ✅ Fixed | #78 | CRITICAL |

---

## 🔧 TEKNISKE FIXES

### Bug 1.1 & 1.2: Overflow Tracking

**Problem:** SW og SW-ISR modes havde ingen overflow detection.

**Fix:**
- Tilføjet `overflow_flag` til `CounterSWState` struct
- Implementeret overflow detection baseret på `bit_width`
- Sat overflow flag ved wrap (både UP og DOWN direction)
- Tilføjet `get_overflow()` og `clear_overflow()` funktioner

**Filer ændret:**
- `include/types.h` - Added overflow_flag field
- `src/counter_sw.cpp` - Overflow detection logic
- `src/counter_sw_isr.cpp` - Overflow detection logic

**Test:** Test 1.6, Test 9.1, Test 9.2

---

### Bug 1.3 & 1.4: PCNT Int16_t Limitation

**Problem:** ESP32 PCNT hardware er signed 16-bit (-32768 til +32767), men koden antog unsigned 32-bit. Ved høje frekvenser wrapper PCNT hver 1.6 sekund (20 kHz), hvilket gav forkerte counts.

**Fix:**
- Korrekt signed int16_t wrap handling med delta calculation
- Detekterer wrap fra +32767 til -32768 og vice versa
- Software accumulation af total count i 64-bit
- Tilføjet omfattende dokumentation i `pcnt_driver.h`

**Filer ændret:**
- `src/counter_hw.cpp` - Delta calculation med wrap detection
- `include/pcnt_driver.h` - Dokumentation

**Kode snippet:**
```cpp
// BUG FIX 1.4: Handle signed int16_t wrap correctly
int16_t signed_current = (int16_t)hw_count;
int16_t signed_last = (int16_t)state->last_count;
int32_t delta = (int32_t)signed_current - (int32_t)signed_last;

if (delta < -32768) {
  delta += 65536;  // Wrap forward
} else if (delta > 32768) {
  delta -= 65536;  // Wrap backward
}
```

**Test:** Test 3

---

### Bug 1.5: Direction (UP/DOWN) Support

**Problem:** Alle counters tællede kun op. DOWN direction var ikke implementeret.

**Fix:**
- Implementeret UP/DOWN support i alle 3 modes (SW, SW-ISR, HW)
- UP: Increment counter
- DOWN: Decrement counter med underflow protection
- Ved underflow: Wrap til `max_val` baseret på `bit_width`
- Tilføjet direction parameter til ISR handlers

**Filer ændret:**
- `src/counter_sw.cpp` - UP/DOWN logic
- `src/counter_sw_isr.cpp` - ISR direction support
- `src/counter_hw.cpp` - Delta inversion for DOWN

**Test:** Test 1.4

---

### Bug 1.6: Start Value Initialization

**Problem:** Counters blev initialiseret til 0 i `counter_engine_init()` INDEN config blev loaded, så `start_value` blev aldrig anvendt.

**Fix:**
- Fjernet premature init calls fra `counter_engine_init()`
- Init sker nu i `counter_engine_configure()` EFTER config er loaded
- `start_value` parameter læses nu korrekt ved init

**Filer ændret:**
- `src/counter_engine.cpp` - Removed premature init

**Test:** Test 5

---

### Bug 1.7: Debounce Logic

**Problem:**
1. `debounce_timer` blev ikke initialiseret (random værdi)
2. `debounce_enabled` flag blev aldrig tjekket

**Fix:**
- Initialiseret `debounce_timer` til `registers_get_millis()` ved init
- Tilføjet check af `cfg.debounce_enabled` før debounce logic
- Hvis disabled: Skip debounce helt

**Filer ændret:**
- `src/counter_sw.cpp` - Debounce init og enable check

**Test:** Test 7

---

### Bug 1.8: Frequency 64-bit Overflow

**Problem:**
1. `max_val` var hardcoded til 32-bit for alle bit widths
2. Frequency calculation kunne overflow ved store delta værdier

**Fix:**
- Dynamic `max_val` calculation baseret på `cfg.bit_width`
- 64-bit math i frequency calculation: `(delta_count * 1000ULL) / delta_time_ms`
- Included `counter_config.h` for at få adgang til bit_width

**Filer ændret:**
- `src/counter_frequency.cpp` - Dynamic max_val, 64-bit math

**Test:** Test 4, Test 6

---

### Bug 1.9: HW Mode GPIO Hardcoding

**Problem:** HW mode brugte hardcoded GPIO pins (19, 25, 31, 37). GPIO 31 og 37 eksisterer ikke på ESP32-WROOM-32!

**Fix:**
- Tilføjet `hw_gpio` field til `CounterConfig` struct
- Implementeret CLI parsing for `hw-gpio:<pin>` parameter
- Removed hardcoded GPIO logic fra `counter_engine.cpp`
- Updated `cli_show.cpp` til at vise hw_gpio fra config

**Filer ændret:**
- `include/types.h` - Added hw_gpio field
- `src/cli_commands.cpp` - Parse hw-gpio parameter
- `src/counter_engine.cpp` - Use cfg.hw_gpio instead of hardcoded
- `src/cli_show.cpp` - Display hw_gpio from config
- `src/counter_config.cpp` - Default hw_gpio=0

**Test:** Test 8

---

### Bug 2.1: Control Register Bits

**Problem:** Control register Bit 1 (Start) og Bit 2 (Stop) var ikke implementeret.

**Fix:**
- Implementeret `start()` og `stop()` funktioner i alle 3 modes
- Added `is_counting` check i alle loop funktioner
- Når `is_counting=0`: Skip counting logic (counter paused)
- Control register handler caller mode-specific start/stop funktioner
- Bit 1 (0x0002): Start counting
- Bit 2 (0x0004): Stop counting
- Bit 0 (0x0001): Reset (already implemented)

**Filer ændret:**
- `include/counter_sw.h` - start/stop function declarations
- `include/counter_sw_isr.h` - start/stop function declarations
- `include/counter_hw.h` - start/stop function declarations
- `src/counter_sw.cpp` - start/stop implementation + is_counting check
- `src/counter_sw_isr.cpp` - start/stop implementation + ISR is_counting check
- `src/counter_hw.cpp` - start/stop implementation + is_counting check
- `src/counter_engine.cpp` - Control register Bit 1/2 handling

**Test:** Test 1.2

---

### Bug 2.3: Bit Width Validation

**Problem:** `counter_config_sanitize()` havde kun lower bound check (≥8), men ingen upper bound eller validering af gyldige værdier.

**Fix:**
- Added upper bound: `if (cfg->bit_width > 64) cfg->bit_width = 64;`
- Rounding til nærmeste valid value (8, 16, 32, 64)
- Eksempler: 7→8, 24→32, 100→64

**Filer ændret:**
- `src/counter_config.cpp` - Bit width validation logic

**Test:** Test 6 (bit width validation tests)

---

### Bug 3.1: ISR Counter Volatile

**Problem:** `isr_state[]` array blev tilgået fra både ISR context og main loop context, men var IKKE erklæret `volatile`. Dette tillod compileren at cache værdien i et register og aldrig reloade fra memory → race conditions.

**Fix:**
- Erklæret `isr_state[]` som `volatile`
- Erklæret `isr_direction[]` som `volatile` (læses i ISR)
- Alle pointer accesses til isr_state bruger nu `volatile CounterSWState*`
- Tilføjet omfattende dokumentation om volatile semantics

**Filer ændret:**
- `src/counter_sw_isr.cpp` - volatile declarations og pointer types

**Test:** Test 2.3 (volatile state verification)

---

## 📊 BUILD STATISTIK

| Metric | Værdi |
|--------|-------|
| Total builds | 79 |
| Successful builds | 79 (100%) |
| Failed builds | 0 (0%) |
| Build time (avg) | 7.2 sekunder |
| Flash usage | 335,081 bytes (25.6%) |
| RAM usage | 28,008 bytes (8.5%) |

**Compiler warnings:**
- Missing initializers (cosmetic, ikke fejl)
- debounce_ms comparison always false (uint16_t vs 0, ignoreres)

---

## 🧪 TESTPLAN

### Hardware Setup
- ESP32-WROOM-32 (DOIT DevKit V1)
- 5 kHz square wave signal på GPIO 13
- 5 kHz square wave signal på GPIO 19
- USB serial connection (115200 baud)

### Test Kategorier

#### Test 1: HW Mode (PCNT) - 7 tests
- Basis counting (UP direction)
- Start/Stop funktionalitet
- Reset funktionalitet
- Direction DOWN
- Prescaler (divide by 10)
- 16-bit overflow
- Total: **7 tests**

#### Test 2: SW-ISR Mode - 3 tests
- Basis counting med interrupt
- Both edges (dobbelt counting)
- Volatile state verification
- Total: **3 tests**

#### Test 3-7: Feature Verification - 10 tests
- PCNT int16_t wrap handling
- Frequency measurement (with/without prescaler)
- Start value initialization
- Bit width validation (24→32, 100→64)
- Debounce enable/disable
- GPIO mapping (config vs hardcoded)
- SW/SW-ISR overflow detection
- Total: **10 tests**

**Total tests:** 20

### Test Dokumentation

Alle test detaljer findes i:
- `test_plan_counter.md` - Fuld testplan med forventede resultater
- `test_quick_reference.txt` - Quick reference til copy/paste CLI kommandoer

---

## ✅ GODKENDELSES PROCEDURE

### Trin 1: Verificer System Boot
1. Åbn serial monitor (115200 baud)
2. Verificer firmware version: "Version: 1.0.0 Build #80"
3. Verificer "Modbus CLI Ready" besked

### Trin 2: Kør Basis Tests
Minimum tests for godkendelse:

**Test A: HW Mode Counting (GPIO 19)**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
set register 140 value:2
```
Vent 10 sekunder:
```
show register 100
show register 120
```
**Pass kriterier:** reg 100 ≈ 50,000 (±5%), reg 120 ≈ 5000 Hz (±2%)

**Test B: SW-ISR Mode Counting (GPIO 13)**
```
set counter 2 mode 1 hw-mode:sw-isr edge:rising direction:up interrupt-pin:13 prescaler:1 bit-width:32 index-reg:200 raw-reg:210 freq-reg:220 overload-reg:230 ctrl-reg:240
set register 240 value:2
```
Vent 10 sekunder:
```
show register 200
show register 220
```
**Pass kriterier:** reg 200 ≈ 50,000 (±5%), reg 220 ≈ 5000 Hz (±2%)

**Test C: Start/Stop Control**
```
set register 140 value:4
```
Vent 5 sekunder, verificer ingen ændring:
```
show register 100
```
Start igen:
```
set register 140 value:2
```
Vent 5 sekunder, verificer ændring:
```
show register 100
```
**Pass kriterier:** Counter stopper når stopped, starter igen ved start command

**Test D: Overflow Detection**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:16 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
set register 140 value:1
set register 140 value:2
```
Vent 15 sekunder:
```
show register 100
show register 130
```
**Pass kriterier:** reg 100 ≈ 9464 (75000 mod 65536), reg 130 = 1 (overflow flag)

### Trin 3: Udfyld Test Resultat Skema
Se `test_plan_counter.md` for komplet skema.

### Trin 4: Godkendelse
Udfyld nedenstående:

---

## 📝 GODKENDELSES FORMULAR

**Tester udført af:** _________________
**Dato:** _________________
**Firmware version verificeret:** Build #___

### Test Resultater

| Kategori | Tests kørt | Tests passed | Success rate |
|----------|-----------|--------------|--------------|
| HW Mode | __/7 | __/7 | ___% |
| SW-ISR Mode | __/3 | __/3 | ___% |
| Features | __/10 | __/10 | ___% |
| **TOTAL** | **__/20** | **__/20** | **___%** |

### Kritiske Bugs Verificeret

- [ ] Bug 1.1: Overflow tracking i SW mode
- [ ] Bug 1.2: Overflow tracking i SW-ISR mode
- [ ] Bug 1.3: PCNT int16_t limitation
- [ ] Bug 1.4: HW mode delta-beregning wrap
- [ ] Bug 1.5: Direction (UP/DOWN) support
- [ ] Bug 1.6: Start value anvendelse
- [ ] Bug 1.7: Debounce logic
- [ ] Bug 1.8: Frequency 64-bit overflow
- [ ] Bug 1.9: HW mode GPIO hardcoding
- [ ] Bug 2.1: Control register bits (start/stop)
- [ ] Bug 2.3: Bit width validation
- [ ] Bug 3.1: ISR counter volatile

### Observerede Problemer

__________________________________________
__________________________________________
__________________________________________

### Godkendelse Status

- [ ] ✅ GODKENDT - Alle tests passed, klar til produktion
- [ ] 🟡 GODKENDT MED FORBEHOLD - Minor issues, specificer ovenfor
- [ ] ❌ IKKE GODKENDT - Major issues, kræver yderligere fixes

**Underskrift:** _________________
**Dato:** _________________

---

## 📁 FILER ÆNDRET

### Include Files (Headers)
- `include/types.h` - Added overflow_flag, hw_gpio
- `include/counter_sw.h` - Added start/stop functions
- `include/counter_sw_isr.h` - Added start/stop functions
- `include/counter_hw.h` - Added start/stop functions
- `include/pcnt_driver.h` - Added documentation

### Source Files
- `src/counter_sw.cpp` - Overflow, debounce, direction, start/stop
- `src/counter_sw_isr.cpp` - Overflow, direction, volatile, start/stop
- `src/counter_hw.cpp` - PCNT wrap, direction, start/stop
- `src/counter_frequency.cpp` - 64-bit overflow fix
- `src/counter_engine.cpp` - Init order, GPIO config, control bits
- `src/counter_config.cpp` - Bit width validation, hw_gpio default
- `src/cli_commands.cpp` - hw-gpio parameter parsing
- `src/cli_show.cpp` - Display hw_gpio
- `src/cli_parser.cpp` - Updated help text

**Total filer ændret:** 14 files

---

## 🎯 KONKLUSION

Alle 12 identificerede bugs er systematisk fixet og verificeret via compilation. Systemet er nu klar til omfattende funktionelle tests med reelle 5 kHz signaler.

**Nøgle forbedringer:**
- ✅ Overflow detection i alle modes
- ✅ Korrekt PCNT signed int16_t wrap handling
- ✅ UP/DOWN direction support
- ✅ Runtime start/stop control
- ✅ Volatile ISR state (race condition fix)
- ✅ Config-based GPIO mapping (ingen hardcoding)
- ✅ 64-bit frequency calculation
- ✅ Proper bit width validation

**Næste steps:**
1. Kør testplan med 5 kHz signaler på GPIO 13 og 19
2. Udfyld test resultat skema
3. Godkend eller rapporter yderligere issues

**Status:** 🟡 AFVENTER MANUEL TEST OG GODKENDELSE

---

**Rapport genereret:** 2025-11-21 21:05
**Firmware version:** Build #80
**Git commit:** main@a4e6bf5
