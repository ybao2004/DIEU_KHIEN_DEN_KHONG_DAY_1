#include <Arduino.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>

// Thông tin WiFi
const char* ssid = "ESP32_AP";       // Tên WiFi của ESP32
const char* password = "00000000";   // Mật khẩu WiFi
IPAddress ip;
const byte maxConnections = 3;
WiFiServer server(100); // Tạo server TCP trên cổng 100
WiFiClient clients[maxConnections];

byte total_client = 0;
byte connectedClients = 0;
const int task_delay = 10;
int new_connect = 0;
byte new_connect_max = 10;

// Danh sách các client
TaskHandle_t ServerHandle = NULL;
TaskHandle_t Led1Handle   = NULL;
TaskHandle_t Led2Handle   = NULL;
TaskHandle_t Led3Handle   = NULL;
TaskHandle_t Led4Handle   = NULL;
TaskHandle_t Led5Handle   = NULL;

// Cấu hình thông số PWM
const int pwmFrequency = 2000; // Tần số PWM
const byte pwmResolution = 10;  // Độ phân giải 10-bit (0-1023)
const int maxbit = 1023;
const long LED_TIME = 1000000;
// Khai báo các chân LED
const byte reset_pin = 23;
const byte LED_PIN_1 = 32;
const byte LED_PIN_2 = 33;
const byte LED_PIN_3 = 27;
const byte LED_PIN_4 = 14;
const byte LED_PIN_5 = 12;
// Kênh PWM tương ứng với từng chân
const byte pwmChannel_1 = 0;
const byte pwmChannel_2 = 1;
const byte pwmChannel_3 = 2;
const byte pwmChannel_4 = 3;
const byte pwmChannel_5 = 4;
// Biến lưu giá trị PWM cho từng LED
int pwmValue_1 = 0;
int pwmValue_2 = 0;
int pwmValue_3 = 0;
int pwmValue_4 = 0;
int pwmValue_5 = 0;

// Các biến lưu trữ điều khiển
String input = "";
String value_1 = "";
String value_2 = "";
String value_3 = "";
String value_4 = "";
String value_5 = "";
String value_6 = "";

String mode   = "";
String mode_1 = "";
String mode_2 = "";
String mode_3 = "";
String mode_4 = "";
String mode_5 = "";

String function   = "";
String function_1 = "";
String function_2 = "";
String function_3 = "";
String function_4 = "";
String function_5 = "";

int time_min = 0;
int time_min_1 = 0;
int time_min_2 = 0;
int time_min_3 = 0;
int time_min_4 = 0;
int time_min_5 = 0;

int time_max = 0;
int time_max_1 = 0;
int time_max_2 = 0;
int time_max_3 = 0;
int time_max_4 = 0;
int time_max_5 = 0;

int brightness_min = 0;
int brightness_min_1 = 0;
int brightness_min_2 = 0;
int brightness_min_3 = 0;
int brightness_min_4 = 0;
int brightness_min_5 = 0;

int brightness_max = 0;
int brightness_max_1 = 0;
int brightness_max_2 = 0;
int brightness_max_3 = 0;
int brightness_max_4 = 0;
int brightness_max_5 = 0;

int brightness = 0;
int brightness_1 = 0;
int brightness_2 = 0;
int brightness_3 = 0;
int brightness_4 = 0;
int brightness_5 = 0;

long led_time   = 0;
long led_time_1 = 0;
long led_time_2 = 0;
long led_time_3 = 0;
long led_time_4 = 0;
long led_time_5 = 0;

bool done   = false;
bool done_1 = false;
bool done_2 = false;
bool done_3 = false;
bool done_4 = false;
bool done_5 = false;

int run_1 = true;
int run_2 = true;
int run_3 = true;
int run_4 = true;
int run_5 = true;

int delay_A_1 = 0;
int delay_A_2 = 0;
int delay_A_3 = 0;
int delay_A_4 = 0;
int delay_A_5 = 0;

int delay_B_1 = 0;
int delay_B_2 = 0;
int delay_B_3 = 0;
int delay_B_4 = 0;
int delay_B_5 = 0;

int led_time_function = 0;

