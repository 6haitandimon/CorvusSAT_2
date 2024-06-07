#include "pico/stdlib.h"#include "BME280.h"#include "sx1276.h"#include "ICM20948.h"#include "cstdio"BME280::BME280 bme280 = BME280::BME280(i2c0, 0x76, 4, 5);LoRa Lora = LoRa(spi0, 16, 19, 18, 17, 20, 21);void toLora(unsigned char* packeg,  uint8_t index, float data){  union{    float data;    struct{      uint8_t byte1;      uint8_t byte2;      uint8_t byte3;      uint8_t byte4;    };  }data32Bit;//  data32Bit.data = data;  packeg[index++] = data32Bit.byte1;  packeg[index++] = data32Bit.byte2;  packeg[index++] = data32Bit.byte3;  packeg[index] = data32Bit.byte4;////  data32Bit.byte1 = packeg[index++];//  data32Bit.byte2 = packeg[index++];//  data32Bit.byte3 = packeg[index++];//  data32Bit.byte4 = packeg[index];//  return data32Bit.data;}bool falseFlag = false;int main(){  stdio_init_all();  gpio_init(2);  gpio_set_dir(2, GPIO_OUT);  gpio_put(2, true);  ICM20948::ICM20948 icm20948 = ICM20948::ICM20948(i2c0, 0x68, 4, 5);  float accel[3];  float gyro[3];  float mag[3];//  float groundAboveSee = 242;  uint8_t packeg[20];  if (Lora.begin()) {    Lora.setFrequency(869000000)        ->setTXPower(10)        ->setSpreadFactor(LoRa::SF_12)        ->setBandwidth(LoRa::BW_125k)        ->setCodingRate(LoRa::CR_48)        ->setSyncWord(0x12)        ->enableCRC();    }else{      falseFlag = true;    }  while(true){//    float temperature = bme280.GetTemp() / 100.f;//    float pressure = bme280.GetPressure() / 1000.f;//    float hum = bme280.GetHumidity() / 1024.f;//    float altitude = bme280.GetAltitude(1010.4);//    float realAltitud = altitude - groundAboveSee;////    toLora(packeg, 0, temperature);//    toLora(packeg, 4, pressure);//    toLora(packeg, 8, hum);//    toLora(packeg, 12, altitude);//    toLora(packeg, 16, realAltitud);////    unsigned char* packeg = (unsigned char *)"01234320123124003241234\00045345";    icm20948.getAccelerometerData(accel);    icm20948.getGyroData(gyro);    icm20948.getMagetometerData(mag);//  busy_wait_ms(50s0);//        toLora(packeg, 0, mag[0]);//        toLora(packeg, 4, mag[1]);//        toLora(packeg, 8, mag[2]);////        LoRaPacket p(packeg, sizeof(packeg));//        Lora.transmitPacket(&p);    printf("Accelerometer\n x: %0.2f, y: %0.2f, z: %0.2f\n\n", accel[0], accel[1], accel[2]);    printf("Gyroscope\n x: %0.2f, y: %0.2f, z: %0.2f\n\n", gyro[0], gyro[1], gyro[2]);    printf("Magnetometer\n x: %0.2f, y: %0.2f, z: %0.2f\n\n", mag[0], mag[1], mag[2]);    busy_wait_ms(100);  }}//    toLora(packeg, 12, static_cast<uint32_t>(altitude));//    toLora(packeg, 16, static_cast<uint32_t>(realAltitud));//    auto p = Lora.receivePacket();//    auto data = p.getPayload();//    printf("temp: %0.3f\n", toData(data, 0));//    printf("press: %0.3f\n", toData(data, 4));//    printf("hum: %0.3f\n", toData(data, 8));//    printf("alt: %0.3f\n", toData(data, 12));//    printf("realAlt: %0.3f\n", toData(data, 16));