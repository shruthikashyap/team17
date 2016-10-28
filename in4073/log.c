/*------------------------------------------------------------------
 *  log.c
 *------------------------------------------------------------------
 */

#include "log.h"

/*------------------------------------------------------------------
 *  void update_log
 *
 *  Writes the values from the global variables to the buffer
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void update_log()
{
	write_data.current_time 	= get_time_us();
	write_data.current_mode 	= drone.current_mode;
	write_data.sp 				= drone.sp;
	write_data.sq 				= drone.sq;
	write_data.sr 				= drone.sr;
	write_data.sax 				= drone.sax;
	write_data.say 				= drone.say;
	write_data.saz 				= drone.saz;
	write_data.phi  			= drone.phi;
	write_data.theta 			= drone.theta;
	write_data.psi 				= drone.psi;

	// Check buffer size
	if((new_addr + sizeof(write_data)) > 0x01FFFF)
	{
		// Reset address to 0
		flash_chip_erase();						
		new_addr = 0x000000;
		new_addr_write = 0x000000;
	}
	
	// Write into buffer
	if(flash_write_bytes(new_addr, (uint8_t*)&write_data, sizeof(write_data)) == true)
	{
		// Update new start address to write next entry
		new_addr += sizeof(write_data);
	}
}

/*------------------------------------------------------------------
 *  void log_upload
 *
 *  Reads values from the buffer and sends byte-by-byte to the PC
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
void log_upload()
{
	struct packet_t p;
	// Upload log only from safe mode
	if (drone.current_mode == SAFE_MODE)
	{
		// Indicate to the PC that log upload is starting 
		p.start = START_BYTE;
		p.command = LOG;
		p.value = LOG_START;
		compute_crc(&p);
		p.stop = STOP_BYTE;

		uart_put(p.start);
		uart_put(p.command);
		uart_put(p.value);
		uart_put(p.crc);
		uart_put(p.stop);

		// Read values stored in the buffer
		while(flash_read_bytes(new_addr_write, (uint8_t*)&read_data, sizeof(read_data)) == true)
		{
			// Send logged values to the PC
			uart_put(LOG_LINE_START);
			uart_put_32bit(read_data.current_time);	
			uart_put(read_data.current_mode);		
			uart_put_16bit(read_data.sp);				
			uart_put_16bit(read_data.sq);
			uart_put_16bit(read_data.sr);
			uart_put_16bit(read_data.sax);
			uart_put_16bit(read_data.say);
			uart_put_16bit(read_data.saz);
			uart_put_16bit(read_data.phi);
			uart_put_16bit(read_data.theta);
			uart_put_16bit(read_data.psi);
			uart_put(LOG_LINE_END);
			new_addr_write += sizeof(read_data);
			// Stop log upload when last line is reached
			if (new_addr_write >= new_addr)
			{	
				log_upload_flag = false;
				break;
			}
			nrf_delay_ms(10);
		}
	}
	else
	{
		// Send ACK_FAILURE to indicate that drone is not in SAFE MODE currently
		p.start = START_BYTE;
		p.command = ACK;
		p.value = ACK_FAILURE;
		compute_crc(&p);
		p.stop = STOP_BYTE;

		uart_put(p.start);
		uart_put(p.command);
		uart_put(p.value);
		uart_put(p.crc);
		uart_put(p.stop);
	}
}

/*------------------------------------------------------------------
 *  void uart_put_16bit
 *
 *  Splits 16-bit values into 2 bytes and sends them to the
 *	PC
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
void uart_put_16bit(int16_t value)
{	
	uint8_t c1, c2;			

	c1 = value;
	c2 = (value >> 8);
	
	uart_put(c1);
	uart_put(c2);
}

/*------------------------------------------------------------------
 *  void uart_put_32bit
 *
 *  Splits 32-bit values into 4 bytes and sends them to the
 *	PC
 *
 *  Author : Evelyn Rashmi Jeyachandra
 *------------------------------------------------------------------
 */
void uart_put_32bit(int32_t value)
{
	uint8_t c1, c2, c3, c4;

	c1 = value;
	c2 = (value >> 8);
	c3 = (value >> 16);
	c4 = (value >> 24);

	uart_put(c1);
	uart_put(c2);
	uart_put(c3);
	uart_put(c4);
}