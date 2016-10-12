#include <stdio.h>
#include <math.h>
#include "command_types.h"
#include "in4073.h"
#include "process_drone.h"
#include <string.h>
#include <stdlib.h>
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
	//nrf_gpio_pin_toggle(RED);
	write_data.current_time = get_time_us();
	write_data.sp = sp;
	write_data.sq = sq;
	write_data.sr = sr;
	write_data.sax = sax;
	write_data.say = say;
	write_data.saz = saz;
	write_data.pressure = pressure;
	write_data.bat_volt = bat_volt;

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

void read_sensor()
{
	drone.sp = 0;
	drone.sq = 0;
	drone.sr = 0;
	drone.sax = 0;
	drone.say = 0;
	drone.saz = 0;
	drone.pressure = 0;
	
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
		clear_sensor_int_flag();
	}
	
	__disable_irq();
	drone.sp = sp - drone.offset_sp;
	drone.sq = sq - drone.offset_sq;
	drone.sr = sr - drone.offset_sr;
	drone.sax = sax - drone.offset_sax;
	drone.say = say - drone.offset_say;
	drone.saz = saz - drone.offset_saz;
	drone.phi = phi - drone.offset_phi;
	drone.theta = theta - drone.offset_theta;
	drone.psi = psi - drone.offset_psi;
	drone.pressure = pressure - drone.offset_pressure ;
	//drone.pressure = pressure;
	__enable_irq();
	
	// Battery voltage check
	if(bat_volt <= BATT_THRESHOLD && drone.current_mode != SAFE_MODE)
	{
		printf("Battery low!\n");
		drone.current_mode = PANIC_MODE;
		drone.change_mode = 1;
		batt_low_flag = true;
		return;
	}
	
	//printf("%6d %6d %6d | ", drone.sp, drone.sq, drone.sr);
	//printf("%6d %6d %6d \n", drone.sax, drone.say, drone.saz);
	//printf("%6d | %6d | %6d \n", drone.phi, drone.theta, drone.psi);
	//printf("%ld\n", drone.pressure);
}

void read_battery_level()
{
	if(check_timer_flag()) 
	{
		if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

		adc_request_sample();
		nrf_delay_ms(1);

		clear_timer_flag();
	}
	
	// Battery voltage check
	if(bat_volt <= BATT_THRESHOLD && drone.current_mode != SAFE_MODE)
	{
		printf("Battery low!\n");
		drone.current_mode = PANIC_MODE;
		drone.change_mode = 1;
		batt_low_flag = true;
		return;
	}
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
	uart_put_16bit(drone.ae[0]);
	//uart_put_16bit(510);
	uart_put_16bit(drone.ae[1]);
	//uart_put_16bit(1000);
	uart_put_16bit(drone.ae[2]);
	//uart_put_16bit(555);
	uart_put_16bit(drone.ae[3]);
	uart_put(TELE_STOP);
}

void check_log_tele_flags()
{
	#if 0
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
	#endif
	
	// Update log
	if(log_flag == true && batt_low_flag == false && log_active_flag == true)
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
	
	if(flash_chip_erase() == true);
		//printf("Memory erase successful\n");
}

void safe_mode()
{
	printf("In SAFE_MODE\n");
	
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

	//printf("In SAFE_MODE again\n");

	while(drone.change_mode == 0 && drone.stop == 0)
	{
		if (batt_low_flag == true)
		{
			//nrf_gpio_pin_toggle(RED);
			drone.stop = 1;
			//printf("Warning! Battery critically low!")
		}
		
		// Update log and telemetry if corresponding flags are set
		check_log_tele_flags();
		// Read battery
		read_battery_level();
		
		if (log_upload_flag == true && telemetry_flag == false)
		{
			//printf("Inside safe mode - log upload\n");
			log_upload();
		}

		nrf_delay_ms(1);
	}
	
	printf("Exit SAFE_MODE\n");
}

void panic_mode()
{
	printf("In PANIC_MODE\n");
	
	// Set moderate RPM values to the motors for hovering
	drone.ae[0] = HOVER_RPM;
	drone.ae[1] = HOVER_RPM;
	drone.ae[2] = HOVER_RPM;
	drone.ae[3] = HOVER_RPM;
	
	run_filters_and_control();
	
	// Stay in this mode for a few seconds
	//nrf_delay_ms(5000);
	nrf_delay_ms(1000); // XXX: For testing
	
	// Gradually reduce RPM of the motors to 0.
	while(drone.ae[0] || drone.ae[1] || drone.ae[2] || drone.ae[3])
	{
		drone.ae[0] = (drone.ae[0]) < RPM_STEP? 0 : drone.ae[0] - 10;
		drone.ae[1] = (drone.ae[1]) < RPM_STEP? 0 : drone.ae[1] - 10;
		drone.ae[2] = (drone.ae[2]) < RPM_STEP? 0 : drone.ae[2] - 10;
		drone.ae[3] = (drone.ae[3]) < RPM_STEP? 0 : drone.ae[3] - 10;
		
		run_filters_and_control();
		
		nrf_delay_ms(250);
	}

	// Go to Safe mode
	drone.current_mode = SAFE_MODE;
	drone.change_mode = 1; // XXX: Is this needed?
	
	printf("Exit PANIC_MODE\n");
}

