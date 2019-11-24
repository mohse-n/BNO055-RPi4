#include <wiringPiI2C.h>

#include <string.h>
#include <errno.h>
/* For cout etc. */
#include <iostream>
/* For usleep. */
#include <unistd.h>

/* Register addresses used in sensor setup. */
#define BNO055_ADDRESS_A         0x28
#define BNO055_CHIP_ID_ADDR      0x00
#define BNO055_ID                0xA0
#define BNO055_OPR_MODE_ADDR     0x3D
#define BNO055_SYS_TRIGGER_ADDR  0x20
#define BNO055_PAGE_ID_ADDR      0X07
#define POWER_MODE_NORMAL        0X00
#define BNO055_PAGE_ID_ADDR      0X07
#define BNO055_UNIT_SEL_ADDR     0X3B
#define BNO055_PWR_MODE_ADDR     0X3E
#define OPERATION_MODE_NDOF      0X0C
/* Address of the most and the least significant bytes of Euler angles. */
#define BNO055_EULER_H_LSB_ADDR  0X1A
#define BNO055_EULER_H_MSB_ADDR  0X1B
#define BNO055_EULER_R_LSB_ADDR  0X1C
#define BNO055_EULER_R_MSB_ADDR  0X1D
#define BNO055_EULER_P_LSB_ADDR  0X1E
#define BNO055_EULER_P_MSB_ADDR  0X1F
/* Sleep in miliseconds. To facilitate copy-pasting the setup part from
 * the Adafruit library. */
#define delay(A) (usleep(1000*A))

using namespace std;
/* I2C device file descriptor. */
int fd;

/* The setup() function is directly copy-pasted from Adafruit's library. */
/* See https://github.com/adafruit/Adafruit_BNO055/blob/master/examples/read_all_data/read_all_data.ino
 * Also https://github.com/adafruit/Adafruit_BNO055/blob/master/Adafruit_BNO055.cpp
 * and https://github.com/adafruit/Adafruit_BNO055/blob/master/Adafruit_BNO055.h */
 
/* I have left the Adafruit's comments mostly unmodified and have added my own. 
 * My comments always end with a period. */
void setup(void){
	
	/* Setup the I2C device on Linux. */
	fd = wiringPiI2CSetup(BNO055_ADDRESS_A) ;
	if (fd < 0)
	cout << "Error setting up the I2C device: " << strerror(errno) << endl;
	/* Unnecessary: Check the chip ID to see if it matches BNO055. */
	int id = wiringPiI2CReadReg8(fd, BNO055_CHIP_ID_ADDR);
	
	if (id != BNO055_ID) {
		sleep(1); // hold on for boot
		id = wiringPiI2CReadReg8(fd, BNO055_CHIP_ID_ADDR);
		if (id != BNO055_ID) {
			cout << "Failed to read sensor ID: Return value: " << id << endl;
		}
	}
	
	/* Unnecessary: Check the chip ID to see if it matches BNO055's. */
	int mode = wiringPiI2CReadReg8(fd, BNO055_OPR_MODE_ADDR);
	if (mode == 0)
		cout << "Current mode: Config mode" << endl;
		
	/* Adafruit code resets the sensor and checks its ID again, for 
	 * some reason. */
	/* Reset */
	if (wiringPiI2CWriteReg8(fd, BNO055_SYS_TRIGGER_ADDR, 0x20) == -1)
		cout << "Error resetting the device: " << strerror(errno) << endl;
	/* Delay incrased to 30ms due to power issues https://tinyurl.com/y375z699 */
	delay(30);
	while (wiringPiI2CReadReg8(fd, BNO055_CHIP_ID_ADDR) != BNO055_ID) {
		delay(10);
	}
	delay(50);

	/* Set to normal power mode */
	if (wiringPiI2CWriteReg8(fd, BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL) == -1)
		cout << "Error setting the power mode to normal: " << strerror(errno) << endl;
	delay(10);

	/* Not sure why we need to this. */
	if (wiringPiI2CWriteReg8(fd, BNO055_PAGE_ID_ADDR, 0) == -1)
		cout << "Failed to write page address: " << strerror(errno) << endl;
	
	/* Clear the SYS_TRIGGER_ADDR register. Why? */
	if (wiringPiI2CWriteReg8(fd, BNO055_SYS_TRIGGER_ADDR, 0x0) == -1)
		cout << "Error clearing the system trigger register: " << strerror(errno) << endl;
	delay(10);
	/* Set mode to NDOF(see section 3.3). In this mode, the M0 processor
	 * does the sensor fusion for us and we just read the roll/pitch/yaw values. */
	wiringPiI2CWriteReg8(fd, BNO055_OPR_MODE_ADDR, OPERATION_MODE_NDOF);
	delay(20);
	
	return;
}

int main(void){
	
	/* Initialize the register, set the operation mode. */
	setup();
	/* H: Heading. 
	 * Why not call it Yaw? To emphasize that we're using magnetometer
	 * to obtain this angle (and is therefore unreliable). */
	int16_t H_LSB, H_MSB;
	/* R: Roll. */
	int16_t R_LSB, R_MSB;
	/* P: Pitch. */
	int16_t P_LSB, P_MSB;
	/* Heading, roll and pitch, before conversion to degrees. */
	int16_t H, R, P;
	/* Heading, roll and pitch in degrees. */
	double Heading, Roll, Pitch;
	
	while (1){
		
		/* Self-explanatory: Read the most and the least siginificant
		 * bytes, concatenate to create the entire number, and convert
		 * the resulting integer to degrees. */
	
		H_LSB = wiringPiI2CReadReg8(fd, BNO055_EULER_H_LSB_ADDR);
		H_MSB = wiringPiI2CReadReg8(fd, BNO055_EULER_H_MSB_ADDR);
	
		R_LSB = wiringPiI2CReadReg8(fd, BNO055_EULER_R_LSB_ADDR);
		R_MSB = wiringPiI2CReadReg8(fd, BNO055_EULER_R_MSB_ADDR);
	
		P_LSB = wiringPiI2CReadReg8(fd, BNO055_EULER_P_LSB_ADDR);
		P_MSB = wiringPiI2CReadReg8(fd, BNO055_EULER_P_MSB_ADDR);
	
		H =  H_LSB | (H_MSB << 8);
		R =  R_LSB | (R_MSB << 8);
		P =  P_LSB | (P_MSB << 8);
	
		Heading = (double)H/16.0;
		Roll    = (double)R/16.0;
		Pitch   = (double)P/16.0;
	
		cout << "\nHeading: " << Heading << endl;
		cout << "Roll: " << Roll << endl;
		cout << "Pitch: " << Pitch << endl << endl;
	
		usleep(10000);
}
	
}




