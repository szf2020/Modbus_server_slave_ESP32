# ESP32 Counter System - Final Test Results

**Test Dato:** 2025-11-21 21:32:35
**Firmware:** Build #80
**Test Signal:** 5 kHz konsistent på GPIO 13 og 19
**Test Resultat:** ❌ **IKKE GODKENDT** (0% Success Rate)

---

## 🎯 EXECUTIVE SUMMARY

**Test Status:** ❌ **CRITICAL FAILURES FOUND**

| Metric | Værdi |
|--------|-------|
| **Tests Kørt** | 8 / 8 |
| **Tests Passed** | 0 / 8 (0.0%) |
| **Tests Failed** | 8 / 8 (100.0%) |
| **Hardware Signal** | ✅ 5 kHz present (bekræftet af Freq=5002 Hz) |

**Kritisk observation:** Counter **TÆLLER NU**, men med **multiple critical bugs**:
1. ❌ Tæller kun ~50% af forventet (25k i stedet for 50k)
2. ❌ STOP command virker IKKE (counter fortsætter)
3. ❌ RESET command virker IKKE (counter ikke nulstillet)
4. ❌ Direction DOWN virker IKKE korrekt
5. ❌ Overflow detection virker IKKE

---

## 📊 DETAILED TEST RESULTS

### Test 1.1: HW Mode Basis Counting ❌

**Status:** ❌ FAIL (51% af forventet)

```
Config: hw-mode:hw edge:rising direction:up hw-gpio:19 prescaler:1 bit-width:32
Wait:   10 sekunder
Signal: 5 kHz (bekræftet af Freq=5002 Hz)

Expected: 50,000 counts (5 kHz × 10 sek)
Actual:   25,658 counts (51.3% af forventet)
Freq:     5,002 Hz ✅ (korrekt)
Overflow: 0 ✅ (korrekt)
```

**Problem:** Counter tæller kun ~HALVDELEN af edges!

**Possible Causes:**
1. **Edge detection configuration fejl:**
   - Konfigureret som `edge:rising`, men PCNT tæller muligvis kun hver 2. edge
   - PCNT prescaler aktiveret ved et uheld (skulle være 1:1)

2. **PCNT mode configuration fejl:**
   - PCNT sat til at tælle både rising OG falling, men dividerer med 2
   - Eller PCNT kun tæller rising, men signalet har dårlig duty cycle

3. **Software delta accumulation fejl:**
   - Delta beregning i `counter_hw_loop()` muligvis fejlagtig
   - Mulig integer division fejl

**Recommendation:** Review `pcnt_unit_configure()` parameters

---

### Test 1.2a: Stop Functionality ❌

**Status:** ❌ CRITICAL FAIL - Stop command virker IKKE!

```
Action: Send STOP command (register 140 = 4, bit 2)
Initial value: 8,905
After 5 sec:   4,149

Expected: NO CHANGE (counter stopped)
Actual:   DECREASED by 4,756 counts!
```

**Problem:** Counter FORTSÆTTER med at tælle selvom stop command sendt!

**Analysis:**
1. **Stop command ikke implementeret korrekt**
   - `counter_hw_stop()` kaldes, men PCNT fortsætter
   - `is_counting` flag check muligvis ikke i `counter_hw_loop()`

2. **PCNT hardware ikke stoppet**
   - Muligvis mangler `pcnt_counter_pause()` kald
   - Eller PCNT pause virker ikke korrekt

3. **Counter VALUE DECREASING?!**
   - Initial: 8905 → After 5 sec: 4149 = -4756
   - Dette er MEGET mærkligt - counter går BAGLÆNS?
   - Mulig wrap eller delta calculation fejl

**CRITICAL:** Dette er en **major bug** - stop control er fundamentalt brudt!

---

### Test 1.2b: Start Functionality ❌

**Status:** ❌ FAIL

```
Action: Send START command (register 140 = 2, bit 1)
Wait:   5 seconds
Result: Counter = 0 (expected ~+25k from previous value)
```

