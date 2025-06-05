#include <ESP8266WiFi.h>

const char* ssid = "ESP32_AP";
const char* password = "00000000";
const char* host = "192.168.4.1";
const int port = 100;

WiFiClient client;

String command = "";
String name_device = "ESP32-Server";
bool wifi_connect = false;
bool client_connect = false;
const long wifi_reconnect_timeout = 15000; // Timeout for WiFi reconnection
unsigned long long wifi_reconnect_time = 0;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("\n\nSTARTING... ");
  delay(200);
  Serial.print("\nRUNING! ");
  Serial.print("\n\nKết nối đến mạng WiFi: " + String(ssid) + "   ");
  wifi_reconnect_time = millis();
  WiFi.begin(ssid, password);
  delay(100);
}

void loop() {
  // Check and reconnect WiFi if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    if (wifi_connect == true) {
      Serial.println("\nMất kết nối với mạng Wifi: "+ String(ssid));
      Serial.print("Đang thử kết nối lại   ");
      WiFi.begin(ssid, password); // Reconnect WiFi
      wifi_connect = false;
      wifi_reconnect_time = millis();
    }
    if (millis() - wifi_reconnect_time > wifi_reconnect_timeout) {
      wifi_reconnect_time = millis();
      Serial.print("\nmất quá nhiều thời gian để kết nối, thử kết nối lại sau 3 giây !\n\n");
      delay(3000);
      Serial.print("Đang thử kết nối lại với mạng WiFi: " + String(ssid) + "   ");
      WiFi.begin(String(ssid), password); // Reconnect WiFi
    } else {
      delay(300);
      Serial.print(".");
    }
    return; // Exit loop to prioritize WiFi reconnection
  } else if (!wifi_connect) { // Print connection message only once
    Serial.print("\nĐã kết nối với mạng Wifi: ");
    Serial.println(String(ssid));
    wifi_connect = true; 
    Serial.print("\nĐang kết nối với " + name_device + " tại: " + String(host) + ":" + String(port) + " ...");
    client.connect(host, port);
  }

  if (!client.connected()) {
    if (client_connect) {
      Serial.print("\nMất kết nối với " + name_device);
      Serial.print("\nĐang kết nối lại với " + name_device + " tại: " + String(host) + ":" + String(port) + " ...");
      client.connect(host, port);
      client_connect = false;
    }

    if (!client_connect) {
      Serial.println("\nKết nối với " + name_device + " thất bại!");
      delay(1000);
      Serial.print("\nĐang kết nối với " + name_device + " tại: " + String(host) + ":" + String(port) + " ...");
      client.connect(host, port);
    }
    else client_connect = true;
  }
  else if(!client_connect) {
    Serial.println("\nĐã kết nối với " + name_device + " -> " + String(host) + ":" + String(port));
    client_connect = true;
  }

  // Gửi lệnh từ ESP8266
  if (Serial.available() > 0) {
    command = Serial.readStringUntil('\n');
    client.write(command.c_str()); // Gửi lệnh với ký tự xuống dòng
    Serial.print("\nĐã gửi lệnh: " + command);
    command = "";
  }

  // Nhận phản hồi từ ESP32
  if (client.available()) {
    String response = client.readStringUntil('\n');
    Serial.print("\nESP32_server: " + response);
  }

  delay(10);
}