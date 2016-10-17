#include "modes.h"

void calibration_mode()
{
	//printf("In CALIBRATION_MODE\n");
	
	nrf_gpio_pin_toggle(YELLOW);

	uint32_t counter = 0;
    int samples = 100;
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

    //for(i = 0; i < samples; i++)
	i = 0;
	while(i < samples)
	{
		if (check_timer_flag()) 
		{
			if (counter++%20 == 0) nrf_gpio_pin_toggle(BLUE);

			read_baro();
			nrf_delay_ms(1);

			clear_timer_flag();
			
			sum_pressure += pressure;
		}
 
		if (check_sensor_int_flag()) 
		{
			get_dmp_data();
			//printf("%6d %6d %6d | ", sp, sq, sr);
			//printf("%6d %6d %6d \n", sax, say, saz);
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

    //printf("New offsets found: \n");
    //printf("i = %d, sp = %d, sq = %d, sr = %d \n", i, drone.offset_sp, drone.offset_sq, drone.offset_sr);
	//printf("sax = %d, say = %d, saz = %d \n", drone.offset_sax, drone.offset_say, drone.offset_saz);
	//printf("phi = %d, theta = %d, psi = %d \n", drone.offset_phi, drone.offset_theta, drone.offset_psi);
	//printf("pressure = %ld \n", drone.offset_pressure);

    drone.current_mode = SAFE_MODE;
    drone.change_mode = 1;
	
	nrf_delay_ms(1);
	
	nrf_gpio_pin_toggle(YELLOW);
    
    //printf("Exit CALIBRATION_MODE\n");
}