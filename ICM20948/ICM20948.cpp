#include "ICM20948.h"void ICM::ICM20948::write(uint8_t reg, uint8_t value) {  uint8_t buff[2] = {reg, value};#ifdef DEBUG  uint8_t valueRet = i2c_write_blocking(this->_i2c, this->_addr, buff, 2, false);  printf("byte write: %d\n", valueRet);#else  i2c_write_blocking(this->_i2c, this->_addr, buff, 2, false);  busy_wait_us(100);#endif}uint8_t ICM::ICM20948::read(uint8_t reg) {  uint8_t value = 0;  i2c_write_blocking(this->_i2c, this->_addr, &reg, 1, true);  i2c_read_blocking(this->_i2c, this->_addr, &value, 1, false);#ifdef DEBUG  printf("ret value: %d", value);#endif  return value;}void ICM::ICM20948::readByte(uint8_t reg, uint8_t *buff, uint8_t len) {  i2c_write_blocking(this->_i2c, this->_addr, &reg, 1, true);  i2c_read_blocking(this->_i2c, this->_addr, buff, len, false);#ifdef DEBUG  printf("ret value:\n");  for(uint8_t i = 0; i < len; i++)    printf("%d ", buff[i]);  printf("\n");#endif}void ICM::ICM20948::bank(uint8_t bank) {  if(this->_bank != bank){    write(ICM20948_BANK_SEL, bank << 4);    this->_bank = bank;  }}void ICM::ICM20948::triggerMagIo() {  uint8_t user = read(ICM20948_USER_CTRL);  write(ICM20948_USER_CTRL, user | 0x20);  busy_wait_ms(50);  write(ICM20948_USER_CTRL, user);}void ICM::ICM20948::magWrite(uint8_t reg, uint8_t value) {  bank(3);  write(ICM20948_I2C_SLV0_ADDR, AK09916_I2C_ADDR);  write(ICM20948_I2C_SLV0_REG, reg);  write(ICM20948_I2C_SLV0_DO, value);  bank(0);  triggerMagIo();}uint8_t ICM::ICM20948::magRead(uint8_t reg) {  bank(3);  write(ICM20948_I2C_SLV0_ADDR, AK09916_I2C_ADDR | 0x80);  write(ICM20948_I2C_SLV0_REG, reg);  write(ICM20948_I2C_SLV0_DO, 0xff);  write(ICM20948_I2C_SLV0_CTRL, 0x80 | 1);  bank(0);  triggerMagIo();  return read(ICM20948_EXT_SLV_SENS_DATA_00);}void ICM::ICM20948::magReadBytes(uint8_t reg, uint8_t *buff, uint8_t len) {  bank(3);  write(ICM20948_I2C_SLV0_CTRL, 0x80 | 0x08 | len);  write(ICM20948_I2C_SLV0_ADDR, AK09916_I2C_ADDR | 0x80);  write(ICM20948_I2C_SLV0_REG, reg);  write(ICM20948_I2C_SLV0_DO, 0xff);  bank(0);  triggerMagIo();  readByte(ICM20948_EXT_SLV_SENS_DATA_00, buff, len);}bool ICM::ICM20948::magnetometerReady() {  return ((magRead(AK09916_ST1) & 0x01) > 0);}ICM::ICM20948::ICM20948(i2c_inst_t *i2c, uint8_t sda, uint8_t scl, uint8_t addr) :  _i2c(i2c), _addr(addr), _sda(sda), _scl(scl){  i2c_init(i2c, 100*1000);  gpio_set_function(sda, GPIO_FUNC_I2C);  gpio_set_function(scl, GPIO_FUNC_I2C);////  gpio_pull_up(sda);//  gpio_pull_up(scl);  this->_bank = 5;//  init();//  autoOffsets();}bool ICM::ICM20948::init() {//  reset();  bank(0);  if(read(ICM20948_WHO_AM_I) != CHIP_ID){#ifdef DEBUG    printf("chip init fault");#endif    return false;  }  write(ICM20948_PWR_MGMT_1, 0x80);  busy_wait_ms(50);  write(ICM20948_PWR_MGMT_1, 0x01);  write(ICM20948_PWR_MGMT_2, 0x00);  bank(2);  setAccelSampleRate();  setAccelLowPass();  setAccelFullScale();  setGyroSampleRate();  setGyroLowPass();  setGyroFullScale();  bank(0);  write(ICM20948_INT_PIN_CFG, 0x30);//  bank(3);//  write(ICM20948_I2C_MST_CTRL, 0x4D);//  write(ICM20948_I2C_MST_DELAY_CTRL, 0x01);////  if(magRead(AK09916_WIA) != AK09916_CHIP_ID) {//#ifdef DEBUG//    printf("Unable to find AK09916");//#endif//    return true;//  }////  //reset magnetometer//  magWrite(AK09916_CNTL3, 0x01);//  while(magRead(AK09916_CNTL3) == 0x01){//    busy_wait_us(100);//  }  return true;}void ICM::ICM20948::getMagetometerData(float *buff) {  uint8_t data[6];  magWrite(AK09916_CNTL2, AK09916_CNTL2_MODE_CONT4);  while(!magnetometerReady()){    busy_wait_ms(50);  }  magReadBytes(AK09916_HXL, data, 6);  buff[0] = static_cast<float>((int16_t)(data[0] | data[1] << 8)) * 0.15;  buff[1] = static_cast<float>((int16_t)(data[2] | data[3] << 8)) * 0.15;  buff[2] = static_cast<float>((int16_t)(data[3] | data[5] << 8)) * 0.15;}void ICM::ICM20948::getAccelerometerData(float *buff) {  bank(0);  uint8_t data[6];  float scaleRangeAccel[] = {16384.0, 8192.0, 4096.0, 2048.0};  readByte(ICM20948_ACCEL_XOUT_H, data, 6);  bank(2);  uint8_t scale = (read(ICM20948_ACCEL_CONFIG) & 0x06) >> 1;  float gs = scaleRangeAccel[scale];  buff[0] = static_cast<float>((int16_t)((data[0] << 8)| data[1])) / gs;  buff[1] = static_cast<float>((int16_t)((data[2] << 8)| data[3])) / gs;  buff[2] = static_cast<float>((int16_t)((data[4] << 8)| data[5])) / gs * (-1);}void ICM::ICM20948::getGyroData(float *buff) {  bank(0);  uint8_t data[6];  float scaleRangeGyro[] = {131.f, 65.5f, 32.8f, 16.4f};  readByte(ICM20948_GRYO_XOUT_H, data, 6);  bank(2);  uint8_t scale = (read(ICM20948_GYRO_CONFIG_1) & 0x06) >> 1;  float dgs = scaleRangeGyro[scale];  buff[0] = static_cast<float>((int16_t)((data[0] << 8)| data[1])) / dgs;  buff[1] = static_cast<float>((int16_t)((data[2] << 8)| data[3])) / dgs;  buff[2] = static_cast<float>((int16_t)((data[4] << 8)| data[5])) / dgs;}void ICM::ICM20948::setAccelSampleRate(uint16_t rate) {  bank(2);  rate = (int)((1125.0 / rate) - 1);  write(ICM20948_ACCEL_SMPLRT_DIV_1, (rate >> 8) & 0xff);  write(ICM20948_ACCEL_SMPLRT_DIV_2, rate);}void ICM::ICM20948::setAccelFullScale(uint16_t scale) {  bank(2);  uint8_t value;  value = read(ICM20948_ACCEL_CONFIG) & 0b11111001;  switch (scale) {  case 2:    value |= 0b00 << 1;    break;  case 4:    value |= 0b01 << 1;    break;  case 8:    value |= 0b10 << 1;    break;  case 16:    value |= 0b11 << 1;    break;  }  write(ICM20948_ACCEL_CONFIG, value);}void ICM::ICM20948::setAccelLowPass(bool enable, uint16_t mode) {  bank(2);  uint8_t value;  value = read(ICM20948_ACCEL_CONFIG) & 0b10001110;  if(enable){      value |= 0b1;  }  value |= (mode & 0x07) << 4;  write(ICM20948_ACCEL_CONFIG, value);}void ICM::ICM20948::setGyroSampleRate(uint16_t rate) {  bank(2);  rate = (int)((1125.0 / rate) - 1);  write(ICM20948_GYRO_SMPLRT_DIV, rate);}void ICM::ICM20948::setGyroFullScale(uint16_t scale) {  bank(2);  uint8_t value;  value = read(ICM20948_GYRO_CONFIG_1) & 0b11111001;  switch (scale) {  case 250:    value |= 0b00 << 1;    break;  case 500:    value |= 0b01 << 1;    break;  case 1000:    value |= 0b10 << 1;    break;  case 2000:    value |= 0b11 << 1;    break;  }  write(ICM20948_GYRO_CONFIG_1, value);}void ICM::ICM20948::setGyroLowPass(bool enable, uint16_t mode) {  bank(2);  uint8_t value;  value = read(ICM20948_GYRO_CONFIG_1) & 0b10001110;  if(enable){    value |= 0b1;  }  value |= (mode & 0x07) << 4;  write(ICM20948_GYRO_CONFIG_1, value);}void ICM::ICM20948::getAllData(ICM::ICM20948Data *data) {  float buff[3];  getAccelerometerData(buff);  data->Accsel_x = buff[0];  data->Accsel_y = buff[1];  data->Accsel_z = buff[2];  getGyroData(buff);  data->Gyro_x = buff[0];  data->Gyro_y = buff[1];  data->Gyro_z = buff[2];}