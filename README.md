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

* Interrupt-driven UART communication
* Ring buffering
* UART error checking
* Data-loss checking
* Stress testing with long data
* Throughput measurement
* CPU load measurement

---

## 4. Test Input

We sent long/repeated data through the UART terminal.

Example:

```text
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa...
```

The test was completed by pressing **ENTER**.

---

## 5. Output

```text
RX BYTES: 456
TX BYTES: 456

DATA MATCH: PASS
NO DATA LOSS: PASS

FRAMING ERRORS: 0
PARITY ERRORS: 0
OVERRUN ERRORS: 0
NOISE ERRORS: 0
BUFFER OVERFLOW: 0

THROUGHPUT: 11283.3 bytes/s
CPU LOAD: 2 %

STRESS TEST: PASS
```

---

## 6. Result

* **Data received and transmitted successfully**
* **No data loss**
* **No UART errors**
* **No buffer overflow**
* **Stress test passed**
* **Throughput measured**
* **CPU load measured**

### Final Status

**PASS – UART driver qualification completed successfully.**
