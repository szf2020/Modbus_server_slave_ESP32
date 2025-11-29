# ESP32 Modbus RTU Server - Feature Inventory (PART 1/2)

**Version:** 1.0.0 | **Dato:** 2025-11-29 | **Platform:** ESP32-WROOM-32

---

## TIMER MODES

### MODE 1: ONE-SHOT (3-fase sekvens)
Afspil forudbestemt sekvens af 3 faser med individuelle timinger.
- **Parametre:** p1-duration, p1-output, p2-duration, p2-output, p3-duration, p3-output, output-coil
- **Aktivering:** Skrivning til output-coil
- **Adfærd:** Fase 1 → Fase 2 → Fase 3 → Stop
- **Kode:** src/timer_engine.cpp:49-88

### MODE 2: MONOSTABLE (retriggerable puls)
Generer enkelt puls af defineret længde.
- **Parametre:** pulse-ms, trigger-level, p1-output (hvile), p2-output (puls), output-coil
- **Adfærd:** Hviler på p1-niveau, trigger → puls (p2) → tilbage til p1
- **Retrigger:** Kan retriggers under kørsel
- **Kode:** src/timer_engine.cpp:94-116

### MODE 3: ASTABLE (oscillator)
Kontinuerlig toggle mellem to niveauer.
- **Parametre:** on-ms, off-ms, p1-output (ON), p2-output (OFF), output-coil, enabled
- **Auto-start:** Starter automatisk når enabled=1
- **Adfærd:** ON-ms → OFF-ms → gentag
- **Forbud:** Kan ikke retriggers når kørende
- **Kode:** src/timer_engine.cpp:122-142

### MODE 4: INPUT-TRIGGERED (kantdetekterings-timer)
Svar på input-coil kantudsving med valgfrit delay.
- **Parametre:** input-dis, trigger-edge (0=falling/1=rising), delay-ms, p1-output, output-coil
- **Adfærd:** Overvåg input, detektér kant, vent delay, sæt output
- **Demodulatør:** Kan køre flere gange (ved hver kant)
- **Kode:** src/timer_engine.cpp:152-216

---

## COUNTER MODES

### MODE 1: SW (Software Polling)
Tæl pulser ved polling af diskret input-niveau.
- **Input:** Diskret input (0-255)
- **Presicion:** ~10kHz max
- **Debounce:** Understøttet (standard 10ms)
- **Kode:** src/counter_sw.cpp

### MODE 2: SW-ISR (Software Interrupt)
Tæl pulser via GPIO-interrupt.
- **Input:** GPIO-pin (0-39)
- **Presicion:** ~100kHz max
- **ISR:** Hardware-accelerated
- **Debounce:** Understøttet
- **Kode:** src/counter_sw_isr.cpp

### MODE 3: HW (Hardware PCNT)
Brug ESP32 hardware pulse counter.
- **Input:** GPIO-pin (0-39)
- **Presicion:** >1MHz
- **RTOS-task:** Kørende hver 10ms for overflow-håndtering
- **32-bit overflow:** Akkumuleres til 64-bit tæller
- **Kode:** src/counter_hw.cpp, src/pcnt_driver.cpp

---

## COUNTER COMMON FEATURES

### Prescaler (Alle modes)
- **Strategi:** Tæl ALLEkanter, divider kun ved register-skrivning
- **Formler:**
  - raw-register = counterValue / prescaler
  - index-register = counterValue × scale_factor
- **Supporterede værdier:** 1, 4, 8, 16, 64, 256, 1024
- **Kode:** src/counter_engine.cpp:285-290

### Bit-Width Clamping
| Bit-Width | Max Value | Type |
|-----------|-----------|------|
| 8 | 0xFF | uint8_t |
| 16 | 0xFFFF | uint16_t |
| 32 | 0xFFFFFFFF | uint32_t |
| 64 | 0xFFFFFFFFFFFFFFFF | uint64_t |

### Control Register (ctrl-reg)
| Bit | Navn | Type | Aktion |
|-----|------|------|--------|
| 0 | reset-on-read | R/W | Nulstil ved læsning |
| 1 | start | W | Start counter |
| 2 | stop | W | Stop counter |
| 7 | running | R | Status |

**Kommando:** `set counter 1 control auto-start:on running:on`

### Frequency Measurement
- **Vindue:** 1 sekund (FREQUENCY_MEAS_WINDOW_MS = 1000)
- **Output:** Hz (0-20000)
- **Uafhængig:** Af prescaler
- **Register:** freq-reg
- **Kode:** src/counter_frequency.cpp

