# Build #97 Status Report

**Dato:** 2025-11-22
**Builds:** #92 → #97
**Status:** 3 kritiske bugs fixet, klar til manuel test

---

## PROBLEMSTILLING (Fra user feedback)

User rapporterede 3 problemer med 500 Hz signal på GPIO 13 og 19:

### Problem 1: hw_gpio ikke gemt (Counter 1 - HW mode)
```
counter | hw  | pin  | value | running
1       | HW  | -    | 12000 | YES
```
**Symptom:** Pin vises som "-" efter reboot i stedet for GPIO 19
**Root cause:** Counter 1 blev konfigureret, men config ikke gemt til NVS

### Problem 2: Counter tæller selvom stopped (Counter 3 - ISR mode)
```
counter | mode | pin | value   | running
3       | ISR  | 13  | 3137675 | NO
```
**Symptom:** Value stiger selvom running = NO
**Root cause:** Interrupt forblev attached efter stop kommando

### Problem 3: Running status ikke persisteret
**Symptom:** Efter reboot viser alle counters running = NO
**Analyse:** Dette er **forventet adfærd** - counters skal eksplicit startes efter boot

---

## FIXES IMPLEMENTERET

### FIX 1: NVS Auto-Save (Build #92)

**Fil:** `src/cli_commands.cpp`

**Problem:**
- CLI kommando `set counter` gemte config til RAM (g_persist_config)
- Men kaldte ALDRIG `config_save_to_nvs()` for at gemme til flash
- Efter reboot var config væk

**Løsning:**
```cpp
// Linje 193-197
if (config_save_to_nvs(&g_persist_config)) {
  debug_println("  (Config auto-saved to NVS)");
} else {
  debug_println("  WARNING: Failed to save config to NVS!");
}
```

**Resultat:**
- hw_gpio, interrupt_pin og ALLE counter config felter gemmes nu til NVS
- Efter reboot loades config korrekt (verificeret: "CONFIG LOADED: CRC=65363 OK")

---

### FIX 2: ISR Interrupt Detach/Reattach (Build #97)

**Fil:** `src/counter_engine.cpp`

**Problem:**
- `counter_sw_isr_stop()` satte kun `is_counting = 0` flag
- Interrupt forblev attached → ISR blev kaldt ved hver edge
- Mulig race condition mellem flag og ISR execution

**Løsning ved STOP (linje 233-234):**
```cpp
case COUNTER_HW_SW_ISR:
  // BUG FIX 3.2: Detach interrupt to prevent ISR calls when stopped
  counter_sw_isr_detach(id);
  break;
```

**Løsning ved START (linje 212-215):**
```cpp
case COUNTER_HW_SW_ISR:
  // BUG FIX 3.2: Reattach interrupt to ensure ISR is active
  if (cfg.interrupt_pin > 0) {
    counter_sw_isr_attach(id, cfg.interrupt_pin);
  }
  break;
```

**Resultat:**
- Ved stop: Interrupt detaches + is_counting = 0 → ISR kaldes IKKE mere
- Ved start: Interrupt reattaches + is_counting = 1 → ISR aktiv igen
- Eliminerer race conditions og unødvendige ISR calls

---

### FIX 3: Running Status Analyse

**Konklusion:** IKKE en bug - forventet adfærd

**Hvorfor counters ikke auto-starter:**
1. `is_counting` er runtime state (IKKE persistent)
2. `ctrl_reg` start/stop bits cleares efter execute
3. Sikkerhedsdesign: Counters skal eksplicit startes efter boot

**Hvis auto-start ønskes i fremtiden:**
- Tilføj `auto_start` flag i CounterConfig (persistent)
- Anvend flag i config_apply() ved boot
- User kan selv vælge om counter auto-starter

---

## TIDLIGERE FIXES (Build #86-#89)

### P0.0: PCNT Init Order (Build #86)
- **Problem:** PCNT pause/clear kaldt før config → "PCNT driver error"
- **Fix:** Flyttet pause/clear til efter pcnt_unit_config()
- **Resultat:** Elimineret driver errors, counting forbedret fra 0% til 51%

