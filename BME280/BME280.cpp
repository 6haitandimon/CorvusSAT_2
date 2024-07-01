#include "BME280.h"uint8_t BME::BME280::readRegistr8Bit(uint8_t RegAddr) {  uint8_t data = 0;  i2c_write_blocking(this->i2c, this->i2cAddr, &RegAddr, 1, true);  i2c_read_blocking(this->i2c, this->i2cAddr, &data, 1, false);  return data;}uint16_t BME::BME280::readRegistr16Bit(uint8_t RegAddr) {  uint8_t rawData[2] = {0};  i2c_write_blocking(this->i2c, this->i2cAddr, &RegAddr, 1, true);  i2c_read_blocking(this->i2c, this->i2cAddr, rawData, 2, false);  uint16_t data = (rawData[0] << 8 | rawData[1]);  return data;}int16_t BME::BME280::readRegistr16Bit_LE(uint8_t RegAddr) {  uint8_t rawData[2] = {0};  i2c_write_blocking(this->i2c, this->i2cAddr, &RegAddr, 1, true);  i2c_read_blocking(this->i2c, this->i2cAddr, rawData, 2, false);  int16_t data = (rawData[1] << 8 | rawData[0]);  return data;}uint32_t BME::BME280::readRegistr24Bit(uint8_t RegAddr) {  uint8_t rawData[3] = {0};  i2c_write_blocking(this->i2c, this->i2cAddr, &RegAddr, 1, true);  i2c_read_blocking(this->i2c, this->i2cAddr, rawData, 3, false);  uint32_t data = ((rawData[0] << 16) | (rawData[1] << 8) | (rawData[2]));  return data;}int BME::BME280::writeRegistr(uint8_t RegAddr, uint8_t data) {  uint8_t buff[2] = {RegAddr, data};  int ret = i2c_write_blocking(this->i2c, this->i2cAddr, buff, 2, false);  return ret;}BME::BME280::BME280(i2c_inst_t *_i2c, uint8_t _sda, uint8_t _scl, uint8_t _configVal, uint8_t _addr){    this->i2c = _i2c;    this->i2cAddr = _addr;    this->scl = _scl;    this->sda = _sda;    this->configVal = _configVal;    i2c_init(this->i2c, 100*1000);    gpio_set_function(this->sda, GPIO_FUNC_I2C);    gpio_set_function(this->scl, GPIO_FUNC_I2C);//    BME280_INIT();////    getCalibData();}bool BME::BME280::BME280_INIT() {  Reset();  busy_wait_ms(50);  bool stat = false;  if(this->configVal == 0){    this->configVal = ((0x04 << 5) | (0x05 << 2)) & 0xFC;  }  int ret = writeRegistr(REG_CONFIG, this->configVal);   if(ret == 2){      stat = true;   }   this->ctrl_hum = 0x04;   ret = writeRegistr(REG_CTRL_HUM, this->ctrl_hum);   if(ret == 2){     stat = true;   }  // osrs_t x1, osrs_p x4, normal mode operation  this->ctrl_maes = (0x01 << 5) | (0x03 << 2) | (0x03);  ret = writeRegistr(REG_CTRL_MEAS, this->ctrl_maes);  if(ret == 2){    stat = true;  }  if(stat)    getCalibData();#ifdef DEBUG  printf("stat: %b\n", stat);#endif  return stat;}int32_t BME::BME280::GetTemp() {  int32_t Temp = 0;  Temp = (readRegistr8Bit(REG_TEMP_MSB) << 12) | (readRegistr8Bit(REG_TEMP_LSB) << 4) | (readRegistr8Bit(REG_TEMP_XLSB) >> 4);  this->t_fine = convertData(Temp);  return ((this->t_fine * 5 + 128) >> 8);}uint32_t BME::BME280::GetPressure(){  if(this->t_fine == 0){    GetTemp();  }  uint32_t data = (readRegistr8Bit(REG_PRESSURE_MSB) << 12) | (readRegistr8Bit(REG_PRESSURE_LSB) << 4) | (readRegistr8Bit(REG_PRESSURE_XLSB) >> 4);  int32_t var1, var2;  uint32_t Pressure = 0;  var1 = (((int32_t)this->t_fine) >> 1) - (int32_t)64000;  var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)this->ParamCalib.dig_P6);  var2 += ((var1 * ((int32_t)this->ParamCalib.dig_P5)) << 1);  var2 = (var2 >> 2) + (((int32_t)this->ParamCalib.dig_P4) << 16);  var1 = (((this->ParamCalib.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)this->ParamCalib.dig_P2) * var1) >> 1)) >> 18;  var1 = ((((32768 + var1)) * ((int32_t)this->ParamCalib.dig_P1)) >> 15);  if (var1 == 0) {    return 0;  // avoid exception caused by division by zero  }  Pressure = (((uint32_t)(((int32_t)1048576) - data) - (var2 >> 12))) * 3125;  if (Pressure < 0x80000000) {    Pressure = (Pressure << 1) / ((uint32_t)var1);  } else {    Pressure = (Pressure / (uint32_t)var1) * 2;  }  var1 = (((int32_t)this->ParamCalib.dig_P9) * ((int32_t)(((Pressure >> 3) * (Pressure >> 3)) >> 13))) >> 12;  var2 = (((int32_t)(Pressure >> 2)) * ((int32_t)this->ParamCalib.dig_P8)) >> 13;  Pressure = (uint32_t)((int32_t)Pressure + ((var1 + var2 + this->ParamCalib.dig_P7) >> 4));  return Pressure;}uint32_t BME::BME280::GetHumidity(){  int32_t var1, var2, var3, var4, var5;    if(this->t_fine == 0){    GetTemp();  }  uint16_t data = readRegistr16Bit(REG_HUM_MSB);  var1 = t_fine - ((int32_t)76800);  var2 = (int32_t)(data * 16384);  var3 = (int32_t)(((int32_t)this->ParamCalib.dig_H4) * 1048576);  var4 = ((int32_t)this->ParamCalib.dig_H5) * var1;  var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;  var2 = (var1 * ((int32_t)this->ParamCalib.dig_H6)) / 1024;  var3 = (var1 * ((int32_t)this->ParamCalib.dig_H3)) / 2048;  var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;  var2 = ((var4 * ((int32_t)this->ParamCalib.dig_H2)) + 8192) / 16384;  var3 = var5 * var2;  var4 = ((var3 / 32768) * (var3 / 32768)) / 128;  var5 = var3 - ((var4 * ((int32_t)this->ParamCalib.dig_H1)) / 16);  var5 = (var5 < 0 ? 0 : var5);  var5 = (var5 > 419430400 ? 419430400 : var5);  uint32_t hum = (uint32_t)(var5 / 4096);  return hum;}float BME::BME280::GetAltitude(float seeLevel) {  float atmospheric = GetPressure() / 100.0F;  return (44330.0 * (1.0 - pow(atmospheric / seeLevel, 0.1903)));}void BME::BME280::Reset() {  writeRegistr(REG_RESET, 0x86);}void BME::BME280::getCalibData() {  this->ParamCalib.dig_T1 = (uint16_t)readRegistr16Bit_LE(REG_DIG_T1);  this->ParamCalib.dig_T2 = readRegistr16Bit_LE(REG_DIG_T2);  this->ParamCalib.dig_T3 = readRegistr16Bit_LE(REG_DIG_T3);  this->ParamCalib.dig_P1 = (uint16_t)readRegistr16Bit_LE(REG_DIG_P1);  this->ParamCalib.dig_P2 = readRegistr16Bit_LE(REG_DIG_P2);  this->ParamCalib.dig_P3 = readRegistr16Bit_LE(REG_DIG_P3);  this->ParamCalib.dig_P4 = readRegistr16Bit_LE(REG_DIG_P4);  this->ParamCalib.dig_P5 = readRegistr16Bit_LE(REG_DIG_P5);  this->ParamCalib.dig_P6 = readRegistr16Bit_LE(REG_DIG_P6);  this->ParamCalib.dig_P7 = readRegistr16Bit_LE(REG_DIG_P7);  this->ParamCalib.dig_P8 = readRegistr16Bit_LE(REG_DIG_P8);  this->ParamCalib.dig_P9 = readRegistr16Bit_LE(REG_DIG_P9);  this->ParamCalib.dig_H1 = readRegistr8Bit(REG_DIG_H1);  this->ParamCalib.dig_H2 = readRegistr16Bit_LE(REG_DIG_H2);  this->ParamCalib.dig_H3 = readRegistr8Bit(REG_DIG_H3);  this->ParamCalib.dig_H4 = ((int8_t)readRegistr8Bit(REG_DIG_H4) << 4) | (readRegistr8Bit(REG_DIG_H4 + 1) & 0xF);  this->ParamCalib.dig_H5 = ((int8_t)readRegistr8Bit(REG_DIG_H5 + 1) << 4) | (readRegistr8Bit(REG_DIG_H5) >> 4);  this->ParamCalib.dig_H6 = (int8_t)readRegistr8Bit(REG_DIG_H6);}// intermediate function that calculates the fine resolution temperature// used for both pressure and temperature conversionsint32_t BME::BME280::convertData(uint32_t data){  // use the 32-bit fixed point compensation implementation given in the  // datasheet  int32_t var1, var2;  var1 = ((((data >> 3) - ((int32_t)this->ParamCalib.dig_T1 << 1))) * ((int32_t)this->ParamCalib.dig_T2)) >> 11;  var2 = (((((data >> 4) - ((int32_t)this->ParamCalib.dig_T1)) * ((data >> 4) - ((int32_t)this->ParamCalib.dig_T2))) >> 12) * ((int32_t)this->ParamCalib.dig_T3)) >> 14;  return var1 + var2;}void BME::BME280::GetAllData(BME::BME280Data *data) {  data->temp = GetTemp() / 100.f;  data->hum = GetHumidity() / 1024.f;  data->press = GetPressure() / 100.f;}