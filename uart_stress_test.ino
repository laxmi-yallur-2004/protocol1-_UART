#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define BAUD 115200UL

/* 115200 baud using UART double-speed mode */
#define UBRR_VALUE ((F_CPU / (8UL * BAUD)) - 1)

#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256


/* =====================================================
   RX RING BUFFER
   ===================================================== */

volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0;


/* =====================================================
   TX RING BUFFER
   ===================================================== */

volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;


/* =====================================================
   UART ERROR COUNTERS
   ===================================================== */

volatile uint16_t framing_errors = 0;
volatile uint16_t parity_errors = 0;
volatile uint16_t overrun_errors = 0;
volatile uint16_t rx_overflows = 0;


/* =====================================================
   UART INITIALIZATION
   ===================================================== */

void uart_init(void)
{
    /* Double speed mode */
    UCSR0A = (1 << U2X0);

    /* Baud rate */
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;

    /* Enable RX, TX and RX interrupt */
    UCSR0B =
        (1 << RXEN0) |
        (1 << TXEN0) |
        (1 << RXCIE0);

    /* 8 data bits, no parity, 1 stop bit = 8N1 */
    UCSR0C =
        (1 << UCSZ01) |
        (1 << UCSZ00);
}


/* =====================================================
   RX BUFFER PUT
   ===================================================== */

uint8_t rx_buffer_put(uint8_t data)
{
    uint8_t next_head;

    next_head = (uint8_t)(rx_head + 1);

    if (next_head == rx_tail)
    {
        rx_overflows++;
        return 0;
    }

    rx_buffer[rx_head] = data;
    rx_head = next_head;

    return 1;
}


/* =====================================================
   RX BUFFER GET
   ===================================================== */

uint8_t rx_buffer_get(uint8_t *data)
{
    if (rx_tail == rx_head)
    {
        return 0;
    }

    *data = rx_buffer[rx_tail];

    rx_tail = (uint8_t)(rx_tail + 1);

    return 1;
}


/* =====================================================
   TX BUFFER PUT
   ===================================================== */

uint8_t tx_buffer_put(uint8_t data)
{
    uint8_t next_head;

    next_head = (uint8_t)(tx_head + 1);

    if (next_head == tx_tail)
    {
        return 0;
    }

    tx_buffer[tx_head] = data;
    tx_head = next_head;

    /* Enable TX Data Register Empty interrupt */
    UCSR0B |= (1 << UDRIE0);

    return 1;
}


/* =====================================================
   SEND STRING
   ===================================================== */

void uart_send_string(const char *str)
{
    while (*str)
    {
        while (!tx_buffer_put((uint8_t)*str))
        {
        }

        str++;
    }
}


/* =====================================================
   SEND NUMBER
   ===================================================== */

void uart_send_number(uint16_t value)
{
    char buffer[6];
    uint8_t i = 0;

    if (value == 0)
    {
        while (!tx_buffer_put('0'))
        {
        }

        return;
    }

    while (value > 0)
    {
        buffer[i++] =
            (char)('0' + (value % 10));

        value /= 10;
    }

    while (i > 0)
    {
        while (!tx_buffer_put(buffer[--i]))
        {
        }
    }
}


/* =====================================================
   SEND ONE BYTE AND WAIT UNTIL TRANSMITTED
   ===================================================== */

void uart_send_byte_blocking(uint8_t data)
{
    /* Clear TX complete flag */
    UCSR0A |= (1 << TXC0);

    /* Put byte into TX buffer */
    while (!tx_buffer_put(data))
    {
    }

    /* Wait until TX ring buffer is empty */
    while (tx_tail != tx_head)
    {
    }

    /* Wait until UART hardware physically transmits */
    while (!(UCSR0A & (1 << TXC0)))
    {
    }

    /* Clear TX complete flag */
    UCSR0A |= (1 << TXC0);
}


/* =====================================================
   RECEIVE ONE BYTE
   ===================================================== */

uint8_t uart_receive_byte_blocking(uint8_t *data)
{
    while (!rx_buffer_get(data))
    {
    }

    return 1;
}


/* =====================================================
   WAIT FOR TX COMPLETE
   ===================================================== */

void uart_wait_tx_complete(void)
{
    while (tx_tail != tx_head)
    {
    }

    while (!(UCSR0A & (1 << TXC0)))
    {
    }

    UCSR0A |= (1 << TXC0);
}


/* =====================================================
   CLEAR RX BUFFER
   ===================================================== */

void uart_clear_rx(void)
{
    uint8_t data;

    while (rx_buffer_get(&data))
    {
    }
}


/* =====================================================
   RESET ERROR COUNTERS
   ===================================================== */

void uart_clear_errors(void)
{
    framing_errors = 0;
    parity_errors = 0;
    overrun_errors = 0;
    rx_overflows = 0;
}


/* =====================================================
   RX INTERRUPT
   ===================================================== */

ISR(USART_RX_vect)
{
    uint8_t status;
    uint8_t data;

    /* Read UART status */
    status = UCSR0A;

    /* Read received byte */
    data = UDR0;

    /* Framing error */
    if (status & (1 << FE0))
    {
        framing_errors++;
    }

    /* Data overrun */
    if (status & (1 << DOR0))
    {
        overrun_errors++;
    }

    /* Parity error */
    if (status & (1 << UPE0))
    {
        parity_errors++;
    }

    /* Store received byte */
    rx_buffer_put(data);
}


/* =====================================================
   TX INTERRUPT
   ===================================================== */

