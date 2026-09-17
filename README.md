# UART Protocol – Task 1 & Task 2

## Overview

This project implements and tests **UART communication on Arduino Uno using register-level programming**.

The UART configuration used in both tasks is:

```text
Baud Rate : 115200
Format    : 8N1
CPU Clock : 16 MHz
```

The project contains two tasks:

* **Task 1:** UART Driver + ISR + Ring Buffer + Error Handling + Stress Test
* **Task 2:** UART Throughput Measurement + CPU Load Estimate

---

# Task 1 – UART Driver + ISR + Ring Buffer + Error Handling + Stress Test

## Objective

The objective of Task 1 is to implement a reliable UART driver using AVR registers and verify that UART communication works correctly without data loss.

### Task 1 Features

* Register-level UART configuration
* UART TX
* UART RX
* RX interrupt
* TX interrupt
* RX ring buffer
* TX ring buffer
* UART error handling
* Stress test
* Data integrity verification
* No-data-loss verification

## Task 1 Files

```text
uart_stress_test.ino
uart_stress_test.py
```

### `uart_stress_test.ino`

This is the Arduino-side UART driver.

It handles:

```text
UART Initialization
      ↓
RX/TX Interrupts
      ↓
RX/TX Ring Buffers
      ↓
Error Detection
      ↓
Stress Test
```

The UART driver checks:

```text
Framing Errors
Parity Errors
Data Overrun
RX Buffer Overflow
```

### `uart_stress_test.py`

This is the PC-side stress-test program.

It communicates with the Arduino and performs an echo test using **1000 bytes**.

The received data is checked against the transmitted data to verify that no bytes are lost or corrupted.

---

## Task 1 Stress Test

The test sends **1000 bytes**.

During the test, the Python program reports the progress:

```text
100/1000
200/1000
300/1000
...
1000/1000
```

The received data is then checked for:

* Mismatches
* RX buffer overflows
* Framing errors
* Parity errors
* Overrun errors

## Task 1 Full Output

```text
UART Stress Test Started
Starting Arduino stress test...


============================
UART STRESS TEST
============================
STRESS_START

Python: Echo started
Python echoed: 100/1000
Python echoed: 200/1000
Python echoed: 300/1000
Python echoed: 400/1000
Python echoed: 500/1000
Python echoed: 600/1000
Python echoed: 700/1000
Python echoed: 800/1000
Python echoed: 900/1000
Python echoed: 1000/1000

RESULT
Sent: 1000
Received: 1000
Mismatches: 0
RX Overflows: 0
Framing Errors: 0
Parity Errors: 0
Overrun Errors: 0
STRESS TEST: PASS

Serial port closed.
```

## Task 1 Result

```text
Sent            : 1000
Received        : 1000
Mismatches      : 0
RX Overflows    : 0
Framing Errors  : 0
Parity Errors   : 0
Overrun Errors  : 0
Result          : PASS
Data Loss       : NONE
```

The stress test successfully received all **1000 transmitted bytes** with **zero mismatches, zero buffer overflows, and zero UART errors**.

---

# Task 2 – UART Throughput Measurement + CPU Load Estimate

## Objective

The objective of Task 2 is to measure UART transmission performance and estimate the CPU load caused by the UART transmit ISR.

### Task 2 Measurements

* Number of transmitted bytes
* Transmission time in CPU cycles
* TX interrupt count
* ISR execution cycles
* UART throughput
* CPU load estimate
* Data integrity

## Task 2 Files

```text
throughput_and_cpu_load.ino
throughput_and_cpu_load.py
```

### `throughput_and_cpu_load.ino`

This is the Arduino-side performance measurement program.

It:

1. Transmits **10,000 bytes**.
2. Measures elapsed CPU cycles using Timer1.
3. Counts TX interrupts.
4. Measures TX ISR cycles.
5. Calculates throughput.
6. Estimates CPU load.

### `throughput_and_cpu_load.py`

This is the PC-side performance test program.

It:

1. Starts the Arduino test.
2. Receives 10,000 bytes.
3. Checks the received byte count.
4. Checks data integrity.
5. Measures PC-side throughput.
6. Displays the Arduino performance results.

---

## Task 2 Test Configuration

```text
CPU Clock : 16 MHz
Baud Rate : 115200
UART      : 8N1
Test Size : 10000 bytes
```

## Task 2 Full Output

```text
====================================
TASK 2 - UART PERFORMANCE
====================================
Port       : COM3
Baud Rate  : 115200
Frame      : 8N1
Test Bytes : 10000

Sending START command...
BEGIN

Receiving data...

====================================
PYTHON MEASUREMENT
====================================
Bytes Received : 10000
Expected Bytes : 10000
Byte Count     : PASS
Data Integrity : PASS
Time           : 0.847925 seconds
Throughput     : 11793.50 bytes/sec

====================================
ARDUINO MEASUREMENT
====================================
END
RESULTS
BYTES=10000
TIME_CYCLES=13602667
TX_INTERRUPTS=10001
ISR_CYCLES=490194
THROUGHPUT_BPS=11762
CPU_LOAD_PERCENT=3
TEST=PASS
READY

====================================
THEORETICAL UART
====================================
Maximum Throughput : 11520.00 bytes/sec

====================================
TASK 2 COMPLETE
====================================
```

## Task 2 Result

```text
Bytes              : 10000
Time Cycles        : 13602667
TX Interrupts      : 10001
ISR Cycles         : 490194
Throughput         : 11762 bytes/sec
CPU Load Estimate  : 3%
Data Integrity     : PASS
Test               : PASS
```

---

## Theoretical UART Throughput

The UART uses **8N1**.

Each byte contains:

```text
1 Start Bit
8 Data Bits
1 Stop Bit
----------------
10 Bits per Byte
```

Therefore:

```text
115200 / 10
= 11520 bytes/sec
```

The theoretical payload throughput is:

```text
11520 bytes/sec
```

The measured Arduino throughput was:

```text
11762 bytes/sec
```

The Python-side measurement was:

```text
11793.50 bytes/sec
```

---

## CPU Load Estimate

The UART transmit ISR execution time is compared with the total measurement time.

Final measured result:

```text
CPU Load Estimate = 3%
```

This represents the estimated CPU time spent executing the UART transmit ISR during the Task 2 measurement.

---

# Project Structure

```text
protocol1-_UART/
│
├── README.md
│
├── uart_stress_test.ino
├── uart_stress_test.py
│
├── throughput_and_cpu_load.ino
└── throughput_and_cpu_load.py
```

---

# Final Results

## Task 1

```text
UART Driver          : PASS
RX/TX ISR            : Implemented
RX/TX Ring Buffer    : Implemented
Error Handling       : Implemented
Bytes Sent           : 1000
Bytes Received       : 1000
Mismatches           : 0
RX Overflows         : 0
Framing Errors       : 0
Parity Errors        : 0
Overrun Errors       : 0
Stress Test          : PASS
Data Loss            : NONE
```

## Task 2

```text
Test Bytes           : 10000
Throughput           : 11762 bytes/sec
TX Interrupts        : 10001
ISR Cycles           : 490194
CPU Load Estimate    : 3%
Data Integrity       : PASS
Test                  : PASS
```

## Conclusion

**Task 1** demonstrates a register-level UART driver with interrupts, ring buffers, error handling, and a stress test confirming no data loss.

**Task 2** measures UART throughput and estimates the CPU load generated by the UART transmit interrupt.
