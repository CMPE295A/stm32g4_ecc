#include "sensor.h"

#define MPU6050_ADDR 0xD0
#define SMPLRT_DIV_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41
#define GYRO_XOUT_H_REG 0x43
#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75
#define NUM_SAMPLES 5

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

HAL_StatusTypeDef status = HAL_OK;

int16_t accel_x_raw = 0;
int16_t accel_y_raw = 0;
int16_t accel_z_raw = 0;

int16_t gyro_x_raw = 0;
int16_t gyro_y_raw = 0;
int16_t gyro_z_raw = 0;


//float horizontal, vertical, lateral, roll, pitch, yaw;

//float kalman_gain = 0;
//float state_estimate = 0;
//float error_covariance = 1; // Initial error covariance
//float process_noise = 0.1; // Process noise covariance
//float measurement_noise = 0.1; // Measurement noise covariance
//
//static float kalman_filter(float measurement) {
//    float predicted_estimate = state_estimate;
//    float predicted_error_covariance = error_covariance + process_noise;
//    kalman_gain = predicted_error_covariance / (predicted_error_covariance + measurement_noise);
//    state_estimate = predicted_estimate + kalman_gain * (measurement - predicted_estimate);
//    error_covariance = (1 - kalman_gain) * predicted_error_covariance;
//    return state_estimate;
//}
//
//static float moving_average(float* buffer, int num_samples, float new_value) {
//    static int index = 0;
//    static float sum = 0;
//    sum -= buffer[index];
//    sum += new_value;
//    buffer[index] = new_value;
//    index = (index + 1) % num_samples;
//    return sum / num_samples;
//}

void sensor__init(void)
{
	uint8_t check;
	uint8_t data;
	// check device ID WHO_AM_I
	status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 1000);
//	if (status != HAL_OK) {
//		Error_Handler();
//	}
	if (check == 0x68) { // 0x68 will be returned by the sensor if everything goes well
		// power management register 0X6B we should write all 0's to wake the sensor up
		data = 0;
		status = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, PWR_MGMT_1_REG, 1,&data, 1, 1000);

		// Set DATA RATE of 1KHz by writing SMPLRT_DIV register
		data = 0x07;
		status = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &data, 1, 1000);

		// Set accelerometer configuration in ACCEL_CONFIG Register
		// XA_ST=0,YA_ST=0,ZA_ST=0, FS_SEL=0 -> ± 2g
		data = 0x00;
		status = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &data, 1, 1000);

		// Set Gyroscopic configuration in GYRO_CONFIG Register
		// XG_ST=0,YG_ST=0,ZG_ST=0, FS_SEL=0 -> ± 250 °/s
		data = 0x00;
		status = HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &data, 1, 1000);
//		if (status != HAL_OK) {
//			Error_Handler();
//		}
	}
}

void sensor__get_accel(sensor_t *sensor)
{
	uint8_t data[6];
//	static float horizontal_buffer[NUM_SAMPLES] = {0};
//	static float vertical_buffer[NUM_SAMPLES] = {0};
//	static float lateral_buffer[NUM_SAMPLES] = {0};

	status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, data, sizeof(data), 1000);
//	if (status != HAL_OK) {
//		Error_Handler();
//	}
	accel_x_raw = (int16_t)(data[0] << 8 | data [1]);
	accel_y_raw = (int16_t)(data[2] << 8 | data [3]);
	accel_z_raw = (int16_t)(data[4] << 8 | data [5]);

	sensor->horizontal = accel_x_raw/16384.0;
	sensor->vertical = accel_y_raw/16384.0;
	sensor->lateral = accel_z_raw/16384.0;

//	horizontal = moving_average(horizontal_buffer, NUM_SAMPLES, horizontal);
//	vertical = moving_average(vertical_buffer, NUM_SAMPLES, vertical);
//	lateral = moving_average(lateral_buffer, NUM_SAMPLES, lateral);

//	horizontal = kalman_filter(horizontal);
//	vertical = kalman_filter(vertical);
//	lateral = kalman_filter(lateral);

	char buf[64];
	sprintf(buf, "horizontal: %.2f, vertical: %.2f, lateral: %.2f\r\n", sensor->horizontal, sensor->vertical, sensor->lateral);
	HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}

void sensor__get_gyro(sensor_t *sensor)
{
	uint8_t data[6];
//	static float roll_buffer[NUM_SAMPLES] = {0};
//	static float pitch_buffer[NUM_SAMPLES] = {0};
//	static float yaw_buffer[NUM_SAMPLES] = {0};

	status = HAL_I2C_Mem_Read (&hi2c1, MPU6050_ADDR, GYRO_XOUT_H_REG, 1, data, sizeof(data), 1000);
//	if (status != HAL_OK) {
//		Error_Handler();
//	}
	gyro_x_raw = (int16_t)(data[0] << 8 | data [1]);
	gyro_y_raw = (int16_t)(data[2] << 8 | data [3]);
	gyro_z_raw = (int16_t)(data[4] << 8 | data [5]);

	sensor->roll = gyro_x_raw/131.0;
	sensor->pitch = gyro_y_raw/131.0;
	sensor->yaw = gyro_z_raw/131.0;

//	roll = moving_average(roll_buffer, NUM_SAMPLES, roll);
//	pitch = moving_average(pitch_buffer, NUM_SAMPLES, pitch);
//	yaw = moving_average(yaw_buffer, NUM_SAMPLES, yaw);

//	roll = kalman_filter(roll);
//	pitch = kalman_filter(pitch);
//	yaw = kalman_filter(yaw);

	char buf[64];
	sprintf(buf, "roll: %.2f, pitch: %.2f, yaw: %.2f\r\n\n", sensor->roll, sensor->pitch, sensor->yaw);
	HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}
