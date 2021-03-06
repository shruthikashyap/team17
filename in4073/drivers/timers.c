/*------------------------------------------------------------------
 *  timers.c -- TIMER2 is for time-keeping, TIMER1 for motors. 
 *		TIMER0 is for soft-device
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"
 
static bool TIMER2_flag;
static uint32_t global_time;
static uint8_t batt_timer = 0;
static uint8_t control_loop_timer = 0;
static uint8_t telemetry_timer = 0;
static uint32_t heartbeat_counter = 0;

void timers_init(void)
{
	global_time = 0;
	TIMER2_flag = false;

	NRF_TIMER2->PRESCALER 	= 0x4UL; 	// 1us 
	NRF_TIMER2->INTENSET	= TIMER_INTENSET_COMPARE0_Msk | TIMER_INTENSET_COMPARE1_Msk | TIMER_INTENSET_COMPARE2_Msk;
	NRF_TIMER2->CC[0]	= 2500; 		// 400 Hz.
	NRF_TIMER2->CC[1]	= TIMER_PERIOD; // defined in in4073.h
	NRF_TIMER2->CC[2]	= 0xffff; 		// "overflow interrupt" for keeping global time
	NRF_TIMER2->TASKS_CLEAR = 1;

	NRF_TIMER1->PRESCALER 	= 0x4UL; 	// 1us
	NRF_TIMER1->INTENSET	= TIMER_INTENSET_COMPARE0_Msk | TIMER_INTENSET_COMPARE1_Msk | TIMER_INTENSET_COMPARE2_Msk | TIMER_INTENSET_COMPARE3_Msk;
	NRF_TIMER1->CC[0]	= 1000; 		// motor signal is 1-2ms, 1000 us is the minimum
	NRF_TIMER1->CC[1]	= 1000;
	NRF_TIMER1->CC[2]	= 1000;
	NRF_TIMER1->CC[3]	= 1000;
	NRF_TIMER1->TASKS_CLEAR = 1;

	NRF_TIMER2->TASKS_START	= 1;
	NRF_TIMER1->TASKS_START	= 1;
	
	NVIC_ClearPendingIRQ(TIMER2_IRQn);
	NVIC_SetPriority(TIMER2_IRQn, 3);
	NVIC_ClearPendingIRQ(TIMER1_IRQn);
	NVIC_SetPriority(TIMER1_IRQn, 3);
	NVIC_EnableIRQ(TIMER2_IRQn);
	NVIC_EnableIRQ(TIMER1_IRQn);
}

/*------------------------------------------------------------------
 *  void TIMER2_IRQHandler
 *
 *  Create seperate timer flags for the control loop (DMP and RAW),
 *	battery check, telemetry and cable disconnect check
 *
 *  Mods : Shruthi Kashyap
 *------------------------------------------------------------------
 */
void TIMER2_IRQHandler(void)
{
	if (NRF_TIMER2->EVENTS_COMPARE[0])
    {		
		if(control_loop_timer % 2 == 0) 			// 200 Hz
		{
			control_loop_flag = true;
			control_loop_timer = 0;
		}
		control_loop_timer++;

		if(batt_timer % 20 == 0) 					// 20 Hz
		{
			batt_timer = 0;
		}
		batt_timer++;

		if(telemetry_timer % 200 == 0) 				// 2 Hz
		{
			telemetry_flag = true;
			telemetry_timer = 0;

			if (cable_disconnect_flag == 0)	
				cable_disconnect_flag = 1;
			else if (cable_disconnect_flag == 1)
				cable_disconnect_flag = 2;

		}
		telemetry_timer++;

		TIMER2_flag = true;
		NRF_TIMER2->CC[0] += 2500;
		NRF_TIMER1->TASKS_CLEAR = 1;
		nrf_gpio_pin_set(MOTOR_0_PIN); 
		nrf_gpio_pin_set(MOTOR_1_PIN); 
		nrf_gpio_pin_set(MOTOR_2_PIN); 
		nrf_gpio_pin_set(MOTOR_3_PIN);
		NRF_TIMER2->EVENTS_COMPARE[0] = 0;

		if (heartbeat_counter++%100 == 0) nrf_gpio_pin_toggle(BLUE);
    }

	if (NRF_TIMER2->EVENTS_COMPARE[1])			// 500 Hz
    {
		log_flag = true;
		control_loop_flag_raw = true;				
		NRF_TIMER2->CC[1] += TIMER_PERIOD;
		NRF_TIMER2->EVENTS_COMPARE[1] = 0;
    }

	if (NRF_TIMER2->EVENTS_COMPARE[2])
    {
		global_time += 0xffff;
		NRF_TIMER2->EVENTS_COMPARE[2] = 0;
    }
}

void TIMER1_IRQHandler(void)
{
	if (NRF_TIMER1->EVENTS_COMPARE[0])
    	{
		nrf_gpio_pin_clear(MOTOR_0_PIN);
		NRF_TIMER1->EVENTS_COMPARE[0] = 0;
    	}

	if (NRF_TIMER1->EVENTS_COMPARE[1])
    	{
		nrf_gpio_pin_clear(MOTOR_1_PIN);
		NRF_TIMER1->EVENTS_COMPARE[1] = 0;
    	}

	if (NRF_TIMER1->EVENTS_COMPARE[2])
    	{
		nrf_gpio_pin_clear(MOTOR_2_PIN);
		NRF_TIMER1->EVENTS_COMPARE[2] = 0;
    	}

	if (NRF_TIMER1->EVENTS_COMPARE[3])
    	{
		nrf_gpio_pin_clear(MOTOR_3_PIN);
		NRF_TIMER1->EVENTS_COMPARE[3] = 0;
    	}
}

uint32_t get_time_us(void)
{
	NRF_TIMER2->TASKS_CAPTURE[3]=1;
	return global_time+NRF_TIMER2->CC[3];
}

bool check_timer_flag(void)
{
	return TIMER2_flag;
}

void clear_timer_flag(void)
{
	TIMER2_flag = false;
}
