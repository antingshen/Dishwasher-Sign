#ifndef I2C_H_
#define I2C_H_

#include "msp430.h"

#define SCL_PIN BIT6
#define SDA_PIN BIT7
#define SCL_LO P1DIR |= SCL_PIN
#define SCL_HI P1DIR &= ~SCL_PIN
#define SDA_LO P1DIR |= SDA_PIN
#define SDA_HI P1DIR &= ~SDA_PIN

void measure();
void i2c_start();
void i2c_stop();
void i2c_send_byte(unsigned char byte);
void i2c_send(unsigned char* data, int length);

#endif /* I2C_H_ */
