#include <Arduino.h>
#include <Wire.h>

#define MPU_ADDRESS  0x68

#define REG_SMPLRT_DIV    0x19
#define REG_CONFIG        0x1A
#define REG_GYRO_CONFIG   0x1B
#define REG_ACCEL_CONFIG  0x1C
#define REG_ACCEL_XOUT_H  0x3B
#define REG_GYRO_XOUT_H   0x43
#define REG_TEMP_OUT_H    0x41
#define REG_PWR_MGMT_1    0x6B
#define REG_WHO_AM_I      0x75

// Độ nhạy (LSB trên đơn vị)
#define ACCEL_LSB_PER_G     8192.0f   // ±4 g
#define GYRO_LSB_PER_DPS    65.5f     // ±500 °/s
#define TEMP_LSB_PER_C      340.0f
#define TEMP_OFFSET_C       36.53f

struct mpuRaw {
    int16_t ax, ay, az;
    int16_t temp;
    int16_t gx, gy, gz;
};

struct mpuData {
    float ax, ay, az;   // g
    float temp;         // °C
    float gx, gy, gz;   // °/s (đã trừ bias gyro)
};

struct mpuGyroOffset
{
    int16_t x = 0, y = 0, z = 0;
};

// Low level //
bool mpuWriteReg(uint8_t reg_addr, uint8_t data);
bool mpuReadReg(uint8_t reg_addr, uint8_t* buf, size_t len);

// High level //
bool mpuInit();
bool mpuCalibrateGyro(uint16_t samples = 500);           // giữ cảm biến đứng yên khi gọi
bool mpuReadRaw(mpuRaw &raw);                            // đọc accel + temp + gyro (14 byte)
bool mpuReadGyro(int16_t &gx, int16_t &gy, int16_t &gz); // chỉ đọc gyro (6 byte)
bool mpuReadTemp(int16_t &temp);
bool mpuReadAccel(int16_t &ax, int16_t &ay, int16_t &az);

// Phiên bản đơn vị vật lý (overload, tham số là float&)
bool mpuReadGyro(float &gx, float &gy, float &gz);      // °/s, đã trừ bias
bool mpuReadTemp(float &temp);                          // °C
bool mpuReadAccel(float &ax, float &ay, float &az);     // g
bool mpuReadData(mpuData &data);                         // đọc 14 byte, quy đổi tất cả
 
const mpuGyroOffset &mpuGetGyroOffset();
 
// Hàm tiện ích đổi đơn vị (dùng khi cần)
inline float mpuAccelToG(int16_t raw)   { return raw / ACCEL_LSB_PER_G; }
inline float mpuGyroToDps(int16_t raw)  { return raw / GYRO_LSB_PER_DPS; }
inline float mpuTempToC(int16_t raw)    { return raw / TEMP_LSB_PER_C + TEMP_OFFSET_C; }