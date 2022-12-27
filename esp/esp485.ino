#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
SoftwareSerial Serial2(12, 14);  //RX=d5,TX=d6
#endif
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("start");
}
long loopInterval = 0;
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - loopInterval >= 10000) {
      //循环时间，不要少于10秒(10000)，否则会封禁ip
      loopInterval = millis();
      rtuLoad();
    }
    
  } else {
    //启动wifi
    wifiInit();
  }
   rtuRead();
}
int staStarted = 0;  //0是未连接 1是连接中
void wifiInit() {
  if (staStarted == 0) {
    Serial.print("Setting STA ... ");
    //初始化网络
    WiFi.mode(WIFI_STA);
    WiFi.begin("ssid", "pwd");
    staStarted = 1;
  } else if (staStarted == 1) {
    Serial.print('.');
    delay(1000);
  }
}