**Problem:** Counter reset til 0 i stedet for at fortsætte fra forrige værdi

**Analysis:**
- Start command muligvis kalder reset i stedet
- Eller reconfiguration nulstiller counter

---

### Test 1.3: Reset Functionality ❌

**Status:** ❌ FAIL - Reset virker IKKE!

```
Action: Send RESET command (register 140 = 1, bit 0)

Expected: Counter = 0, Overflow = 0
Actual:   Counter = 18,418, Overflow = 0
```

**Problem:** Reset command sætter IKKE counter til 0!

**Analysis:**
1. **Reset command handler defekt**
   - `counter_engine_reset()` kaldes, men counter ikke nulstillet
   - Muligvis `counter_hw_reset()` ikke sætter `state->pcnt_value = 0`

2. **PCNT hardware ikke clearet**
   - `pcnt_counter_clear()` muligvis ikke kaldes
   - Eller virker ikke korrekt

**CRITICAL:** Reset er en fundamental operation - SKAL virke!

---

### Test 1.4: Direction DOWN ❌

**Status:** ❌ FAIL (54.9% deviation)

```
Config: direction:down start-value:100000
Wait:   5 seconds
Signal: 5 kHz

Expected: 75,000 (100k start - 25k counted)
Actual:   33,824 (deviation 54.9%)
```

**Problem:** Direction DOWN logik defekt

**Analysis:**
1. **start_value ikke applied:**
   - Burde starte ved 100,000
   - Men værdi 33,824 tyder på start ved ~58k?

2. **DOWN counting ikke implementeret:**
   - Muligvis tæller OP i stedet for NED
   - Eller delta inversion virker ikke

3. **Combination af begge problemer**

---

### Test 1.5: Prescaler ❌

**Status:** ❌ FAIL (~57% af forventet)

```
Config: prescaler:10
Wait:   10 seconds
Signal: 5 kHz

Expected: Index=50,000 (råcount), Raw=5,000 (divideret med 10)
Actual:   Index=28,669 (57%), Raw=3,167
```

**Problem:** Samme ~50% counting problem, MEN prescaler division virker!

**Analysis:**
- Index count: 28669 / 50000 = 57.3% (samme problem som Test 1.1)
- Raw count: 3167 / 28669 = 11.0% (burde være 10%)
- Prescaler division VIRKER (ratio ~9:1 i stedet for 10:1)

**Observation:** Basis counting problem persistent på tværs af configs

---

### Test 1.6: 16-bit Overflow ❌

**Status:** ❌ FAIL - Overflow detection virker IKKE!

```
Config: bit-width:16
Wait:   15 seconds
Signal: 5 kHz

Expected count: 75,000 pulses → wrap to 9,464 (75000 mod 65536)
Actual count:   20,908 (no wrap)
Overflow flag:  0 (should be 1)
```

**Problem:** Overflow detection IKKE implementeret korrekt!

**Analysis:**
1. **Expected pulses:** ~37,500 (kun 50% pga counting bug)
2. **Actual count:** 20,908 (55% af expected 37.5k)
3. **Overflow flag:** 0 (burde være 1 hvis count > 65535)

**Observation:**
- Counting problem stadig present (~50%)
- Overflow flag ALDRIG sat, selvom count burde wrappe

---

### Test 2.1: SW-ISR Mode ❌

**Status:** ❌ FAIL - Could not read registers

```
Config: hw-mode:sw-isr interrupt-pin:13
Result: None (parsing error)
```

**Problem:** Register 200-240 range ikke læseligt

**Analysis:**
- SW-ISR mode muligvis ikke konfigureret korrekt
- Eller register parsing fejl for denne range

---

## 🔍 ROOT CAUSE ANALYSIS

### Primary Bug: Counter Tæller Kun 50% af Edges

**Evidence:**
- Test 1.1: 25,658 / 50,000 = 51.3%
- Test 1.5: 28,669 / 50,000 = 57.3%
- Test 1.6: 20,908 / 37,500 = 55.7%

