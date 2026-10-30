#pragma once

#include <Arduino.h>

#define LDR_DEFAULT_PIN      34       // GPIO34 thuộc ADC1, chỉ làm input
#define LDR_DEFAULT_SAMPLES  16       // Số mẫu lấy trung bình mỗi lần đọc
#define LDR_ADC_MAX          4095.0f  // ADC 12 bit
#define LDR_VCC_MV           3300.0f  // Điện áp cấp cho cầu phân áp (mV)
#define LDR_FIXED_OHM        10000.0f // Trở cố định 10kΩ nối xuống GND

// Sơ đồ: 3V3 --- LDR ---+--- 10kΩ --- GND
//                       |
//                    GPIO34 (Vout)
// Trời sáng -> LDR giảm điện trở -> Vout tăng.

struct ldrData {
    uint16_t raw;       // 0..4095, đã lấy trung bình
    float volt;         // V tại chân ADC
    float percent;      // 0..100 %, độ sáng tương đối (không phải lux)
    float ohm;          // Điện trở LDR tính ngược từ Vout
};

// High level //
bool ldrInit(uint8_t pin = LDR_DEFAULT_PIN);
bool ldrReadRaw(uint16_t &raw, uint8_t samples = LDR_DEFAULT_SAMPLES);
bool ldrReadData(ldrData &data, uint8_t samples = LDR_DEFAULT_SAMPLES);   // đọc 1 lần, quy đổi tất cả

// Hàm tiện ích đổi đơn vị (dùng khi cần)
inline float ldrRawToPercent(float raw)    { return raw * 100.0f / LDR_ADC_MAX; }
inline float ldrMvToVolt(float mv)         { return mv / 1000.0f; }
// Vout = Vcc * Rfixed / (Rldr + Rfixed)  =>  Rldr = Rfixed * (Vcc - Vout) / Vout
inline float ldrMvToOhm(float mv) {
    if (mv < 10.0f) mv = 10.0f;   // Tránh chia cho 0 khi quá tối
    return LDR_FIXED_OHM * (LDR_VCC_MV - mv) / mv;
}