//__________________________________________________________//
void infor(){
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  float percent = (float)freeHeap / totalHeap * 100;
  feedback("\n\n(!) Có: ", String(connectedClients), " thiết bị đang kết nối. Đếm: ", String(new_connect));
  feedback("\n(!) CPU: ", String(ESP.getCpuFreqMHz()), "(MHz)");
  feedback("\n(i) Ram tổng: ", String(totalHeap / 1024), ",", String(totalHeap % 1024), "(Kb), trống: ", String(freeHeap / 1024), ",", String(freeHeap % 1024), "(Kb)");
  feedback("\t", String(percent), "%");
  unsigned long long times = millis();
  int gio  = times / 3600000;
  int phut = times % 3600000 / 60000;
  int giay = times % 3600000 % 60000 / 1000;
  int ms   = times % 3600000 % 60000 % 1000;
  feedback("\n(i) Đã chạy: ", String(gio), " giờ, ", String(phut), " phút, ", String(giay), " giây, ", String(ms), "ms.\n"); }
int brightness_IN(String str){
  int i = str.toInt();
  if(0 <= i && i <= maxbit) return i;
  else return maxbit;
 }
int led_time_IN(String str){
  long i = str.toInt();
  if(0 <= i && i <= LED_TIME) return i;
  else return LED_TIME;
 }
void showIP(IPAddress ip){
  Serial.print(ip);
  for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write(ip);
 }
void feedback(String s1){
  String Message = s1;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2){
  String Message = s1 + s2;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2, String s3){
  String Message = s1 + s2 + s3;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2, String s3, String s4){
  String Message = s1 + s2 + s3 + s4;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2, String s3, String s4, String s5){
  String Message = s1 + s2 + s3 + s4 + s5;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2, String s3, String s4, String s5, String s6){
  String Message = s1 + s2 + s3 + s4 + s5 + s6;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2, String s3, String s4, String s5, String s6, String s7){
  String Message = s1 + s2 + s3 + s4 + s5 + s6 + s7;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2, String s3, String s4, String s5, String s6, String s7, String s8){
  String Message = s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2, String s3, String s4, String s5, String s6, String s7, String s8, String s9){
  String Message = s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }
void feedback(String s1, String s2, String s3, String s4, String s5, String s6, String s7, String s8, String s9, String s10){
  String Message = s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10;
  if (Message != "" && Message != " " && Message != "\n"){
    Serial.print(Message);
    for (int i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].write((Message).c_str());
  }
  Message = "";
 }

void Default(){
  time_min = 700;
  time_max = 2500;
  brightness_min = 40;
  brightness_max = maxbit;
  led_time = 2000;
  run_1 = run_2 =  run_3 = run_4 = run_5 = true;
  done_1 = done_2 = done_3 = done_4 = done_5 = true;
  mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "control";
  input = "on.7000";
  led_time_function = 1000;
  }
void reset(){
  feedback("\n\n\t\t(!) ESP32 Server sẽ được khởi dộng lại !\n\t\t(*)Đang tắt...\n\n\n");
  brightness_1  = brightness_2  = brightness_3  = brightness_4  = brightness_5  = 0;
  led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = 1000;
  mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "control";
  done_1 = done_2 = done_3 = done_4 = done_5 = true;
  delay(1500);
  feedback("\n\t\t(->) Đã tắt !\n\n\n");
  digitalWrite(reset_pin, HIGH);
  delay(50);
  digitalWrite(reset_pin, LOW); }
void resetWifi() {
  feedback("\n\n\n(!) Số lần client kết nối với ESP32 Server quá nhiều !\nKhởi động lại mạng để đảo bảo an toàn !");
  delay(20);
  // Ngắt tất cả các client đã kết nối// Reset các biến
  new_connect = 0;
  total_client = 0;
  connectedClients = 0;
  for (byte i = 0; i < maxConnections; i++) if (clients[i].connected()) clients[i].stop();
  // Ngắt kết nối Access Point
  WiFi.softAPdisconnect();  // Ngắt AP hiện tại
  delay(1000);  // Đợi một chút trước khi bật lại AP
  WiFi.softAP(ssid, password);
  server.begin();
  feedback("\n\nkhởi động mạng WiFi ...\nSSID: ", String(ssid), "\nPassword: ", String(password), "\nServer đang chạy tại: ");
  showIP(WiFi.softAPIP());
 }

