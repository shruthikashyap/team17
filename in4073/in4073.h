/*------------------------------------------------------------------
 *  in4073.h -- defines, globals, function prototypes
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#ifndef IN4073_H__
#define IN4073_H__

#include <inttypes.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "ml.h"

#define RED					22
#define YELLOW				24
#define GREEN				28
#define BLUE				30
#define INT_PIN				5

#define MOTOR_0_PIN			21
#define MOTOR_1_PIN			23
#define MOTOR_2_PIN			25
#define MOTOR_3_PIN			29

bool demo_done;

// Control
void run_filters_and_control();

// Timers
#define TIMER_PERIOD	2000 	//500 Hz 
void timers_init(void);
uint32_t get_time_us(void);
bool check_timer_flag(void);
void clear_timer_flag(void);

// GPIO
void gpio_init(void);
bool check_sensor_int_flag(void);
void clear_sensor_int_flag(void);

// Queue
#define QUEUE_SIZE 128
typedef struct {
	uint8_t Data[QUEUE_SIZE];
	uint8_t first,last;
  	uint8_t count; 
} queue;
void init_queue(queue *q);
void enqueue(queue *q, char x);
char dequeue(queue *q);

// UART
#define RX_PIN_NUMBER  16
#define TX_PIN_NUMBER  14
queue rx_queue;
queue tx_queue;
void uart_init(void);
void uart_put(uint8_t);

// TWI
#define TWI_SCL	4
#define TWI_SDA	2
void twi_init(void);
bool i2c_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t const *data);
bool i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t *data);

// MPU wrapper
int16_t phi, theta, psi;
int16_t sp, sq, sr;
int16_t sax, say, saz;
uint8_t sensor_fifo_count;
void imu_init(bool dmp, uint16_t interrupt_frequency); 
void get_dmp_data(void);
void get_raw_sensor_data(void);

// Barometer
int32_t pressure;
int32_t temperature;
void read_baro(void);
void baro_init(void);

// ADC
uint16_t bat_volt;
void adc_init(void);
void adc_request_sample(void);

// Flash
bool spi_flash_init(void);
bool flash_chip_erase(void);
bool flash_write_byte(uint32_t address, uint8_t data);
bool flash_write_bytes(uint32_t address, uint8_t *data, uint32_t count);
bool flash_read_byte(uint32_t address, uint8_t *buffer);
bool flash_read_bytes(uint32_t address, uint8_t *buffer, uint32_t count);

// BLE
queue ble_rx_queue;
queue ble_tx_queue;
void ble_init(void);
void ble_send(void);

// QR object
struct qr_t
{
	char current_mode;			// Current mode of QR
	int16_t change_mode;		// Flag to indicate change of mode
	int16_t stop;				// Flag to stop QR
	int16_t ae[4];				// Current ae[] values
	
	// Keyboard lift, roll, pitch and yaw values
	int16_t key_lift;
	int16_t key_roll;
	int16_t key_pitch;
	int16_t key_yaw;
	
	// Joystick lift, roll, pitch and yaw values
	int16_t joy_lift;
	int16_t joy_roll;
	int16_t joy_pitch;
	int16_t joy_yaw;
	
	// Calibration offset values
	int16_t offset_sp;
    int16_t offset_sq;
    int16_t offset_sr;
	int16_t offset_sax;
    int16_t offset_say;
    int16_t offset_saz;
	int16_t offset_phi;
    int16_t offset_theta;
    int16_t offset_psi;
	int32_t offset_pressure;
	
	// Sensor values
	int16_t sp;
	int16_t sq;
	int16_t sr;
	int16_t sax;
	int16_t say;
	int16_t saz;
	int16_t phi;
	int16_t theta;
	int16_t psi;
	int32_t pressure;

	// Control gain values
	int16_t controlgain_yaw;
	int16_t controlgain_p1;
	int16_t controlgain_p2;
	int16_t controlgain_height;
	
	// Height control 
	int32_t	height_control_pressure;
	int16_t height_control_lift;
};

extern struct qr_t drone;
extern struct log write_data;
extern struct log read_data;
extern uint32_t new_addr;
extern uint32_t new_addr_write;
extern bool log_flag;
extern bool control_loop_flag;
extern bool control_loop_flag_raw;
extern bool batt_low_flag;
extern bool telemetry_flag;
extern bool log_upload_flag;
extern bool log_active_flag;
extern bool height_control_flag;
extern int cable_disconnect_flag;
extern bool abort_flag;

#endif // IN4073_H__
