#include <stdlib.h>
#include <stdio.h>
#if defined(ESP32)
#include <HTTPClient.h>
String unionid = String((uint32_t)ESP.getEfuseMac(), HEX);
#elif defined(ESP8266)
WiFiClient HTTPClient;
uint64_t chipid = ESP.getChipId();
String unionid = String((uint32_t)chipid);
#endif
#define HTTP_URL "http://noteapi.yoyolife.fun"
String rtu_payload;
String rtu_cmd;
int rtu_interval;
int rtu_baud;
int rtu_cmd_len;
bool is_set_baud = false;
long next_time = 60000;
long last_time_count = 0;
// 获取下发数据
void rtuLoad() {
  if (last_time_count != 0 and millis() - last_time_count < next_time) {
    return;
  }
  rtuClean();
  unionid.toUpperCase();
  Serial.print("id:");
  Serial.println(unionid);
  String url = "/api/rtu/beat?id=" + unionid;
  String httpStr = httpReq(url);

  if (httpStr != "no") {
    rtu_payload = getValue(httpStr, 'end', 0);
    rtu_cmd = getValue(rtu_payload, ',', 1);
    rtu_interval = getValue(rtu_payload, ',', 2).toInt();
    rtu_baud = getValue(rtu_payload, ',', 4).toInt();
    rtu_cmd_len = getValue(rtu_payload, ',', 3).toInt();
    Serial.println("r:" + rtu_cmd);
    if (is_set_baud == false) {
      Serial2.begin(rtu_baud);
      is_set_baud = true;
    }

    int len = rtu_cmd.length();
    char arr[len / 2];
    for (int i = 0; i < len; i = i + 2) {
      arr[i / 2] = (unsigned char)strtol(rtu_cmd.substring(i, i + 2).c_str(), NULL, 16);
    }
    Serial2.write(arr, len / 2);
  } else {
    next_time = rtu_interval * 60 * 1000;
    last_time_count = millis();
  }
}
// 获取串口数据
unsigned char rtu_incoming_byte;
String rtu_readstring;
long rtu_time_count = 0;
String rtu_cb_string;
void rtuRead() {
  if (Serial2.available()) {
    rtu_incoming_byte = Serial2.read();
    rtu_readstring += String(rtu_incoming_byte, HEX);
  }
  if (rtu_readstring.length() > 0) {
    if (rtu_readstring.length() < 2) {
      rtu_cb_string += "0" + rtu_readstring;
    } else {
      rtu_cb_string += rtu_readstring;
    }
    rtu_readstring = "";
  }
  if (rtu_cb_string.length() > 0 and millis() - rtu_time_count > 1000) {
    if (rtu_cb_string.length() == rtu_cmd_len) {
      Serial.println("s:" + rtu_cb_string);
      String url = "/api/rtu/beat_cb?id=" + unionid + "&cmd=" + rtu_cb_string;

      if (httpReq(url) == "next") {
        Serial.println("rtuLoad");
        rtuLoad();
      }
      rtu_cb_string = "";
    }
    rtu_time_count = millis();
  } else if (rtu_time_count == 0) {
    rtu_time_count = millis();
  }
}
// 清空数据
void rtuClean() {
  rtu_cb_string = "";
}
// http请求
String httpReq(String url) {
  String httpStr = "";
#if defined(ESP32)
  HTTPClient http;
  String serverPath = HTTP_URL + url;
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    httpStr = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    return "";
  }
  http.end();
#elif defined(ESP8266)
  if (!HTTPClient.connect(HTTP_URL, 80)) {
    Serial.println("connection failed");
    return "";
  }
  String HttpRequest = (String)("GET ") + url + " HTTP/1.1\r\n" + "Content-Type: text/html;charset=utf-8\r\n" + "Host: " + HTTP_URL + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: Keep Alive\r\n\r\n";
  HTTPClient.print(HttpRequest);
  String line = HTTPClient.readStringUntil('\n');
  while (line.length() != 0) {
    if (line.indexOf("{\"status\"") != -1) {
      httpStr = String(line);
      break;
    }
    line = HTTPClient.readStringUntil('\n');
  }

  HTTPClient.stop();
#endif
  Serial.println(httpStr);
  return httpStr;
}
//拆分字符
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
//获取拆分字符数量
int countValue(String data, char separator) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found;
}
