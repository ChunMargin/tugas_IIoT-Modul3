#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>

constexpr uint8_t SWITCH_PIN = 3;
constexpr uint8_t POT_PIN = 1;
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = DHT11;

BH1750 lightMeter;
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Started!");

  pinMode(SWITCH_PIN, INPUT_PULLDOWN);
  analogReadResolution(12);

  Wire.begin(20, 21);

  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 siap.");
  } else {
    Serial.println("BH1750 tidak terdeteksi.");
  }

  dht.begin();
}

void loop() {
  static unsigned long lastPrint = 1000;
  const unsigned long interval = 1000;
  const unsigned long timestamp = millis();

  if (timestamp - lastPrint >= interval) {
    lastPrint = timestamp;

    Serial.println("=== NODE AKUISISI IIoT ===");
    Serial.print("Timestamp: ");
    Serial.print(timestamp);
    Serial.println(" ms");

    const int spdtState = digitalRead(SWITCH_PIN);
    const int raw = analogRead(POT_PIN);
    const float voltage = (raw / 4095.0f) * 3.3f;
    const float lux = lightMeter.readLightLevel();
    const float humidity = dht.readHumidity();
    const float temperature = dht.readTemperature();

    Serial.print("SPDT = ");
    Serial.println(spdtState == HIGH ? "HIGH / ON" : "LOW / OFF");

    Serial.print("Potensio ADC: ");
    Serial.print(raw);
    Serial.print(" | Tegangan pendekatan: ");
    Serial.print(voltage, 3);
    Serial.println(" V");

    String potLevel;
    if (raw <= 1365) {
      potLevel = "LOW";
    } else if (raw <= 2730) {
      potLevel = "MEDIUM";
    } else {
      potLevel = "HIGH";
    }
    
    Serial.print("Simulasi Level: ");
    Serial.println(potLevel);

    if (isnan(lux)) {
      Serial.println("BH1750 gagal membaca cahaya.");
    } else {
      Serial.print("Cahaya: ");
      Serial.print(lux, 2);
      Serial.println(" lx");
    }

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Gagal membaca sensor DHT11!");
    } else {
      Serial.print("Kelembapan: ");
      Serial.print(humidity);
      Serial.print("% | Suhu: ");
      Serial.print(temperature);
      Serial.println("°C");
    }

    if (!isnan(temperature) && !isnan(lux)) {       // Memastikan bacaan suhu dan lux valid (bukan NaN) sebelum mengeksekusi logika alarm
      if (temperature > 30.0f || lux < 50.0f) {     // Mengecek kondisi alarm: apakah suhu lebih dari 30 °C ATAU lux kurang dari 50 lx
        Serial.println("[PERINGATAN ALARM]"); // Mencetak pesan peringatan utama jika salah satu kondisi terpenuhi
        if (temperature > 30.0f) {                  // Mengecek spesifik apakah pemicu alarm berasal dari suhu tinggi
          Serial.println(" -> P (> 30 °C)");   // Mencetak detail penyebab peringatan suhu tinggi
        }                                           // Menutup blok pengecekan kondisi suhu
        if (lux < 50.0f) {                          // Mengecek spesifik apakah pemicu alarm berasal dari intensitas cahaya rendah
          Serial.println(" -> L (< 50 lx)");  // Mencetak detail penyebab peringatan cahaya redup
        }                                           // Menutup blok pengecekan kondisi lux
      }                                             // Menutup blok eksekusi peringatan alarm
    }                                               // Menutup blok validasi data sensor

    Serial.println("----------------------------------------");
  }
}