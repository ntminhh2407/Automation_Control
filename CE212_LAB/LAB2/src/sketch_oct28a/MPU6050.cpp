#include "MPU6050.h"

mpuGyroOffset GyroOffset;

// Low level //
bool mpuWriteReg(uint8_t reg_addr, uint8_t data)
{
    Wire.beginTransmission(MPU_ADDRESS);
    Wire.write(reg_addr);
    Wire.write(data);
    uint8_t err = Wire.endTransmission(true);
    if (err != 0) {
        Serial.printf("Loi ghi reg 0x02X, ma loi = %d\n", reg_addr, err);
        return false;
    }
    return true;
}

bool mpuReadReg(uint8_t reg_addr, uint8_t* buf, size_t len)
{
    Wire.beginTransmission(MPU_ADDRESS);
    Wire.write(reg_addr);
    uint8_t err = Wire.endTransmission(false);
    if (err != 0) {
        Serial.printf("Loi gui dia chi reg 0x02X, ma loi = %d\n", reg_addr, err);
        return false;
    }
    size_t got = Wire.requestFrom((uint16_t)MPU_ADDRESS, (size_t)len, true);
    if (got != len){
        Serial.printf("Doc thieu byte: %u/%u\n", (unsigned)got, (unsigned)len);
        return false;
    }
    for (size_t i = 0; i < len; i++){
        buf[i] = Wire.read();
    }
    return true;
}

// High level //
bool mpuInit() {
    if (!mpuWriteReg(REG_PWR_MGMT_1, 0x01)) return false; 
    delay(100);
    if (!mpuWriteReg(REG_SMPLRT_DIV, 0x07)) return false; 
    if (!mpuWriteReg(REG_CONFIG, 0x03)) return false;     
    if (!mpuWriteReg(REG_GYRO_CONFIG, 0x08)) return false; 
    if (!mpuWriteReg(REG_ACCEL_CONFIG, 0x08)) return false;
    return true;
}

bool mpuCalibrateGyro(uint16_t samples)
{
    if (samples == 0) {
        return false;
    }

    int32_t sx = 0, sy = 0, sz = 0;
    uint16_t ok_samples = 0;

    for (uint16_t i = 0; i < samples; i++) {
        int16_t gx, gy, gz;
        if (mpuReadGyro(gx, gy, gz)){
            sx += gx;
            sy += gy;
            sz += gz;
            ok_samples++;
        }
        delay(5); // Tần số lấy mẫu 200Hz
    }

    if (ok_samples < samples/2){ // Quá nhiều mẫu lỗi
        return false;
    }

    return true;
}

bool mpuReadRaw(mpuRaw &raw)
{
    uint8_t b[14];
    if (!mpuReadReg(REG_ACCEL_XOUT_H, b, 14)) return false;

    raw.ax   = (int16_t)((b[0]  << 8) | b[1]);
    raw.ay   = (int16_t)((b[2]  << 8) | b[3]);
    raw.az   = (int16_t)((b[4]  << 8) | b[5]);
    raw.temp = (int16_t)((b[6]  << 8) | b[7]);
    raw.gx   = (int16_t)((b[8]  << 8) | b[9]);
    raw.gy   = (int16_t)((b[10] << 8) | b[11]);
    raw.gz   = (int16_t)((b[12] << 8) | b[13]);
    return true;
}

bool mpuReadGyro(int16_t &gx, int16_t &gy, int16_t &gz){
    uint8_t b[6];
    if (!mpuReadReg(REG_GYRO_XOUT_H, b, 6)) return false;

    gx = (int16_t)((b[0] << 8) | b[1]);
    gy = (int16_t)((b[2] << 8) | b[3]);
    gz = (int16_t)((b[4] << 8) | b[5]);
    return true;
}

bool mpuReadTemp(int16_t &temp) {
    uint8_t b[2];
    if (!mpuReadReg(REG_TEMP_OUT_H, b, 2)) return false;
    temp = (int16_t)(((uint16_t)b[0] << 8) | b[1]);
    return true;
}

bool mpuReadAccel(int16_t &ax, int16_t &ay, int16_t &az) {
    uint8_t b[6];
    if (!mpuReadReg(REG_ACCEL_XOUT_H, b, 6)) return false;

    ax = (int16_t)(((uint16_t)b[0] << 8) | b[1]);
    ay = (int16_t)(((uint16_t)b[2] << 8) | b[3]);
    az = (int16_t)(((uint16_t)b[4] << 8) | b[5]);
    return true;    
}

bool mpuReadGyro(float &gx, float &gy, float &gz)
{
    int16_t raw_gx, raw_gy, raw_gz;
    if (!mpuReadGyro(raw_gx, raw_gy, raw_gz)){
        return false;
    }
    gx = mpuGyroToDps(raw_gx - GyroOffset.x);
    gy = mpuGyroToDps(raw_gy - GyroOffset.y);
    gz = mpuGyroToDps(raw_gz - GyroOffset.z);
    return true;
}   

bool mpuReadTemp(float &temp)   
{
    int16_t raw_temp;
    if (!mpuReadTemp(raw_temp)){
        return false;
    }
    temp = mpuTempToC(raw_temp);
    return true;
}  

bool mpuReadAccel(float &ax, float &ay, float &az)    
{
    int16_t raw_ax, raw_ay, raw_az;
    if (!mpuReadAccel(raw_ax, raw_ay, raw_az)) return false;

    ax = mpuAccelToG(raw_ax);
    ay = mpuAccelToG(raw_ay);
    az = mpuAccelToG(raw_az);
    return true;
}

bool mpuReadData(mpuData &data)       
{
    mpuRaw r;
    if (!mpuReadRaw(r)) return false;

    data.ax   = mpuAccelToG(r.ax);
    data.ay   = mpuAccelToG(r.ay);
    data.az   = mpuAccelToG(r.az);
    data.temp = mpuTempToC(r.temp);
    data.gx   = mpuGyroToDps(r.gx - GyroOffset.x);
    data.gy   = mpuGyroToDps(r.gy - GyroOffset.y);
    data.gz   = mpuGyroToDps(r.gz - GyroOffset.z);
    return true;    
}                

const mpuGyroOffset &mpuGetGyroOffset()
{
    return GyroOffset;
}