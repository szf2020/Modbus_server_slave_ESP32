# 🚀 ST Logic Mode - Complete System Guide

**ESP32 Modbus RTU Server v2.1.0** - Structured Text Programming Engine

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Quick Start](#quick-start)
4. [ST Language Features](#st-language-features)
5. [Variable Bindings](#variable-bindings)
6. [CLI Commands](#cli-commands)
7. [Error Diagnostics](#error-diagnostics)
8. [Examples](#examples)
9. [Testing](#testing)
10. [Troubleshooting](#troubleshooting)

---

## Overview

ST Logic Mode allows you to upload and execute **Structured Text (ST) programs** directly on the ESP32 Modbus server. Programs run independently, control GPIO pins and Modbus registers, and provide real-time logic automation.

### Key Features

- ✅ **4 Independent Logic Programs** (Logic1-Logic4)
- ✅ **IEC 61131-3 Structured Text** (ST-Light Profile)
- ✅ **Bytecode Compilation** (<100ms per program)
- ✅ **Non-Blocking Execution** (10ms cycle time, 100 Hz)
- ✅ **Modbus Integration** - Direct register/coil access
- ✅ **GPIO Control** - UP to 34 GPIO pins via variable binding
- ✅ **Persistent Storage** - Programs and bindings saved to NVS
- ✅ **Error Diagnostics** - Compilation errors, runtime errors, statistics
- ✅ **Variable Binding** - Intuitive register mapping by variable name

---

## Architecture

### System Layers

```
┌─────────────────────────────────────────────────────────┐
│ Layer 7: CLI Interface                                  │
│ set logic <id> upload/bind/enabled                      │
│ show logic <id|all|program|errors|stats>                │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│ Layer 6: ST Logic Engine                                │
│ - Program compilation (ST → bytecode)                   │
│ - Bytecode execution (10ms cycle)                       │
│ - Error handling & statistics                           │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│ Layer 5: Unified Variable Mapping                       │
│ - GPIO pin ↔ Discrete Input/Coil mapping               │
│ - ST variable ↔ Register mapping                        │
│ - Persistent NVS storage                                │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│ Layer 4: Modbus Server                                  │
│ - FC01-10 implementation                                │
│ - Register/Coil arrays                                  │
│ - RTU protocol handling                                 │
└─────────────────────────────────────────────────────────┘
```

### 3-Phase Execution Model (Every 10ms)

```
Phase 1: SYNC INPUTS (read all inputs)
  ├─ GPIO pins → Discrete inputs
  └─ Modbus registers → ST variables (INPUT bindings)

Phase 2: EXECUTE PROGRAMS (run enabled ST logic)
  ├─ Logic1 executes (if enabled)
  ├─ Logic2 executes (if enabled)
  ├─ Logic3 executes (if enabled)
  └─ Logic4 executes (if enabled)

Phase 3: SYNC OUTPUTS (write all outputs)
  ├─ Coils → GPIO pins
  └─ ST variables → Modbus registers (OUTPUT bindings)
```

---

## Quick Start

### 1. Enable GPIO2 (Optional - for LED control demo)

```bash
set gpio2 user_mode:true
```

### 2. Upload ST Program

```bash
set logic 1 upload "VAR counter: INT; led: BOOL; END_VAR IF counter > 50 THEN led := TRUE; ELSE led := FALSE; END_IF;"
```

**Output:**
```
✓ COMPILATION SUCCESSFUL
  Program: Logic1
  Source: 150 bytes
  Bytecode: 32 instructions
  Variables: 2
```

### 3. Bind Variables to Modbus

```bash
# Bind counter to read from Holding Register #100
set logic 1 bind counter reg:100

# Bind led to write to Coil #0 (GPIO2)
set logic 1 bind led coil:0
```

**Output:**
```
[OK] Logic1: var[0] (counter) ← Modbus HR#100 (input)
[OK] Logic1: var[1] (led) → Modbus Coil#0 (output)
```

### 4. Enable Program

```bash
set logic 1 enabled:true
```

**Output:**
```
[OK] Logic1 ENABLED
```

### 5. Test It!

```bash
# Set counter value (will affect LED)
set holding_register 100 75

# Verify program is working
show logic 1
```

---

## ST Language Features

### Supported Data Types

| Type | Size | Range | Example |
|------|------|-------|---------|
| **BOOL** | 1 bit | TRUE/FALSE | `led := TRUE` |
| **INT** | 16-bit | -32768 to 32767 | `counter := 100` |
| **DWORD** | 32-bit | 0 to 4294967295 | `value := 1000000` |
| **REAL** | 32-bit float | IEEE 754 | `temp := 25.5` |

### Variable Declaration

```structured-text
VAR
  counter: INT;           (* Input counter *)
  led: BOOL;              (* Output LED state *)
  temperature: REAL;      (* Sensor temperature *)
  flags: DWORD;           (* Status flags *)
END_VAR
```

### Control Flow Statements

#### IF/ELSIF/ELSE

```structured-text
IF counter > 100 THEN
  led := TRUE;
ELSIF counter > 50 THEN
  led := FALSE;
ELSE
  led := FALSE;
END_IF;
```

#### FOR Loop

```structured-text
VAR
  i: INT;
  sum: INT;
END_VAR

sum := 0;
FOR i := 1 TO 10 DO
  sum := sum + i;
END_FOR;

(* Result: sum = 55 *)
```

#### WHILE Loop

```structured-text
VAR
  counter: INT;
END_VAR

counter := 0;
WHILE counter < 100 DO
  counter := counter + 1;
END_WHILE;
```

#### CASE Statement

```structured-text
VAR
  mode: INT;
  output: INT;
END_VAR

CASE mode OF
  1:
    output := 10;
  2:
    output := 20;
  ELSE
    output := 0;
END_CASE;
```

### Built-in Functions

#### Mathematical

```structured-text
result := ABS(-42);                    (* → 42 *)
min_val := MIN(10, 5);                 (* → 5 *)
max_val := MAX(10, 5);                 (* → 10 *)
sum_val := SUM(10, 5);                 (* → 15 *)
sqrt_val := SQRT(16.0);                (* → 4.0 *)
rounded := ROUND(3.7);                 (* → 4 *)
truncated := TRUNC(3.9);               (* → 3 *)
floored := FLOOR(3.7);                 (* → 3 *)
ceiled := CEIL(3.2);                   (* → 4 *)
```

#### Type Conversion

```structured-text
real_val := INT_TO_REAL(42);           (* 42 → 42.0 *)
int_val := REAL_TO_INT(3.7);           (* 3.7 → 3 *)
bool_from_int := INT_TO_BOOL(1);       (* 1 → TRUE *)
int_from_bool := BOOL_TO_INT(TRUE);    (* TRUE → 1 *)
dword_val := INT_TO_DWORD(42);         (* 42 → 42 (32-bit) *)
int_from_dword := DWORD_TO_INT(1000);  (* 1000 → 1000 *)
```

---

## Variable Bindings

### Binding Syntax (New!)

Variables connect to Modbus via intuitive binding syntax:

```bash
set logic <id> bind <var_name> reg:<register>        # Holding Register (INT/DWORD)
set logic <id> bind <var_name> coil:<coil>           # Coil (BOOL output)
set logic <id> bind <var_name> input-dis:<input>     # Discrete Input (BOOL input)
```

### Binding Types

#### 1. Holding Register Binding (`reg:`)

**For INT or DWORD variables**

```bash
set logic 1 bind counter reg:100
```

**Behavior:**
- **Before execution:** Read HR#100 → `counter` variable
- **After execution:** Write `counter` → HR#100
- **Direction:** Bidirectional (read & write each cycle)

#### 2. Coil Binding (`coil:`)

**For BOOL variables (output only)**

```bash
set logic 1 bind led coil:10
```

**Behavior:**
- **Before execution:** Coil #10 value is ignored
- **After execution:** Write `led` → Coil#10
- **Direction:** Output only
- **Mapping:** TRUE=1, FALSE=0

#### 3. Discrete Input Binding (`input-dis:`)

**For BOOL variables (input only)**

```bash
set logic 1 bind button input-dis:5
```

**Behavior:**
- **Before execution:** Read Discrete Input #5 → `button`
- **After execution:** Value is ignored
- **Direction:** Input only
- **Mapping:** 1=TRUE, 0=FALSE

### Persistent Bindings

All bindings are **automatically saved to NVS** (persistent storage):

```bash
set logic 1 bind counter reg:100
# ✓ Binding saved to NVS
# ✓ Survives reboot
# ✓ Bindings reload on startup
```

### View Bindings

```bash
# Show bindings for specific program
show logic 1

# Output:
# Variable Bindings:
#   [0] counter ← HR#100 (input)
#   [1] led → Coil#0 (output)
#   [2] button ← Input#5 (input)
#   Total: 3 bindings
```

---

## CLI Commands

### Upload & Compile

```bash
# Upload and compile ST program
set logic <id> upload "<st_code>"

# Example:
set logic 1 upload "VAR x: INT; END_VAR x := 10; IF x > 5 THEN x := 1; END_IF;"
```

**Output on Success:**
```
✓ COMPILATION SUCCESSFUL
  Program: Logic1
  Source: 123 bytes
  Bytecode: 32 instructions
  Variables: 2
```

**Output on Error:**
```
╔════════════════════════════════════════════════════════╗
║            COMPILATION ERROR - Logic Program          ║
╚════════════════════════════════════════════════════════╝
Program ID: Logic1
Error: Parse error at line 3: Missing semicolon
Source size: 256 bytes
```

### Program Control

```bash
# Enable/disable program
set logic <id> enabled:true
set logic <id> enabled:false

# Delete program
set logic <id> delete

# Bind variables
set logic <id> bind <var_name> reg:<register>
set logic <id> bind <var_name> coil:<coil>
set logic <id> bind <var_name> input-dis:<input>
```

### Show/Monitor

```bash
# Show detailed program info with bindings
show logic <id>

# Show overview of all programs (with status icons)
show logic program

# Show only programs with errors
show logic errors

# Show all programs summary
show logic all

# Show engine statistics
show logic stats
```

### Show Logic Program (Status Icons)

```bash
show logic program

Output:
=== All Logic Programs ===

  [1] Logic1 🟢 ACTIVE
      Source: 150 bytes | Variables: 2
      Executions: 1540 | Errors: 0

  [2] Logic2 🟡 DISABLED
      Source: 200 bytes | Variables: 3
      Executions: 0 | Errors: 0

  [3] Logic3 🔴 FAILED
      Source: 300 bytes | Variables: 4
      Last error: Stack overflow at instruction 15

  [4] Logic4 ⚪ EMPTY
```

**Status Icons:**
- 🟢 **ACTIVE** = Compiled and enabled (running)
- 🟡 **DISABLED** = Compiled but not enabled
- 🔴 **FAILED** = Compilation error or runtime errors
- ⚪ **EMPTY** = No program uploaded

### Show Logic Errors (Error Diagnostics)

```bash
show logic errors

Output:
=== Logic Program Errors ===

  [Logic3] Logic3
    Status: NOT COMPILED
    Error: Parse error: Missing semicolon after variable declaration
    Runtime Errors: 5
    Error Rate: 3.33% (5/150 executions)

  Total programs with errors: 1/4
```

---

## Error Diagnostics

### Compilation Errors

**When you upload invalid ST code:**

```bash
set logic 1 upload "VAR x: INT END_VAR"
(missing semicolon)

Output:
╔════════════════════════════════════════════════════════╗
║            COMPILATION ERROR - Logic Program          ║
╚════════════════════════════════════════════════════════╝
Program ID: Logic1
Error: Parse error at line 1: Missing semicolon after variable declaration
Source size: 30 bytes
```

### Runtime Errors

**Programs can fail during execution:**

```bash
# Stack overflow (too many nested loops/calls)
# Division by zero
# Invalid type conversion
# Out of bounds memory access

# View runtime errors:
show logic errors

# Shows:
# Error Rate: 3.33% (5/150 executions)
# Last Error: Stack overflow at instruction 15
```

### Error Statistics

```bash
show logic stats

Output:
=== Logic Engine Statistics ===
Programs Compiled: 2/4
Programs Enabled: 1/4
Total Executions: 3500
Total Errors: 5
Error Rate: 0.14%
```

---

## Examples

### Example 1: Simple Threshold Logic

```structured-text
VAR
  sensor_value: INT;
  heater: BOOL;
  cooler: BOOL;
END_VAR

IF sensor_value < 15 THEN
  heater := TRUE;
  cooler := FALSE;
ELSIF sensor_value > 25 THEN
  heater := FALSE;
  cooler := TRUE;
ELSE
  heater := FALSE;
  cooler := FALSE;
END_IF;
```

**CLI:**
```bash
set logic 1 upload "VAR sensor_value: INT; heater: BOOL; cooler: BOOL; END_VAR IF sensor_value < 15 THEN heater := TRUE; cooler := FALSE; ELSIF sensor_value > 25 THEN heater := FALSE; cooler := TRUE; ELSE heater := FALSE; cooler := FALSE; END_IF;"

set logic 1 bind sensor_value reg:10
set logic 1 bind heater coil:0
set logic 1 bind cooler coil:1

set logic 1 enabled:true

# Test
set holding_register 10 20    # Heater & cooler OFF
set holding_register 10 10    # Heater ON
set holding_register 10 30    # Cooler ON
```

### Example 2: Counter with Accumulator

```structured-text
VAR
  pulse_input: BOOL;
  counter: INT;
  total: INT;
END_VAR

IF pulse_input THEN
  counter := counter + 1;
  IF counter > 10 THEN
    total := total + 1;
    counter := 0;
  END_IF;
END_IF;
```

**CLI:**
```bash
set logic 2 upload "VAR pulse_input: BOOL; counter: INT; total: INT; END_VAR IF pulse_input THEN counter := counter + 1; IF counter > 10 THEN total := total + 1; counter := 0; END_IF; END_IF;"

set logic 2 bind pulse_input input-dis:0
set logic 2 bind counter reg:20
set logic 2 bind total reg:21

set logic 2 enabled:true
```

### Example 3: State Machine (Traffic Light)

```structured-text
VAR
  state: INT;
  red: BOOL;
  yellow: BOOL;
  green: BOOL;
  timer: INT;
END_VAR

timer := timer + 1;

CASE state OF
  0:  (* RED state - 30 cycles *)
    red := TRUE;
    yellow := FALSE;
    green := FALSE;
    IF timer > 30 THEN
      state := 1;
      timer := 0;
    END_IF;
  1:  (* YELLOW state - 5 cycles *)
    red := FALSE;
    yellow := TRUE;
    green := FALSE;
    IF timer > 5 THEN
      state := 2;
      timer := 0;
    END_IF;
  2:  (* GREEN state - 25 cycles *)
    red := FALSE;
    yellow := FALSE;
    green := TRUE;
    IF timer > 25 THEN
      state := 0;
      timer := 0;
    END_IF;
END_CASE;
```

**CLI:**
```bash
set logic 3 upload "VAR state: INT; red: BOOL; yellow: BOOL; green: BOOL; timer: INT; END_VAR timer := timer + 1; CASE state OF 0: red := TRUE; yellow := FALSE; green := FALSE; IF timer > 30 THEN state := 1; timer := 0; END_IF; 1: red := FALSE; yellow := TRUE; green := FALSE; IF timer > 5 THEN state := 2; timer := 0; END_IF; 2: red := FALSE; yellow := FALSE; green := TRUE; IF timer > 25 THEN state := 0; timer := 0; END_IF; END_CASE;"

set logic 3 bind state reg:30
set logic 3 bind timer reg:31
set logic 3 bind red coil:0
set logic 3 bind yellow coil:1
set logic 3 bind green coil:2

set logic 3 enabled:true

# Traffic light cycles automatically!
```

---

## Testing

### Automated Test Suite

**Run comprehensive tests (18 test cases):**

```bash
python test_st_logic_comprehensive.py
```

**Tests:**
- Program overview (`show logic program`)
- Error diagnostics (`show logic errors`)
- Upload & compilation
- Variable binding
- Program enable/disable
- Error handling
- Statistics

**Expected Output:**
```
╔══════════════════════════════════════════════╗
║   ST Logic Mode - Comprehensive Test Suite   ║
╚══════════════════════════════════════════════╝

Results: 18/18 tests passed (100.0%)

  Show Commands: 4/4 ✓
  Upload/Compile: 2/2 ✓
  Binding: 2/2 ✓
  Control: 3/3 ✓
  Error Handling: 2/2 ✓
  Final Status: 2/2 ✓

✓ All tests passed!
```

### Interactive GPIO2 LED Demo

**Control GPIO2 LED via ST Logic (11 steps):**

```bash
python demo_gpio2_led.py
```

**What it does:**
1. Activates GPIO2 user mode
2. Uploads ST program
3. Binds counter to register
4. Binds LED to GPIO2 coil
5. Enables program
6. Tests LED ON (counter=75)
7. Tests LED OFF (counter=25)
8. Tests LED ON (counter=100)
9. Shows program overview
10. Shows statistics

**Physical Result:** Watch GPIO2 LED toggle on/off! 🔵⚫

---

## Troubleshooting

### Problem: Program won't compile

**Error:**
```
Parse error at line 3: Missing semicolon
```

**Solution:**
- Check syntax: every statement needs semicolon (`;`)
- Keywords are case-insensitive
- VAR/END_VAR must be paired

**Example (WRONG):**
```
VAR x: INT END_VAR IF x > 10 THEN x := 1 END_IF
```

**Example (CORRECT):**
```
VAR x: INT; END_VAR IF x > 10 THEN x := 1; END_IF;
```

### Problem: Program compiles but no output

**Cause:** Program not enabled or variables not bound

**Solution:**
```bash
# Check status
show logic 1

# Enable if disabled
set logic 1 enabled:true

# Check bindings
show logic 1

# If no bindings, add them
set logic 1 bind counter reg:100
set logic 1 bind output coil:0
```

### Problem: LED not responding to program

**Cause:** GPIO2 still in heartbeat mode or binding not set

**Solution:**
```bash
# Enable GPIO2 user mode
set gpio2 user_mode:true

# Verify with
show gpio

# Should show: GPIO2 user_mode: YES

# Check bindings
show logic 1

# Should show: [1] led → Coil#0 (output)
```

### Problem: Execution errors appearing

**Check errors:**
```bash
show logic errors

# Output shows:
# Error Rate: 3.33% (5/150 executions)
# Last Error: Stack overflow at instruction 15

# Fix by:
# 1. Simplify loops
# 2. Reduce variable count
# 3. Check for infinite loops
```

---

## Performance Notes

- **Compilation Time:** <100ms per program
- **Execution Time:** ~1-5ms per cycle (10ms default interval)
- **Cycle Frequency:** 100 Hz (every 10ms)
- **Memory:** ~50KB for 4 programs with source + bytecode
- **Register Limit:** 160 holding registers (0-159)
- **Coil Limit:** 256 coils (0-255)
- **Max Variables:** 32 per program
- **Max Instructions:** 512 per program
- **Execution Steps:** 10,000 step limit (prevents infinite loops)

---

## Architecture Summary

### ST Logic Mode = Complete Automation Engine

```
User writes ST code
        ↓
Compiler validates syntax
        ↓
Bytecode generator optimizes
        ↓
Program stored in NVS (persistent)
        ↓
Main loop executes 100Hz:
  - Phase 1: Sync inputs (registers/coils → variables)
  - Phase 2: Execute enabled programs
  - Phase 3: Sync outputs (variables → registers/coils)
        ↓
Modbus RTU master reads/writes registers
        ↓
GPIO pins updated in real-time
```

---

## Resources

- **Documentation:** `/docs/ST_USAGE_GUIDE.md`
- **Demo Guide:** `/ST_GPIO2_DEMO.md`
- **Test Suite:** `test_st_logic_comprehensive.py`
- **Demo Script:** `demo_gpio2_led.py`

---

## Summary

ST Logic Mode provides:

✅ **Complete programming environment** on ESP32
✅ **Modbus integration** for register access
✅ **GPIO control** via variable binding
✅ **Persistent storage** of programs & bindings
✅ **Real-time execution** at 100 Hz
✅ **Error diagnostics** for debugging
✅ **Intuitive CLI** for easy use
✅ **Non-blocking** (doesn't interfere with Modbus server)

**Perfect for industrial automation, PLC logic, and real-time control!**

---

**Questions?** Check `/docs/ST_USAGE_GUIDE.md` or run the test suite!

**Happy Programming!** 🚀
