#include <WiFi.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyCk06v8QxT12eBR2Wkucyc0lC0uO-IOIpQ"
#define DATABASE_URL "https://iotmonitoring-a99ff-default-rtdb.asia-southeast1.firebasedatabase.app"
#define DHTPIN 14
#define DHTTYPE DHT11
#define PIR_PIN 13
#define LDR_PIN 34

DHT dht(DHTPIN, DHTTYPE);

// ====== Firebase Setup ======
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
const unsigned long interval = 15000;
bool signupOK = false;

// ====== Fungsi Kirim Data ke Firebase ======
void kirimFirebase(const String& path, float value) {
  if (Firebase.RTDB.setFloat(&fbdo, path, value)) {
    Serial.println(path + " terkirim");
  } else {
    Serial.println("Gagal kirim " + path + ": " + fbdo.errorReason());
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  dht.begin();


  WiFiManager wm;
  // wm.resetSettings(); // Uncomment kalau mau selalu muncul portal konfigurasi WiFi
  if (!wm.autoConnect("IOT-Setup", "12345678")) {
    Serial.println("Gagal connect ke WiFi");
    while (true);
  }

  Serial.println("WiFi Terhubung: " + WiFi.localIP().toString());

  // ====== Setup Firebase ======
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  if (Firebase.signUp(&config, &auth, "", "")) {
    signupOK = true;
    Serial.println("Firebase signup sukses");
  } else {
    Serial.printf("Signup gagal: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > interval)) {
    sendDataPrevMillis = millis();
    int gerakan1 = digitalRead(PIR_PIN);
    int cahaya1 = analogRead(LDR_PIN);
    float suhu1 = dht.readTemperature();
    int gerakan2 = 0;
    int cahaya2 = 2500;
    float suhu2 = 28.8;

   
    Serial.printf("Ruangan1 => Gerakan: %d | Cahaya: %d | Suhu: %.2f °C\n", gerakan1, cahaya1, suhu1);
    kirimFirebase("Ruangan1/suhu", suhu1);
    kirimFirebase("Ruangan1/gerakan", gerakan1);
    kirimFirebase("Ruangan1/cahaya", cahaya1);

    Serial.printf("Ruangan2 => Gerakan: %d | Cahaya: %d | Suhu: %.2f °C\n", gerakan2, cahaya2, suhu2);
    kirimFirebase("Ruangan2/suhu", suhu2);
    kirimFirebase("Ruangan2/gerakan", gerakan2);
    kirimFirebase("Ruangan2/cahaya", cahaya2);

    Serial.println("---");
  }
}
