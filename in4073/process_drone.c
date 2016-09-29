#include <stdio.h>
#include <math.h>
#include "command_types.h"
#include "in4073.h"
#include "process_drone.h"
#include <string.h>
#include <stdlib.h>
//#include "drivers/process_packet.h"
#include "crc.h"

void send_packet_to_pc(struct packet_t p);

struct log write_data;
struct log read_data;
uint32_t new_addr;
uint32_t new_addr_write;
bool log_flag = false;
bool sensor_flag = false;
bool telemetry_flag = false;
uint32_t counter = 0;
bool batt_low_flag = false;

void update_log()
{
	write_data.current_time = get_time_us();
	write_data.sp = sp;
	write_data.sq = sq;
	write_data.sr = sr;
	write_data.sax = sax;
	write_data.say = say;
	write_data.saz = saz;
	write_data.bat_volt = bat_volt;

	//Battery voltage check
	//printf("Battery voltage = %d\n", bat_volt);
	if(bat_volt <= BATT_THRESHOLD && drone.current_mode != SAFE_MODE)
	{
		//printf("Battery low");
		drone.current_mode = PANIC_MODE;
		drone.change_mode = 1;
		batt_low_flag = true;
		return;
	}

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
		//printf("Data write = %d\n", write_data.phi);

		// Update new_addr
		new_addr += sizeof(write_data);
		//printf("new_addr = %d\n", (int)new_addr);
	}
}

void read_sensor()
{
	// Read sensor data
	if (check_timer_flag()) 
	{
		if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

		adc_request_sample();
		read_baro();
		nrf_delay_ms(1);

		clear_timer_flag();
	}

	if (check_sensor_int_flag()) 
	{
		get_dmp_data();
		//printf("%6d %6d %6d | ", sp, sq, sr);
		//printf("%6d %6d %6d \n", sax, say, saz);
		clear_sensor_int_flag();
	}
	
	sp -= drone.offset_sp;
	sq -= drone.offset_sq;
	sr -= drone.offset_sr;
	sax -= drone.offset_sax;
	say -= drone.offset_say;
	saz -= drone.offset_saz;
	pressure -= drone.offset_pressure;
}

void send_telemetry_data()
{
	// Send telemetry command to PC
	struct packet_t p;
	p.start = START_BYTE;
	p.command = TELEMETRY_DATA;
	compute_crc(&p);
	p.stop = STOP_BYTE;
	
	send_packet_to_pc(p);
	
	uart_put(TELE_START);
	//uart_put_32bit(4057748);
	uart_put_32bit(get_time_us());
	//uart_put(0x20);
	uart_put(drone.current_mode);
	//uart_put_16bit(574);
	uart_put_16bit(bat_volt);
	//uart_put_16bit(710);
	uart_put_16bit(ae[0]);
	//uart_put_16bit(510);
	uart_put_16bit(ae[1]);
	//uart_put_16bit(1000);
	uart_put_16bit(ae[2]);
	//uart_put_16bit(555);
	uart_put_16bit(ae[3]);
	uart_put(TELE_STOP);
}

void check_sensor_log_tele_flags()
{
	// Read sensor
	if(sensor_flag == true && batt_low_flag == false)
	{
		//printf("Start sensor %10ld\n", get_time_us());
		// Clear sensor flag
		sensor_flag = false;

		// Read sensor data
		read_sensor();
		//printf("Stop sensor %10ld\n", get_time_us());
	}

	// Update log
	if(log_flag == true && batt_low_flag == false)
	{
		//printf("Start log %10ld\n", get_time_us());
		// Clear log flag
		log_flag = false;

		// Log sensor data
		update_log();
		//printf("Stop log %10ld\n", get_time_us());
	}

	// Send telemetry
	if(telemetry_flag == true && batt_low_flag == false && log_upload_flag == false)
	{
		//printf("Start tele %10ld\n", get_time_us());
		// Clear telemetry flag
		telemetry_flag = false;

		// Send telemetry data
		//send_telemetry_data();
		//printf("Stop tele %10ld\n", get_time_us());
	}
}