ISR(USART_UDRE_vect)
{
    if (tx_tail != tx_head)
    {
        UDR0 = tx_buffer[tx_tail];

        tx_tail = (uint8_t)(tx_tail + 1);
    }
    else
    {
        /* No more data */
        UCSR0B &= ~(1 << UDRIE0);
    }
}


/* =====================================================
   STRESS TEST
   ===================================================== */

void stress_test(void)
{
    uint16_t sent = 0;
    uint16_t received = 0;
    uint16_t mismatches = 0;

    uint8_t received_byte;
    uint8_t expected_byte;


    /* -----------------------------------------------
       Clear any old RX data
       ----------------------------------------------- */

    uart_clear_rx();


    /* -----------------------------------------------
       Clear error counters
       ----------------------------------------------- */

    uart_clear_errors();


    /* -----------------------------------------------
       Header
       ----------------------------------------------- */

    uart_send_string(
        "\r\n"
        "============================\r\n"
        "UART STRESS TEST\r\n"
        "============================\r\n"
    );

    uart_wait_tx_complete();


    /* -----------------------------------------------
       Start marker
       ----------------------------------------------- */

    uart_send_string(
        "STRESS_START\r\n"
    );

    uart_wait_tx_complete();


    /* -----------------------------------------------
       1000 BYTE ECHO TEST
       ----------------------------------------------- */

    for (uint16_t i = 0; i < 1000; i++)
    {
        /* Generate test pattern A-Z */
        uint8_t test_byte =
            (uint8_t)('A' + (i % 26));


        /* Send byte to PC */
        uart_send_byte_blocking(test_byte);

        sent++;


        /* Receive echoed byte */
        uart_receive_byte_blocking(&received_byte);

        received++;


        /* Expected byte */
        expected_byte = test_byte;


        /* Compare */
        if (received_byte != expected_byte)
        {
            mismatches++;
        }
    }


    /* -----------------------------------------------
       Result
       ----------------------------------------------- */

    uart_send_string(
        "\r\n"
        "RESULT\r\n"
    );

    uart_send_string("Sent: ");
    uart_send_number(sent);

    uart_send_string("\r\nReceived: ");
    uart_send_number(received);

    uart_send_string("\r\nMismatches: ");
    uart_send_number(mismatches);

    uart_send_string("\r\nRX Overflows: ");
    uart_send_number(rx_overflows);

    uart_send_string("\r\nFraming Errors: ");
    uart_send_number(framing_errors);

    uart_send_string("\r\nParity Errors: ");
    uart_send_number(parity_errors);

    uart_send_string("\r\nOverrun Errors: ");
    uart_send_number(overrun_errors);

    uart_send_string("\r\n");


    /* -----------------------------------------------
       PASS / FAIL
       ----------------------------------------------- */

    if (
        sent == 1000 &&
        received == 1000 &&
        mismatches == 0 &&
        rx_overflows == 0 &&
        framing_errors == 0 &&
        parity_errors == 0 &&
        overrun_errors == 0
    )
    {
        uart_send_string(
            "STRESS TEST: PASS\r\n"
            "NO DATA LOSS\r\n"
        );
    }
    else
    {
        uart_send_string(
            "STRESS TEST: FAIL\r\n"
        );
    }
}


/* =====================================================
   SETUP
   ===================================================== */

void setup(void)
{
    uart_init();

    /* Enable global interrupts */
    sei();


    uart_send_string(
        "\r\n"
        "============================\r\n"
        "UART REGISTER TEST\r\n"
        "============================\r\n"
        "Baud: 115200\r\n"
        "Frame: 8N1\r\n"
        "RX Interrupt: ENABLED\r\n"
        "TX Interrupt: ENABLED\r\n"
        "RX Ring Buffer: ENABLED\r\n"
        "TX Ring Buffer: ENABLED\r\n"
        "Error Handling: ENABLED\r\n"
        "UART READY\r\n"
        "\r\n"
        "Type TEST to start stress test.\r\n"
    );
}


/* =====================================================
   MAIN LOOP
   ===================================================== */

void loop(void)
{
    static char command[5];
    static uint8_t index = 0;

    /*
       Important:
       We do NOT start stress_test() immediately
       when CR is received.

       We wait until LF is also consumed.
    */
    static uint8_t command_ready = 0;

    uint8_t data;


    while (rx_buffer_get(&data))
    {
        /* ---------------------------------------------
           CARRIAGE RETURN
           --------------------------------------------- */

        if (data == '\r')
        {
            command[index] = '\0';

            if (
                index == 4 &&
                command[0] == 'T' &&
                command[1] == 'E' &&
                command[2] == 'S' &&
                command[3] == 'T'
            )
            {
                /*
                   Do NOT start test yet.

                   Wait for the following LF.
                */
                command_ready = 1;
            }
            else
            {
                /* Normal command echo */
                for (uint8_t i = 0; i < index; i++)
                {
                    while (!tx_buffer_put(
                        (uint8_t)command[i]
                    ))
                    {
                    }
                }

                while (!tx_buffer_put('\r'))
                {
                }

                while (!tx_buffer_put('\n'))
                {
                }
            }

            index = 0;

            continue;
        }


        /* ---------------------------------------------
           LINE FEED
           --------------------------------------------- */

        if (data == '\n')
        {
            /*
               TEST command is now completely consumed.

               Only now start the stress test.
            */
            if (command_ready)
            {
                command_ready = 0;

                stress_test();
            }

            continue;
        }


        /* ---------------------------------------------
           NORMAL CHARACTER
           --------------------------------------------- */

        if (index < 4)
        {
            command[index++] = (char)data;
        }
        else
        {
            /*
               Invalid/too-long command
            */
            index = 0;
            command_ready = 0;
        }
    }
}
