/*------------------------------------------------------------------
 *  uart.c -- configures uart
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
#include "process_packet.h"

bool txd_available = true;
struct store_packet_t
{
	char start;
	char command;
	char value;
	char stop;
};

struct store_packet_t st_p;
struct packet_t p;
int packet_flag = 0;
int rcv_byte_count = 0;

void uart_put(uint8_t byte)
{
	NVIC_DisableIRQ(UART0_IRQn);

	if (txd_available) {txd_available = false; NRF_UART0->TXD = byte;}
	else enqueue(&tx_queue, byte);

	NVIC_EnableIRQ(UART0_IRQn);
}

// Reroute printf
int _write(int file, const char * p_char, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		uart_put(*p_char++);
	}

	return len;
}

void UART0_IRQHandler(void)
{
	if (NRF_UART0->EVENTS_RXDRDY != 0)
	{
		NRF_UART0->EVENTS_RXDRDY  = 0;
		enqueue( &rx_queue, NRF_UART0->RXD);
	}

	if (NRF_UART0->EVENTS_TXDRDY != 0)
	{
		NRF_UART0->EVENTS_TXDRDY = 0;
		if (tx_queue.count) NRF_UART0->TXD = dequeue(&tx_queue);
		else txd_available = true;
	}

	if (NRF_UART0->EVENTS_ERROR != 0)
	{
		NRF_UART0->EVENTS_ERROR = 0;
		printf("uart error: %lu\n", NRF_UART0->ERRORSRC);
	}
	
	// XXX: Receive in struct order and handle acknowledgement
	if (rx_queue.count)
	{
		char ch = dequeue(&rx_queue);

		if(ch == '?')
		{
			st_p.start = ch;

			// Start receiving
			packet_flag = 1;
			rcv_byte_count = 1;
		}
		else if(ch == '!')
		{
			st_p.stop = ch;

			// Reset flags
			rcv_byte_count = 0;
			packet_flag = 0;
		}
		else
		{
			if(rcv_byte_count == 1)
			{
				st_p.command = ch;
				rcv_byte_count++;
			}
			else if(rcv_byte_count == 2)
			{
				st_p.value = ch;
				rcv_byte_count++;
				printf("command = %d, value = %d\n", st_p.command, (unsigned char)st_p.value);
			}
		}

		if(rcv_byte_count == 4)
		{
			rcv_byte_count = 0;
			packet_flag = 0;
		}

		//process_key(p);
	}
}

void uart_init(void)
{
	init_queue(&rx_queue); // Initialize receive queue
	init_queue(&tx_queue); // Initialize transmit queue

	nrf_gpio_cfg_output(TX_PIN_NUMBER);
	nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_NOPULL); 
	NRF_UART0->PSELTXD = TX_PIN_NUMBER;
	NRF_UART0->PSELRXD = RX_PIN_NUMBER;
	NRF_UART0->BAUDRATE        = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);

	NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->EVENTS_RXDRDY    = 0;
	NRF_UART0->EVENTS_TXDRDY    = 0;
	NRF_UART0->TASKS_STARTTX    = 1;
	NRF_UART0->TASKS_STARTRX    = 1;

	NRF_UART0->INTENCLR = 0xffffffffUL;
	NRF_UART0->INTENSET = 	(UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos) |
		(UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos) |
		(UART_INTENSET_ERROR_Set << UART_INTENSET_ERROR_Pos);

	NVIC_ClearPendingIRQ(UART0_IRQn);
	NVIC_SetPriority(UART0_IRQn, 3); // either 1 or 3, 3 being low. (sd present)
	NVIC_EnableIRQ(UART0_IRQn);
}