void reset_drone()
{
	// XXX: Set mode as SAFE_MODE and clear all flags
	drone.current_mode = SAFE_MODE; // XXX: Test
	drone.stop = 0;
	drone.change_mode = 0;
	
	// Reset drone control variables
	drone.key_lift = 0;
	drone.key_roll = 0;
	drone.key_pitch = 0;
	drone.key_yaw = 0;
	drone.joy_lift = 0;
	drone.joy_roll = 0;
	drone.joy_pitch = 0;
	drone.joy_yaw = 0;
	drone.offset_sp = 0;
    drone.offset_sq = 0;
    drone.offset_sr = 0;
	drone.offset_pressure = 0;
	
	// Other parameters
	new_addr = 0x000000;
	new_addr_write = 0x000000;
	batt_low_flag = false;
	
	if(flash_chip_erase() == true)
		printf("Memory erase successful\n");
}

void safe_mode()
{
	//printf("In SAFE_MODE\n");
	
	__disable_irq();
	// Don't read lift/roll/pitch/yaw data from PC link.
	// Reset drone control variables
	drone.key_lift = 0;
	drone.key_roll = 0;
	drone.key_pitch = 0;
	drone.key_yaw = 0;
	drone.joy_lift = 0;
	drone.joy_roll = 0;
	drone.joy_pitch = 0;
	drone.joy_yaw = 0;
	
	// Gradually reduce RPM of the motors to 0.
	while(drone.ae[0] || drone.ae[1] || drone.ae[2] || drone.ae[3])
	{
		drone.ae[0] = (drone.ae[0]) < RPM_STEP? 0 : drone.ae[0] - 10;
		drone.ae[1] = (drone.ae[1]) < RPM_STEP? 0 : drone.ae[1] - 10;
		drone.ae[2] = (drone.ae[2]) < RPM_STEP? 0 : drone.ae[2] - 10;
		drone.ae[3] = (drone.ae[3]) < RPM_STEP? 0 : drone.ae[3] - 10;
		//printf("\nDrone decreasing motor values: %3d %3d %3d %3d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3]);
		nrf_delay_ms(1000);
	}
	__enable_irq();
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		if (batt_low_flag == true)
		{
			nrf_gpio_pin_toggle(RED);
			drone.stop = 1;
			//printf("Warning! Battery critically low!")
		}
		
		//printf("SAFE - drone.change_mode = %d\n", drone.change_mode);
		
		check_sensor_log_tele_flags();
		
		if (log_upload_flag == true && telemetry_flag == false)
		{
			//printf("Value of log_upload_flag = %d\n", log_upload_flag);
			log_upload();
		}
		
		nrf_delay_ms(1);
	}
	//printf("Exit SAFE_MODE\n");
}

void panic_mode()
{
	//printf("In PANIC_MODE\n");
	
	// Disable UART interrupts
	//NVIC_DisableIRQ(UART0_IRQn);
	__disable_irq();

	// Set moderate RPM values to the motors for hovering
	drone.ae[0] = HOVER_RPM;
	drone.ae[1] = HOVER_RPM;
	drone.ae[2] = HOVER_RPM;
	drone.ae[3] = HOVER_RPM;
	//printf("Drone motor values: %3d %3d %3d %3d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3]);
	
	// Stay in this mode for a few seconds
	nrf_delay_ms(5000);
	
	// Go to Safe mode
	drone.current_mode = SAFE_MODE;
	drone.change_mode = 1; // XXX: Is this needed?
	
	// Enable UART interrupts
	//NVIC_EnableIRQ(UART0_IRQn);
	__enable_irq();

	//printf("Exit PANIC_MODE\n");
}

