#include "DS18B20.h"
#include <OneWire.h>
#include <DallasTemperature.h>

static OneWire           *s_ow      = nullptr;
static DallasTemperature *s_sensors = nullptr;
static DeviceAddress      s_addr[DS_MAX_DEVICES];
static uint8_t            s_count   = 0;

// High level //
bool dsInit(uint8_t pin, uint8_t resolution)
{
    // Đã khởi tạo rồi thì không tạo lại
    if (s_sensors != nullptr) {
        return s_count > 0;
    }

    s_ow      = new OneWire(pin);
    s_sensors = new DallasTemperature(s_ow);
    s_sensors->begin();
    s_sensors->setWaitForConversion(false);   // Không chặn khi ra lệnh đo

    uint8_t found = s_sensors->getDeviceCount();
    if (found == 0) {
        Serial.println("Khong tim thay DS18B20 nao! Kiem tra day DQ, tro 4.7k, nguon.");
        return false;
    }

    s_count = (found > DS_MAX_DEVICES) ? DS_MAX_DEVICES : found;

    if (s_sensors->isParasitePowerMode()) {
        Serial.println("Canh bao: DS18B20 dang o che do nguon ky sinh. Nen noi VDD vao 3V3.");
    }

    for (uint8_t i = 0; i < s_count; i++) {
        if (!s_sensors->getAddress(s_addr[i], i)) {
            Serial.printf("Khong lay duoc ma ROM cua cam bien %u\n", i);
            s_count = 0;
            return false;
        }
        s_sensors->setResolution(s_addr[i], resolution);
    }

    Serial.printf("Tim thay %u cam bien DS18B20\n", s_count);
    return true;
}

uint8_t dsGetDeviceCount()
{
    return s_count;
}

bool dsGetAddress(uint8_t index, uint8_t addr[8])
{
    if (index >= s_count) return false;
    memcpy(addr, s_addr[index], 8);
    return true;
}

bool dsStartConversion()
{
    if (s_sensors == nullptr || s_count == 0) return false;
    s_sensors->requestTemperatures();   // Trả về ngay vì đã tắt waitForConversion
    return true;
}

bool dsIsConversionDone()
{
    if (s_sensors == nullptr || s_count == 0) return false;
    return s_sensors->isConversionComplete();
}

bool dsReadTemp(float &temp, uint8_t index)
{
    if (s_sensors == nullptr || index >= s_count) return false;

    float t = s_sensors->getTempC(s_addr[index]);

    if (t == DEVICE_DISCONNECTED_C) {
        Serial.println("DS18B20 mat ket noi hoac loi CRC");
        return false;
    }
    // 85.0 °C là giá trị mặc định lúc bật nguồn: nghĩa là chưa đo xong hoặc nguồn bị sụt
    if (t == 85.0f) {
        Serial.println("DS18B20 tra ve 85.0 (chua do xong hoac loi nguon)");
        return false;
    }

    temp = t;
    return true;
}

bool dsReadTempBlocking(float &temp, uint8_t index)
{
    if (!dsStartConversion()) return false;

    uint32_t t0 = millis();
    while (!dsIsConversionDone()) {
        if (millis() - t0 > DS_CONVERSION_TIMEOUT_MS) {
            Serial.println("DS18B20 het thoi gian cho chuyen doi");
            return false;
        }
        delay(5);
    }
    return dsReadTemp(temp, index);
}
