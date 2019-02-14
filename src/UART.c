#include <lcom/lcf.h>
#include "UART.h"
#include "macro.h"

#include <stdint.h>

int uartHookId = 4;

int serial_port_subscribe_int(int *uart_id)
{
    // Subscribe the uart interrupts
    if (sys_irqsetpolicy(4, (IRQ_REENABLE | IRQ_EXCLUSIVE), uart_id) != OK)
        return 1;

    sys_outb(COM1 + IER, RD_EN | RLS_EN);
    return 0;
}

int serial_port_unsubscribe_int(int *uart_id)
{
    sys_irqdisable(uart_id);
    sys_irqrmpolicy(uart_id);
    return 0;
}

int uart_read(u_char *content, int register_)
{
    uint32_t cont;

    if (sys_inb(COM1 + register_, &cont))
        return 1;
    else
    {
        *content = (u_char)cont;
        return 0;
    }
}

int clean_RBR()
{
    u_char cleaner;
    u_char iir_byte;
    u_char lcr_byte;

    uart_read(&lcr_byte,LCR);
    sys_outb(COM1+LCR,(uint32_t)lcr_byte & 0x7F);

    uart_read(&iir_byte, IIR);
    if (iir_byte & RD)
    {
        uart_read(&cleaner, RBR);
    }
    return 0;
}
/*
int uart_write(u_char *data, int register_)
{

    while (!(LSR & SER_TX_RDY))
    {
        tickdelay(micros_to_ticks(DELAY_US));
        sys_inb(COM1 + SER_LSR, &LSR);
    }

    return (sys_outb(COM1 + register_, (uint32_t)data));
}*/

int write_THR(int *_byte)
{
    u_char lsr_byte;

    for (unsigned int i = 0; i < 5; i++)
    {
        if (uart_read(&lsr_byte, LSR) != 0)
        {
            printf("Error: unable to read LSR\n");
        }
        else if (lsr_byte & THR_EMP)
        {
            sys_outb(COM1 + THR, (uint32_t)_byte);
            printf("Transmit: %d \n", _byte);
            return 0;
        }
    }
    return 1;
}
/*
int configure_uart_SnakeyNix()
{
    // Line Control Register Byte - 8 bits per character, with 2 stop bits and even parity verification
    u_char line_control_register_byte = (UART_LCR_8BPC | UART_LCR_2_STOPBIT | UART_LCR_EVEN_PARITY);

    // Interrupt Enable Register Byte - Received Data Available Interupts and Error Interrupts
    u_char interrupt_enable_register_byte = (UART_IER_RCVD_DATA_INT | UART_IER_ERROR_INT);

    // Write the Interrupt Enable Register Byte
    if (uart_write_IER(interrupt_enable_register_byte) != OK)
        return UART_CONFIG_FAILED;

    // Write the Line Control Register Byte
    if (uart_write_LCR(line_control_register_byte) != OK)
        return UART_CONFIG_FAILED;

    // Set the correct bit-rate
    if (uart_set_bit_rate(9600) != OK)
        return UART_CONFIG_FAILED;

    // Flush the buffer to make sure it is clean
    uart_flush_RBR();

    return OK;
}*/

int uart_handler(uint32_t *_byte)
{
    u_char iir_byte;
	u_char lsr_byte;
	if(uart_read(&iir_byte,IIR) != 0){
		return 1;
	}
    if(iir_byte & RD){
        if(uart_read((u_char *)_byte,RBR) != 0){
            return 1;
        }
	}
	else if ((iir_byte & RLS) == RLS){
        if(uart_read(&lsr_byte,LSR) != 0){
            return 1;
        }
	}
	return 0;
   /* if (sys_inb(COM1 + RBR, _byte) != OK)
    {
        return 1;
    }
    else
    {
        printf("%d", (int)_byte);
        return 0;
    }*/
}