void manual_mode()
{
	printf("In MANUAL_MODE\n");
	
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
		__disable_irq();

		// joystick goes from -127 to 128. key_lift goes from -127 to 128 so they weigh the same
		lift_force   = drone.joy_lift  + drone.key_lift;
		roll_moment  = (signed char)drone.joy_roll  + (signed char)drone.key_roll;
		pitch_moment = (signed char)drone.joy_pitch + (signed char)drone.key_pitch;
		yaw_moment   = (signed char)drone.joy_yaw   + (signed char)drone.key_yaw;

		// Enable UART interrupts
		__enable_irq();

		lift  = DRONE_LIFT_CONSTANT * lift_force;
		pitch = DRONE_PITCH_CONSTANT * pitch_moment;
		roll  = DRONE_ROLL_CONSTANT * roll_moment;
		yaw   = DRONE_YAW_CONSTANT * yaw_moment;

		if ((signed char)drone.joy_lift < 5)
		{
			pitch = 0;
			roll  = 0;
			yaw   = 0;
		}
		
		ae_[0] = 0.25*(lift + 2*pitch - yaw);
		ae_[1] = 0.25*(lift - 2*roll  + yaw);
		ae_[2] = 0.25*(lift - 2*pitch - yaw);
		ae_[3] = 0.25*(lift + 2*roll  + yaw);
		
		ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
		ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
		ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
		ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);
		
		if (drone.joy_lift > 5)
		{
			ae_[0] = ae_[0] < MIN_RPM ? MIN_RPM : ae_[0];
			ae_[1] = ae_[1] < MIN_RPM ? MIN_RPM : ae_[1];
			ae_[2] = ae_[2] < MIN_RPM ? MIN_RPM : ae_[2];
			ae_[3] = ae_[3] < MIN_RPM ? MIN_RPM : ae_[3];
		}

		ae_[0] = ae_[0] > MAX_RPM ? MAX_RPM : ae_[0];
		ae_[1] = ae_[1] > MAX_RPM ? MAX_RPM : ae_[1];
		ae_[2] = ae_[2] > MAX_RPM ? MAX_RPM : ae_[2];
		ae_[3] = ae_[3] > MAX_RPM ? MAX_RPM : ae_[3];	

		// setting drone rotor speeds
		drone.ae[0] = ae_[0];
		drone.ae[1] = ae_[1];
		drone.ae[2] = ae_[2];
		drone.ae[3] = ae_[3];

		printf("%3d %3d %3d %3d | %d\n", drone.ae[0], drone.ae[1], drone.ae[2], drone.ae[3], bat_volt);
		
		// Update log and telemetry if corresponding flags are set
		check_log_tele_flags();
		// Read battery
		read_battery_level();

		run_filters_and_control();

		nrf_delay_ms(1);
	}
	
	printf("Exit MANUAL_MODE\n");
}

