#include "sensor.h"
#include "stm32g4xx_hal_i2c.h"
//#include "stm32g431xx.h"

#define MPU6050_ADDR 0xD0
#define SMPLRT_DIV_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41
#define GYRO_XOUT_H_REG 0x43
#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;

HAL_StatusTypeDef status = HAL_OK;

int16_t accel_x_raw = 0;
int16_t accel_y_raw = 0;
int16_t accel_z_raw = 0;

int16_t gyro_x_raw = 0;
int16_t gyro_y_raw = 0;
int16_t gyro_z_raw = 0;

float horizontal, vertical, lateral, roll, pitch, yaw;

void sensor__init(void)
{
	uint8_t check;
	uint8_t data;
	// check device ID WHO_AM_I
	status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 1000);
	if (status != HAL_OK) {
		Error_Handler();
	}
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
		if (status != HAL_OK) {
			Error_Handler();
		}
	}
}

void sensor__get_accel(void)
{
	uint8_t data[6];

	// Read 6 BYTES of data starting from ACCEL_XOUT_H register

	status = HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, data, sizeof(data), 1000);
	if (status != HAL_OK) {
		Error_Handler();
	}
	accel_x_raw = (int16_t)(data[0] << 8 | data [1]);
	accel_y_raw = (int16_t)(data[2] << 8 | data [3]);
	accel_z_raw = (int16_t)(data[4] << 8 | data [5]);

	horizontal = accel_x_raw/16384.0;
	vertical = accel_y_raw/16384.0;
	lateral = accel_z_raw/16384.0;

	char buf[64];
	sprintf(buf, "horizontal: %.2f, vertical: %.2f, lateral: %.2f\r\n", horizontal, vertical, lateral);
	HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}

void sensor__get_gyro(void)
{
	uint8_t data[6];

	// Read 6 BYTES of data starting from GYRO_XOUT_H register

	status = HAL_I2C_Mem_Read (&hi2c1, MPU6050_ADDR, GYRO_XOUT_H_REG, 1, data, sizeof(data), 1000);
	if (status != HAL_OK) {
		Error_Handler();
	}
	gyro_x_raw = (int16_t)(data[0] << 8 | data [1]);
	gyro_y_raw = (int16_t)(data[2] << 8 | data [3]);
	gyro_z_raw = (int16_t)(data[4] << 8 | data [5]);

	roll = gyro_x_raw/131.0;
	pitch = gyro_y_raw/131.0;
	yaw = gyro_z_raw/131.0;

	char buf[64];
	sprintf(buf, "roll: %.2f, pitch: %.2f, yaw: %.2f\r\n\n", roll, pitch, yaw);
	HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
}

void sensor_Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	char buf[64];
	sprintf(buf, "Sensor not detected! \r\n");
	HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