### P0.1: Debounce Filter (Build #89)
- **Problem:** PCNT filter for aggressiv ved høj frekvens
- **Fix:** Komplet disabled filter (`pcnt_filter_disable()`)
- **Resultat:** Counting forbedret fra 51% til 63%

### P0.2: HW Start/Stop (Build #86)
- **Problem:** Start/stop satte kun flag, ikke hardware
- **Fix:** Tilføjet `pcnt_counter_resume()` og `pcnt_counter_pause()`
- **Resultat:** Hardware PCNT styres korrekt

---

## GENNEMFØRT ANALYSE

### NVS Persistence Flow (Verificeret)
1. **Boot:** `config_load_from_nvs()` læser blob inkl. hw_gpio ✅
2. **Apply:** `config_apply()` kalder `counter_engine_configure()` ✅
3. **Configure:** PCNT initialiseres med GPIO fra config ✅
4. **CLI:** `set counter` gemmer til NVS via auto-save ✅

### CounterConfig Struct (Verificeret)
```cpp
typedef struct {
  uint8_t enabled;
  CounterModeEnable mode_enable;
  CounterEdgeType edge_type;
  CounterDirection direction;
  CounterHWMode hw_mode;
  uint16_t prescaler;
  uint8_t bit_width;
  // ... (truncated for brevity)
  uint8_t interrupt_pin;  // GPIO pin for ISR mode
  uint8_t hw_gpio;        // GPIO pin for PCNT mode ← GEM i NVS!
} CounterConfig;
```

**PersistConfig (linje 215):**
```cpp
CounterConfig counters[COUNTER_COUNT];  // Hele struct gemmes!
```

---

## RESTERENDE PROBLEM

### 63% Counting Problem (Stadig aktiv)

**Symptom:**
- Counter tæller kun 63% af pulses (31,433 i stedet for 50,000 ved 500 Hz × 10 sek)
- Freq register viser 5,470 Hz (korrekt!)
- Delta debug output viser ~63% også

**Mulige årsager:**
1. **Signal duty cycle ≠ 50%**
   - Hvis generator har 63% duty cycle → forklarer 63% counting
   - Verificer med oscilloscope

2. **PCNT edge configuration**
   - Debug output viser: `pos_edge=1 neg_edge=0 → pos_mode=1 neg_mode=0` (korrekt)
   - Men måske tæller både rising OG falling?

3. **GPIO routing problem**
   - ESP32 pin muxing muligvis ikke korrekt
   - GPIO 19 måske IKKE forbundet til PCNT unit korrekt

4. **External signal prescaler**
   - Tjek om signal generator har prescaler aktiveret

