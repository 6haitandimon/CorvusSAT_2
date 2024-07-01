#pragma once#include "ICM20948Reg.h"#include "hardware/i2c.h"#include "pico/stdlib.h"#include "cstdio"namespace ICM{struct ICM20948Data{  struct {    float Accsel_x;    float Accsel_y;    float Accsel_z;  };  struct {    float Gyro_x;    float Gyro_y;    float Gyro_z;  };}__packed;class ICM20948{private:  i2c_inst_t *_i2c;  uint8_t _addr;  uint8_t _sda;  uint8_t _scl;  uint8_t _bank;  void write(uint8_t reg, uint8_t value);  uint8_t read(uint8_t reg);  void readByte(uint8_t reg, uint8_t* buff, uint8_t len);  void bank(uint8_t bank);  void triggerMagIo();  void magWrite(uint8_t reg, uint8_t value);  uint8_t magRead(uint8_t reg);  void magReadBytes(uint8_t reg, uint8_t* buff, uint8_t len = 1);  bool magnetometerReady();  void autoOffsets();public:  ICM20948(i2c_inst_t* i2c, uint8_t sda, uint8_t scl, uint8_t addr = I2C_ADDR);  bool init();  void getMagetometerData(float *buff);  void getAccelerometerData(float *buff);  void getGyroData(float *buff);  void getAllData(ICM20948Data* data);  void setAccelSampleRate(uint16_t rate = 125);  void setGyroSampleRate(uint16_t rate = 125);  void setAccelFullScale(uint16_t scale = 16);  void setGyroFullScale(uint16_t scale = 250);  void setAccelLowPass(bool enable = true, uint16_t mode = 5);  void setGyroLowPass(bool enable = true, uint16_t mode = 5);};}