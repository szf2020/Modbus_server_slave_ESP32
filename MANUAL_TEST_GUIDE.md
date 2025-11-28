# ESP32 Counter System - Manual Test Guide

**Firmware:** Build #89
**Dato:** 2025-11-22
**Status:** Debug mode - filter disabled

---

## 🎯 FORMÅL

Manuel verifikation af counter funktionalitet med 5 kHz signal på GPIO 13 og 19.
Denne test skal identificere root cause til 63% counting problem.

---

## 🔧 SETUP

1. **Hardware:**
   - ESP32 forbundet til COM11
   - 5 kHz signal på GPIO 13 (for SW-ISR mode)
   - 5 kHz signal på GPIO 19 (for HW mode)

2. **Serial Connection:**
   ```bash
   pio device monitor -b 115200
   ```

3. **Verificer boot:**
   ```
   === Modbus RTU Server (ESP32) ===
   Version: 1.0.0 Build #89
   Modbus CLI Ready. Type 'help' for commands.
   ```

---

## 📝 TEST 1: HW Mode - GPIO 19 (10 sekunder)

### Step 1: Konfigurer Counter 1
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:100 raw-reg:101 freq-reg:102 overload-reg:103 ctrl-reg:140
```

**Forventet debug output:**
```
I (xxxxx) PCNT: Unit0 GPIO19: pos_edge=1 neg_edge=0 → pos_mode=1 neg_mode=0
I (xxxxx) CNTR_HW: C1 configured: PCNT_U0 GPIO19 edge=1 dir=0 start=0
```

**Hvis du IKKE ser debug output:**
- PCNT konfiguration fejlede
- hw_gpio parameter muligvis ikke modtaget korrekt

### Step 2: Start Counter
```
show registers 140 1
```
Læs current ctrl register værdi, skal være 0.

```
show registers 100 4
```
Læs initial counter værdier:
- 100: Index (burde være 0)
- 101: Raw (burde være 0)
- 102: Frequency (burde være 0 initially)
- 103: Overflow (burde være 0)

### Step 3: Start Counting (via Modbus register 140, bit 1)

Brug Modbus master til at skrive register 140 = 2 (bit 1 = start).

ELLER via Python:
```python
import serial
ser = serial.Serial('COM11', 115200, timeout=1)
# Send CLI command direkte (hvis muligt via Modbus FC06)
```

### Step 4: Vent 10 sekunder

Mens counter tæller, burde du se debug output hver 2. sekund:
```
I (xxxxx) CNTR_HW: C1: hw=10002 last=0 delta=10002 dir=0 pcnt_val=10002
I (xxxxx) CNTR_HW: C1: hw=20004 last=10002 delta=10002 dir=0 pcnt_val=20004
I (xxxxx) CNTR_HW: C1: hw=30006 last=20004 delta=10002 dir=0 pcnt_val=30006
```

**Analyser delta:**
- Delta burde være ~10,000 hver 2. sekund (5 kHz × 2 sek)
- Hvis delta = ~5,000 → counter tæller kun 50%
- Hvis delta = ~6,300 → counter tæller kun 63%

### Step 5: Læs Final Værdier
```
show registers 100 4
```

**Forventet:**
- Register 100 (Index): ~50,000 counts
- Register 102 (Freq): ~5,000 Hz

**Hvis du får:**
- Index = ~31,000 (63%) → PCNT hardware problem eller edge configuration
- Index = ~25,000 (50%) → Muligvis tæller både rising OG falling, dividerer med 2

---

## 📝 TEST 2: Verificer PCNT Hardware Direkte

For at isolere om det er software eller hardware problem:

### Via ESP32 direkte:

Tilføj debug print i `counter_hw_loop()` for HVER iteration (ikke kun hver 2 sek):

```cpp
// In counter_hw_loop(), efter hw_count read:
static uint32_t last_print = 0;
if (millis() - last_print >= 500) {  // Print hver 0.5 sek
  ESP_LOGI("PCNT_RAW", "U%d raw_count=%d", pcnt_unit, signed_current);
  last_print = millis();
}
```

Dette vil vise om PCNT faktisk modtager alle pulses eller ej.

---

## 📝 TEST 3: Verificer Signal Quality

Med oscilloscope/logic analyzer:
1. Måle 5 kHz signal på GPIO 19
2. Verificer duty cycle er ~50%
3. Verificer voltage levels (3.3V logic)
4. Tjek for noise/glitches

---

## 🔍 DEBUGGING CHECKLIST

### Symptom: Counter tæller 63% (31k i stedet for 50k)

**Mulige årsager:**

1. **PCNT edge configuration fejl:**
   - Check debug output: `pos_mode=1 neg_mode=0` (korrekt for rising only)
   - Hvis `neg_mode=1` → tæller både edges

2. **Signal duty cycle problem:**
   - Hvis signal har duty cycle ≠ 50%, kan PCNT tælle forkert
   - Ved 63% duty cycle ville give ~31k counts

3. **PCNT filter/glitch elimination:**
   - Build #89 har filter disabled
   - Hvis problem persist → ikke filter problem

4. **GPIO routing problem:**
   - GPIO 19 muligvis IKKE forbundet til PCNT unit 0
   - Verificer ESP32 pin muxing

5. **External prescaler på signal generator:**
   - Tjek om signal generator har prescaler aktiveret

---

## ✅ SUCCESS CRITERIA

Test anses for GODKENDT hvis:
- Debug output viser delta = ~10,000 hver 2 sekund
- Final count efter 10 sek = 47,500 - 52,500 (95-105%)
- Frequency = 4,750 - 5,250 Hz

---

## 📞 NEXT STEPS HVIS TEST FEJLER

1. **Hvis debug output viser delta korrekt (~10k hver 2 sek):**
   - Software delta accumulation virker
   - Problem er i register output eller test script

2. **Hvis debug output viser delta = ~6.3k hver 2 sek:**
   - PCNT hardware problem
   - Tjek GPIO routing
   - Verificer signal quality

3. **Hvis INGEN debug output:**
   - PCNT konfiguration fejlede
   - hw_gpio parameter ikke modtaget
   - Check CLI command parsing

---

**God testning!** 🚀

Rapporter tilbage hvad debug output viser, så kan jeg hjælpe videre.
