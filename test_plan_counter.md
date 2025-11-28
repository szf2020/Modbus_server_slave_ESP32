# ESP32 Counter System - Komplet Testplan

**Test Dato:** 2025-11-21
**Firmware:** Build #80
**Hardware:** ESP32-WROOM-32 (DOIT DevKit V1)
**Test Signaler:** 5 kHz på GPIO 13 og GPIO 19

---

## 🔧 PRE-TEST SETUP

Verificer nuværende system status:
```
show version
show counters
show config
```

**Forventet output:**
- Version: 1.0.0 Build #80
- Counters 1-3 muligvis konfigureret fra tidligere config

---

## TEST 1: HW MODE (PCNT) - GPIO 19 med 5 kHz signal

### Test 1.1: Basis counting (UP direction, rising edge)

**Konfiguration:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Start counter:**
```
set register 140 value:2
```
_(Bit 1 = Start command)_

**Vent 10 sekunder, læs derefter:**
```
show register 100
show register 110
show register 120
show register 130
```

**Forventet resultat:**
- Register 100 (index): ~50,000 (5 kHz × 10 sek = 50k pulser)
- Register 110 (raw): ~50,000 (prescaler=1)
- Register 120 (freq): ~5000 Hz (5 kHz målt)
- Register 130 (overload): 0 (ingen overflow)

**Test 1.2: Stop/Start funktionalitet**

**Stop counter:**
```
set register 140 value:4
```
_(Bit 2 = Stop command)_

**Læs værdi:**
```
show register 100
```
_(Gem denne værdi som V1)_

**Vent 5 sekunder, læs igen:**
```
show register 100
```
_(Skal være samme som V1 - counter er stoppet)_

**Start igen:**
```
set register 140 value:2
```

**Vent 5 sekunder, læs:**
```
show register 100
```
_(Skal være V1 + ~25,000)_

**Test 1.3: Reset funktionalitet**

**Reset counter:**
```
set register 140 value:1
```
_(Bit 0 = Reset command)_

**Læs værdi:**
```
show register 100
show register 130
```

**Forventet resultat:**
- Register 100: 0 (counter reset til 0)
- Register 130: 0 (overflow flag cleared)

**Test 1.4: Direction DOWN**

**Rekonfigurer med DOWN direction og start_value=100000:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:down hw-gpio:19 prescaler:1 bit-width:32 start-value:100000 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Start counter:**
```
set register 140 value:2
```

**Vent 5 sekunder, læs:**
```
show register 100
```

**Forventet resultat:**
- Register 100: ~75,000 (100,000 - 25,000 pulser = 75k)
- Tæller ned i stedet for op

**Test 1.5: Prescaler (divide by 10)**

**Rekonfigurer med prescaler=10:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:10 bit-width:32 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Reset og start:**
```
set register 140 value:1
set register 140 value:2
```

**Vent 10 sekunder, læs:**
```
show register 100
show register 110
```

**Forventet resultat:**
- Register 100 (index): ~50,000 (tæller ALLE edges, prescaler kun i raw reg)
- Register 110 (raw): ~5,000 (50,000 / 10 = 5k)

**Test 1.6: Bit width 16-bit med overflow**

**Rekonfigurer med 16-bit:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:16 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Reset og start:**
```
set register 140 value:1
set register 140 value:2
```

**Vent 15 sekunder (75k pulser - overstiger 65535):**
```
show register 100
show register 130
```

**Forventet resultat:**
- Register 100: Wrapped værdi (75000 mod 65536 = 9464)
- Register 130: 1 (overflow flag sat)

---

## TEST 2: SW-ISR MODE - GPIO 13 med 5 kHz signal

### Test 2.1: Basis counting med interrupt

**Konfiguration:**
```
set counter 2 mode 1 hw-mode:sw-isr edge:rising direction:up interrupt-pin:13 prescaler:1 bit-width:32 index-reg:200 raw-reg:210 freq-reg:220 overload-reg:230 ctrl-reg:240
```

**Start counter:**
```
set register 240 value:2
```

**Vent 10 sekunder, læs:**
```
show register 200
show register 220
```

**Forventet resultat:**
- Register 200: ~50,000 (5 kHz × 10 sek)
- Register 220: ~5000 Hz

**Test 2.2: Both edges (dobbelt counting)**

**Rekonfigurer med BOTH edges:**
```
set counter 2 mode 1 hw-mode:sw-isr edge:both direction:up interrupt-pin:13 prescaler:1 bit-width:32 index-reg:200 raw-reg:210 freq-reg:220 overload-reg:230 ctrl-reg:240
```

**Reset og start:**
```
set register 240 value:1
set register 240 value:2
```

**Vent 10 sekunder, læs:**
```
show register 200
```

**Forventet resultat:**
- Register 200: ~100,000 (tæller både rising OG falling = 2× edges)