void Control(void *pvParameters) {

  while(1){
    if(input == "") {
      // Client mới
      WiFiClient newClient = server.available();
      if (newClient) {
        for (int i = 0; i < maxConnections; ++i) {
          if (!clients[i]) {
            clients[i] = newClient;
            for (int x = 0; x < maxConnections; x++) if (clients[x].connected()) total_client ++;
            new_connect ++;
            connectedClients = total_client;
            feedback("\nclient mới: [", String(i), "], tổng: ", String(total_client), " thiết bị đang kết nối. Đếm: ", String(new_connect));
            total_client = 0;
            break;
          }
        }
        if (new_connect > new_connect_max) resetWifi();
      }

      if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n'); // Đọc lệnh từ Serial
        if (command != "" && command != " " && command != "\n"){
          feedback("\nSERIAL: ", command);
          input = command;
        }
      }

      // Kiểm tra ngắt kết nối và xử lý dữ liệu
      for (int i = 0; i < maxConnections; i++) {
        if (clients[i]) {
          if (!clients[i].connected()) {
            clients[i].stop();
            clients[i] = WiFiClient();
            connectedClients--;
            feedback("\nCó 1 clinet vừa ngắt kết nối [", String(i), "]\nBiến đếm số lượng clinet hiện tại: ", String(connectedClients)); }
          else if (clients[i].available() > 0) { // Có dữ liệu từ client, xử lý tại đây
            // Đọc và xử lý dữ liệu từ client i 
            String request = clients[i].readStringUntil('\r');
            if (request != "" && request != " " && request != "\n"){
              feedback("\nclient [", String(i) ,"]: ", request);
              input = request;
            }
          }
        }
      }
      delay(task_delay); 
    }
    else{
      feedback("\n(!) Chờ xử lý lệnh và thực thi lệnh");
      unsigned long as = millis();
      int point = 0;
      int Do_dai_chuoi = input.length();
      for (int i = 0; i < Do_dai_chuoi; i++) {
        if(input[i] == '.' || input[i] == ',' || input[i] == '(' || input[i] == ')' ) point ++ ;
        else {
          if(point == 0) value_1 += input[i];
          if(point == 1) value_2 += input[i];
          if(point == 2) value_3 += input[i];
          if(point == 3) value_4 += input[i];
          if(point == 4) value_5 += input[i];
          if(point == 5) value_6 += input[i];
        }
      }// het vong lap thi chay lenh nhan duoc
      feedback("\nTổng thời gian xử lý dữ liệu nhận: ", String(millis() - as), "ms", "\nvalue_1 = ", value_1, "\nvalue_2 = ", value_2, "\nvalue_3 = ", value_3);
      feedback("\nvalue_4 = ", value_4, "\nvalue_5 = ", value_5, "\nvalue_6 = ", value_6);

      // bắt đầu điều khiển lệnh

      if (value_1 == "all"){
        if (value_2 == "random") {
          if (value_3 != "") brightness_min = brightness_IN(value_3);
          if (value_4 != "") brightness_max = brightness_IN(value_4);
          if (value_5 != "") time_min       = led_time_IN(value_5);
          if (value_6 != "") time_max       = led_time_IN(value_6);
          mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "random"; }

        else if(value_2 != ""){
          brightness_1 = brightness_2 = brightness_3 = brightness_4 = brightness_5 = led_time_IN(value_2);
          if (value_3 != "") led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = led_time_IN(value_3);
          else led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = led_time;
          mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "control";
          done_1 = done_2 = done_3 = done_4 = done_5 = true; }}

      else if(value_1 == "random"){
        if (value_2 != "") brightness_min = brightness_IN(value_2);
        if (value_3 != "") brightness_max = brightness_IN(value_3);
        if (value_4 != "") time_min       = led_time_IN(value_4);
        if (value_5 != "") time_max       = led_time_IN(value_5);
        mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "random"; }

      else if(value_1 == "on"){
        brightness_1  = brightness_2  = brightness_3  = brightness_4  = brightness_5  = brightness_max;
        if (value_2 != "") led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = led_time_IN(value_2);
        else led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = led_time;
        mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "control";
        done_1 = done_2 = done_3 = done_4 = done_5 = true; }

      else if(value_1 == "off"){
        brightness_1  = brightness_2  = brightness_3  = brightness_4  = brightness_5  = 0;
        if (value_2 != "") led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = led_time_IN(value_2);
        else led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = led_time;
        mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "control";
        done_1 = done_2 = done_3 = done_4 = done_5 = true; }
    
      else if(value_1 == "change"){
        if(value_2 == "only1" || value_1 == "only_1" || value_1 == "only 1" || value_2 == "o1"){
          if (value_3 != "") brightness_min_1 = brightness_IN(value_3);
          if (value_4 != "") brightness_max_1 = brightness_IN(value_4);
          if (value_5 != "") time_min_1       = led_time_IN(value_5);
          if (value_6 != "") time_max_1       = led_time_IN(value_6); }
        else if(value_2 == "only2" || value_1 == "only_2" || value_1 == "only 2" || value_2 == "o2"){
          if (value_3 != "") brightness_min_2 = brightness_IN(value_3);
          if (value_4 != "") brightness_max_2 = brightness_IN(value_4);
          if (value_5 != "") time_min_2       = led_time_IN(value_5);
          if (value_6 != "") time_max_2       = led_time_IN(value_6); }
        else if(value_2 == "only3" || value_2 == "only_3" || value_2 == "only 3" || value_2 == "o3"){
          if (value_3 != "") brightness_min_3 = brightness_IN(value_3);
          if (value_4 != "") brightness_max_3 = brightness_IN(value_4);
          if (value_5 != "") time_min_3       = led_time_IN(value_5);
          if (value_6 != "") time_max_3       = led_time_IN(value_6); }
        else if(value_2 == "only4" || value_2 == "only_4" || value_2 == "only 4" || value_2 == "o4"){
          if (value_3 != "") brightness_min_4 = brightness_IN(value_3);
          if (value_4 != "") brightness_max_4 = brightness_IN(value_4);
          if (value_5 != "") time_min_4       = led_time_IN(value_5);
          if (value_6 != "") time_max_4       = led_time_IN(value_6); }
        else if(value_2 == "only5" || value_2 == "only_5" || value_2 == "only 5" || value_2 == "o5"){
          if (value_3 != "") brightness_min_5 = brightness_IN(value_3);
          if (value_4 != "") brightness_max_5 = brightness_IN(value_4);
          if (value_5 != "") time_min_5       = led_time_IN(value_5);
          if (value_6 != "") time_max_5       = led_time_IN(value_6); }
        else {
          if (value_2 != "") brightness_min = brightness_IN(value_2);
          if (value_3 != "") brightness_max = brightness_IN(value_3);
          if (value_4 != "") time_min       = led_time_IN(value_4);
          if (value_5 != "") time_max       = led_time_IN(value_5); }}

      else if(value_1 == "only1" || value_1 == "only_1" || value_1 == "only 1" || value_1 == "1"){
        if (value_2 == "random"){
          brightness_min_1 = (value_3 == "") ? brightness_min : brightness_IN(value_3);
          brightness_max_1 = (value_4 == "") ? brightness_max : brightness_IN(value_4);
          time_min_1       = (value_5 == "") ? time_min       : led_time_IN(value_5);
          time_max_1       = (value_6 == "") ? time_max       : led_time_IN(value_6);
          mode_1  = "only";
          done_1 = true; }
        else {
          if (value_2 != "") brightness_1 = brightness_IN(value_2);
          if (value_3 != "") led_time_1   = led_time_IN(value_3);
          else led_time_1 = led_time;
          mode_1 = "control";
          done_1 = true; }}
      else if(value_1 == "only2" || value_1 == "only_2" || value_1 == "only 2" || value_1 == "2"){
        if (value_2 == "random"){
          brightness_min_2 = (value_3 == "") ? brightness_min : brightness_IN(value_3);
          brightness_max_2 = (value_4 == "") ? brightness_max : brightness_IN(value_4);
          time_min_2       = (value_5 == "") ? time_min       : led_time_IN(value_5);
          time_max_2       = (value_6 == "") ? time_max       : led_time_IN(value_6);
          mode_2  = "only";
          done_2 = true; }
        else {
          if (value_2 != "") brightness_2 = brightness_IN(value_2);
          if (value_3 != "") led_time_2   = led_time_IN(value_3);
          else led_time_2 = led_time;
          mode_2 = "control";
          done_2 = true; }}
      else if(value_1 == "only3" || value_1 == "only_3" || value_1 == "only 3" || value_1 == "3"){
        if (value_2 == "random"){
          brightness_min_3 = (value_3 == "") ? brightness_min : brightness_IN(value_3);
          brightness_max_3 = (value_4 == "") ? brightness_max : brightness_IN(value_4);
          time_min_3       = (value_5 == "") ? time_min       : led_time_IN(value_5);
          time_max_3       = (value_6 == "") ? time_max       : led_time_IN(value_6);
          mode_3  = "only";
          done_3 = true; }
        else {
          if (value_2 != "") brightness_3 = brightness_IN(value_2);
          if (value_3 != "") led_time_3   = led_time_IN(value_3);
          else led_time_3 = led_time;
          mode_3 = "control";
          done_3 = true; }}
      else if(value_1 == "only4" || value_1 == "only_4" || value_1 == "only 4" || value_1 == "4"){
        if (value_2 == "random"){
          brightness_min_4 = (value_3 == "") ? brightness_min : brightness_IN(value_3);
          brightness_max_4 = (value_4 == "") ? brightness_max : brightness_IN(value_4);
          time_min_4       = (value_5 == "") ? time_min       : led_time_IN(value_5);
          time_max_4       = (value_6 == "") ? time_max       : led_time_IN(value_6);
          mode_4  = "only";
          done_4 = true; }
        else {
          if (value_2 != "") brightness_4 = brightness_IN(value_2);
          if (value_3 != "") led_time_4   = led_time_IN(value_3);
          else led_time_4 = led_time;
          mode_4 = "control";
          done_4 = true; }}
      else if(value_1 == "only5" || value_1 == "only_5" || value_1 == "only 5" || value_1 == "5"){
        if (value_2 == "random"){
          brightness_min_5 = (value_3 == "") ? brightness_min : brightness_IN(value_3);
          brightness_max_5 = (value_4 == "") ? brightness_max : brightness_IN(value_4);
          time_min_5       = (value_5 == "") ? time_min       : led_time_IN(value_5);
          time_max_5       = (value_6 == "") ? time_max       : led_time_IN(value_6);
          mode_5  = "only";
          done_5 = true; }
        else {
          if (value_2 != "") brightness_5 = brightness_IN(value_2);
          if (value_3 != "") led_time_5   = led_time_IN(value_3);
          else led_time_5 = led_time;
          mode_5 = "control";
          done_5 = true; }}
      
      else if(value_1 == "f0" || value_1 == "F0") {
        delay_A_1 = delay_A_2 = delay_A_3 = delay_A_4 = delay_A_5 = 100;
        mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "function 0";
        done_1 = done_2 = done_3 = done_4 = done_5 = true; }
      else if(value_1 == "f1" || value_1 == "F1"){
        while(pwmValue_1 > 0 || pwmValue_2 > 0 || pwmValue_3 > 0 || pwmValue_4 > 0 || pwmValue_5 > 0){
          brightness_1 = brightness_2  = brightness_3 = brightness_4 = brightness_5 = 0;
          led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = 1000;
          mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "control"; }
        
        brightness_1 = brightness_2  = brightness_3 = brightness_4 = brightness_5 = brightness_max;
        led_time_1 = led_time_2 = led_time_3 = led_time_4 = led_time_5 = 2000;
        delay_A_1 = 1*led_time_function;
        delay_A_2 = 2*led_time_function;
        delay_A_3 = 3*led_time_function;
        delay_A_4 = 4*led_time_function;
        delay_A_5 = 5*led_time_function;
        mode_1 = mode_2 = mode_3 = mode_4 = mode_5 = "function 1";
        done_1 = done_2 = done_3 = done_4 = done_4 = true; }
      else if(value_1 == "infor") infor();
      else if(value_1 == "reset") reset();
      else if(value_1 == "default") Default();
      else feedback("\n-> (!) Lệnh: ", String(input), " là không hợp lệ");

      // Xóa chuỗi nhận sau khi xử lý hoàn tất
      point = 0;
      Do_dai_chuoi = 0;
      value_1 = value_2 = value_3 = value_4 = value_5 = value_6 = "";
      feedback("\n-> Hoàn thành lệnh: ", input, "\nTổng thời gian xử lý dữ liệu nhận và điều khiển : ", String(millis() - as), "ms");
      input = "";
      feedback("\n");
      //infor();
    }
    vTaskDelay(pdMS_TO_TICKS(task_delay));
  }
 }

