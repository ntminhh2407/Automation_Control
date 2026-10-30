#include <Arduino.h>
#include "LDR.h"

void setup() {
  Serial.begin(115200);
  delay(500);

  if (!ldrInit()) {
    Serial.println("Khoi tao LDR THAT BAI!");
    while (true) delay(1000);
  }

  Serial.println("LDR san sang (GPIO34).");
}

void loop() {
  ldrData d;
  if (!ldrReadData(d)) {
    delay(100);
    return;
  }

  Serial.printf("raw:%u,volt:%.2f,light:%.1f,kohm:%.1f\n",
                d.raw, d.volt, d.percent, d.ohm / 1000.0f);

  delay(200);
}