**Test 2.3: Volatile ISR state (Bug 3.1 verification)**

**Konfigurer counter 2 igen:**
```
set counter 2 mode 1 hw-mode:sw-isr edge:rising direction:up interrupt-pin:13 prescaler:1 bit-width:32 index-reg:200 raw-reg:210 freq-reg:220 overload-reg:230 ctrl-reg:240
```

**Reset og start:**
```
set register 240 value:1
set register 240 value:2
```

**Vent 20 sekunder (100k pulser):**
```
show register 200
```

**Verificer:**
- Register 200: ~100,000 (hvis ikke volatile, ville der være race conditions og forkerte counts)
- Test består hvis count er indenfor ±5% af forventet værdi

---

## TEST 3: PCNT INT16_T WRAP (Bug 1.3/1.4 verification)

**Konfigurer counter 1 til HW mode:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Reset og start:**
```
set register 140 value:1
set register 140 value:2
```

**Vent 10 sekunder (50k pulser - overstiger PCNT 16-bit signed range ±32768):**
```
show register 100
```

**Forventet resultat:**
- Register 100: ~50,000 (korrekt akkumuleret på trods af PCNT wrap)
- Hvis bug 1.3/1.4 ikke var fixet, ville værdien være forkert efter PCNT wrap

---

## TEST 4: FREQUENCY MEASUREMENT (Bug 1.8 verification)

**Konfigurer counter 1:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Reset og start:**
```
set register 140 value:1
set register 140 value:2
```

**Vent 5 sekunder, læs frequency:**
```
show register 120
```

**Forventet resultat:**
- Register 120: ~5000 Hz (målt frekvens)
- Acceptabel range: 4900-5100 Hz (±2%)

**Test med prescaler:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:100 bit-width:32 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Reset og start:**
```
set register 140 value:1
set register 140 value:2
```

**Vent 5 sekunder, læs:**
```
show register 120
```

**Forventet resultat:**
- Register 120: STADIG ~5000 Hz (frequency er IKKE divideret med prescaler - Bug 1.8 fix)

---

## TEST 5: START VALUE (Bug 1.6 verification)

**Konfigurer med start_value=12345:**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32 start-value:12345 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Reset (skal sætte til start_value, ikke 0):**
```
set register 140 value:1
```

**Læs straks:**
```
show register 100
```

**Forventet resultat:**
- Register 100: 12345 (starter ved start_value, ikke 0)

---

## TEST 6: BIT WIDTH VALIDATION (Bug 2.3 verification)

**Test invalid bit width (skal rundes til nærmeste valid value):**

**Test 1: bit-width:24 (skal blive 32):**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:24 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Verificer:**
```
show counters
```
_(Se om bit-width er 32, ikke 24)_

**Test 2: bit-width:100 (skal blive 64 - upper bound):**
```
set counter 1 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:100 index-reg:100 raw-reg:110 freq-reg:120 overload-reg:130 ctrl-reg:140
```

**Verificer:**
```
show counters
```
_(Se om bit-width er 64, ikke 100)_

---

## TEST 7: DEBOUNCE (Bug 1.7 verification)

**Test at debounce_enabled faktisk respekteres:**

**Konfigurer med debounce disabled:**
```
set counter 2 mode 1 hw-mode:sw-isr edge:rising direction:up interrupt-pin:13 prescaler:1 bit-width:32 debounce-enabled:0 index-reg:200 raw-reg:210 freq-reg:220 overload-reg:230 ctrl-reg:240
```

**Reset og start:**
```
set register 240 value:1
set register 240 value:2
```

**Vent 10 sekunder:**
```
show register 200
```

**Forventet:**
- Register 200: ~50,000 (debounce disabled betyder alle edges tælles)

**Konfigurer med debounce enabled (10ms):**
```
set counter 2 mode 1 hw-mode:sw-isr edge:rising direction:up interrupt-pin:13 prescaler:1 bit-width:32 debounce-enabled:1 debounce-ms:10 index-reg:200 raw-reg:210 freq-reg:220 overload-reg:230 ctrl-reg:240
```

**Reset og start:**
```
set register 240 value:1
set register 240 value:2
```

**Vent 10 sekunder:**
```
show register 200
```

**Forventet:**
- Register 200: ~50,000 (ved 5 kHz er 10ms debounce OK, ingen filtrering)

---

## TEST 8: GPIO MAPPING (Bug 1.9 verification)

**Test at hw-gpio faktisk bruges (ikke hardcoded værdier):**

**Konfigurer counter 3 med GPIO 19 (samme signal som counter 1):**
```
set counter 3 mode 1 hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:300 raw-reg:310 freq-reg:320 overload-reg:330 ctrl-reg:340
```

**Start counter 3:**
```
set register 340 value:2
```