void LED_1(void *pvParameters) {

  int new_value = 0;
  int old_value = 0;
  bool point = true;

  while(true){
    if (run_1){
      if(mode_1 == "random"){
        led_time_1         = random(time_min, time_max);
        new_value = random(brightness_min, brightness_max);
        old_value = pwmValue_1;
        feedback("\nLED_1:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_1) + "ms"); 
      }
      else if(mode_1 == "control"){
        led_time_1  = led_time_1;
        new_value   = brightness_1;
        old_value   = pwmValue_1; }
      else if(mode_1 == "only"){
        led_time_1         = random(time_min_1, time_max_1);
        new_value = random(brightness_min_1, brightness_max_1);
        old_value = pwmValue_1;
        feedback("\nLED_1:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_1) + "ms"); 
      }
      else if (mode_1 == "function 0") {
        ledcWrite(pwmChannel_1, 0);
        delay(delay_A_1);
        ledcWrite(pwmChannel_1, brightness_max);
        delay(delay_A_1);
        ledcWrite(pwmChannel_1, 0);
        delay(delay_A_1);
        ledcWrite(pwmChannel_1, brightness_max);
        delay(delay_A_1);
        ledcWrite(pwmChannel_1, 0);
        delay(delay_A_1);
        ledcWrite(pwmChannel_1, brightness_max);
        mode_1 = "control"; }
      else if (mode_1 == "function 1"){
        delay(delay_A_1);
        led_time_1  = led_time_1;
        new_value   = brightness_1;
        old_value   = pwmValue_1;
        mode_1 = "control"; }

      // Điều chỉnh tăng dần hoặc giảm dần giá trị PWM
      if (old_value != new_value) {
        int this_delay = abs(led_time_1 / (new_value - old_value));
        done_1 = false;
        long step = (new_value > old_value) ? 1 : -1;
        while (pwmValue_1 != new_value && !done_1) {
          if (done_1) old_value = new_value = pwmValue_1;
          pwmValue_1 += step;
          ledcWrite(pwmChannel_1, pwmValue_1);
          delay(this_delay);
          //vTaskDelay(pdMS_TO_TICKS(abs(led_time_1 / (new_value - old_value))));
        }
        done_1 = true;
      } else vTaskDelay(pdMS_TO_TICKS(task_delay));
    } else vTaskDelay(pdMS_TO_TICKS(task_delay));
  }
 }

void LED_2(void *pvParameters) {

  int new_value = 0;
  int old_value = 0;
  bool point = true;

  while(true){
    if (run_2){
      if(mode_2 == "random"){
        led_time_2         = random(time_min, time_max);
        new_value = random(brightness_min, brightness_max);
        old_value = pwmValue_2;
        feedback("\nLED_2:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_2) + "ms"); 
      }
      else if(mode_2 == "control"){
        led_time_2  = led_time_2;
        new_value   = brightness_2;
        old_value   = pwmValue_2; }
      else if(mode_2 == "only"){
        led_time_2         = random(time_min_2, time_max_2);
        new_value = random(brightness_min_2, brightness_max_2);
        old_value = pwmValue_2;
        feedback("\nLED_2:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_2) + "ms"); 
      }
      else if (mode_2 == "function 0") {
        ledcWrite(pwmChannel_2, 0);
        delay(delay_A_2);
        ledcWrite(pwmChannel_2, brightness_max);
        delay(delay_A_2);
        ledcWrite(pwmChannel_2, 0);
        delay(delay_A_2);
        ledcWrite(pwmChannel_2, brightness_max);
        delay(delay_A_2);
        ledcWrite(pwmChannel_2, 0);
        delay(delay_A_2);
        ledcWrite(pwmChannel_2, brightness_max);
        mode_2 = "control"; }
      else if (mode_2 == "function 1"){
        delay(delay_A_2);
        led_time_2  = led_time_2;
        new_value   = brightness_2;
        old_value   = pwmValue_2;
        mode_2 = "control"; }

      // Điều chỉnh tăng dần hoặc giảm dần giá trị PWM
      if (old_value != new_value) {
        int this_delay = abs(led_time_2 / (new_value - old_value));
        done_2 = false;
        long step = (new_value > old_value) ? 1 : -1;
        while (pwmValue_2 != new_value && !done_2) {
          if (done_2) old_value = new_value = pwmValue_2;
          pwmValue_2 += step;
          ledcWrite(pwmChannel_2, pwmValue_2);
          delay(this_delay);
          //vTaskDelay(pdMS_TO_TICKS(abs(led_time_2 / (new_value - old_value))));
        }
        done_2 = true;
      } else vTaskDelay(pdMS_TO_TICKS(task_delay));
    } else vTaskDelay(pdMS_TO_TICKS(task_delay));
  }
 }

void LED_3(void *pvParameters) {

  int new_value = 0;
  int old_value = 0;
  bool point = true;

  while(true){
    if (run_3){
      if(mode_3 == "random"){
        led_time_3         = random(time_min, time_max);
        new_value = random(brightness_min, brightness_max);
        old_value = pwmValue_3;
        feedback("\nLED_3:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_3) + "ms"); 
      }
      else if(mode_3 == "control"){
        led_time_3  = led_time_3;
        new_value   = brightness_3;
        old_value   = pwmValue_3; }
      else if(mode_3 == "only"){
        led_time_3         = random(time_min_3, time_max_3);
        new_value = random(brightness_min_3, brightness_max_3);
        old_value = pwmValue_3;
        feedback("\nLED_3:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_3) + "ms"); 
      }
      else if (mode_3 == "function 0") {
        ledcWrite(pwmChannel_3, 0);
        delay(delay_A_3);
        ledcWrite(pwmChannel_3, brightness_max);
        delay(delay_A_3);
        ledcWrite(pwmChannel_3, 0);
        delay(delay_A_3);
        ledcWrite(pwmChannel_3, brightness_max);
        delay(delay_A_3);
        ledcWrite(pwmChannel_3, 0);
        delay(delay_A_3);
        ledcWrite(pwmChannel_3, brightness_max);
        mode_3 = "control"; }
      else if (mode_3 == "function 1"){
        delay(delay_A_3);
        led_time_3  = led_time_3;
        new_value   = brightness_3;
        old_value   = pwmValue_3;
        mode_3 = "control"; }

      // Điều chỉnh tăng dần hoặc giảm dần giá trị PWM
      if (old_value != new_value) {
        int this_delay = abs(led_time_3 / (new_value - old_value));
        done_3 = false;
        long step = (new_value > old_value) ? 1 : -1;
        while (pwmValue_3 != new_value && !done_3) {
          if (done_3) old_value = new_value = pwmValue_3;
          pwmValue_3 += step;
          ledcWrite(pwmChannel_3, pwmValue_3);
          delay(this_delay);
          //vTaskDelay(pdMS_TO_TICKS(abs(led_time_3 / (new_value - old_value))));
        }
        done_3 = true;
      } else vTaskDelay(pdMS_TO_TICKS(task_delay));
    } else vTaskDelay(pdMS_TO_TICKS(task_delay));
  }
 }

void LED_4(void *pvParameters) {

  int new_value = 0;
  int old_value = 0;
  bool point = true;

  while(true){
    if (run_4){
      if(mode_4 == "random"){
        led_time_4         = random(time_min, time_max);
        new_value = random(brightness_min, brightness_max);
        old_value = pwmValue_4;
        feedback("\nLED_4:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_4) + "ms"); 
      }
      else if(mode_4 == "control"){
        led_time_4  = led_time_4;
        new_value   = brightness_4;
        old_value   = pwmValue_4; }
      else if(mode_4 == "only"){
        led_time_4         = random(time_min_4, time_max_4);
        new_value = random(brightness_min_4, brightness_max_4);
        old_value = pwmValue_4;
        feedback("\nLED_4:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_4) + "ms"); 
      }
      else if (mode_4 == "function 0") {
        ledcWrite(pwmChannel_4, 0);
        delay(delay_A_4);
        ledcWrite(pwmChannel_4, brightness_max);
        delay(delay_A_4);
        ledcWrite(pwmChannel_4, 0);
        delay(delay_A_4);
        ledcWrite(pwmChannel_4, brightness_max);
        delay(delay_A_4);
        ledcWrite(pwmChannel_4, 0);
        delay(delay_A_4);
        ledcWrite(pwmChannel_4, brightness_max);
        mode_4 = "control"; }
      else if (mode_4 == "function 1"){
        delay(delay_A_4);
        led_time_4  = led_time_4;
        new_value   = brightness_4;
        old_value   = pwmValue_4;
        mode_4 = "control"; }

      // Điều chỉnh tăng dần hoặc giảm dần giá trị PWM
      if (old_value != new_value) {
        int this_delay = abs(led_time_4 / (new_value - old_value));
        done_4 = false;
        long step = (new_value > old_value) ? 1 : -1;
        while (pwmValue_4 != new_value && !done_4) {
          if (done_4) old_value = new_value = pwmValue_4;
          pwmValue_4 += step;
          ledcWrite(pwmChannel_4, pwmValue_4);
          delay(this_delay);
          //vTaskDelay(pdMS_TO_TICKS(abs(led_time_4 / (new_value - old_value))));
        }
        done_4 = true;
      } else vTaskDelay(pdMS_TO_TICKS(task_delay));
    } else vTaskDelay(pdMS_TO_TICKS(task_delay));
  }
 }

void LED_5(void *pvParameters) {

  int new_value = 0;
  int old_value = 0;
  bool point = true;

  while(true){
    if (run_5){
      if(mode_5 == "random"){
        led_time_5         = random(time_min, time_max);
        new_value = random(brightness_min, brightness_max);
        old_value = pwmValue_5;
        feedback("\nLED_5:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_5) + "ms"); 
      }
      else if(mode_5 == "control"){
        led_time_5  = led_time_5;
        new_value   = brightness_5;
        old_value   = pwmValue_5; }
      else if(mode_5 == "only"){
        led_time_5         = random(time_min_5, time_max_5);
        new_value = random(brightness_min_5, brightness_max_5);
        old_value = pwmValue_5;
        feedback("\nLED_5:\t" + String(old_value) + "\t" + String(new_value) + "\t" + String(led_time_5) + "ms"); 
      }
      else if (mode_5 == "function 0") {
        ledcWrite(pwmChannel_5, 0);
        delay(delay_A_5);
        ledcWrite(pwmChannel_5, brightness_max);
        delay(delay_A_5);
        ledcWrite(pwmChannel_5, 0);
        delay(delay_A_5);
        ledcWrite(pwmChannel_5, brightness_max);
        delay(delay_A_5);
        ledcWrite(pwmChannel_5, 0);
        delay(delay_A_5);
        ledcWrite(pwmChannel_5, brightness_max);
        mode_5 = "control"; }
      else if (mode_5 == "function 1"){
        delay(delay_A_5);
        led_time_5  = led_time_5;
        new_value   = brightness_5;
        old_value   = pwmValue_5;
        mode_5 = "control"; }

      // Điều chỉnh tăng dần hoặc giảm dần giá trị PWM
      if (old_value != new_value) {
        int this_delay = abs(led_time_5 / (new_value - old_value));
        done_5 = false;
        long step = (new_value > old_value) ? 1 : -1;
        while (pwmValue_5 != new_value && !done_5) {
          if (done_5) old_value = new_value = pwmValue_5;
          pwmValue_5 += step;
          ledcWrite(pwmChannel_5, pwmValue_5);
          delay(this_delay);
          //vTaskDelay(pdMS_TO_TICKS(abs(led_time_5 / (new_value - old_value))));
        }
        done_5 = true;
      } else vTaskDelay(pdMS_TO_TICKS(task_delay));
    } else vTaskDelay(pdMS_TO_TICKS(task_delay));
  }
 }

void setup() {
  Serial.begin(115200);
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, LOW);
  
  // Khởi tạo giá trị mặc định
  Default();
  // Cấu hình kênh PWM cho từng chân
  ledcSetup(pwmChannel_1, pwmFrequency, pwmResolution);
  ledcAttachPin(LED_PIN_1, pwmChannel_1);
  ledcSetup(pwmChannel_2, pwmFrequency, pwmResolution);
  ledcAttachPin(LED_PIN_2, pwmChannel_2);
  ledcSetup(pwmChannel_3, pwmFrequency, pwmResolution);
  ledcAttachPin(LED_PIN_3, pwmChannel_3);
  ledcSetup(pwmChannel_4, pwmFrequency, pwmResolution);
  ledcAttachPin(LED_PIN_4, pwmChannel_4);
  ledcSetup(pwmChannel_5, pwmFrequency, pwmResolution);
  ledcAttachPin(LED_PIN_5, pwmChannel_5);

  //wifi
  WiFi.softAP(ssid, password);
  // Lấy địa chỉ IP của ESP32 khi ở chế độ SoftAP
  IPAddress ip = WiFi.softAPIP();
  server.begin();
  feedback("\n\nkhởi động mạng WiFi ...\nSSID: ", String(ssid), "\nPassword: ", String(password), "\nServer đang chạy tại: ");
  showIP(WiFi.softAPIP());

  // Tạo các luồng 
  xTaskCreate(Control, "Control", 16384,  NULL, 1, NULL);
  xTaskCreate(LED_1,    "LED_1",  2048,   NULL, 1, NULL);
  xTaskCreate(LED_2,    "LED_2",  2048,   NULL, 1, NULL);
  xTaskCreate(LED_3,    "LED_3",  2048,   NULL, 1, NULL);
  xTaskCreate(LED_4,    "LED_4",  2048,   NULL, 1, NULL);
  xTaskCreate(LED_5,    "LED_5",  2048,   NULL, 1, NULL);
  
  /*
  // 1024 2048 4096 8192 16384  32768
  xTaskCreatePinnedToCore(Control, "Control", 16384,  NULL, 1, &ServerHandle, 0); // Tên chức năng, Kích thước (ăn ram), tham số, độ ưu tiên, con trỏ, chạy trên nhân_cpu
  delay(1000);
  xTaskCreatePinnedToCore(LED_1 ,  "LED_1",   2048,   NULL, 1, &Led1Handle  , 1);
  xTaskCreatePinnedToCore(LED_2 ,  "LED_2",   2048,   NULL, 1, &Led2Handle  , 1);
  xTaskCreatePinnedToCore(LED_3 ,  "LED_3",   2048,   NULL, 1, &Led3Handle  , 1);
  xTaskCreatePinnedToCore(LED_4 ,  "LED_4",   2048,   NULL, 1, &Led4Handle  , 1);
  xTaskCreatePinnedToCore(LED_5 ,  "LED_5",   2048,   NULL, 1, &Led5Handle  , 1);
  */
 }
void loop() {
    // Vòng lặp chính không làm gì
 }
