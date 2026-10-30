#include <Arduino.h>

#define SENSOR_PIN 34
#define CHU_KY_US 10000

hw_timer_t *timer = nullptr;
volatile bool newSample = false;
unsigned long soMau = 0;

void IRAM_ATTR timerISR()
{
  newSample = true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("thoi_gian_ms,adc");

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timerISR, true);
  timerAlarmWrite(timer, CHU_KY_US, true);
  timerAlarmEnable(timer);
}

void loop() {
  if (newSample){
    newSample = false;

    int GiaTri = analogRead(SENSOR_PIN);
    unsigned long thoiGian = soMau * 10;

    Serial.print(thoiGian);
    Serial.print(',');
    Serial.println(GiaTri);

    soMau++;
  }
}