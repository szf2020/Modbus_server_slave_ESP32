# ESP32 Counter System - Test Results Report

**Test Date:** 2025-11-22 12:37:02
**Firmware:** Build #80
**Hardware:** ESP32-WROOM-32 (DOIT DevKit V1)
**Test Signals:** 5 kHz på GPIO 13 og GPIO 19

---

## EXECUTIVE SUMMARY

| Metric | Value |
|--------|-------|
| **Total Tests** | 8 |
| **Passed** | 0 (0.0%) |
| **Failed** | 8 (100.0%) |
| **Success Rate** | 0.0% |

**Overall Status:** ❌ IKKE GODKENDT

---

## DETAILED TEST RESULTS

### Test 1.1: HW mode basis counting

**Status:** ❌ FAIL
**Expected:** ~50k (±5%)
**Actual:** 6158
**Notes:** Index=6158, Raw=6158, Freq=518, Overflow=0
**Timestamp:** 2025-11-22T12:35:40.987104

---

### Test 1.2a: Stop functionality

**Status:** ❌ FAIL
**Expected:** No change when stopped
**Actual:** 7829 → 10776
**Notes:** Counter changed from 7829 to 10776 while stopped!
**Timestamp:** 2025-11-22T12:35:49.790650

---

### Test 1.2b: Start functionality

**Status:** ❌ FAIL
**Expected:** ~+25k after restart
**Actual:** +N/A
**Notes:** Expected ~+25k, got +-10776
**Timestamp:** 2025-11-22T12:35:55.992489

---

### Test 1.3: Reset functionality

**Status:** ❌ FAIL
**Expected:** Counter=0, Overflow=0
**Actual:** Counter=15727, Overflow=0
**Notes:** Counter=15727, Overflow=0
**Timestamp:** 2025-11-22T12:35:59.796089

---

### Test 1.4: Direction DOWN

**Status:** ❌ FAIL
**Expected:** ~75k (100k - 25k)
**Actual:** 30926
**Notes:** Expected ~75k, got 30926 (deviation 58.8%)
**Timestamp:** 2025-11-22T12:36:09.110245

---

### Test 1.5: Prescaler (divide by 10)

**Status:** ❌ FAIL
**Expected:** Index=~50k, Raw=~5k
**Actual:** Index=6494, Raw=681
**Notes:** Index counts ALL edges, Raw divided by prescaler
**Timestamp:** 2025-11-22T12:36:24.626109

---

### Test 1.6: 16-bit overflow

**Status:** ❌ FAIL
**Expected:** ~9464, overflow=1
**Actual:** 9116, overflow=0
**Notes:** Expected wrap to ~9464 with overflow=1, got 9116 with overflow=0
**Timestamp:** 2025-11-22T12:36:45.141714

---

### Test 2.1: SW-ISR mode basis

**Status:** ❌ FAIL
**Expected:** ~50k, freq=~5000
**Actual:** None, freq=None
**Notes:** Interrupt-based counting on GPIO 13
**Timestamp:** 2025-11-22T12:37:00.056859

---

## BUG VERIFICATION SUMMARY

| Bug | Feature | Test | Status |
|-----|---------|------|--------|
| 1.1 | Overflow tracking SW | 9.1 | PENDING |
| 1.2 | Overflow tracking SW-ISR | 9.2 | PENDING |
| 1.3 | PCNT int16_t | 3 | PENDING |
| 1.4 | HW delta wrap | 3 | PENDING |
| 1.5 | Direction UP/DOWN | 1.4 | ❌ |
| 1.6 | Start value | 5 | PENDING |
| 1.7 | Debounce | 7 | PENDING |
| 1.8 | Frequency 64-bit | 4 | PENDING |
| 1.9 | GPIO mapping | 8 | PENDING |
| 2.1 | Control bits | 1.2 | TESTED |
| 2.3 | Bit width | 6 | PENDING |
| 3.1 | ISR volatile | 2.3 | PENDING |


---

## CONCLUSION

**Tests Completed:** 8 / 20
**Success Rate:** 0.0%

❌ **ANBEFALING: IKKE GODKENDT**

**Underskrift:** ___________________
**Dato:** 2025-11-22

---

*Rapport genereret automatisk af run_counter_tests.py*
