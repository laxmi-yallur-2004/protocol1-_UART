import serial
import time

# -----------------------------------------
# UART SETTINGS
# -----------------------------------------

PORT = "COM3"
BAUD = 115200

TEST_SIZE = 10000


# -----------------------------------------
# OPEN SERIAL PORT
# -----------------------------------------

ser = serial.Serial(
    PORT,
    BAUD,
    timeout=5
)

time.sleep(2)

ser.reset_input_buffer()


print("====================================")
print("TASK 2 - UART PERFORMANCE")
print("====================================")

print("Port       :", PORT)
print("Baud Rate  :", BAUD)
print("Frame      : 8N1")
print("Test Bytes :", TEST_SIZE)

print()
print("Sending START command...")


# -----------------------------------------
# START TEST
# -----------------------------------------

ser.write(b"S")


# -----------------------------------------
# WAIT FOR BEGIN
# -----------------------------------------

while True:

    line = ser.readline()

    if not line:
        continue

    text = line.decode(
        "ascii",
        errors="ignore"
    ).strip()

    if text:
        print(text)

    if text == "BEGIN":
        break


# -----------------------------------------
# RECEIVE 10,000 BYTES
# -----------------------------------------

print()
print("Receiving data...")

start_time = time.perf_counter()

data = ser.read(TEST_SIZE)

end_time = time.perf_counter()


# -----------------------------------------
# CALCULATE THROUGHPUT
# -----------------------------------------

elapsed_time = end_time - start_time

received_bytes = len(data)

if elapsed_time > 0:
    throughput = received_bytes / elapsed_time
else:
    throughput = 0


# -----------------------------------------
# CHECK BYTE COUNT
# -----------------------------------------

if received_bytes == TEST_SIZE:
    byte_result = "PASS"
else:
    byte_result = "FAIL"


# -----------------------------------------
# CHECK DATA INTEGRITY
# -----------------------------------------

expected_data = b"A" * TEST_SIZE

if data == expected_data:
    integrity_result = "PASS"
else:
    integrity_result = "FAIL"


# -----------------------------------------
# DISPLAY PYTHON RESULTS
# -----------------------------------------

print()
print("====================================")
print("PYTHON MEASUREMENT")
print("====================================")

print("Bytes Received :", received_bytes)
print("Expected Bytes :", TEST_SIZE)

print("Byte Count     :", byte_result)

print("Data Integrity :", integrity_result)

print(
    "Time           : {:.6f} seconds".format(
        elapsed_time
    )
)

print(
    "Throughput     : {:.2f} bytes/sec".format(
        throughput
    )
)


# -----------------------------------------
# READ ARDUINO RESULTS
# -----------------------------------------

print()
print("====================================")
print("ARDUINO MEASUREMENT")
print("====================================")

while True:

    line = ser.readline()

    if not line:
        break

    text = line.decode(
        "ascii",
        errors="ignore"
    ).strip()

    if text:
        print(text)

    if text == "READY":
        break


# -----------------------------------------
# THEORETICAL THROUGHPUT
# -----------------------------------------

theoretical = BAUD / 10


print()
print("====================================")
print("THEORETICAL UART")
print("====================================")

print(
    "Maximum Throughput : {:.2f} bytes/sec".format(
        theoretical
    )
)


# -----------------------------------------
# CLOSE
# -----------------------------------------

ser.close()

print()
print("====================================")
print("TASK 2 COMPLETE")
print("====================================")
