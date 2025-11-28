# CLI Debug Test - 500 Hz Signal

**Dato:** 2025-11-22
**Build:** #89
**Signal:** 500 Hz på GPIO 13 og 19

---

## PROBLEMER IDENTIFICERET

### Problem 1: Counter 1 (HW) - Pin = "-" (ikke konfigureret)
```
counter | hw  | pin  | value | running
1       | HW  | -    | 12000 | YES
```
**Symptom:** Pin vises som "-" i stedet for 19
**Konsekvens:** PCNT aldrig konfigureret → counter tæller 0

### Problem 2: Counter 3 (ISR) - Tæller selvom stopped
```
counter | mode | pin | value   | running
3       | ISR  | 13  | 3137675 | NO
```
**Symptom:** Value stiger selvom running = NO
**Konsekvens:** Stop command virker ikke

---

## TEST 1: Verificer HW Mode Configuration (Counter 1 - GPIO 19)

### Step 1: Konfigurer Counter 1
```
set counter 1 mode 1 hw-mode:hw edge:falling direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:20 raw-reg:30 freq-reg:35 overload-reg:44 ctrl-reg:40
```

### Step 2: Verificer konfiguration blev gemt
```
show config counters
```

**Forventet output:**
```
counter | hw  | pin  | value
1       | HW  | 19   | 0
```

**Hvis pin stadig = "-":**
- CLI parser har IKKE sat `hw_gpio` korrekt
- Eller config blev ikke applied korrekt

### Step 3: Start counter via control register
```
show registers 40 1
```
Læs current værdi (burde være 0).

Skriv register 40 = 2 (bit 1 = start) via Modbus master ELLER gem config:
```
save config
```
Dette skulle auto-starte counter hvis auto-start enabled.

### Step 4: Vent 10 sekunder og læs værdier
```
show registers 20 16
```

**Forventet @ 500 Hz × 10 sek:**
- Register 20 (Index): ~5,000 counts
- Register 35 (Freq): ~500 Hz

**Hvis du får:**
- Index = 0, Freq = 0 → PCNT ikke konfigureret (hw_gpio problem)
- Index = ~3,000 (60%) → Samme 63% problem som før

---

## TEST 2: Verificer ISR Mode Stop (Counter 3 - GPIO 13)

### Step 1: Stop counter 3
Skriv register 70 = 4 (bit 2 = stop) via Modbus master.

ELLER via CLI (hvis muligt):
```
show registers 70 1
```
Læs current værdi.

### Step 2: Verificer counter blev stoppet
```
show config counters
```

**Forventet:**
```
counter | running
3       | NO
```

### Step 3: Vent 5 sekunder og læs værdi igen
```
show registers 15 1
```
Læs register 15 (Index for counter 3).

Læs igen efter 5 sekunder:
```
show registers 15 1
```

**Forventet:** Værdien burde IKKE ændre sig (counter stopped).

**Hvis værdien stiger:**
- ISR handler ignorerer `is_counting` flag
- Interrupt pin stadig attached

---

## TEST 3: Debug via Serial Monitor

Start monitor og observer debug output:
```
pio device monitor -b 115200
```

### Step 3.1: Konfigurer counter 1 med hw-gpio:19

Send CLI command:
```
set counter 1 mode 1 hw-mode:hw edge:falling direction:up hw-gpio:19 prescaler:1 bit-width:32 index-reg:20 raw-reg:30 freq-reg:35 overload-reg:44 ctrl-reg:40
```

**Forventet debug output:**
```
I (xxxxx) PCNT: Unit0 GPIO19: pos_edge=0 neg_edge=2 → pos_mode=0 neg_mode=1
I (xxxxx) CNTR_HW: C1 configured: PCNT_U0 GPIO19 edge=2 dir=0 start=0
```

**Hvis INGEN debug output:**
- `counter_hw_configure()` blev ALDRIG kaldt
- `hw_gpio` er stadig 0 efter CLI parse

### Step 3.2: Observer counting (hver 2 sekund)

Efter counter started, burde du se:
```
I (xxxxx) CNTR_HW: C1: hw=1000 last=0 delta=1000 dir=0 pcnt_val=1000
I (xxxxx) CNTR_HW: C1: hw=2000 last=1000 delta=1000 dir=0 pcnt_val=2000
```

**Delta burde være ~1000 hver 2. sekund** (500 Hz × 2 sek)

**Hvis delta = ~630:**
- 63% counting problem persist

**Hvis INGEN output:**
- Counter ikke started ELLER is_counting check blocker loop

---

## ANALYSE GUIDE

### Hvis Counter 1 pin stadig = "-":

**Mulige årsager:**
1. CLI parser ikke parse `hw-gpio` korrekt
2. `counter_engine_configure()` ikke called efter CLI command
3. Config ikke applied til runtime state

**Debug:**
Tilføj debug print i `cli_commands.cpp` efter parse:
```cpp
debug_print("DEBUG: Parsed hw_gpio = ");
debug_print_uint(cfg.hw_gpio);
debug_println("");
```

### Hvis Counter 3 tæller selvom stopped:

**Mulige årsager:**
1. `is_counting` flag ikke volatile (race condition)
2. Interrupt pin ikke detached ved stop
3. Multiple interrupt sources attached

**Debug:**
Tilføj debug print i ISR handler:
```cpp
void IRAM_ATTR counter_isr_2() {
  if (!isr_state[2].is_counting) {
    // Log at ISR blev kaldt selvom stopped
    return;
  }
  // ...
}
```

---

## FORVENTET RESULTATER

### Success Criteria (Counter 1 @ 500 Hz, 10 sek):
- ✅ Pin vist som "19" i show config counters
- ✅ Index = 4,750 - 5,250 counts (95-105%)
- ✅ Freq = 475 - 525 Hz (95-105%)
- ✅ Debug output viser delta ~1000 hver 2 sek

### Success Criteria (Counter 3 stop test):
- ✅ Running = NO efter stop command
- ✅ Value IKKE ændrer sig efter stop
- ✅ Frequency går til 0 efter stop

---

**Kør disse tests og rapporter tilbage:**
1. Hvad viser "show config counters" for counter 1 pin?
2. Viser serial monitor debug output fra PCNT/CNTR_HW?
3. Stopper counter 3 korrekt?

Så kan jeg fixe de præcise problemer baseret på dit output! 🚀
