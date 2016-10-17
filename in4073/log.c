#include "log.h"

void update_log()
{
	//nrf_gpio_pin_toggle(RED);
	write_data.current_time 	= get_time_us();
	write_data.current_mode 	= drone.current_mode;
#if 0
	write_data.joy_lift 		= drone.joy_lift;
	write_data.joy_roll 		= drone.joy_roll;
	write_data.joy_pitch 		= drone.joy_pitch;
	write_data.joy_yaw 			= drone.joy_yaw;
	write_data.key_lift 		= drone.key_lift;
	write_data.key_roll 		= drone.key_roll;
	write_data.key_pitch 		= drone.key_pitch;
	write_data.key_yaw 			= drone.key_yaw;
	write_data.ae_0				= drone.ae[0];
	write_data.ae_1				= drone.ae[1];
	write_data.ae_2				= drone.ae[2];
	write_data.ae_3				= drone.ae[3];
#endif
	write_data.sp 				= drone.sp;
	write_data.sq 				= drone.sq;
	write_data.sr 				= drone.sr;
	write_data.sax 				= drone.sax;
	write_data.say 				= drone.say;
	write_data.saz 				= drone.saz;
#if 0
	write_data.controlgain_yaw  = drone.controlgain_yaw;
	write_data.controlgain_p1 	= drone.controlgain_p1;
	write_data.controlgain_p2 	= drone.controlgain_p2;
	write_data.pressure 		= drone.pressure;
	write_data.bat_volt 		= bat_volt;
#endif

	// Check buffer size
	if((new_addr + sizeof(write_data)) > 0x01FFFF)
	{
		//printf("Buffer full!\n");
		// Reset address to 0
		flash_chip_erase();						
		new_addr = 0x000000;
		new_addr_write = 0x000000;
		//return;
	}
	
	// Write into buffer
	if(flash_write_bytes(new_addr, (uint8_t*)&write_data, sizeof(write_data)) == true)
	{
		// Update new_addr
		new_addr += sizeof(write_data);
	}
}

void log_upload()
{
	// printf("In LOG_UPLOAD\n");
	// Read from buffer
	struct packet_t p;
	if (drone.current_mode == SAFE_MODE)
	{
		//printf("Inside log_upload \n");
		#if 1
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
		#endif

		while(flash_read_bytes(new_addr_write, (uint8_t*)&read_data, sizeof(read_data)) == true)
		{
			// Update new_addr_write
			//printf("%d %lu %d %d %d %d %d %d %ld %d %d\n", LOG_LINE_START, read_data.current_time, read_data.sp, read_data.sq, read_data.sr, read_data.sax, read_data.say, read_data.saz, read_data.pressure, read_data.bat_volt, LOG_LINE_END);
			#if 1
			uart_put(LOG_LINE_START);
			uart_put_32bit(read_data.current_time);	
			uart_put(read_data.current_mode);
			//uart_put_16bit(read_data.joy_lift);
			//uart_put_16bit(read_data.joy_roll);
			//uart_put_16bit(read_data.joy_pitch);
			//uart_put_16bit(read_data.joy_yaw);
			//uart_put_16bit(read_data.key_lift);
			//uart_put_16bit(read_data.key_roll);
			//uart_put_16bit(read_data.key_pitch);
			//uart_put_16bit(read_data.key_yaw);
			//uart_put_16bit(read_data.ae_0);
			//uart_put_16bit(read_data.ae_1);
			//uart_put_16bit(read_data.ae_2);
			//uart_put_16bit(read_data.ae_3);		
			//uart_put_32bit(4057748);			
			uart_put_16bit(read_data.sp);	
			//uart_put_16bit(-46);				
			uart_put_16bit(read_data.sq);
			//uart_put_16bit(-45);
			uart_put_16bit(read_data.sr);
			//uart_put_16bit(-23);
			uart_put_16bit(read_data.sax);
			//uart_put_16bit(-1626);
			uart_put_16bit(read_data.say);
			//uart_put_16bit(1232);
			uart_put_16bit(read_data.saz);
			//uart_put_16bit(16884);
			//uart_put_16bit(read_data.controlgain_yaw);
			//uart_put_16bit(read_data.controlgain_p1);
			//uart_put_16bit(read_data.controlgain_p2);
			//uart_put_32bit(read_data.pressure);
			//uart_put_16bit(read_data.bat_volt);
			//uart_put_16bit(574);
			uart_put(LOG_LINE_END);
			#endif
			new_addr_write += sizeof(read_data);
			if (new_addr_write >= new_addr)
			{	
				log_upload_flag = false;
				break;
			}
			//printf("new_addr_write = %d\n", (int)new_addr_write);
			
			nrf_delay_ms(10);
		}

		uart_put(LOG_END);					// Finish sending log
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

void uart_put_16bit(int16_t value)
{	
	uint8_t c1, c2;			// unsigned int

	c1 = value;
	c2 = (value >> 8);
	
	//printf("%d %d\n",c1,c2 );
	uart_put(c1);
	uart_put(c2);
}

void uart_put_32bit(int32_t value)
{
	uint8_t c1, c2, c3, c4;

	c1 = value;
	c2 = (value >> 8);
	c3 = (value >> 16);
	c4 = (value >> 24);

	//printf("%d %d %d %d\n",c1, c2, c3, c4 );

	uart_put(c1);
	uart_put(c2);
	uart_put(c3);
	uart_put(c4);
}