**Vent 10 sekunder:**
```
show register 300
```

**Forventet resultat:**
- Register 300: ~50,000 (samme signal som counter 1)
- Hvis Bug 1.9 ikke var fixet, ville counter 3 bruge hardcoded GPIO og ikke tælle

---

## TEST 9: OVERFLOW TRACKING (Bug 1.1/1.2 verification)

### Test 9.1: SW mode overflow

**Konfigurer counter 4 i SW mode med 8-bit width:**
```
set counter 4 mode 1 hw-mode:sw edge:rising direction:up input-dis:32 prescaler:1 bit-width:8 index-reg:400 raw-reg:410 freq-reg:420 overload-reg:430 ctrl-reg:440
```

**Note:** SW mode kræver en discrete input signal. Hvis GPIO 13/19 ikke er mappet til discrete input 32, skal du først mappe dem:
```
set gpio-map 13 input:32
```

**Derefter start counter 4:**
```
set register 440 value:2
```

**Vent indtil counter når 255 (8-bit max), derefter læs:**
```
show register 400
show register 430
```

**Forventet resultat:**
- Register 400: 0-255 (wrapped til 0 efter 255)
- Register 430: 1 (overflow flag sat)

### Test 9.2: SW-ISR mode overflow

**Konfigurer counter 2 med 16-bit width:**
```
set counter 2 mode 1 hw-mode:sw-isr edge:rising direction:up interrupt-pin:13 prescaler:1 bit-width:16 index-reg:200 raw-reg:210 freq-reg:220 overload-reg:230 ctrl-reg:240
```

**Reset og start:**
```
set register 240 value:1
set register 240 value:2
```

**Vent 15 sekunder (75k pulser, overstiger 65535):**
```
show register 200
show register 230
```

**Forventet resultat:**
- Register 200: 9464 (75000 mod 65536)
- Register 230: 1 (overflow flag sat)

---

## POST-TEST VERIFICATION

**Verificer alle counters status:**
```
show counters
show registers 100 150
show registers 200 250
show registers 300 350
show registers 400 450
```

**Gem system config:**
```
show config
save config
```

---

## TEST RESULTAT SKEMA

| Test | Feature | Status | Værdi | Forventet | Pass/Fail | Noter |
|------|---------|--------|-------|-----------|-----------|-------|
| 1.1 | HW mode basis | ⬜ | | ~50k | | |
| 1.2 | Start/Stop | ⬜ | | Ingen ændring når stopped | | |
| 1.3 | Reset | ⬜ | | 0 | | |
| 1.4 | Direction DOWN | ⬜ | | 75k | | |
| 1.5 | Prescaler | ⬜ | | raw=5k, index=50k | | |
| 1.6 | 16-bit overflow | ⬜ | | 9464, overflow=1 | | |
| 2.1 | SW-ISR basis | ⬜ | | ~50k | | |
| 2.2 | Both edges | ⬜ | | ~100k | | |
| 2.3 | Volatile state | ⬜ | | ~100k (±5%) | | |
| 3 | PCNT wrap | ⬜ | | ~50k | | |
| 4 | Frequency | ⬜ | | ~5000 Hz | | |
| 5 | Start value | ⬜ | | 12345 | | |
| 6.1 | Bit width 24→32 | ⬜ | | 32 | | |
| 6.2 | Bit width 100→64 | ⬜ | | 64 | | |
| 7 | Debounce | ⬜ | | ~50k begge tests | | |
| 8 | GPIO mapping | ⬜ | | ~50k | | |
| 9.1 | SW overflow | ⬜ | | 0-255, overflow=1 | | |
| 9.2 | SW-ISR overflow | ⬜ | | 9464, overflow=1 | | |

---

## KONKLUSION

**Antal tests kørt:** ____ / 20
**Antal tests bestået:** ____ / 20
**Success rate:** ____%

**Kritiske bugs verificeret fixet:**
- ✅ Bug 1.1: Overflow tracking i SW mode
- ✅ Bug 1.2: Overflow tracking i SW-ISR mode
- ✅ Bug 1.3: PCNT int16_t limitation
- ✅ Bug 1.4: HW mode delta-beregning wrap
- ✅ Bug 1.5: Direction (UP/DOWN) support
- ✅ Bug 1.6: Start value anvendelse
- ✅ Bug 1.7: Debounce logic
- ✅ Bug 1.8: Frequency 64-bit overflow
- ✅ Bug 1.9: HW mode GPIO hardcoding
- ✅ Bug 2.1: Control register bits (start/stop)
- ✅ Bug 2.3: Bit width validation
- ✅ Bug 3.1: ISR counter volatile

**Anbefaling:**
- [ ] Godkendt til produktion
- [ ] Kræver yderligere tests
- [ ] Ikke godkendt

**Underskrift:** ___________________
**Dato:** 2025-11-21
