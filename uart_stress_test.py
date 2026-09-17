import serial
import time

PORT = "COM3"
BAUD = 115200
TEST_BYTES = 1000

# Open Arduino COM port
ser = serial.Serial(
    PORT,
    BAUD,
    timeout=1
)

# Arduino Uno resets when COM port opens
time.sleep(2)

print("UART Stress Test Started")
print("Starting Arduino stress test...")
print()

# Tell Arduino to start the test
ser.write(b"TEST\r\n")
ser.flush()

echoed = 0
test_started = False
result_received = False
line_buffer = ""

try:

    while not result_received:

        # Read one byte from Arduino
        data = ser.read(1)

        # Nothing received
        if not data:
            continue

        # Convert byte to character
        char = data.decode(
            "ascii",
            errors="replace"
        )

        # Display Arduino output
        print(
            char,
            end="",
            flush=True
        )

        # Build a complete line
        line_buffer += char

        # Check when a line is complete
        if char == "\n":

            line = line_buffer.strip()

            line_buffer = ""

            # Arduino is starting the 1000-byte test
            if line == "Sending 1000 bytes...":

                test_started = True

                print()
                print("Python: Echo started")

            # Arduino finished the test
            if line == "STRESS TEST: PASS":

                result_received = True

            elif line == "STRESS TEST: FAIL":

                result_received = True

        # Echo the 1000 test bytes back to Arduino
        if test_started and echoed < TEST_BYTES:

            # Test data is A-Z
            if 65 <= data[0] <= 90:

                # Send byte back to Arduino
                ser.write(data)
                ser.flush()

                echoed += 1

                # Show progress every 100 bytes
                if echoed % 100 == 0:

                    print(
                        f"\nPython echoed: "
                        f"{echoed}/{TEST_BYTES}",
                        flush=True
                    )

                # All 1000 bytes have been echoed
                if echoed == TEST_BYTES:

                    print()
                    print("Python: 1000 bytes echoed.")
                    print(
                        "Python: Waiting for Arduino result..."
                    )

except KeyboardInterrupt:

    print()
    print("Test stopped by user.")

finally:

    ser.close()

    print("Serial port closed.")
