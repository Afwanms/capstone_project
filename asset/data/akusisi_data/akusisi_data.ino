#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#define buzzerPin 25

Adafruit_MPU6050 mpu;

bool akuisisi = false;

void setup() {
  delay(3000);
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("Gagal menghubungkan ke MPU6050!");
    while (1)
      ;
  }

  Serial.println("MPU6050 terhubung!");
  Serial.println("Ketik '1' untuk mulai akuisisi, tekan Enter (kirim kosong) untuk berhenti.");
  Serial.println("accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input == "1") {
      akuisisi = true;
      Serial.println("# Mulai akuisisi data...");
    } else if (input.length() == 0) {
      akuisisi = false;
      Serial.println("# Berhenti akuisisi data.");
    }
  }

  if (akuisisi) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    Serial.print(a.acceleration.x);
    Serial.print(",");
    Serial.print(a.acceleration.y);
    Serial.print(",");
    Serial.print(a.acceleration.z);
    Serial.print(",");
    Serial.print(g.gyro.x);
    Serial.print(",");
    Serial.print(g.gyro.y);
    Serial.print(",");
    Serial.println(g.gyro.z);
    if(a.acceleration.x > 10){
      digitalWrite(buzzerPin, HIGH);
      delay(2000);
      digitalWrite(buzzerPin, LOW);
    }
    delay(1000);
  }
}