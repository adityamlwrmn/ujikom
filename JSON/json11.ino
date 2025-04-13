#include "secrets.h"\
#include <DHT.h>
#include <DHT_U.h>
#include <Firebase.h>
#include <ArduinoJson.h>

/* Use the following instance for Test Mode (No Authentication) */
Firebase fb(REFERENCE_URL);

#define ledPin1     D1
#define ledPin2     D2
#define ledPin3     D3
#define triggerPin  D7
#define echoPin     D6
#define DHTPIN D5  
#define DHTTYPE DHT11

long duration, jarak;
int led1,led2,led3;
float t,h;
DHT dht(DHTPIN, DHTTYPE);

 
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

  /* Connect to WiFi */
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("-");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println();

  #if defined(ARDUINO_UNOWIFIR4)
    digitalWrite(LED_BUILTIN, HIGH);
  #endif

  /* ----- */ 

  /* ----- Serialization: Set example data in Firebase ----- */

  // Create a JSON document to hold the output data
  //JsonDocument docOutput;
  StaticJsonDocument<200> docOutput;

  // Add various data types to the JSON document
  docOutput["led1"] = "0"; //key1
  docOutput["led2"] = "0"; //key2
  docOutput["led3"] = "0"; //key3


  // Create a string to hold the serialized JSON data
  String output;

  // Optional: Shrink the JSON document to fit its contents exactly
  //docOutput.shrinkToFit();

  // Serialize the JSON document to a string
  serializeJson(docOutput, output);

  // Set the serialized JSON data in Firebase
  fb.setJson("dataLED", output); //mengirim ke firebase

  /* ----- Deserialization: Retrieve example data from Firebase ----- */

  // Retrieve the serialized JSON data from Firebase
  String input = fb.getJson("dataLED"); //mengambil dari firebase bedasarkan topik

  // Check if the retrieved data is "NULL", indicating a retrieval error
  if (input == "NULL") {
    Serial.println("Could not retrieve data from Firebase");
  } else {
    // Create a JSON document to hold the deserialized data
    //JsonDocument docInput;
    StaticJsonDocument<200> docInput;

    // Deserialize the JSON string into the JSON document
    DeserializationError error = deserializeJson(docInput, input);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Extract the values from the deserialized JSON document (mengambil data dari firebase)
    led1 = docInput["led1"]; //0 atau 1
    led2 = docInput["led2"]; //0 atau 1
    led3 = docInput["led3"]; //0 atau 1
    /* Print the deserialized input */
    Serial.print("Retrieved String LED1:\t");
    Serial.println(led1);
    Serial.print("Retrieved String LED2:\t");
    Serial.println(led2);
    Serial.print("Retrieved String LED3:\t");
    Serial.println(led3);
  }

  // Remove the example data from Firebase
  //fb.remove("Example");
}

void loop() {
  // Nothing
  datajarak();
  StaticJsonDocument<200> docOutput;
  docOutput["jarak"] = jarak;
  String output;
  serializeJson(docOutput, output);
  fb.setJson("Sensor", output);

  dataDHT();
  StaticJsonDocument<200> docSuhu;
  docSuhu["t"] = String (t, 2);
  docSuhu["h"] = String(h, 2);
  String suhu;
  serializeJson(docSuhu, suhu);
  fb.setJson("dataDHT", suhu);

  //menerima dari firebase
  String input = fb.getJson("dataLED");

  // Check if the retrieved data is "NULL", indicating a retrieval error
  if (input == "NULL") {
    Serial.println("Could not retrieve data from Firebase");
  } else {
    // Create a JSON document to hold the deserialized data
    //JsonDocument docInput;
    StaticJsonDocument<200> docInput;

    // Deserialize the JSON string into the JSON document
    DeserializationError error = deserializeJson(docInput, input);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    led1 = docInput["led1"]; //0 atau 1
    led2 = docInput["led2"]; //0 atau 1
    led3 = docInput["led3"]; //0 atau 1

    //kendali led ke 1
    if(led1 == 1)digitalWrite(ledPin1, HIGH);
    else digitalWrite(ledPin1, LOW);
    //kendali led ke 2
    if(led2 == 1)digitalWrite(ledPin2, HIGH);
    else digitalWrite(ledPin2, LOW);
    //kendali led ke 3
    if(led3 == 1)digitalWrite(ledPin3, HIGH);
    else digitalWrite(ledPin3, LOW);
  }

    Serial.print("Retrieved String LED1:\t");
    Serial.println(led1);
    Serial.print("Retrieved String LED2:\t");
    Serial.println(led2);
    Serial.print("Retrieved String LED3:\t");
    Serial.println(led3);

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print("\n");
    Serial.print(F("  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));

    Serial.print("Jarak:\t");
    Serial.println(jarak);

}

void datajarak(){
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2); 
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  jarak = (duration/2) / 29.1;
}

void dataDHT(){
  h = dht.readHumidity();
  t = dht.readTemperature();
}