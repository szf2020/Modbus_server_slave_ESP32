# ST Functions v4.7+ - Komplet Brugerdokumentation

**Version:** 4.7.3
**Build:** #999+
**Dato:** 2026-01-07
**Status:** ✅ Production Ready

---

## 📋 Indholdsfortegnelse

1. [Oversigt](#oversigt)
2. [Eksponentielle Funktioner](#eksponentielle-funktioner)
3. [Stateful Funktioner](#stateful-funktioner)
   - [Edge Detection](#edge-detection-r_trig--f_trig)
   - [Timers](#timers-ton--tof--tp)
   - [Counters](#counters-ctu--ctd--ctud)
   - [Bistable Latches](#bistable-latches-srrs) (NEW v4.7.3)
4. [Memory Usage](#memory-usage)
5. [Limitations](#limitations)
6. [Eksempler](#eksempler)
7. [Troubleshooting](#troubleshooting)

---

## Oversigt

v4.7 tilføjer **13 nye ST funktioner** med fokus på:
- **Avanceret matematik**: EXP, LN, LOG, POW
- **Stateful function blocks**: R_TRIG, F_TRIG, TON, TOF, TP, CTU, CTD, CTUD

v4.7.3 tilføjer **2 nye bistable latches**:
- **SR/RS latches**: IEC 61131-3 standard set-reset og reset-set function blocks

### Nøglefunktioner

✅ **IEC 61131-3 Compliance** - Følger international PLC standard
✅ **DRAM Optimeret** - Kun 452 bytes per program (v4.7.3+)
✅ **Input Validation** - Automatisk håndtering af ugyldige værdier
✅ **Zero Copy** - Effektiv pointer-baseret arkitektur
✅ **Compile-time Instance Allocation** - Ingen runtime overhead

---

## Eksponentielle Funktioner

### EXP(x) - Exponential Function

**Syntax:**
```st
result := EXP(x);
```

**Beskrivelse:**
Beregner e^x (eksponentiel funktion).

**Parameter:**
- `x` (REAL) - Eksponent værdi

**Return:**
- (REAL) - e^x, eller 0.0 hvis overflow

**Validation:**
- ✅ Overflow check: `expf(>88.7)` → returnerer `0.0`
- ✅ INF protection: `isinf()` → returnerer `0.0`

**Eksempel:**
```st
VAR
  x : REAL;
  result : REAL;
END_VAR

x := 2.0;
result := EXP(x);  (* result = 7.389 (e^2) *)

x := 100.0;
result := EXP(x);  (* result = 0.0 (overflow protection) *)
```

**Range:**
- Valid input: `-∞` til `~88.7`
- Output: `0.0` til `~2.68e+43`

---

### LN(x) - Natural Logarithm

**Syntax:**
```st
result := LN(x);
```

**Beskrivelse:**
Beregner naturlig logaritme (base e).

**Parameter:**
- `x` (REAL) - Input værdi (skal være > 0)

**Return:**
- (REAL) - ln(x), eller 0.0 hvis x ≤ 0

**Validation:**
- ✅ Input check: `x ≤ 0` → returnerer `0.0`
- ✅ Konsistent med SQRT behavior (BUG-065)

**Eksempel:**
```st
VAR
  x : REAL;
  result : REAL;
END_VAR

x := 2.718;
result := LN(x);  (* result ≈ 1.0 (ln(e) = 1) *)

x := -5.0;
result := LN(x);  (* result = 0.0 (invalid input) *)
```

**Range:**
- Valid input: `0 < x < ∞`
- Output: `-∞` til `+∞`

---

### LOG(x) - Base-10 Logarithm

**Syntax:**
```st
result := LOG(x);
```

**Beskrivelse:**
Beregner logaritme med base 10.

**Parameter:**
- `x` (REAL) - Input værdi (skal være > 0)

**Return:**
- (REAL) - log10(x), eller 0.0 hvis x ≤ 0

**Validation:**
- ✅ Input check: `x ≤ 0` → returnerer `0.0`

**Eksempel:**
```st
VAR
  x : REAL;
  result : REAL;
END_VAR

x := 100.0;
result := LOG(x);  (* result = 2.0 (log10(100) = 2) *)

x := 0.001;
result := LOG(x);  (* result = -3.0 *)
```

**Range:**
- Valid input: `0 < x < ∞`
- Output: `-∞` til `+∞`

---

### POW(x, y) - Power Function

**Syntax:**
```st
result := POW(base, exponent);
```

**Beskrivelse:**
Beregner x^y (x opløftet i y).

**Parametre:**
- `x` (REAL) - Base værdi
- `y` (REAL) - Eksponent værdi

**Return:**
- (REAL) - x^y, eller 0.0 hvis NaN/INF

**Validation:**
- ✅ **NaN check**: Negativ base med fractional exponent → returnerer `0.0`
  - Eksempel: `POW(-2.0, 0.5)` → `0.0` (ikke sqrt af negativ)
- ✅ **INF check**: Division by zero eller overflow → returnerer `0.0`
  - Eksempel: `POW(0.0, -1.0)` → `0.0` (division by zero)
  - Eksempel: `POW(1000.0, 1000.0)` → `0.0` (overflow)

**Eksempler:**
```st
VAR
  base, exp, result : REAL;
END_VAR

(* Valid operations *)
result := POW(2.0, 3.0);    (* result = 8.0 *)
result := POW(9.0, 0.5);    (* result = 3.0 (square root) *)
result := POW(10.0, -2.0);  (* result = 0.01 *)

(* Invalid operations (protected) *)
result := POW(-2.0, 0.5);   (* result = 0.0 (NaN protection) *)
result := POW(0.0, -1.0);   (* result = 0.0 (INF protection) *)
```

**Special Cases:**
- `POW(0, 0)` → `1.0` (matematisk konvention)
- `POW(x, 0)` → `1.0` (alt i nul'te er 1)
- `POW(0, x)` hvor x > 0 → `0.0`

---

## Stateful Funktioner

Stateful funktioner kræver **persistent state** mellem execution cycles.

### Architecture

**Instance Allocation:**
- Max 8 timer instances per program
- Max 8 edge detector instances per program
- Max 8 counter instances per program
- **Compile-time allocation** - ingen runtime overhead
- **Zero-copy architecture** - pointer-baseret

**Memory:**
- 420 bytes total per program
  - Timers: 8 × 24 bytes = 192 bytes
  - Edges: 8 × 8 bytes = 64 bytes
  - Counters: 8 × 20 bytes = 160 bytes

---

## Edge Detection (R_TRIG & F_TRIG)

### R_TRIG(CLK) - Rising Edge Detection

**Syntax:**
```st
edge_detected := R_TRIG(CLK);
```

**Beskrivelse:**
Detekterer stigende flanke (0→1 transition).

**Parameter:**
- `CLK` (BOOL) - Input signal

**Return:**
- (BOOL) - TRUE i én cycle når CLK går fra FALSE til TRUE

**Behavior:**
```
CLK:    ___/‾‾‾‾\___/‾‾‾
R_TRIG: ___/\_____/\___ (pulse for 1 cycle)
```

**Eksempel:**
```st
VAR
  button : BOOL;
  edge : BOOL;
  counter : INT;
END_VAR

(* Count button presses *)
edge := R_TRIG(button);
IF edge THEN
  counter := counter + 1;
END_IF;
```

**Truth Table:**

| Cycle | CLK (current) | Last State | R_TRIG Output |
|-------|---------------|------------|---------------|
| 1 | FALSE | FALSE | FALSE |
| 2 | TRUE | FALSE | **TRUE** ← Rising edge |
| 3 | TRUE | TRUE | FALSE |
| 4 | FALSE | TRUE | FALSE |
| 5 | TRUE | FALSE | **TRUE** ← Rising edge |

**Use Cases:**
- Button press detection
- Event counting
- One-shot triggers
- State change detection

---

### F_TRIG(CLK) - Falling Edge Detection

**Syntax:**
```st
edge_detected := F_TRIG(CLK);
```

**Beskrivelse:**
Detekterer faldende flanke (1→0 transition).

**Parameter:**
- `CLK` (BOOL) - Input signal

**Return:**
- (BOOL) - TRUE i én cycle når CLK går fra TRUE til FALSE

**Behavior:**
```
CLK:    ___/‾‾‾‾\___/‾‾‾
F_TRIG: ______/\_______/\ (pulse for 1 cycle)
```

**Eksempel:**
```st
VAR
  sensor : BOOL;
  falling : BOOL;
  stop_count : INT;
END_VAR

(* Detect sensor deactivation *)
falling := F_TRIG(sensor);
IF falling THEN
  stop_count := stop_count + 1;
END_IF;
```

**Use Cases:**
- Release detection
- End-of-motion detection
- Off-transition counting

---

## Timers (TON & TOF & TP)

Alle timers bruger **millisekund præcision** via `millis()`.

### TON(IN, PT) - On-Delay Timer

**Syntax:**
```st
timer_done := TON(IN, preset_time);
```

**Beskrivelse:**
Output går HIGH efter preset time når input er HIGH.

**Parametre:**
- `IN` (BOOL) - Enable input
- `PT` (INT) - Preset time i millisekunder

**Return:**
- (BOOL) - TRUE når timer er done

**Validation:**
- ✅ **PT validation**: Negative værdier → behandles som `0`
- ⚠️ **millis() overflow**: Korrekt håndtering af 32-bit unsigned overflow

**Timing Diagram:**
```
IN:  ___/‾‾‾‾‾‾‾‾‾‾\______

Q:   _______/‾‾‾‾‾‾\______ (delays turn-on)
         |← PT →|
```

**Eksempel:**
```st
VAR
  start_button : BOOL;
  motor_on : BOOL;
  delay_ms : INT := 2000;  (* 2 seconds *)
END_VAR

(* Delayed motor start *)
motor_on := TON(start_button, delay_ms);

(* Motor starts 2 seconds after button press *)
(* Motor stops immediately when button released *)
```

**Behavior:**
1. **Rising edge på IN**: Start timer, Q=FALSE
2. **Timer running**: Q=FALSE, ET incrementing
3. **ET ≥ PT**: Q=TRUE, timer done
4. **IN goes LOW**: Q=FALSE immediately, timer reset

**Use Cases:**
- Delayed start sequences
- Debounce timers
- Settling time delays
- Safety interlocks

---

### TOF(IN, PT) - Off-Delay Timer

**Syntax:**
```st
timer_done := TOF(IN, preset_time);
```

**Beskrivelse:**
Output forbliver HIGH i preset time efter input går LOW.

**Parametre:**
- `IN` (BOOL) - Enable input
- `PT` (INT) - Preset time i millisekunder

**Return:**
- (BOOL) - TRUE mens timer kører

**Timing Diagram:**
```
IN:  ___/‾‾‾‾‾‾\__________

Q:   ___/‾‾‾‾‾‾‾‾‾‾\_____ (delays turn-off)
                |← PT →|
```

**Eksempel:**
```st
VAR
  presence_sensor : BOOL;
  light_on : BOOL;
  delay_ms : INT := 5000;  (* 5 seconds *)
END_VAR

(* Light stays on 5 seconds after person leaves *)
light_on := TOF(presence_sensor, delay_ms);
```

**Behavior:**
1. **IN goes HIGH**: Q=TRUE immediately
2. **IN goes LOW**: Start timer, Q remains TRUE
3. **Timer running**: Q=TRUE, ET incrementing
4. **ET ≥ PT**: Q=FALSE, timer done

**Use Cases:**
- Courtesy lighting
- Cooldown timers
- Fan run-on timers
- Delay-off sequences

---

### TP(IN, PT) - Pulse Timer

**Syntax:**
```st
pulse_active := TP(IN, preset_time);
```

**Beskrivelse:**
Genererer fast-length pulse ved rising edge.

**Parametre:**
- `IN` (BOOL) - Trigger input
- `PT` (INT) - Pulse width i millisekunder

**Return:**
- (BOOL) - TRUE i PT millisekunder efter rising edge

**Timing Diagram:**
```
IN:  ___/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\___

Q:   ___/‾‾‾‾‾‾\______________ (fixed pulse width)
        |← PT →|
```

**Eksempel:**
```st
VAR
  trigger : BOOL;
  valve_pulse : BOOL;
  pulse_width : INT := 500;  (* 500ms pulse *)
END_VAR

(* Generate 500ms valve pulse *)
valve_pulse := TP(trigger, pulse_width);
```

**Behavior:**
1. **Rising edge på IN**: Q=TRUE, start timer
2. **Timer running**: Q=TRUE, ET incrementing
3. **ET ≥ PT**: Q=FALSE, pulse done
4. **Non-retriggerable**: Ignorer nye edges mens pulsing

**Use Cases:**
- Fixed-width pulses
- Valve actuation
- Short command signals
- Retriggering prevention

---

## Counters (CTU & CTD & CTUD)

### CTU(CU, RESET, PV) - Count Up

**Syntax:**
```st
done := CTU(count_input, reset, preset_value);
```

**Beskrivelse:**
Tæller op på rising edges, output når CV ≥ PV.

**Parametre:**
- `CU` (BOOL) - Count-up input (counts on rising edge)
- `RESET` (BOOL) - Reset til 0 (high priority)
- `PV` (INT) - Preset value (target count)

**Return:**
- (BOOL) - TRUE når CV ≥ PV

**Validation:**
- ✅ **Overflow protection**: CV clamped at INT32_MAX

**Behavior:**
```
CU:     _/‾\_/‾\_/‾\_/‾\_/‾\_ (5 rising edges)
RESET:  ___________________/‾\___
CV:     0  1  2  3  4  5  0
Q:      ________/‾‾‾‾‾‾‾\_____ (Q when CV ≥ PV=3)
```

**Eksempel:**
```st
VAR
  sensor : BOOL;
  reset_btn : BOOL;
  done : BOOL;
  target : INT := 100;
END_VAR

(* Count 100 parts *)
done := CTU(sensor, reset_btn, target);

IF done THEN
  (* Trigger batch complete *)
END_IF;
```

**Use Cases:**
- Production counting
- Event accumulation
- Batch processing
- Quota monitoring

---

### CTD(CD, LOAD, PV) - Count Down

**Syntax:**
```st
zero_reached := CTD(count_input, load, preset_value);
```

**Beskrivelse:**
Tæller ned fra PV, output når CV ≤ 0.

**Parametre:**
- `CD` (BOOL) - Count-down input (counts on rising edge)
- `LOAD` (BOOL) - Load PV into CV (rising edge)
- `PV` (INT) - Preset value (starting count)

**Return:**
- (BOOL) - TRUE når CV ≤ 0

**Validation:**
- ✅ **Underflow protection**: CV clamped at 0

**Behavior:**
```
CD:     _/‾\_/‾\_/‾\_/‾\_/‾\_
LOAD:   /‾\___________________
CV:     5  4  3  2  1  0
Q:      __________________/‾‾ (Q when CV ≤ 0)
```

**Eksempel:**
```st
VAR
  dispense : BOOL;
  reload : BOOL;
  empty : BOOL;
  capacity : INT := 50;
END_VAR

(* Countdown dispenser *)
empty := CTD(dispense, reload, capacity);

IF empty THEN
  (* Trigger refill *)
END_IF;
```

**Use Cases:**
- Inventory tracking
- Remaining count display
- Depletion monitoring
- Reverse counting

---

### CTUD(CU, CD, RESET, LOAD, PV) - Count Up/Down

**Syntax:**
```st
up_done := CTUD(count_up, count_down, reset, load, preset);
```

**Beskrivelse:**
Kombineret up/down counter med 2 outputs.

**Parametre:**
- `CU` (BOOL) - Count-up input
- `CD` (BOOL) - Count-down input
- `RESET` (BOOL) - Reset til 0 (highest priority)
- `LOAD` (BOOL) - Load PV into CV
- `PV` (INT) - Preset value

**Return:**
- (BOOL) - QU status (TRUE når CV ≥ PV)
- **Note**: CTUD has secondary output QD (true når CV ≤ 0)

**Validation:**
- ✅ **Overflow protection**: CV clamped at INT32_MAX
- ✅ **Underflow protection**: CV clamped at 0

**⚠️ LIMITATION:**
CTUD er **kun delvist implementeret** i VM (mangler 5-argument support).
**Workaround**: Brug separate CTU + CTD indtil v4.8.

**Eksempel (teoretisk):**
```st
VAR
  increment, decrement, reset, load : BOOL;
  up_done : BOOL;
  setpoint : INT := 100;
END_VAR

(* Bidirectional counter *)
up_done := CTUD(increment, decrement, reset, load, setpoint);
```

**Use Cases:**
- Bidirectional position tracking
- Stock level management
- Reversible counting

---

## Bistable Latches (SR/RS)

### SR(S1, R) - Set-Reset Latch (Reset Priority)

**Syntax:**
```st
output := SR(set_input, reset_input);
```

**Beskrivelse:**
Bistable latch med Reset-prioritet. Når både S og R er TRUE, vil output være FALSE (reset vinder).

**Parametre:**
- `S1` (BOOL) - Set input (sætter output til TRUE)
- `R` (BOOL) - Reset input (nulstiller output til FALSE) **PRIORITY**

**Return:**
- (BOOL) - Q output (latched state)

**Truth Table:**

| S1 | R | Q (Output) |
|----|---|------------|
| 0  | 0 | Hold (forrige tilstand) |
| 0  | 1 | 0 (reset) |
| 1  | 0 | 1 (set) |
| 1  | 1 | **0 (reset priority!)** |

**Validation:**
- ✅ **State persistence**: Output holder værdi mellem cycles
- ✅ **Edge insensitive**: Reagerer på niveau, ikke kanter

**Eksempel:**
```st
VAR
  sensor_triggered : BOOL;
  reset_button : BOOL;
  alarm_active : BOOL;
END_VAR

(* Alarm system - reset button has priority *)
alarm_active := SR(sensor_triggered, reset_button);

IF alarm_active THEN
  (* Activate siren *)
END_IF;
```

**Use Cases:**
- Alarm systems (reset priority)
- Emergency stop circuits
- Safety interlocks
- Fault latching

---

### RS(S, R1) - Reset-Set Latch (Set Priority)

**Syntax:**
```st
output := RS(set_input, reset_input);
```

**Beskrivelse:**
Bistable latch med Set-prioritet. Når både S og R er TRUE, vil output være TRUE (set vinder).

**Parametre:**
- `S` (BOOL) - Set input (sætter output til TRUE) **PRIORITY**
- `R1` (BOOL) - Reset input (nulstiller output til FALSE)

**Return:**
- (BOOL) - Q output (latched state)

**Truth Table:**

| S  | R1 | Q (Output) |
|----|----|-----------|
| 0  | 0  | Hold (forrige tilstand) |
| 0  | 1  | 0 (reset) |
| 1  | 0  | 1 (set) |
| 1  | 1  | **1 (set priority!)** |

**Validation:**
- ✅ **State persistence**: Output holder værdi mellem cycles
- ✅ **Edge insensitive**: Reagerer på niveau, ikke kanter

**Eksempel:**
```st
VAR
  start_button : BOOL;
  stop_button : BOOL;
  motor_running : BOOL;
END_VAR

(* Motor control - start button has priority *)
motor_running := RS(start_button, stop_button);

IF motor_running THEN
  (* Run motor *)
END_IF;
```

**Use Cases:**
- Motor control (start priority)
- Process sequencing
- ON/OFF control with override
- State machines

---

### SR vs RS - Når skal man bruge hvad?

| Anvendelse | Anbefalet Latch | Begrundelse |
|------------|-----------------|-------------|
| **Alarm systemer** | SR (reset priority) | Sikkerhed: Reset skal altid virke |
| **Emergency stop** | SR (reset priority) | Stop-knap skal have prioritet |
| **Motor start/stop** | RS (set priority) | Start-kommando skal kunne overrule |
| **Process override** | RS (set priority) | Manuel start skal kunne forcere |
| **Fault latching** | SR (reset priority) | Reset skal kunne cleare fejl |

---

## Memory Usage

### Per Program (v4.7.3+)

| Component | Instances | Size Each | Total |
|-----------|-----------|-----------|-------|
| **Timers** | 8 | 24 bytes | 192 bytes |
| **Edges** | 8 | 8 bytes | 64 bytes |
| **Counters** | 8 | 20 bytes | 160 bytes |
| **Latches** (v4.7.3) | 8 | 4 bytes | 32 bytes |
| **Overhead** | - | 4 bytes | 4 bytes |
| **TOTAL** | - | - | **452 bytes** |

### System-Wide (4 Programs)

- **Maximum**: 4 × 452 = **1.81 KB**
- **Actual usage**: Allocated only when stateful functions used
- **DRAM Impact**: +64 bytes vs. baseline (optimized via union)

---

## Limitations

### Instance Limits

Per program:
- ✅ Max 8 R_TRIG instances
- ✅ Max 8 F_TRIG instances
- ✅ Max 8 TON instances
- ✅ Max 8 TOF instances
- ✅ Max 8 TP instances
- ✅ Max 8 CTU instances
- ✅ Max 8 CTD instances
- ✅ Max 8 CTUD instances (teoretisk - VM support mangler)
- ✅ Max 8 SR instances (v4.7.3)
- ✅ Max 8 RS instances (v4.7.3)

**Compiler Error:**
```
Too many timer instances (max 8)
Too many edge detector instances (max 8)
Too many counter instances (max 8)
Too many latch instances (max 8)
```

### Function Support

| Function | Status | VM Support | Notes |
|----------|--------|------------|-------|
| EXP | ✅ Full | ✅ Yes | With overflow protection |
| LN | ✅ Full | ✅ Yes | Input validation |
| LOG | ✅ Full | ✅ Yes | Input validation |
| POW | ✅ Full | ✅ Yes | NaN/INF protection |
| R_TRIG | ✅ Full | ✅ Yes | Edge detection |
| F_TRIG | ✅ Full | ✅ Yes | Edge detection |
| TON | ✅ Full | ✅ Yes | On-delay timer |
| TOF | ✅ Full | ✅ Yes | Off-delay timer |
| TP | ✅ Full | ✅ Yes | Pulse timer |
| CTU | ✅ Full | ✅ Yes | Count-up |
| CTD | ✅ Full | ✅ Yes | Count-down |
| CTUD | ⚠️ Partial | ❌ No | Requires VM update |

---

## Eksempler

### Eksempel 1: Temperature Controller med Hysteresis

```st
PROGRAM TempControl
VAR
  temp : REAL;
  setpoint : REAL := 25.0;
  heater : BOOL;
  heating_edge : BOOL;
  heating_time : BOOL;
  on_delay : INT := 5000;  (* 5 second delay *)
END_VAR

(* Simple hysteresis *)
IF temp < (setpoint - 1.0) THEN
  heater := TRUE;
ELSIF temp > (setpoint + 1.0) THEN
  heater := FALSE;
END_IF;

(* Log heating events *)
heating_edge := R_TRIG(heater);

(* Delayed heater activation (protection) *)
heating_time := TON(heater, on_delay);
END_PROGRAM
```

### Eksempel 2: Production Line Counter

```st
PROGRAM ProductionCounter
VAR
  sensor : BOOL;
  reset_btn : BOOL;
  batch_done : BOOL;
  batch_size : INT := 100;
  total_batches : INT := 0;
  batch_complete_edge : BOOL;
END_VAR

(* Count products in batch *)
batch_done := CTU(sensor, reset_btn, batch_size);

(* Increment batch counter on completion *)
batch_complete_edge := R_TRIG(batch_done);
IF batch_complete_edge THEN
  total_batches := total_batches + 1;
END_IF;
END_PROGRAM
```

### Eksempel 3: Exponential Moving Average

```st
PROGRAM ExponentialFilter
VAR
  raw_value : REAL;
  filtered : REAL;
  alpha : REAL := 0.1;  (* Smoothing factor *)
  one_minus_alpha : REAL;
END_VAR

(* Calculate (1 - alpha) *)
one_minus_alpha := 1.0 - alpha;

(* EMA formula: filtered = alpha * raw + (1-alpha) * filtered *)
filtered := (alpha * raw_value) + (one_minus_alpha * filtered);
END_PROGRAM
```

### Eksempel 4: Multi-Timer Sequence

```st
PROGRAM SequenceControl
VAR
  start_btn : BOOL;
  step1_done, step2_done, step3_done : BOOL;
  valve1, valve2, valve3 : BOOL;
END_VAR

(* Step 1: Open valve 1 for 3 seconds *)
valve1 := TON(start_btn, 3000);
step1_done := F_TRIG(valve1);

(* Step 2: Open valve 2 for 2 seconds *)
valve2 := TON(step1_done, 2000);
step2_done := F_TRIG(valve2);

(* Step 3: Open valve 3 for 5 seconds *)
valve3 := TON(step2_done, 5000);
step3_done := F_TRIG(valve3);
END_PROGRAM
```

---

## Troubleshooting

### Problem: "Too many X instances (max 8)"

**Cause**: Exceeded instance limit for stateful function type.

**Solution:**
1. Reduce number of function calls in program
2. Split into multiple ST Logic programs
3. Reuse instances where possible

**Example:**
```st
(* BAD - 9 instances *)
e1 := R_TRIG(in1);
e2 := R_TRIG(in2);
(* ... 7 more ... *)
e9 := R_TRIG(in9);  (* ERROR! *)

(* GOOD - Reuse with OR *)
any_edge := R_TRIG(in1 OR in2 OR in3);
```

---

### Problem: Timer aldrig timeout

**Cause**: Negativ PT værdi konverteret til huge unsigned number.

**Solution:** Brug kun positive PT værdier.

**Fixed in v4.7:**
```st
(* Old behavior *)
timer := TON(input, -1000);  (* Never times out! *)

(* v4.7+ behavior *)
timer := TON(input, -1000);  (* Treated as PT=0, immediate timeout *)
```

---

### Problem: POW() returnerer 0 uventet

**Cause**: Invalid operation → NaN/INF → protected til 0.

**Diagnostics:**
```st
(* These all return 0.0 in v4.7+ *)
result := POW(-2.0, 0.5);    (* Negative base, fractional exp *)
result := POW(0.0, -1.0);    (* Division by zero *)
result := POW(1000.0, 1000.0); (* Overflow *)
```

**Solution:** Valider inputs:
```st
IF base > 0.0 THEN
  result := POW(base, exponent);
ELSE
  (* Handle error *)
END_IF;
```

---

### Problem: Counter holder ved MAX value

**Cause**: Overflow protection kicking in.

**Expected Behavior (v4.7+):**
```st
(* CV reaches INT32_MAX and stops incrementing *)
CV = 2147483647
(* Further CU pulses ignored *)
```

**Solution:** Designed behavior - no fix needed. Reset counter periodically.

---

### Problem: millis() overflow fejl

**Cause**: 32-bit unsigned timer overflow hver 49.7 dage.

**Status:** ✅ **CORRECTLY HANDLED**

Unsigned arithmetic håndterer overflow korrekt:
```cpp
uint32_t now = millis();
uint32_t start = 4294967200;  // Near overflow
uint32_t elapsed = now - start;  // Works correctly even after overflow
```

**No action required.**

---

## Version History

### v4.7.0 (Build #920 - 2026-01-01)

**New Functions:**
- ✅ EXP(x) - Exponential function
- ✅ LN(x) - Natural logarithm
- ✅ LOG(x) - Base-10 logarithm
- ✅ POW(x,y) - Power function
- ✅ R_TRIG(CLK) - Rising edge detection
- ✅ F_TRIG(CLK) - Falling edge detection
- ✅ TON(IN,PT) - On-delay timer
- ✅ TOF(IN,PT) - Off-delay timer
- ✅ TP(IN,PT) - Pulse timer
- ✅ CTU(CU,RESET,PV) - Count up
- ✅ CTD(CD,LOAD,PV) - Count down
- ⚠️ CTUD (partial - VM support pending)

**Validations Added:**
- ✅ POW() NaN/INF protection
- ✅ EXP() overflow protection
- ✅ Timer PT negative value protection
- ✅ Counter CV overflow protection (INT32_MAX)

**Optimizations:**
- ✅ DRAM optimization via union (16 KB savings)
- ✅ Compile-time instance allocation (zero runtime overhead)
- ✅ 420 bytes per program (50% reduction from initial design)

**Known Issues:**
- ⚠️ CTUD VM integration pending (requires 5-argument support)
- ⚠️ No memory deallocation on program unload (minor leak on reload)

---

## Support & Feedback

**Bug Reports:** https://github.com/anthropics/claude-code/issues
**Documentation:** See `CODE_REVIEW_ST_FUNCTIONS.md` for detailed code review
**System Architecture:** See `CLAUDE_ARCH.md` for system overview

---

**© 2026 ESP32 Modbus RTU Server Project**
**License:** See project root LICENSE file