void yaw_control_mode()
{
	printf("In YAW_CONTROL_MODE\n");

	int yawrate_setpoint;
	int yaw_error;

	int yaw_moment;
	int lift_force;

	int ae_[4];

	int lift, roll, pitch, yaw;
	
	//int pressure_error;

	drone.controlgain_yaw = 2;
	drone.controlgain_height = 1;
	//drone.key_lift = 20; // XXX: For testing
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		check_log_tele_flags();
		read_sensor();

		__disable_irq();
		lift_force = drone.joy_lift + drone.key_lift;
		yawrate_setpoint = (signed char)drone.joy_yaw  + (signed char)drone.key_yaw;
		__enable_irq();
		
		#if 0
		if(height_control_flag == true)
		{
			if(drone.height_control_lift != drone.joy_lift)
				height_control_flag = false;
			else
			{
				pressure_error = pressure - drone.height_control_pressure;
				lift_force = HOVER_LIFT_FORCE + ((int)drone.controlgain_height * pressure_error);
				//printf("Lift force = %d, pressure_error = %d\n", lift_force, pressure_error);
			}
		}
		#endif
		
		// setpoint is keyboard + joystick, so in range of -127 to 128
		// gyro is in 16 bit range with a max of 250deg/s 
		// yaw rate of max 3000 seems decent (trial and error) so max of setpoint should be 3000
		// 3000/128 = 23.43, so setpoints times 24 should be in decent range
		// dividing sr by 24 to keep yaw_error a low number, so the controlgainnumber doesnt have to be so small
		
		if (lift_force > 10) 
		{
			yaw_error = -(int)((1*yawrate_setpoint) - (drone.sr/24));

			// yaw error in range of -255 to 256 (although the extremes probably wont happen)
			yaw_moment = (int)drone.controlgain_yaw*yaw_error;
			//printf("yaw moment:  %d \n", yaw_moment);
		}
		else
		{
			yaw_moment = 0;
		}

		lift  = DRONE_LIFT_CONSTANT * lift_force;
		roll  = 0; // no roll or pitch in yaw mode
		pitch = 0;
		yaw   = (int)(DRONE_YAW_CONSTANT/4 * yaw_moment); // misschien deze drone constant aanpassen voor yaw mode

		// Solving drone rotor dynamics equations
		ae_[0] = 0.25*(lift + 2*pitch - yaw);
		ae_[1] = 0.25*(lift - 2*roll  + yaw);
		ae_[2] = 0.25*(lift - 2*pitch - yaw);
		ae_[3] = 0.25*(lift + 2*roll  + yaw);

		// no negative number for sqrt
		ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
		ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
		ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
		ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);

		if (drone.joy_lift > 5)
		{
			ae_[0] = ae_[0] < MIN_RPM ? MIN_RPM : ae_[0];
			ae_[1] = ae_[1] < MIN_RPM ? MIN_RPM : ae_[1];
			ae_[2] = ae_[2] < MIN_RPM ? MIN_RPM : ae_[2];
			ae_[3] = ae_[3] < MIN_RPM ? MIN_RPM : ae_[3];
		}

		ae_[0] = ae_[0] > MAX_RPM ? MAX_RPM : ae_[0];
		ae_[1] = ae_[1] > MAX_RPM ? MAX_RPM : ae_[1];
		ae_[2] = ae_[2] > MAX_RPM ? MAX_RPM : ae_[2];
		ae_[3] = ae_[3] > MAX_RPM ? MAX_RPM : ae_[3];

		printf("%3d %3d %3d %3d | %d\n", ae_[0], ae_[1], ae_[2], ae_[3], bat_volt);

		// setting drone rotor speeds
		drone.ae[0] = ae_[0];
		drone.ae[1] = ae_[1];
		drone.ae[2] = ae_[2];
		drone.ae[3] = ae_[3];

		run_filters_and_control();
		nrf_delay_ms(1); // maybe remove delay?!
	}
	
	printf("Exit YAW_CONTROL_MODE\n");
}

void full_control_mode()
{
	printf("In FULL_CONTROL_MODE\n");

	int rollrate_setpoint;
	int roll_s;
	int pitchrate_setpoint;
	int pitch_s;
	int roll_moment;
	int pitch_moment;
	int lift_force;

	int ae_[4];

	int lift, roll, pitch, yaw;
	
	drone.key_lift = 20; // XXX: For testing
	
	while(drone.change_mode == 0 && drone.stop == 0)
	{
		check_log_tele_flags();
		read_sensor();

		__disable_irq();
		lift_force = drone.joy_lift + drone.key_lift;
		roll_s = (signed char)drone.joy_roll + (signed char)drone.key_roll;
		pitch_s = (signed char)drone.joy_pitch + (signed char)drone.key_pitch;
		__enable_irq();
				
		// sax ~ 7000, 7000/127 = 56 ???
		// phi ~ 2500/128 = 20
		
		if (lift_force > 10) 
		{
			// XXX: Find range
			rollrate_setpoint = drone.controlgain_p1 * (roll_s - (drone.phi/20));
			roll_moment = drone.controlgain_p2 * (rollrate_setpoint - (drone.sq/6));
			
			pitchrate_setpoint = drone.controlgain_p1 * (pitch_s - (drone.theta/20));
			pitch_moment = drone.controlgain_p2 * (pitchrate_setpoint - (drone.sp/6));
		}
		else
		{
			roll_moment = 0;
			pitch_moment = 0;
		}

		lift  = DRONE_LIFT_CONSTANT * lift_force;
		roll  = (int)(DRONE_ROLL_CONSTANT/50 * roll_moment);
		pitch = (int)(DRONE_PITCH_CONSTANT/50 * pitch_moment);
		yaw   = 0;
		
		// Solving drone rotor dynamics equations
		ae_[0] = 0.25*(lift + 2*pitch - yaw);
		ae_[1] = 0.25*(lift - 2*roll  + yaw);
		ae_[2] = 0.25*(lift - 2*pitch - yaw);
		ae_[3] = 0.25*(lift + 2*roll  + yaw);
				
		// no negative number for sqrt
		ae_[0] = ae_[0] < 0 ? 1 : (int)sqrt(ae_[0]);
		ae_[1] = ae_[1] < 0 ? 1 : (int)sqrt(ae_[1]);
		ae_[2] = ae_[2] < 0 ? 1 : (int)sqrt(ae_[2]);
		ae_[3] = ae_[3] < 0 ? 1 : (int)sqrt(ae_[3]);

		if (drone.joy_lift > 5)
		{
			ae_[0] = ae_[0] < MIN_RPM ? MIN_RPM : ae_[0];
			ae_[1] = ae_[1] < MIN_RPM ? MIN_RPM : ae_[1];
			ae_[2] = ae_[2] < MIN_RPM ? MIN_RPM : ae_[2];
			ae_[3] = ae_[3] < MIN_RPM ? MIN_RPM : ae_[3];
		}

		ae_[0] = ae_[0] > MAX_RPM ? MAX_RPM : ae_[0];
		ae_[1] = ae_[1] > MAX_RPM ? MAX_RPM : ae_[1];
		ae_[2] = ae_[2] > MAX_RPM ? MAX_RPM : ae_[2];
		ae_[3] = ae_[3] > MAX_RPM ? MAX_RPM : ae_[3];

		printf("%3d %3d %3d %3d | %d %d | %d\n", ae_[0], ae_[1], ae_[2], ae_[3], drone.controlgain_p1, drone.controlgain_p2, bat_volt);

		// setting drone rotor speeds
		drone.ae[0] = ae_[0];
		drone.ae[1] = ae_[1];
		drone.ae[2] = ae_[2];
		drone.ae[3] = ae_[3];

		run_filters_and_control();
		nrf_delay_ms(1); // maybe remove delay?!
	}

	printf("Exit FULL_CONTROL_MODE\n");
}

