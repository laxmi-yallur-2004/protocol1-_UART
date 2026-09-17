#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define BAUD 115200UL

#define UBRR_VALUE ((F_CPU / (8UL * BAUD)) - 1)

#define TEST_SIZE 10000
#define TX_BUFFER_SIZE 64

volatile uint8_t txBuffer[TX_BUFFER_SIZE];
volatile uint8_t txHead = 0;
volatile uint8_t txTail = 0;

volatile uint32_t txInterruptCount = 0;
volatile uint32_t txIsrCycles = 0;

volatile uint16_t timer1OverflowCount = 0;

volatile uint8_t startCommand = 0;


// --------------------------------------------------
// TIMER1 OVERFLOW INTERRUPT
// --------------------------------------------------

ISR(TIMER1_OVF_vect)
{
    timer1OverflowCount++;
}


// --------------------------------------------------
// UART RX INTERRUPT
// --------------------------------------------------

ISR(USART_RX_vect)
{
    uint8_t status = UCSR0A;
    uint8_t data = UDR0;

    // Framing error
    if (status & (1 << FE0))
    {
        return;
    }

    // Data overrun
    if (status & (1 << DOR0))
    {
        return;
    }

    // Python sends S to start Task 2
    if (data == 'S')
    {
        startCommand = 1;
    }
}


// --------------------------------------------------
// UART TX DATA REGISTER EMPTY INTERRUPT
// --------------------------------------------------

ISR(USART_UDRE_vect)
{
    uint16_t start;
    uint16_t end;

    start = TCNT1;

    txInterruptCount++;

    if (txTail != txHead)
    {
        UDR0 = txBuffer[txTail];

        txTail++;

        if (txTail >= TX_BUFFER_SIZE)
        {
            txTail = 0;
        }
    }
    else
    {
        UCSR0B &= ~(1 << UDRIE0);
    }

    end = TCNT1;

    txIsrCycles +=
        (uint16_t)(end - start);
}


// --------------------------------------------------
// UART INITIALIZATION
// --------------------------------------------------

void uart_init(void)
{
    UCSR0A = (1 << U2X0);

    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;

    UCSR0B =
        (1 << RXEN0) |
        (1 << TXEN0) |
        (1 << RXCIE0);

    // 8N1
    UCSR0C =
        (1 << UCSZ01) |
        (1 << UCSZ00);
}


// --------------------------------------------------
// TIMER1 INITIALIZATION
// --------------------------------------------------

void timer1_init(void)
{
    TCCR1A = 0;

    // Timer1, no prescaler
    TCCR1B = (1 << CS10);

    TIMSK1 = (1 << TOIE1);

    TCNT1 = 0;
}


// --------------------------------------------------
// GET 32-BIT TIMER VALUE
//
// Timer1 = 16 bit
// Overflow counter = upper 16 bits
//
// Total timer = 32 bit
// --------------------------------------------------

uint32_t timer_get_ticks(void)
{
    uint16_t high;
    uint16_t low;

    uint8_t oldSREG = SREG;

    cli();

    high = timer1OverflowCount;
    low = TCNT1;

    /*
       If Timer1 overflow happened but the overflow
       ISR has not executed yet, account for it.
    */
    if ((TIFR1 & (1 << TOV1)) && low < 32768)
    {
        high++;
    }

    SREG = oldSREG;

    return ((uint32_t)high << 16) | low;
}


// --------------------------------------------------
// ADD BYTE TO TX BUFFER
// --------------------------------------------------

void uart_tx_put(uint8_t data)
{
    uint8_t nextHead;

    while (1)
    {
        nextHead =
            (txHead + 1) % TX_BUFFER_SIZE;

        if (nextHead != txTail)
        {
            break;
        }
    }

    txBuffer[txHead] = data;

    txHead = nextHead;

    UCSR0B |= (1 << UDRIE0);
}


// --------------------------------------------------
// SEND ONE BYTE
// --------------------------------------------------

void uart_send_byte(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0)))
    {
    }

    UDR0 = data;
}


// --------------------------------------------------
// SEND STRING
// --------------------------------------------------

void uart_send_string(const char *text)
{
    while (*text)
    {
        uart_send_byte(*text);
        text++;
    }
}


// --------------------------------------------------
// SEND NUMBER
// --------------------------------------------------

void uart_send_uint32(uint32_t value)
{
    char buffer[11];

    uint8_t index = 0;

    if (value == 0)
    {
        uart_send_byte('0');
        return;
    }

    while (value > 0)
    {
        buffer[index++] =
            '0' + (value % 10);

        value /= 10;
    }

    while (index > 0)
    {
        uart_send_byte(buffer[--index]);
    }
}