void manual_mode()
{
	//printf("In MANUAL_MODE\n");
	
	int ae_[4]; // Motor rpm values
	int lift_force;
	int roll_moment;
	int pitch_moment;
	int yaw_moment;
	int lift;
	int pitch;
	int roll;
	int yaw;	

	while(drone.change_mode == 0 && drone.stop == 0)
	{
		// Disable UART interrupts
		//NVIC_DisableIRQ(UART0_IRQn);
		__disable_irq();

		// joystick goes from -127 to 128. key_lift goes from -127 to 128 so they weigh the same
		lift_force   = (signed char)drone.joy_lift+1  + (signed char)drone.key_lift;
		roll_moment  = (signed char)drone.joy_roll+1  + (signed char)drone.key_roll;
		pitch_moment = (signed char)drone.joy_pitch+1 + (signed char)drone.key_pitch;
		yaw_moment   = (signed char)drone.joy_yaw+1   + (signed char)drone.key_yaw;

		// Enable UART interrupts
		//NVIC_EnableIRQ(UART0_IRQn);
		__enable_irq();

		//printf("%d, %d, %d, %d\n", (signed char)drone.joy_roll, (signed char)drone.joy_pitch, (signed char)drone.joy_yaw, (signed int)drone.joy_lift);
		//printf("%d, %d, %d, %d\n", lift_force, roll_moment, pitch_moment, yaw_moment);

		lift  = DRONE_LIFT_CONSTANT * lift_force;
		pitch = DRONE_PITCH_CONSTANT * pitch_moment;
		roll  = DRONE_ROLL_CONSTANT * roll_moment;
		yaw   = DRONE_YAW_CONSTANT * yaw_moment;
#if 0
		// no need to do this, rotor speeds are getting maxed/mined a few lines later
		lift  = lift  < MIN_LIFT  ? MIN_LIFT  : lift;
		roll  = roll  < MIN_ROLL  ? MIN_ROLL  : roll;
		pitch = pitch < MIN_PITCH ? MIN_PITCH : pitch;
		yaw   = yaw   < MIN_YAW   ? MIN_YAW   : yaw;

		lift  = lift  > MAX_LIFT  ? MAX_LIFT  : lift;
		roll  = roll  > MAX_ROLL  ? MAX_ROLL  : roll;
		pitch = pitch > MAX_PITCH ? MAX_PITCH : pitch;
		yaw   = yaw   > MAX_YAW   ? MAX_YAW   : yaw;
#endif
		
		ae_[0] = 0.25*(lift + 2*pitch - yaw);
		ae_[1] = 0.25*(lift - 2*roll  + yaw);
		ae_[2] = 0.25*(lift - 2*pitch - yaw);
		ae_[3] = 0.25*(lift + 2*roll  + yaw);
		
		ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
		ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
		ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
		ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);
		
		// checking min/max	
		ae_[0] = ae_[0] < MIN_RPM ? MIN_RPM : ae_[0];
		ae_[1] = ae_[1] < MIN_RPM ? MIN_RPM : ae_[1];
		ae_[2] = ae_[2] < MIN_RPM ? MIN_RPM : ae_[2];
		ae_[3] = ae_[3] < MIN_RPM ? MIN_RPM : ae_[3];

		ae_[0] = ae_[0] > MAX_RPM ? MAX_RPM : ae_[0];
		ae_[1] = ae_[1] > MAX_RPM ? MAX_RPM : ae_[1];
		ae_[2] = ae_[2] > MAX_RPM ? MAX_RPM : ae_[2];
		ae_[3] = ae_[3] > MAX_RPM ? MAX_RPM : ae_[3];	

		// setting drone rotor speeds
		drone.ae[0] = ae_[0];
		drone.ae[1] = ae_[1];
		drone.ae[2] = ae_[2];
		drone.ae[3] = ae_[3];

		//printf("%3d %3d %3d %3d %3d\n", ae_[0], ae_[1], ae_[2], ae_[3], lift_force);
		//printf("%3d, %3d, %3d, %3d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3]);
		
		//printf("%3d %3d %3d %3d %3d \n",ae[0], lift, roll, pitch, yaw);
		//printf("MANUAL - drone.change_mode = %d\n", drone.change_mode);
		
		check_sensor_log_tele_flags();
		
		nrf_delay_ms(1);
	}
	
	//printf("Exit MANUAL_MODE\n");
}

void yaw_control_mode()
{
	printf("In YAW_CONTROL_MODE\n");
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		//printf("YAW - drone.change_mode = %d\n", drone.change_mode);		
		nrf_delay_ms(1);
	}
	
	printf("Exit YAW_CONTROL_MODE\n");
}

void full_control_mode()
{
	//printf("In FULL_CONTROL_MODE\n");
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		//printf("FULL - drone.change_mode = %d\n", drone.change_mode);
		nrf_delay_ms(500);
	}
	
	//printf("Exit FULL_CONTROL_MODE\n");
}

