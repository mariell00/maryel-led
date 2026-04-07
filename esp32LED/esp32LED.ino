#include <WiFi.h>
#include <WiFiClientSecure.h>   // ✅ ADD THIS
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID     = "Salalima wifi 4G";
const char* WIFI_PASSWORD = "salalima424";

const char* SUPABASE_URL = "https://oqeucwpbovszthusqddx.supabase.co";
const char* SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im9xZXVjd3Bib3ZzenRodXNxZGR4Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzQ5Mjk5NjIsImV4cCI6MjA5MDUwNTk2Mn0.MxF9CKcFq-FD-DBAP8AMkg62d8f1G7rl39gzOOvC2qU";

#define LED_PIN        2
#define POLL_INTERVAL  3000

bool lastLedStatus = false;
unsigned long lastPollTime = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);
  Serial.println("ESP32 Supabase LED Controller");
  connectToWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Disconnected! Reconnecting...");
    connectToWiFi();
    return;
  }
  unsigned long now = millis();
  if (now - lastPollTime >= POLL_INTERVAL) {
    lastPollTime = now;
    fetchLedStatus();
  }
}

void connectToWiFi() {
  Serial.print("[WiFi] Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  delay(500);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WiFi] Failed. Will retry...");
  }
}

void fetchLedStatus() {
  WiFiClientSecure client;
  client.setInsecure(); // 🔥 FIX HTTPS

  HTTPClient http;

  String endpoint = String(SUPABASE_URL) +
    "/rest/v1/device_control?id=eq.1&select=led_status";

  http.begin(client, endpoint);

  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
  http.addHeader("Content-Type", "application/json");

  int code = http.GET();

  if (code > 0) {
    String payload = http.getString();
    Serial.println(payload);

    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, payload);

    if (!err) {
      bool status = doc[0]["led_status"] | false;

      digitalWrite(LED_PIN, status ? HIGH : LOW);
      Serial.println(status ? "[LED] ON" : "[LED] OFF");
    } else {
      Serial.println("[JSON] Error");
    }

  } else {
    Serial.print("[HTTP ERROR]: ");
    Serial.println(code);
  }

  http.end();
}