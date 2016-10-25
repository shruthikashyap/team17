#include "modes.h"
void send_telemetry_data();

void panic_mode()
{
	//printf("In PANIC_MODE\n");
	nrf_gpio_pin_toggle(RED);
	
	if(drone.ae[0] || drone.ae[1] || drone.ae[2] || drone.ae[3])
	{
		// Set moderate RPM values to the motors for hovering
		drone.ae[0] = HOVER_RPM;
		drone.ae[1] = HOVER_RPM;
		drone.ae[2] = HOVER_RPM;
		drone.ae[3] = HOVER_RPM;
	}
	
	send_telemetry_data();
	run_filters_and_control();
	
	// Stay in this mode for a few seconds
	nrf_delay_ms(2000);
	
	// Gradually reduce RPM of the motors to 0.
	while(drone.ae[0] || drone.ae[1] || drone.ae[2] || drone.ae[3])
	{
		drone.ae[0] = (drone.ae[0]) < RPM_STEP? 0 : drone.ae[0] - 10;
		drone.ae[1] = (drone.ae[1]) < RPM_STEP? 0 : drone.ae[1] - 10;
		drone.ae[2] = (drone.ae[2]) < RPM_STEP? 0 : drone.ae[2] - 10;
		drone.ae[3] = (drone.ae[3]) < RPM_STEP? 0 : drone.ae[3] - 10;
		
		run_filters_and_control();
		
		// Update log and telemetry if corresponding flags are set
		check_log_tele_flags();
		
		nrf_delay_ms(250);
	}

	// Go to safe mode
	drone.current_mode = SAFE_MODE;
	drone.change_mode = 1;
	nrf_gpio_pin_toggle(RED);
	
	//printf("Exit PANIC_MODE\n");
}