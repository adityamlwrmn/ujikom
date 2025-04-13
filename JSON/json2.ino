#include "secrets.h"
#include <DHT.h>
#include <DHT_U.h>
#include <Firebase.h>
#include <ArduinoJson.h>

#define ledPin1     D1
#define ledPin2     D2
#define ledPin3     D3
#define triggerPin  D7
#define echoPin     D6
#define DHTPIN      D5
#define DHTTYPE     DHT11

long duration, jarak;
int led1, led2, led3;
float t, h;
DHT dht(DHTPIN, DHTTYPE);

Firebase fb(REFERENCE_URL); // Firebase reference dari secrets.h

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);

#if !defined(ARDUINO_UNOWIFIR4)
  WiFi.mode(WIFI_STA);
#else
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  WiFi.disconnect();
  delay(1000);

  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Connected");

#if defined(ARDUINO_UNOWIFIR4)
  digitalWrite(LED_BUILTIN, HIGH);
#endif

  // Inisialisasi nilai LED di Firebase ke 0 (matikan semua LED saat booting)
  StaticJsonDocument<200> docInit;
  docInit["led1"] = 0;
  docInit["led2"] = 0;
  docInit["led3"] = 0;

  String outputInit;
  serializeJson(docInit, outputInit);
  fb.setJson("dataLED", outputInit);

  // Ambil status LED awal dari Firebase
  updateLEDFromFirebase();
}

void loop() {
  dataJarak();
  updateJarakToFirebase();

  dataDHT();
  updateDHTToFirebase();

  updateLEDFromFirebase(); // Ambil dan atur status LED dari Firebase

  // Print ke serial monitor
  Serial.print("LED1: "); Serial.print(led1);
  Serial.print(" | LED2: "); Serial.print(led2);
  Serial.print(" | LED3: "); Serial.println(led3);

  Serial.print("Suhu: "); Serial.print(t); Serial.print("°C");
  Serial.print(" | Kelembaban: "); Serial.print(h); Serial.println("%");

  Serial.print("Jarak: "); Serial.print(jarak); Serial.println(" cm");
  Serial.println("--------");

  delay(1000); // Delay 1 detik antar pengambilan
}

// ------------------------ FUNGSI TAMBAHAN ------------------------

void dataJarak() {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  jarak = (duration / 2) / 29.1; // Konversi ke cm
}

void dataDHT() {
  h = dht.readHumidity();
  t = dht.readTemperature();
}

void updateJarakToFirebase() {
  StaticJsonDocument<200> docJarak;
  docJarak["jarak"] = jarak;

  String outputJarak;
  serializeJson(docJarak, outputJarak);
  fb.setJson("Sensor", outputJarak);
}

void updateDHTToFirebase() {
  StaticJsonDocument<200> docSuhu;
  docSuhu["t"] = t;  // SUHU sebagai angka (bukan string)
  docSuhu["h"] = h;

  String outputSuhu;
  serializeJson(docSuhu, outputSuhu);
  fb.setJson("dataDHT", outputSuhu);
}

void updateLEDFromFirebase() {
  String input = fb.getJson("dataLED");

  if (input == "NULL") {
    Serial.println("Gagal mengambil data LED dari Firebase");
    return;
  }

  StaticJsonDocument<200> docInput;
  DeserializationError error = deserializeJson(docInput, input);

  if (error) {
    Serial.print("deserializeJson() gagal: ");
    Serial.println(error.c_str());
    return;
  }

  led1 = docInput["led1"];
  led2 = docInput["led2"];
  led3 = docInput["led3"];

  digitalWrite(ledPin1, led1 ? HIGH : LOW);
  digitalWrite(ledPin2, led2 ? HIGH : LOW);
  digitalWrite(ledPin3, led3 ? HIGH : LOW);
}
