/*
 * tmp102.h
 *
 *  Created on: Nov 17, 2024
 *      Author: omen
 */

#ifndef TMP102_TMP102_H_
#define TMP102_TMP102_H_

/*
 * EECS3215 - Embedded Systems Final Project
 *
 *  This TMP102 driver was created by
 *      Leroy,Nicole,Justin,Connor,MameMor
 *
 */


/*********i2c & Reg Defs************/
#define TMP102_I2C_ADDR     (0x48<<1)   //Shift left for HAL
#define TMP102_TEMP_REG     0x00        //Temp Reg
#define TMP102_CONFIG_REG   0x01        //Config Reg

/*********Error Codes************/
#define TMP102_ERR_OK       0           //No error
#define TMP102_ERR_WRITE    1           //Write error
#define TMP102_ERR_READ     2           //Read error

/*********Config Options************/
#define TMP102_SD_MODE      (1<<8)      //Shutdown mode


/*********Conversion Constant****/
#define TMP102_RESOLUTION   0.0625      //Resolution in Celsius per LSB

/**********Function Prototypes*********/
int TMP102_Init(void);
int TMP102_ReadTemperature(float *temperature);




#endif /* TMP102_TMP102_H_ */