void calibration_mode()
{
	printf("In CALIBRATION_MODE\n");
	
	uint32_t counter = 0;
    int samples = 100;
    int sum_sp = 0;
	int sum_sq = 0;
	int sum_sr = 0;
	int sum_sax = 0;
	int sum_say = 0;
	int sum_saz = 0;
	int sum_pressure = 0;
	int i;

	if(get_time_us() < 20000000)
		nrf_delay_ms(20000);
	
	// Discard some samples to avoid junk data
	for(i = 0; i < 25000; i++)
	{
		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

			read_baro();
			nrf_delay_ms(1);

			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			//printf("%6d %6d %6d | ", sp, sq, sr);
			//printf("%6d %6d %6d \n", sax, say, saz);
			nrf_delay_ms(1);
			clear_sensor_int_flag();
		}
	}

    for(i = 0; i < samples; i++)
	{
		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

			read_baro();
			nrf_delay_ms(1);

			clear_timer_flag();
		}
 
		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			//printf("%6d %6d %6d | ", sp, sq, sr);
			//printf("%6d %6d %6d \n", sax, say, saz);
			nrf_delay_ms(1);
			clear_sensor_int_flag();
		}
		
        sum_sp += sp;
		sum_sq += sq;
		sum_sr += sr;
		sum_sax += sax;
		sum_say += say;
		sum_saz += saz;
		sum_pressure += pressure;
		
		nrf_delay_ms(10);
    }

    drone.offset_sp = (int)(sum_sp / samples);
	drone.offset_sq = (int)(sum_sq / samples);
	drone.offset_sr = (int)(sum_sr / samples);
	drone.offset_sax = (int)(sum_sax / samples);
	drone.offset_say = (int)(sum_say / samples);
	drone.offset_saz = (int)(sum_saz / samples);
	drone.offset_pressure = (int)(sum_pressure / samples);

    //nrf_delay_ms(500);

    printf("New offsets found: \n");
    printf("sp = %d, sq = %d, sr = %d \n", drone.offset_sp, drone.offset_sq, drone.offset_sr);
	printf("sax = %d, say = %d, saz = %d \n", drone.offset_sax, drone.offset_say, drone.offset_saz);
	printf("pressure = %d \n", drone.offset_pressure);

    drone.current_mode = SAFE_MODE;
    drone.change_mode = 1;
    
    printf("Exit CALIBRATION_MODE\n");
}

void raw_mode()
{
	//printf("In RAW_MODE\n");
}

void height_control_mode()
{
	//printf("In HEIGHT_CONTROL_MODE\n");
}

void wireless_mode()
{
	//printf("In WIRELESS_MODE\n");
}

//************************* L O G   U P L O A D ******************
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
			//printf("%d %lu %d %d %d %d %d %d %d %d\n", LOG_LINE_START, read_data.current_time, read_data.sp, read_data.sq, read_data.sr, read_data.sax, read_data.say, read_data.saz, read_data.bat_volt, LOG_LINE_END);
			#if 1
			uart_put(LOG_LINE_START);
			uart_put_32bit(read_data.current_time);			
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
			uart_put_16bit(read_data.bat_volt);
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
			
			nrf_delay_ms(100);
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
//******************************************************************

void process_drone()
{
#if 0
	// XXX: Interrupt test
	while (drone.stop == 0)
	{
		//printf("\nDrone motor values: %3d %3d %3d %3d, size = %d\n", ae[0], ae[1], ae[2], ae[3], sizeof(int16_t));
		nrf_delay_ms(500);
	}
#endif

#if 1
	// Set defaults for the drone
	reset_drone();
	
	while (drone.stop == 0)
	{
		drone.change_mode = 0;

		switch (drone.current_mode)
		{
			case SAFE_MODE:
					// XXX: Test
					//ae[0] = ae[1] = ae[2] = ae[3] = 100;
					safe_mode();
					break;
			case PANIC_MODE:
					// XXX: Test
					//ae[0] = ae[1] = ae[2] = ae[3] = 500;
					//printf("\nDrone motor values before: %3d %3d %3d %3d\n", ae[0], ae[1], ae[2], ae[3]);
					panic_mode();
					// XXX: Test
					//printf("\nDrone motor values after: %3d %3d %3d %3d\n", ae[0], ae[1], ae[2], ae[3]);
					break;
			case MANUAL_MODE:
					manual_mode();
					break;
			case YAW_CONTROL_MODE:
					yaw_control_mode();
					break;
			case FULL_CONTROL_MODE:
					full_control_mode();
					break;
			case CALIBRATION_MODE:
					calibration_mode();
					break;
			case RAW_MODE:
					raw_mode();
					break;
			case HEIGHT_CONTROL_MODE:
					height_control_mode();
					break;
			case WIRELESS_MODE:
					wireless_mode();
					break;
			default :
					// XXX: Needs to be handled
					break;
		}
	}
#endif
}
