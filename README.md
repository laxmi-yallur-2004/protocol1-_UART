# UART Driver – Ring Buffer + Error Handling + Stress Test

## WHAT

* RX/TX ring buffers
* UART error checking
* Data match check
* No-data-loss check
* Stress test
* Throughput measurement
* CPU load estimate

## CONNECTION

| STM32F401CCU6 | USB-TTL |
| ------------- | ------- |
| PA9 TX        | RX      |
| PA10 RX       | TX      |
| GND           | GND     |

**UART:** 115200 baud, 8N1

## TEST INPUT

Send data from the serial terminal and press **Enter**.

```text
laxmi
```

## STRESS TEST

The test checks:

```text
RX BYTES = TX BYTES
DATA MATCH = PASS
UART ERRORS = 0
```

## THROUGHPUT

```text
Throughput = TX bytes / measured transmission time
```

## CPU LOAD

```text
CPU Load = Processing cycles / measured test cycles × 100
```

## TEST RESULT

```text
RX BYTES: 6
TX BYTES: 6

DATA MATCH: PASS
NO DATA LOSS: PASS

FRAMING ERRORS: 0
PARITY ERRORS: 0
OVERRUN ERRORS: 0
NOISE ERRORS: 0

THROUGHPUT: 10345.0 bytes/s
CPU LOAD: 4 %

STRESS TEST: PASS
```