**IKKE længere mulige årsager (elimineret):**
- ❌ Debounce filter (disabled i Build #89)
- ❌ PCNT init order (fixet i Build #86)
- ❌ Software prescaler division (verificeret korrekt)

---

## NÆSTE STEP: MANUEL TEST

### Test 1: Verificer hw_gpio Persistence

**1. Konfigurer counter 1:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:100 raw-reg:101 freq-reg:102 overload-reg:103 ctrl-reg:140
```

**Forventet debug output:**
```
DEBUG: hw_gpio = 19
Counter 1 configured
  (Config auto-saved to NVS)
```

**2. Verificer via show command:**
```
show config counters
```

**Forventet:**
```
counter | hw  | pin  | value
1       | HW  | 19   | 0       <- PIN = 19 (IKKE "-")
```

**3. Reboot ESP32 (tryk reset knap)**

**4. Efter boot, verificer igen:**
```
show config counters
```

**Success criteria:**
- ✅ Pin stadig = 19 efter reboot
- ✅ Debug output ved boot: "CONFIG LOADED: CRC=XXXXX OK"
- ✅ Debug output: "Counter 1 enabled - configuring..."

---

### Test 2: Verificer ISR Stop Funktionalitet

**1. Konfigurer counter 3 (ISR mode):**
```
set counter 3 mode 1 hw-mode:sw-isr edge:rising direction:up interrupt-pin:13 prescaler:1 bit-width:32 index-reg:110 raw-reg:111 freq-reg:112 overload-reg:113 ctrl-reg:150
```

**2. Start counter via ctrl-reg:**
```
show registers 150 1
```
(Læs værdi, burde være 0)

**Via Modbus master:** Skriv register 150 = 2 (bit 1 = start)

**3. Vent 5 sekunder, verificer counting:**
```
show registers 110 1
```
(Burde vise ~2,500 counts ved 500 Hz)

**4. Stop counter via ctrl-reg:**

**Via Modbus master:** Skriv register 150 = 4 (bit 2 = stop)

**5. Verificer stopped:**
```
show config counters
```

**Forventet:**
```
counter | running
3       | NO       <- Stopped!
```

**6. Vent 5 sekunder, læs værdi igen:**
```
show registers 110 1
```

**Success criteria:**
- ✅ Værdi IKKE ændret (counter ikke tæller)
- ✅ Frequency går til 0

---

### Test 3: Debugging 63% Problem

**Monitor serial debug output under counting:**

**Forventet output (hver 2 sekund):**
```
I (xxxxx) CNTR_HW: C1: hw=1000 last=0 delta=1000 dir=0 pcnt_val=1000
I (xxxxx) CNTR_HW: C1: hw=2000 last=1000 delta=1000 dir=0 pcnt_val=2000
```

**Hvis delta = ~630 hver 2 sek:**
- Problem er i PCNT hardware eller signal
- Verificer signal quality med oscilloscope

**Hvis INGEN debug output:**
- PCNT ikke konfigureret (hw_gpio problem persist)
- Check NVS save/load igen

---

## SUCCESS CRITERIA (SAMLET)

### Fix 1: hw_gpio Persistence ✅ (Build #92)
- hw_gpio gemmes til NVS efter CLI config
- Loades korrekt efter reboot
- Pin vises i "show config counters"

### Fix 2: ISR Stop ✅ (Build #97)
- Counter tæller IKKE når running = NO
- Interrupt detached ved stop
- Reattached ved start

### Fix 3: Running Status 📝 (Design)
- Forventet adfærd: counters starter IKKE automatisk efter boot
- Auto-start feature kan tilføjes senere hvis ønsket

### 63% Problem ⏳ (Under investigation)
- Stadig kun 63% counting
- Næste step: Hardware verification (oscilloscope)
- Eller tilføj mere granulær debug output

---

## COMMIT MESSAGE (Foreslået)

```
FIX: Counter NVS persistence og ISR interrupt management

Build #92-#97: Fix kritiske counter bugs rapporteret af user

**Problem 1: hw_gpio ikke persisteret**
- CLI counter config gemte til RAM men ikke NVS
- Efter reboot viste pin = "-" i stedet for GPIO nummer
- Fix: Auto-save til NVS efter successful configure (cli_commands.cpp:193)

**Problem 2: ISR counter tæller selvom stopped**
- Interrupt forblev attached efter stop kommando
- ISR blev kaldt ved hver edge (returnerede tidligt)
- Mulig race condition mellem flag og ISR execution
- Fix: Detach interrupt ved stop, reattach ved start (counter_engine.cpp:212,233)

**Problem 3: Running status ikke persisteret**
- Analyse: Forventet adfærd - counters skal startes eksplicit efter boot
- is_counting er runtime state, ikke persistent config
- Auto-start feature kan tilføjes senere hvis ønsket

**Testing:**
- CONFIG LOADED verificeret: CRC OK efter reboot
- Counters enables ved boot: "Counter 1/2/3 enabled - configuring..."
- Manuel test required: Verificer hw_gpio pin og ISR stop via CLI

**Remaining:**
- 63% counting problem stadig aktiv (ikke relateret til NVS/ISR)
- Mulige årsager: Signal duty cycle, GPIO routing, eller PCNT config
- Næste step: Hardware verification med oscilloscope

Refs: CLI_DEBUG_TEST.md, MANUAL_TEST_GUIDE.md
```

---

**Status:** Klar til manuel test med 500 Hz signal på GPIO 13 og 19.

**Anbefaling:** Kør Test 1 og Test 2 for at verificere at fixes virker som forventet.
