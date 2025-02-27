void uart_init();
void uart_set_process_line_cb(void (*process)(const char *));
void uart_set_interrupt_cb(void (*process)(void));
void uart_set_new_connection_cb(void (*connection)(void));
void uart_send(const char *data);
void uart_disconnect(void);
#define uart_printf(fmt, ...) do { \
    char data[1024]; \
    sprintf(data, fmt, ##__VA_ARGS__); \
    telnetd_send(data); \
    } while(0)
    