void calibration_mode()
{
	printf("In CALIBRATION_MODE\n");

	uint32_t counter = 0;
    int samples = 2500;
    int sum_sp = 0;
	int sum_sq = 0;
	int sum_sr = 0;
	int sum_sax = 0;
	int sum_say = 0;
	int sum_saz = 0;
	int sum_phi = 0;
	int sum_theta = 0;
	int sum_psi = 0;
	int sum_pressure = 0;
	int i;

	if(get_time_us() < 20000000)
		nrf_delay_ms(20000);
	
	// Discard some samples to avoid junk data
	for(i = 0; i < 2500; i++)
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
		sum_phi += phi;
		sum_theta += theta;
		sum_psi += psi;
		sum_pressure += pressure;
		
		nrf_delay_ms(1);
    }
	
    drone.offset_sp = (int)(sum_sp / samples);
	drone.offset_sq = (int)(sum_sq / samples);
	drone.offset_sr = (int)(sum_sr / samples);
	drone.offset_sax = (int)(sum_sax / samples);
	drone.offset_say = (int)(sum_say / samples);
	drone.offset_saz = (int)(sum_saz / samples);
	drone.offset_phi = (int)(sum_phi / samples);
	drone.offset_theta = (int)(sum_theta / samples);
	drone.offset_psi = (int)(sum_psi / samples);
	drone.offset_pressure = (int)(sum_pressure / samples);

    //printf("New offsets found: \n");
    //printf("sp = %d, sq = %d, sr = %d \n", drone.offset_sp, drone.offset_sq, drone.offset_sr);
	//printf("sax = %d, say = %d, saz = %d \n", drone.offset_sax, drone.offset_say, drone.offset_saz);
	//printf("phi = %d, theta = %d, psi = %d \n", drone.offset_phi, drone.offset_theta, drone.offset_psi);
	//printf("pressure = %ld \n", drone.offset_pressure);

    drone.current_mode = SAFE_MODE;
    drone.change_mode = 1;
    
    printf("Exit CALIBRATION_MODE\n");
}

void raw_mode()
{
	//printf("In RAW_MODE\n");
}

void wireless_mode()
{
	//printf("In WIRELESS_MODE\n");
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
			uart_put_32bit(read_data.pressure);
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

void process_drone()
{
	// Set defaults for the drone
	reset_drone();
	
	while (drone.stop == 0)
	{
		drone.change_mode = 0;

		if((drone.joy_lift > 10) && !(drone.current_mode == SAFE_MODE || drone.current_mode == PANIC_MODE))
		{
			drone.current_mode = SAFE_MODE;
			//printf("Make lift 0 before changing mode %d :)\n", drone.joy_lift);
			continue;
		}

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
			case WIRELESS_MODE:
					wireless_mode();
					break;
			default :
					// XXX: Needs to be handled
					break;
		}
	}
}
