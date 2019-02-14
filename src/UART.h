#include <lcom/lcf.h>
/**
 * @brief subscribes the serial port
 * 
 * @param uart_id    bit number 
 * @return int if everyone went ok
 */
int serial_port_subscribe_int(int * uart_id);
/**
 * @brief unsubscribes the serial port
 * 
 * @param uart_id 
 * @return int  everyone went ok 
 */
int serial_port_unsubscribe_int(int * uart_id);
/**
 * @brief 
 * 
 * @param content 
 * @param register_ 
 * @return int 
 */
int uart_read(u_char *content, int register_);
int uart_write(u_char *data, int register_);
int uart_handler(uint32_t *_byte);
int clean_RBR();  
int write_THR(int *_byte);    