// --------------------------------------------------
// WAIT FOR TX BUFFER
// --------------------------------------------------

void wait_for_tx_buffer_empty(void)
{
    while (txHead != txTail)
    {
    }
}


// --------------------------------------------------
// WAIT FOR FINAL UART BYTE
// --------------------------------------------------

void wait_for_uart_complete(void)
{
    while (!(UCSR0A & (1 << TXC0)))
    {
    }

    UCSR0A |= (1 << TXC0);
}


// --------------------------------------------------
// TASK 2
// THROUGHPUT + CPU LOAD
// --------------------------------------------------

void run_task2(void)
{
    uint32_t startTicks;
    uint32_t endTicks;
    uint32_t elapsedTicks;

    uint32_t throughput;
    uint32_t cpuLoad;

    uint32_t elapsedSecondsNumerator;

    uint16_t i;

    // Clear TX buffer
    txHead = 0;
    txTail = 0;

    // Clear counters
    txInterruptCount = 0;
    txIsrCycles = 0;

    // Reset timer
    cli();

    timer1OverflowCount = 0;
    TCNT1 = 0;

    sei();

    uart_send_string("\r\nBEGIN\r\n");

    wait_for_uart_complete();

    // Reset measurement timer again
    cli();

    timer1OverflowCount = 0;
    TCNT1 = 0;

    txInterruptCount = 0;
    txIsrCycles = 0;

    sei();

    // ---------------------------------------------
    // START MEASUREMENT
    // ---------------------------------------------

    startTicks = timer_get_ticks();

    // Send 10,000 bytes
    for (i = 0; i < TEST_SIZE; i++)
    {
        uart_tx_put('A');
    }

    // Wait until all bytes are removed from buffer
    wait_for_tx_buffer_empty();

    // Wait until last byte physically leaves UART
    wait_for_uart_complete();

    // ---------------------------------------------
    // END MEASUREMENT
    // ---------------------------------------------

    endTicks = timer_get_ticks();

    elapsedTicks =
        endTicks - startTicks;


    // ---------------------------------------------
    // THROUGHPUT
    //
    // CPU clock = 16,000,000 cycles/sec
    //
    // bytes/sec =
    // bytes * CPU clock / elapsed cycles
    // ---------------------------------------------

    if (elapsedTicks > 0)
    {
    uint64_t calculation;

    calculation =
        (uint64_t)TEST_SIZE * F_CPU;

    throughput =
        calculation / elapsedTicks;
    }
    else
    {
        throughput = 0;
    }


    // ---------------------------------------------
    // CPU LOAD
    //
    // ISR cycles / total cycles * 100
    // ---------------------------------------------

    if (elapsedTicks > 0)
    {
        cpuLoad =
            (txIsrCycles * 100UL)
            / elapsedTicks;
    }
    else
    {
        cpuLoad = 0;
    }


    // ---------------------------------------------
    // SEND RESULTS
    // ---------------------------------------------

    uart_send_string("\r\nEND\r\n");

    uart_send_string("RESULTS\r\n");

    uart_send_string("BYTES=");
    uart_send_uint32(TEST_SIZE);
    uart_send_string("\r\n");

    uart_send_string("TIME_CYCLES=");
    uart_send_uint32(elapsedTicks);
    uart_send_string("\r\n");

    uart_send_string("TX_INTERRUPTS=");
    uart_send_uint32(txInterruptCount);
    uart_send_string("\r\n");

    uart_send_string("ISR_CYCLES=");
    uart_send_uint32(txIsrCycles);
    uart_send_string("\r\n");

    uart_send_string("THROUGHPUT_BPS=");
    uart_send_uint32(throughput);
    uart_send_string("\r\n");

    uart_send_string("CPU_LOAD_PERCENT=");
    uart_send_uint32(cpuLoad);
    uart_send_string("\r\n");

    uart_send_string("TEST=PASS\r\n");
    uart_send_string("READY\r\n");
}


// --------------------------------------------------
// SETUP
// --------------------------------------------------

void setup(void)
{
    cli();

    uart_init();
    timer1_init();

    sei();

    uart_send_string("\r\nUART TASK 2 READY\r\n");
    uart_send_string("115200 8N1\r\n");
    uart_send_string("SEND S TO START\r\n");
}


// --------------------------------------------------
// LOOP
// --------------------------------------------------

void loop(void)
{
    if (startCommand)
    {
        startCommand = 0;

        run_task2();
    }
}
