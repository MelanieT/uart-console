#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "mem.h"
//#include "os.h"
#include "driver/uart.h"
#include "string.h"

#include "uart-console.h"

static void (*process_line)(const char *) = 0;
static void (*interrupt)(void) = 0;
static void (*connection)(void) = 0;

void uart_send(const char *data)
{
    uart_write_bytes(UART_NUM_0, (const char *) data, strlen(data));
}

static void uart_recv_cb(char *data, unsigned short len)
{
    static char real_data[256];
    static int l = 0;
    static char crlf[] = {0x0d, 0x0a, 0};
    static char bs[] = {0x08, 0x20, 0x08, 0};

    for (int i = 0 ; i < len ; i++)
    {
        if (data[i] == 0)
            continue;

        if (data[i] == 0x0d) // cr
        {
            uart_send(crlf);

            real_data[l] = 0;
            if (process_line)
                process_line(real_data);

            l = 0;
        }
        if (data[i] == 0x08 || data[i] == 0x7f) // bs + del
        {
            if (l > 0)
            {
                uart_send(bs);
                l--;
            }
            continue;
        }
        if (data[i] == 0x03) // CTRL-C
        {
            uart_send(crlf);

            real_data[l] = 0;
            if (interrupt)
                interrupt();

            l = 0;
        }
        if ((uint8_t)data[i] < 32 || (uint8_t)data[i] > 126)
            continue;

        if (l < 255)
        {
            real_data[l++] = data[i];
            char echo[] = {0, 0};
            echo[0] = data[i];
            uart_send(echo);
        }
    }
}

static void uart_task()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    //uart_set_pin(UART_NUM_1, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(1024);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(UART_NUM_0, data, 1024, 20 / portTICK_PERIOD_MS);
        if (len)
            uart_recv_cb((char *)data, len);
    }
}

void uart_set_process_line_cb(void (*process)(const char *))
{
    process_line = process;
}

void uart_set_interrupt_cb(void (*intr)(void))
{
    interrupt = intr;
}

void uart_set_new_connection_cb(void (*conn)(void))
{
    connection = conn;
    (*connection)();
}

void uart_disconnect()
{
}

void uart_init()
{
    xTaskCreate(uart_task, "uart_echo_task", 4096, NULL, 10, NULL);
}

