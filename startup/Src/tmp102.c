/*
 * tmp102.c
 *
 *  Created on: Nov 17, 2024
 *      Author: omen
 */

/*
 * EECS3215 - Embedded Systems Final Project
 *
 *  This TMP102 driver was created by
 *      Leroy,Nicole,Justin,Connor,MameMor
 *
 */

/***The HAL functions used are being explained because this driver was made outside
 * the STM32CubeIDE, and was made purely from studying the datasheet
 * HAL Functions Used:
 *  - HAL_StatusTypeDef is an enumeration type used for return statuses from HAL functions,
 *                      such as HAL_OK, HAL_ERROR, etc.
 *  - HAL_I2C_Master_Transmit() is used for transmitting data over i2c.
 *  - HAL_I2C_Master_Receive() is used for receiving data over i2c.
 *  - HAL_Delay() is used for creating a delay in milliseconds.
 ***/

#include "tmp102.h"
#include "stm32wbxx_hal.h"

extern I2C_HandleTypeDef hi2c3;

int TMP102_Init(void) { return TMP102_ERR_OK;   }


int TMP102_ReadTemperature(float *temperature /*pointer to store the temperature in celsius*/)
{
    HAL_StatusTypeDef leroy;
    uint8_t buffer[2];
    int16_t raw_temp;


    /* Transmit the temperature register address to the TMP102 */

    if ((leroy = HAL_I2C_Master_Transmit(&hi2c3, TMP102_I2C_ADDR, (uint8_t[]){TMP102_TEMP_REG}, 1, HAL_MAX_DELAY)) != HAL_OK) {
        return TMP102_ERR_WRITE; //Transmission error
    }


    /* Receive 2 bytes of temperature data */

    if ((leroy = HAL_I2C_Master_Receive(&hi2c3, TMP102_I2C_ADDR, buffer, 2, HAL_MAX_DELAY)) != HAL_OK) {
        return TMP102_ERR_READ; //Receiving error
    }


    //Combine bytes into a 12-bit raw temperature value
    raw_temp = ((int16_t)buffer[0] << 4) | (buffer[1] >> 4);

    /*Bit Representation Table:
   ------------------------------------------------------------------------
   | Byte 1 (buffer[0]) | Byte 2 (buffer[1]) | Combined 12-bit raw_temp    |
   | 1010 0101          | 1101 0000          | 1010 0101 0011 (after shift)|
   ------------------------------------------------------------------------
   Explanation:
   - buffer[0]: The most significant 8 bits of the temperature data.
   - buffer[1]: The least significant 8 bits; only the upper 4 bits are used.
   - Combined using: (buffer[0] << 4) | (buffer[1] >> 4)    */


/*=========================================================================================*/
/*=========================================================================================*/

    //Handle Negative Temp Vals
    //Page 9 of the datasheet

    //Sign-extension logic is if the 12th bit (MSB) is 1, it indicates a negative temperature.
    //We need to extend the sign bit to interpret the value correctly in 16-bit format.
    if (raw_temp > 0x7FF) raw_temp |= 0xF000;

/*
   -----------------------------------------------------------------------
   | 12-bit raw_temp  | Is Negative? | Action Taken (Sign-extension)     |
   |-------------------|--------------|-----------------------------------|
   | 0b0111 1111 1111  | No           | No change needed                  |
   | 0b1000 0000 0000  | Yes          | OR with 0xF000 to sign-extend     |
   | 0b1100 1010 1100  | Yes          | OR with 0xF000 to sign-extend     |
   -----------------------------------------------------------------------

*/

    *temperature = raw_temp * TMP102_RESOLUTION; //convert to celsius

    return TMP102_ERR_OK; //successful

}
