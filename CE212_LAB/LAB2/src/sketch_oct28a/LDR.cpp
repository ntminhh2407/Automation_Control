#include "LDR.h"

static uint8_t s_pin = LDR_DEFAULT_PIN;
static bool    s_ready = false;

// High level //
bool ldrInit(uint8_t pin)
{
    s_pin = pin;

#ifdef CONFIG_IDF_TARGET_ESP32
    // ESP32 gốc: ADC2 (GPIO0, 2, 4, 12-15, 25-27) xung đột với WiFi
    if (pin < 32 || pin > 39) {
        Serial.printf("Canh bao: GPIO%u thuoc ADC2, se loi khi bat WiFi. Nen dung GPIO32-39.\n", pin);
    }
#endif

    pinMode(s_pin, INPUT);
    analogReadResolution(12);                    // 0..4095
    analogSetPinAttenuation(s_pin, ADC_11db);    // Đo được khoảng 0..3.3V

    s_ready = true;
    return true;
}

bool ldrReadRaw(uint16_t &raw, uint8_t samples)
{
    if (!s_ready || samples == 0) return false;

    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sum += analogRead(s_pin);
    }
    raw = (uint16_t)(sum / samples);
    return true;
}

bool ldrReadData(ldrData &data, uint8_t samples)
{
    if (!s_ready || samples == 0) return false;

    uint32_t sumRaw = 0;
    uint32_t sumMv  = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sumRaw += analogRead(s_pin);
        sumMv  += analogReadMilliVolts(s_pin);   // Đã hiệu chỉnh theo eFuse của chip
    }

    float avgRaw = (float)sumRaw / samples;
    float avgMv  = (float)sumMv  / samples;

    data.raw     = (uint16_t)avgRaw;
    data.volt    = ldrMvToVolt(avgMv);
    data.percent = ldrRawToPercent(avgRaw);
    data.ohm     = ldrMvToOhm(avgMv);
    return true;
}
