#include <Arduino.h>
#include "DS18B20.h"

#define READ_INTERVAL_MS 1000   // Chu kỳ đo

static uint32_t lastStartMs = 0;
static bool     waiting     = false;   // true = đang chờ cảm biến đo xong

void printAddress(uint8_t index)
{
  uint8_t addr[8];
  if (!dsGetAddress(index, addr)) return;

  Serial.printf("Cam bien %u, ma ROM: ", index);
  for (uint8_t i = 0; i < 8; i++) {
    Serial.printf("%02X", addr[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  if (!dsInit()) {
    Serial.println("Khoi tao DS18B20 THAT BAI! Kiem tra day DQ (GPIO4), tro 4.7k, nguon.");
    while (true) delay(1000);
  }

  for (uint8_t i = 0; i < dsGetDeviceCount(); i++) {
    printAddress(i);
  }

  Serial.println("DS18B20 san sang.");
}

void loop() {
  uint32_t now = millis();

  if (!waiting) {
    // Đến chu kỳ thì ra lệnh đo, không chờ tại chỗ
    if (now - lastStartMs >= READ_INTERVAL_MS) {
      if (dsStartConversion()) {
        waiting = true;
        lastStartMs = now;
      }
    }
  }
  else if (dsIsConversionDone()) {
    waiting = false;

    for (uint8_t i = 0; i < dsGetDeviceCount(); i++) {
      float t;
      if (dsReadTemp(t, i)) {
        Serial.printf("temp%u:%.2f\n", i, t);
      }
    }
  }
  else if (now - lastStartMs > DS_CONVERSION_TIMEOUT_MS) {
    // Chờ quá lâu mà chưa xong: bỏ lượt này, thử lại ở chu kỳ sau
    waiting = false;
    Serial.println("DS18B20 het thoi gian cho chuyen doi");
  }
}