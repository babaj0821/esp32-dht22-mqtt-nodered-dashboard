// ESP32 — DHT22 Temperature & Humidity Sensor + MQTT Publish
// Wiring: DATA → GPIO4 | VCC → 3V3 | GND → GND

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHT_PIN  4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

const char* mqtt_server = "192.168.x.x";   
const int mqtt_port = 1884;
const char* mqtt_topic = "data/sensor/test";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSendTime = 0;
const unsigned long interval = 5000; 

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT... ");

    String clientId = "ESP32-DHT22-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected.");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 2 seconds...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  delay(2000);

  Serial.println("ESP32 DHT22 MQTT ready!");

  connectToWiFi();

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    connectToMQTT();
  }

  client.loop();

  unsigned long now = millis();

  if (now - lastSendTime >= interval) {
    lastSendTime = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("DHT22: waiting for sensor...");
      return;
    }

    String payload = String(t, 1) + "," + String(h, 1);

    Serial.print("Publishing to MQTT: ");
    Serial.println(payload);

    client.publish(mqtt_topic, payload.c_str());
  }
}
