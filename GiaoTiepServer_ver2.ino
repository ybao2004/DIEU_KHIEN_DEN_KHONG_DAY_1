#include <ESP8266WiFi.h>

String ssid = "ESP32_AP";
String password = "00000000";
String host = "192.168.4.1";
int port = 100;

WiFiClient client;
String command = "";
String name_device = "ESP32-Server";
bool wifi_connect = false;
bool client_connect = false;
const long wifi_reconnect_timeout = 15000;
unsigned long long wifi_reconnect_time = 0;
bool config_mode = true;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("\n\nSTARTING...");
  delay(200);
  Serial.println("RUNNING!");
  Serial.println("\nChế độ cấu hình - Chờ lệnh từ GUI...");
  Serial.println("Lệnh có sẵn:");
  Serial.println("- SCAN_WIFI: Quét mạng WiFi");
  Serial.println("- SET_WIFI:<ssid>:<password>: Cài đặt WiFi");
  Serial.println("- SET_SERVER:<ip>:<port>: Cài đặt server");
  Serial.println("- CONNECT: Kết nối WiFi và server");
  Serial.println("- STATUS: Kiểm tra trạng thái");
}

void loop() {
  if (Serial.available() > 0) {
    command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
    command = "";
  }

  if (!config_mode) {
    handleWiFiConnection();
    handleServerConnection();
  }

  delay(10);
}

void processCommand(String cmd) {
  if (cmd == "SCAN_WIFI") {
    scanWiFi();
  } else if (cmd.startsWith("SET_WIFI:")) {
    setWiFi(cmd);
  } else if (cmd.startsWith("SET_SERVER:")) {
    setServer(cmd);
  } else if (cmd == "CONNECT") {
    startConnection();
  } else if (cmd == "STATUS") {
    printStatus();
  } else { //if (cmd == "on" || cmd == "off" || cmd == "reset" || cmd == "random" || cmd == "infor" || cmd.startsWith("f") ) {
    if (client.connected()) {
      client.print(cmd);
      //Serial.println("GUI_SENT: " + cmd);
    } else {
      Serial.println("GUI_ERROR: Không kết nối với server");
    }
  } /*else {
    Serial.println("GUI_ERROR: Lệnh không hợp lệ");
  }*/
}

void scanWiFi() {
  Serial.println("GUI_SCAN_START");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Thực hiện quét WiFi và chờ hoàn tất
  int n = WiFi.scanNetworks(false, true); // Bật chế độ quét ẩn (true)
  delay(500); // Đợi thêm để đảm bảo quét hoàn tất

  if (n == 0) {
    Serial.println("GUI_SCAN_NONE");
  } else {
    for (int i = 0; i < n; ++i) {
      String security = (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "Open" : "Secured";
      Serial.println("GUI_WIFI:" + WiFi.SSID(i) + ":" + String(WiFi.RSSI(i)) + ":" + security);
      delay(20); // Đợi nhỏ để gửi dữ liệu Serial ổn định
    }
  }
  Serial.println("GUI_SCAN_END");
  WiFi.scanDelete(); // Xóa bộ nhớ quét để tránh tràn bộ nhớ
}

void setWiFi(String cmd) {
  int first_colon = cmd.indexOf(':', 9);
  int second_colon = cmd.indexOf(':', first_colon + 1);
  
  if (first_colon > 0 && second_colon > 0) {
    ssid = cmd.substring(9, first_colon);
    password = cmd.substring(first_colon + 1, second_colon);
    Serial.println("GUI_WIFI_SET: " + ssid);
  } else {
    Serial.println("GUI_ERROR: Định dạng SET_WIFI không đúng");
  }
}

void setServer(String cmd) {
  int first_colon = cmd.indexOf(':', 11);
  int second_colon = cmd.indexOf(':', first_colon + 1);
  
  if (first_colon > 0 && second_colon > 0) {
    host = cmd.substring(11, first_colon);
    port = cmd.substring(first_colon + 1, second_colon).toInt();
    Serial.println("GUI_SERVER_SET: " + host + ":" + String(port));
  } else {
    Serial.println("GUI_ERROR: Định dạng SET_SERVER không đúng");
  }
}

void startConnection() {
  config_mode = false;
  wifi_connect = false;
  client_connect = false;
  Serial.println("GUI_CONNECTING");
  Serial.print("Kết nối đến mạng WiFi: " + ssid + "   ");
  wifi_reconnect_time = millis();
  WiFi.begin(ssid.c_str(), password.c_str());
  delay(100);
}

void printStatus() {
  Serial.println("GUI_STATUS_START");
  Serial.println("GUI_STATUS_WIFI: " + ssid);
  Serial.println("GUI_STATUS_SERVER: " + host + ":" + String(port));
  Serial.println("GUI_STATUS_WIFI_CONNECTED: " + String(wifi_connect));
  Serial.println("GUI_STATUS_SERVER_CONNECTED: " + String(client_connect));
  Serial.println("GUI_STATUS_CONFIG_MODE: " + String(config_mode));
  Serial.println("GUI_STATUS_END");
}

void handleWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    if (wifi_connect == true) {
      Serial.println("GUI_WIFI_DISCONNECTED");
      WiFi.begin(ssid.c_str(), password.c_str());
      wifi_connect = false;
      wifi_reconnect_time = millis();
    }
    if (millis() - wifi_reconnect_time > wifi_reconnect_timeout) {
      wifi_reconnect_time = millis();
      Serial.println("GUI_WIFI_TIMEOUT");
      delay(3000);
      WiFi.begin(ssid.c_str(), password.c_str());
    } else {
      delay(300);
      //Serial.print(".");
    }
    return;
  } else if (!wifi_connect) {
    Serial.println("GUI_WIFI_CONNECTED: " + ssid);
    wifi_connect = true;
    Serial.println("GUI_SERVER_CONNECTING: " + host + ":" + String(port));
    client.connect(host.c_str(), port);
  }
}

void handleServerConnection() {
  if (!client.connected()) {
    if (client_connect) {
      Serial.println("GUI_SERVER_DISCONNECTED");
      client_connect = false;
    }
    delay(1000);
    if (client.connect(host.c_str(), port)) {
      Serial.println("GUI_SERVER_CONNECTED: " + host + ":" + String(port));
      client_connect = true;
    }
  } else if (!client_connect) {
    Serial.println("GUI_SERVER_CONNECTED: " + host + ":" + String(port));
    client_connect = true;
  }

  if (client.available()) {
    String response = client.readStringUntil('\n');
    //Serial.println("GUI_SERVER_RESPONSE: " + response);
    Serial.println(response);
  }
}