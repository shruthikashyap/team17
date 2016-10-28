/*------------------------------------------------------------------
 *  mode_calibration.c
 *------------------------------------------------------------------
 */

#include "modes.h"
#include "filters.h"
void send_telemetry_data();
extern bool raw_mode_flag;

/*------------------------------------------------------------------
 *  void calibration_mode
 *
 *  This function is the calibration mode for the ES. It calibrates the
 *  drone sensor values (gyroscope, accelerometer and pressure sensor).
 *  It averages a number a sensor values, which will be the offset of the sensor.
 *  The yellow led will be on during calibration. After calibration, the ES 
 *  switches back to safe mode.
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void calibration_mode()
{	
	nrf_gpio_pin_toggle(YELLOW);

    int samples = 500;
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

	// send telemetry manually because we don't check for timer flags in this mode
	send_telemetry_data();
	
	// first 20 seconds are junk values so wait 20 seconds
	if(get_time_us() < 20000000)
		nrf_delay_ms(20000);
	
	// Discard some sensor values to avoid junk data
	for(i = 0; i < 2500; i++)
	{
		if (check_timer_flag()) 
		{
			read_baro();
			nrf_delay_ms(1);
			clear_timer_flag();
		}

		if (check_sensor_int_flag()) 
		{
			if (raw_mode_flag == true)
			{
				get_raw_sensor_data();
				butterworth();
				kalman();
			}
			else
			{
				get_dmp_data();
			}
			nrf_delay_ms(1);
			clear_sensor_int_flag();
		}
	}

	// loop until we have enough samples to average
	i = 0;
	while(i < samples)
	{
		if (check_timer_flag()) 
		{
			read_baro();
			nrf_delay_ms(1);

			clear_timer_flag();
			
			sum_pressure += pressure;
		}

		if (check_sensor_int_flag()) 
		{
			if (raw_mode_flag == true)
			{
				get_raw_sensor_data();
				butterworth();
				kalman();
			}
			else
			{
				get_dmp_data();
			}

			nrf_delay_ms(1);
			clear_sensor_int_flag();
			
			sum_sp += sp;
			sum_sq += sq;
			sum_sr += sr;
			sum_sax += sax;
			sum_say += say;
			sum_saz += saz;
			sum_phi += phi;
			sum_theta += theta;
			sum_psi += psi;
			
			i++;
		}
		
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

	drone.current_mode = SAFE_MODE;
	drone.change_mode = 1;
	
	nrf_delay_ms(1);
	
	nrf_gpio_pin_toggle(YELLOW);
}
