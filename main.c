#include "uart.h"
#include "twi_master.h"
#include <avr/io.h>
#include <util/delay.h>

/************************************************************************/
/*							Initializations                             */
/************************************************************************/

#define MPU6050_ADDR	0x68

/* MPU6050 register address */
#define ACCEL_XOUT_H	0x3B
#define ACCEL_XOUT_L	0x3C
#define ACCEL_YOUT_H	0x3D
#define ACCEL_YOUT_L	0x3E
#define ACCEL_ZOUT_H	0x3F
#define ACCEL_ZOUT_L	0x40
#define PWR_MGMT_1		0x6B
#define GYRO_XOUT_H		0x43
#define GYRO_XOUT_I		0x44
#define GYRO_YOUT_H		0x45
#define GYRO_YOUT_I		0x46
#define GYRO_ZOUT_H		0x47
#define GYRO_ZOUT_I		0x48

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
} mpu_data_t;

typedef struct
{
	int16_t gx;
	int16_t gy;
	int16_t gz;
} mpu_gy_data_t;


/************************************************************************/
/*							Prototype functions                         */
/************************************************************************/

void ERROR_CHECK(ret_code_t error_code);
void mpu_init(void);
void mpu_get_accel_raw(mpu_data_t* mpu_data);
void mpu_get_accel(mpu_data_t* mpu_data);


/************************************************************************/
/*							Function definitions                        */
/************************************************************************/

void ERROR_CHECK(ret_code_t error_code)
{
	if (error_code != SUCCESS)
	{
		/* エラーコードを出力し、リセットされるまで無限にループ */
		printf(BR "App error! error_code = 0x%02X\n" RESET, error_code);
		while (1); // loop indefinitely
	}
}


void mpu_init(void)
{
	ret_code_t error_code;
	puts("Write 0 to PWR_MGMT_1 reg to wakeup MPU.");
	uint8_t data[2] = {PWR_MGMT_1, 0};
	error_code = tw_master_transmit(MPU6050_ADDR, data, sizeof(data), false);
	ERROR_CHECK(error_code);
}


void mpu_get_accel_raw(mpu_data_t* mpu_data)
{
	ret_code_t error_code;
	
	uint8_t data[6];	// 加速度x、y、zデータ各2bit 計6のレジスタを読み取る
	
	data[0] = ACCEL_XOUT_H;												//配列にアクセスしたいアドレスを入れて
	error_code = tw_master_transmit(MPU6050_ADDR, data, 1, true);		//1byte送信
	ERROR_CHECK(error_code);
	
	error_code = tw_master_receive(MPU6050_ADDR, data, sizeof(data));	//帰ってくるデータをsizeof(data[6])つまり6byte読む
	ERROR_CHECK(error_code);
	
	/* デフォルトの加速設定+/- 2g */
	mpu_data->x = (int16_t)(data[0] << 8 | data[1]);
	mpu_data->y = (int16_t)(data[2] << 8 | data[3]);
	mpu_data->z = (int16_t)(data[4] << 8 | data[5]);
}


void mpu_get_accel(mpu_data_t* mpu_data)
{
	/* 計算するならここの右辺等で(flout使うならmakefileの-lｍ忘れずに) */
	mpu_get_accel_raw(mpu_data);
	mpu_data->x = mpu_data->x;
	mpu_data->y = mpu_data->y;
	mpu_data->z = mpu_data->z;
}

void mpu_get_gyro_raw(mpu_gy_data_t* mpu_gy_data)
{
	ret_code_t error_code;

	uint8_t data_g[6];
	
	data_g[0] = GYRO_XOUT_H;
	error_code = tw_master_transmit(MPU6050_ADDR, data_g, 1, true);
	ERROR_CHECK(error_code);
	
	error_code = tw_master_receive(MPU6050_ADDR, data_g, sizeof(data_g));
	ERROR_CHECK(error_code);

	mpu_gy_data->gx = (int16_t)(data_g[0] << 8 | data_g[1]);
	mpu_gy_data->gy = (int16_t)(data_g[2] << 8 | data_g[3]);
	mpu_gy_data->gz = (int16_t)(data_g[4] << 8 | data_g[5]);
}

void mpu_get_gyro(mpu_gy_data_t* mpu_gy_data)
{
	mpu_get_gyro_raw(mpu_gy_data);
	mpu_gy_data->gx = mpu_gy_data->gx;
	mpu_gy_data->gy = mpu_gy_data->gy;
	mpu_gy_data->gz = mpu_gy_data->gz;
}

/************************************************************************/
/*							Main application                            */
/************************************************************************/

int main(void)
{
	/* Initialize UART */
	uart_init(250000); // bps
	cli_reset();
	puts(BY "Initializing TWI_Test Project...\n" RESET);
	
	/* Initialize project configuration */
	tw_init(TW_FREQ_100K, true); // I2C 周波数セット, 内部プルアップを許可
	mpu_init();
	mpu_data_t accel;
	mpu_gy_data_t gyro;
	
	puts(BG CURSOR_RIGHT("14")
	"--------------- Application Started ---------------\n" RESET);
	
	while (1)
	{
		puts("Read accelerometer data.");
		mpu_get_accel(&accel);
		printf("Accel X: %d\n", accel.x);
		printf("Accel Y: %d\n", accel.y);
		printf("Accel Z: %d\n", accel.z);
		puts("Read gyro data.");
		mpu_get_gyro(&gyro);
		printf("Gyro X: %d\n", gyro.gx);
		printf("Gyro Y: %d\n", gyro.gy);
		printf("Gyro Z: %d\n", gyro.gz);
		_delay_ms(100);
	}
}
