# UART Driver Qualification

## 1. Task

**Implement a UART driver using ISR and ring buffering, with error handling, stress testing, throughput measurement, and CPU load measurement.**

---

## 2. Setup

**Board:** STM32F401CCU6
**UART:** USART1
**Baud Rate:** 115200
**Format:** 8N1

**Connections:**

| STM32F401CCU6 | USB-TTL |
| ------------- | ------- |
| PA9 (TX)      | RX      |
| PA10 (RX)     | TX      |
| GND           | GND     |

---

## 3. What We Implemented

* Interrupt-driven UART communication with ring buffers (STM32 core Serial1)
* State machine: IDLE, ECHO, WAIT_CMD, SELFTEST, REPORT
* UART error checking (framing, parity, overrun, noise)
* Data-loss checking
* Stress testing with long data (built-in 4096-byte loopback test)
* Throughput measurement
* CPU load measurement

---

## 4. Test Input

Terminal line ending set to **Newline**. Three tests were done:

1. **Echo test:** typed `ROOPA` and pressed **ENTER**.
2. **Stress test:** typed `#` and pressed **ENTER**. The board sends 4096 bytes through its own UART and checks every byte it receives back.
3. **Long data test:** pasted about 2 KB of text as one line and pressed **ENTER** (repeated 10 times).

---

## 5. Output

Stress test output:

```text
===== UART QUALIFICATION RESULT (SELF-TEST) =====
RX BYTES: 4096
TX BYTES: 4096

DATA MATCH: PASS
NO DATA LOSS: PASS
PATTERN MISMATCHES: 0

FRAMING ERRORS: 0
PARITY ERRORS: 0
OVERRUN ERRORS: 0
NOISE ERRORS: 0
BUFFER OVERFLOW: 0

THROUGHPUT: 11468.5 bytes/s
CPU LOAD: 10.6 %

STRESS TEST: PASS
```

Summary of all tests:

| Test | RX / TX bytes | Errors | Throughput | CPU load | Result |
| ---- | ------------- | ------ | ---------- | -------- | ------ |
| Echo (ROOPA) | 6 / 6 | 0 | 10350.6 bytes/s | 3.3 % | PASS |
| Stress test (#) | 4096 / 4096 | 0 | 11468.5 bytes/s | 10.6 % | PASS |
| Long data (about 2 KB) | 2190 / 2190 | 0 | 11406.2 bytes/s | 3 % | PASS |

---

## 6. Result

* **Data received and transmitted successfully**
* **No data loss**
* **No UART errors**
* **No buffer overflow**
* **Stress test passed**
* **Throughput measured (about 99% of the 115200 baud limit)**
* **CPU load measured**

### Final Status

**PASS – UART driver qualification completed successfully.**
