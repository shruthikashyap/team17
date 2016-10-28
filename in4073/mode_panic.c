/*------------------------------------------------------------------
 *  mode_panic.c
 *------------------------------------------------------------------
 */

#include "modes.h"
void send_telemetry_data();


 /*------------------------------------------------------------------
 *  void panic_mode
 *
 *  This function is the panic mode for the ES. In panic mode, the rotor speeds
 *  are set to a fixed RPM which represents hovering. When falling, the drone will, 
 *  therefore, stop falling and start hovering. After two seconds of hovering the 
 *  drone will come down graduatly to make sure touch-down occurs.
 *  In panic mode, the red LED is on. After touch-down, the drone switches
 *  to safe mode.
 *
 *  Author : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void panic_mode()
{

	nrf_gpio_pin_toggle(RED);
	
	// when one of the rotors speeds is not zero, start hovering
	if(drone.ae[0] || drone.ae[1] || drone.ae[2] || drone.ae[3])
	{
		// Set moderate RPM values to the motors for hovering
		drone.ae[0] = HOVER_RPM;
		drone.ae[1] = HOVER_RPM;
		drone.ae[2] = HOVER_RPM;
		drone.ae[3] = HOVER_RPM;
	}
	
	// send manual telemetry data to display we entered panic mode.
	send_telemetry_data();

	// update the rotors speeds for hovering.
	run_filters_and_control();
	
	// Hover for 2 seconds
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
}