### Scale Factor (Multiplikator)
Konverter counter-værdi til ønskede enheder.
- **Eksempel:** scale:2.5 → hver puls = 2.5 units
- **Type:** Float
- **Formlen:** index-register = counter × scale_factor
- **Kode:** src/counter_engine.cpp:293

---

## CLI COMMANDS (KOMPLET LISTE)

### SHOW COMMANDS
- `show config` - Komplet konfiguration
- `show counters` - Counter-tabel med alle parametre
- `show timers` - Timer-status (placeholder)
- `show registers [start] [count]` - Holding-registers (0-159)
- `show coils` - Coil-tilstande (0-255)
- `show inputs` - Diskrete inputs (0-255)
- `show version` - Firmware-version + build-info
- `show gpio` - GPIO-mappings
- `show echo` - Remote echo-status
- `show reg` - Register-konfigurationer (STATIC+DYNAMIC)

### COUNTER COMMANDS
- `set counter <id> mode 1 parameter ...` - Konfigurér counter
- `set counter <id> control <flags>` - Start/stop/reset-bits
- `reset counter <id>` - Nulstil tæller
- `clear counters` - Nulstil alle tællere

### TIMER COMMANDS
- `set timer <id> mode <1-4> parameter ...` - Konfigurér timer

### GPIO COMMANDS
- `set gpio <pin> static map input:<idx>` - GPIO → Diskret input
- `set gpio <pin> static map coil:<idx>` - Coil → GPIO
- `no set gpio <pin>` - Fjern GPIO-mapping
- `set gpio 2 enable` - Frigiv GPIO2 (disable heartbeat)
- `set gpio 2 disable` - Reservér GPIO2 (enable heartbeat)

### REGISTER COMMANDS
- `set reg STATIC <addr> Value <value>` - Fast register-værdi
- `set reg DYNAMIC <addr> counter<id>:<func>` - Counter-bundet register
- `set reg DYNAMIC <addr> timer<id>:<func>` - Timer-bundet register

Funktioner:
- Counter: index, raw, freq, overflow, ctrl
- Timer: output

### COIL COMMANDS
- `set coil STATIC <addr> Value <ON|OFF>` - Fast coil-værdi
- `set coil DYNAMIC <addr> counter<id>:<func>` - Counter-bundet coil
- `set coil DYNAMIC <addr> timer<id>:<func>` - Timer-bundet coil

Funktioner:
- Counter: overflow
- Timer: output

### READ/WRITE COMMANDS
- `read reg <addr> [count]` - Læs holding-registers
- `read coil <addr> [count]` - Læs coils
- `read input <addr> [count]` - Læs diskrete inputs
- `write reg <addr> value <value>` - Skriv holding-register
- `write coil <addr> value <on|off>` - Skriv coil

### SYSTEM COMMANDS
- `set hostname <name>` - Sæt systemnavn (placeholder)
- `set baud <rate>` - Sæt Modbus baudrate (300-115200)
- `set id <slave_id>` - Sæt Modbus slave-ID (0-247)
- `set echo <on|off>` - Remote echo on/off

### PERSISTENCE COMMANDS
- `save` - Gem hele konfiguration til NVS
- `load` - Indlæs konfiguration fra NVS
- `defaults` - Nulstil til factory-defaults
- `reboot` - Genstarte ESP32

### UTILITY COMMANDS
- `help` eller `?` - Viser kommando-hjælp

---

## GPIO FEATURES

### GPIO Zones
| Type | Range | Brug | Modbus-link |
|------|-------|------|-------------|
| Fysiske GPIO | 0-39 | Direkte ESP32-pins | Hvis kortlagt |
| Virtuelle GPIO | 100-255 | Læser/skriver coils | Til diskret input/coil |
| Reserverede | 0, 2, 4-5, 15, 19-21, 33 | System/Modbus/counter HW | Nej |

### STATIC GPIO Mapping
Kortlægger GPIO ↔ Modbus register/coil (persistent).

**Input-mode (GPIO → Diskret input):**
```
set gpio 16 static map input:45
```
Læs GPIO16 hver loop → Skriv til diskret input 45

**Output-mode (Coil → GPIO):**
```
set gpio 23 static map coil:112
```
Når coil 112 ændres → Sæt/clear GPIO23

**Max mappings:** 8 total (inkl. virtuelle GPIO)

**Struktur:** PersistConfig.gpio_maps[8]

### GPIO2 Heartbeat
- **Default:** Blinker hver 500ms (system alive)
- **Enable heartbeat:** `set gpio 2 disable`
- **Disable heartbeat:** `set gpio 2 enable` (GPIO2 til bruger)
- **Persistent:** Gem med `save`

**Kontrolstruktur:** PersistConfig.gpio2_user_mode

---

Fortsætter i PART 2...