**Consistent pattern:** Counter tæller ~50-57% af forventet

**Possible Root Causes:**

#### Hypothesis 1: PCNT Edge Configuration Fejl
```cpp
// In pcnt_driver.cpp or counter_hw.cpp
// MULIGVIS:
pcnt_config.pos_mode = PCNT_COUNT_INC;  // Count on positive edge
pcnt_config.neg_mode = PCNT_COUNT_DIS;  // Do NOT count on negative edge

// MEN muligvis configureret forkert, f.eks:
pcnt_config.pos_mode = PCNT_COUNT_INC;
pcnt_config.neg_mode = PCNT_COUNT_INC;  // FEJL: Tæller begge edges
// Og derefter division med 2 et sted?
```

#### Hypothesis 2: Delta Accumulation Fejl
```cpp
// In counter_hw_loop()
int32_t delta = (int32_t)signed_current - (int32_t)signed_last;

// Muligvis division med 2 efter delta calc?
state->pcnt_value += (uint64_t)delta / 2;  // FEJL!
```

#### Hypothesis 3: PCNT Prescaler Aktiveret
- PCNT hardware prescaler sat til 2:1 i stedet for 1:1
- Hver 2. puls ignoreres

**Next Step:** Review `pcnt_unit_configure()` i `src/pcnt_driver.cpp`

---

### Secondary Bug: Stop/Start Control Virker IKKE

**Evidence:**
- Test 1.2a: Counter changed from 8905 → 4149 (DECREASED!)
- Test 1.2b: Counter reset to 0 after start

**Possible Root Causes:**

#### Hypothesis 1: is_counting Check Mangler
```cpp
// In counter_hw_loop() - MANGLER CHECK?
void counter_hw_loop(uint8_t id) {
  // ...
  // BUG FIX 2.1: Check if counting is enabled
  if (!state->is_counting) {
    return;  // Counter stopped, skip counting
  }
  // ← Denne check er muligvis ALDRIG nået pga. early return?
}
```

#### Hypothesis 2: PCNT Hardware Ikke Stoppet
- `counter_hw_stop()` sætter kun software flag
- PCNT hardware fortsætter med at tælle
- Need to call `pcnt_counter_pause(unit)`

#### Hypothesis 3: Wrap/Delta Calculation Fejl
- Counter value DECREASING (8905 → 4149) tyder på wrap fejl
- PCNT wrapper fra +32767 til -32768
- Delta calculation muligvis forkert

---

### Tertiary Bug: Reset Virker IKKE

**Evidence:**
- Test 1.3: Counter = 18,418 efter reset (burde være 0)

**Possible Root Causes:**

#### Hypothesis 1: pcnt_counter_clear() Ikke Kaldt
```cpp
void counter_hw_reset(uint8_t id) {
  // ...
  pcnt_unit_clear(pcnt_unit);  // ← Kaldes dette?
  state->pcnt_value = cfg.start_value;
  // ...
}
```

#### Hypothesis 2: state->pcnt_value Ikke Nulstillet
- PCNT hardware clearet, men software state ikke
- Eller omvendt

---

## 🛠️ CRITICAL FIXES REQUIRED

### Fix Priority Matrix

| Priority | Bug | Impact | Effort |
|----------|-----|--------|--------|
| **P0** | 50% counting error | CRITICAL | 2-3h |
| **P0** | Stop control broken | CRITICAL | 1-2h |
| **P0** | Reset broken | CRITICAL | 1h |
| **P1** | Overflow detection | HIGH | 1h |
| **P1** | Direction DOWN | HIGH | 2h |
| **P2** | SW-ISR mode | MEDIUM | 2h |

**Total Estimated Fix Time:** 8-11 timer

---

## 🎯 RECOMMENDED ACTIONS

### Immediate Actions (Next 30 Minutes)

