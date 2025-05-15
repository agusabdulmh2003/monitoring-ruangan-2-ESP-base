#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


#define API_KEY "AIzaSyCk06v8QxT12eBR2Wkucyc0lC0uO-IOIpQ"
#define DATABASE_URL "https://iotmonitoring-a99ff-default-rtdb.asia-southeast1.firebasedatabase.app"
#define BUZZER_PIN D6
#define LED1_PIN D1  
#define LED2_PIN D2   

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
unsigned long sendDataPrevMillis = 0;

const int ambangCahaya = 2000;
const float ambangSuhu = 30.0;


void aturLED(int pin, bool nyala) {
  digitalWrite(pin, nyala ? HIGH : LOW);
}

void kedipkanLED(int pin, int jumlahKedip, int delayMs) {
  for (int i = 0; i < jumlahKedip; i++) {
    digitalWrite(pin, HIGH);
    delay(delayMs);
    digitalWrite(pin, LOW);
    delay(delayMs);
  }
}

void nyalakanBuzzer(bool led1Nyala, bool led2Nyala) {
  for (int i = 0; i < 10; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    if (led1Nyala) digitalWrite(LED1_PIN, HIGH);
    if (led2Nyala) digitalWrite(LED2_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    if (led1Nyala) digitalWrite(LED1_PIN, LOW);
    if (led2Nyala) digitalWrite(LED2_PIN, LOW);
    delay(200);
  }
}


bool periksaRuangan(const String& ruangan) {
  int gerakan = 0, cahaya = 0;
  float suhu = 0.0;

  String base = "/" + ruangan + "/";

  if (Firebase.RTDB.getFloat(&fbdo, base + "suhu"))
    suhu = fbdo.floatData();
  else
    Serial.println("Gagal baca suhu: " + fbdo.errorReason());

  if (Firebase.RTDB.getInt(&fbdo, base + "gerakan"))
    gerakan = fbdo.intData();
  else
    Serial.println("Gagal baca gerakan: " + fbdo.errorReason());

  if (Firebase.RTDB.getInt(&fbdo, base + "cahaya"))
    cahaya = fbdo.intData();
  else
    Serial.println("Gagal baca cahaya: " + fbdo.errorReason());

  Serial.printf("%s | Suhu: %.2f | Gerakan: %d | Cahaya: %d\n", ruangan.c_str(), suhu, gerakan, cahaya);

  bool mencurigakan = (gerakan == 0) &&
    ((suhu < ambangSuhu && cahaya < ambangCahaya) ||
     (suhu < ambangSuhu && cahaya > ambangCahaya) ||
     (suhu > ambangSuhu && cahaya < ambangCahaya));

  if (mencurigakan) {
    Serial.println("⚠️ Kondisi mencurigakan di " + ruangan);
  } else {
    Serial.println("✅ Aman di " + ruangan);
  }

  Serial.println("-------------------");
  return mencurigakan;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  WiFiManager wm;
  wm.autoConnect("ESP_Config");

  Serial.println("WiFi Tersambung. IP: " + WiFi.localIP().toString());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  if (Firebase.signUp(&config, &auth, "", "")) {
    signupOK = true;
    Serial.println("Firebase signup berhasil");
  } else {
    Serial.printf("Signup gagal: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    bool bahayaRuangan1 = periksaRuangan("Ruangan1");
    bool bahayaRuangan2 = periksaRuangan("Ruangan2");

    if (bahayaRuangan1 || bahayaRuangan2) {
      Serial.println("🚨 Kondisi mencurigakan!");
      nyalakanBuzzer(bahayaRuangan1, bahayaRuangan2);
    } else {
      aturLED(LED1_PIN, false);
      aturLED(LED2_PIN, false);
      Serial.println("✅ Ruangan aman.\n");
    }
  }
}