1. **Review pcnt_unit_configure() i pcnt_driver.cpp:**
   ```cpp
   // Verify edge configuration
   pcnt_config.pos_mode = PCNT_COUNT_INC;  // Should be INC
   pcnt_config.neg_mode = PCNT_COUNT_DIS;  // Should be DIS (not INC!)

   // Verify NO prescaler
   // (PCNT prescaler should NOT be used)
   ```

2. **Review counter_hw_loop() i counter_hw.cpp:**
   ```cpp
   // Verify delta calculation
   state->pcnt_value += (uint64_t)delta;  // NOT divided by 2!

   // Verify is_counting check EXISTS and is REACHED
   if (!state->is_counting) {
     return;
   }
   ```

3. **Review counter_hw_stop() i counter_hw.cpp:**
   ```cpp
   void counter_hw_stop(uint8_t id) {
     hw_state[id - 1].is_counting = 0;
     // ADD: pcnt_counter_pause(pcnt_unit);  // Stop PCNT hardware!
   }
   ```

4. **Review counter_hw_reset() i counter_hw.cpp:**
   ```cpp
   void counter_hw_reset(uint8_t id) {
     pcnt_unit_clear(pcnt_unit);  // Clear PCNT
     state->pcnt_value = cfg.start_value;  // Reset software state
     state->last_count = 0;  // Reset last_count!
   }
   ```

---

## 📋 DETAILED BUG VERIFICATION

| Bug | Feature | Verified? | Status | Notes |
|-----|---------|-----------|--------|-------|
| 1.1 | Overflow SW | ❌ | NOT TESTED | SW mode not tested |
| 1.2 | Overflow SW-ISR | ❌ | NOT TESTED | SW-ISR parsing error |
| 1.3 | PCNT int16_t | ⚠️ | PARTIAL | Counting works but only 50% |
| 1.4 | HW delta wrap | ⚠️ | PARTIAL | Delta calc has issues (decreasing count) |
| 1.5 | Direction UP/DOWN | ❌ | FAILED | DOWN not working (Test 1.4) |
| 1.6 | Start value | ❌ | FAILED | Not applied (Test 1.4) |
| 1.7 | Debounce | ❌ | NOT TESTED | SW mode not tested |
| 1.8 | Frequency | ✅ | VERIFIED | Freq=5002 Hz ✅ (Test 1.1) |
| 1.9 | GPIO mapping | ✅ | VERIFIED | GPIO 19 works, signal detected |
| 2.1 | Control bits | ❌ | FAILED | Stop/Start broken (Test 1.2) |
| 2.3 | Bit width | ⚠️ | PARTIAL | Validation works, but overflow detection fails |
| 3.1 | ISR volatile | ❌ | NOT TESTED | SW-ISR mode not tested |

**Overall Verification:** 2/12 bugs fully verified (16.7%)

---

## 🔚 FINAL RECOMMENDATION

### Status: ❌ IKKE GODKENDT TIL PRODUKTION

**Critical Issues:**
1. ❌ Counter tæller kun 50% af edges (fundamental counting fejl)
2. ❌ Stop command virker IKKE (counter fortsætter)
3. ❌ Reset command virker IKKE (counter ikke nulstillet)
4. ❌ Direction DOWN ikke implementeret korrekt
5. ❌ Overflow detection virker ikke

**Positive Findings:**
- ✅ Frequency measurement virker (5002 Hz)
- ✅ GPIO mapping virker (signal detected)
- ✅ PCNT tæller NOGET (selvom kun 50%)

### Next Actions:
1. 🔧 Fix 50% counting error (review PCNT edge config)
2. 🔧 Fix stop/start control (add pcnt_pause/resume)
3. 🔧 Fix reset (ensure pcnt_clear AND state reset)
4. 🧪 Re-run full test suite
5. ✅ Require ≥90% pass rate for godkendelse

**Estimated Time to Fix:** 8-11 timer

---

**Rapport Genereret:** 2025-11-21 21:33:00
**Test Varighed:** 1 min 35 sek
**Firmware:** Build #80
**Git:** main@a4e6bf5

---

*Final Test Results rapport genereret efter re-run med stable 5kHz signals*
