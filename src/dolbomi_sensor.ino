#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FirebaseArduino.h>
#include "FirebaseArduino.h"
#include <time.h>

#ifndef STASSID
#define STASSID "" // 입력 필요
#define STAPSK  "" // 입력 필요
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

#include <DHT.h>
#define DHTPIN 5
#define touchSensor1 4
#define touchSensor2 14
#define touchSensor3 12
#define touchSensor4 13
#define DHTTYPE DHT11
DHT dht11(DHTPIN, DHTTYPE);
int timezone = 3; 
int dst = 0; 
 
unsigned long previousMillis = 0;
const long interval = 10; 

#define FIREBASE_HOST "" // 입력 
#define FIREBASE_AUTH "" // 입력 필요

ESP8266WebServer server(80);

int count = 0;
float humi, temp;
int value1;
int value2;
int value3;
int value4;
int flag1=0;
int flag2=0;
int flag3=0;
int flag4=0;
int gasdata = 0;
int GasPin = A0;   // 가스센서 입력을 위한 아날로그 핀

void handleRoot() {
  //digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  //digitalWrite(led, 0);
}

void handleNotFound() {
  //digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);

}

void setup(void) {
  //pinMode(led, OUTPUT);
  //digitalWrite(led, 0);
  Serial.begin(115200);
  
  dht11.begin();


  // 가스 센서
  pinMode(GasPin ,INPUT);   // 아날로그 핀 A0를 입력모드로 설정  
  pinMode(touchSensor1, INPUT);      // 정전식 터치센서 입력으로 설정
  pinMode(touchSensor2, INPUT);      // 정전식 터치센서 입력으로 설정
  pinMode(touchSensor3, INPUT);      // 정전식 터치센서 입력으로 설정
  pinMode(touchSensor4, INPUT);      // 정전식 터치센서 입력으로 설정
  //pinMode(led1, OUTPUT);            // LED 출력으로 설정
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/gif", []() {
    static const uint8_t gif[] PROGMEM = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00, 0x10, 0x00, 0x80, 0x01,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x2c, 0x00, 0x00, 0x00, 0x00,
      0x10, 0x00, 0x10, 0x00, 0x00, 0x02, 0x19, 0x8c, 0x8f, 0xa9, 0xcb, 0x9d,
      0x00, 0x5f, 0x74, 0xb4, 0x56, 0xb0, 0xb0, 0xd2, 0xf2, 0x35, 0x1e, 0x4c,
      0x0c, 0x24, 0x5a, 0xe6, 0x89, 0xa6, 0x4d, 0x01, 0x00, 0x3b
    };
    char gif_colored[sizeof(gif)];
    memcpy_P(gif_colored, gif, sizeof(gif));
    // Set the background to a random set of colors
    gif_colored[16] = millis() % 256;
    gif_colored[17] = millis() % 256;
    gif_colored[18] = millis() % 256;
    server.send(200, "image/gif", gif_colored, sizeof(gif_colored));
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov"); 
   Serial.println("\nWaiting for time"); 
   while (!time(nullptr)) { 
     Serial.print("."); 
     delay(1000); 
   }
   Serial.println("");
   
   Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);


}


void loop(void) {
  
  temp = dht11.readTemperature();
  humi = dht11.readHumidity();
  gasdata = analogRead(GasPin);

  delay(1000);
  
  unsigned long currentMillis = millis();
  String date;
  String detailTime;
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    time_t now = time(nullptr); 
    Serial.println(ctime(&now)); 
    struct tm* pTimeInfo;
    
    pTimeInfo = localtime(&now);    // 현재 시간을 이쁘게 struct tm에 넣음
 
    int year = pTimeInfo->tm_year + 1900;    //연도에는 1900 더해줌
    int month = pTimeInfo->tm_mon + 1;    // 월에는 1 더해줌
    int day = pTimeInfo->tm_mday;
    int hour = pTimeInfo->tm_hour;
    int min = pTimeInfo->tm_min;
    int sec = pTimeInfo->tm_sec;

    hour = hour + 6;
    if (hour >= 24) {
      hour = hour - 24;
      day += 1;
    }

    String month_s = String(month);
    String day_s = String(day);
    String hour_s = String(hour);
    String min_s = String(min);
    String sec_s = String(sec);

    if(month<10){
      month_s="0"+String(month);
    }
    if(day<10){
      day_s="0"+String(day);
    }
    if(hour<10){
      hour_s="0"+String(hour);
    }
    if(min<10){
      min_s="0"+String(min);
    }
    if(sec<10){
      sec_s="0"+String(sec);
    }

    
    date = String(year)+"-"+String(month_s)+"-"+String(day_s);
    detailTime = String(hour_s)+":"+String(min_s)+":"+String(sec_s);

    delay(10);
  }

 
  Serial.println("Gas data : " + gasdata); 

  Firebase.setInt("Home1/" + date + "/gas", gasdata);
  
    if(Firebase.failed()) {
      Serial.print("gas / nuagoaifeoijfafioejf :");
      Serial.println(Firebase.error());
      return;
    }
    
  if(temp && humi) {  // 온도, 습도 값을 읽어오면
    Serial.print("humidity:");          // ‘시리얼 플로터’ 사용위해 이부분 주석 필요
    Serial.println(humi);                  // 습도값 출력
    Serial.print("temperature:");       // ‘시리얼 플로터’ 사용위해 이부분 주석 필요
    Serial.println(temp);                  // 온도값 출력
    
    Firebase.setFloat("Home1/" + date + "/humidity", humi);
    if(Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      return;
    }

    Firebase.setFloat("Home1/" + date + "/temperature", temp);
    if(Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      return;
    }
  } 
  else{ 
    Serial.print("Error:");                    
  }  

  value1 = digitalRead(touchSensor1);   // 터치가 되었는지 안도
  value2 = digitalRead(touchSensor2);   // 터치가 되었는지 안도
  value3 = digitalRead(touchSensor3);   // 터치가 되었는지 안도
  value4 = digitalRead(touchSensor4);   // 터치가 되었는지 안도


 
  if(flag1==0 && value1 == 1){                       // 터치가 되었을 때
    Serial.println("터치!");           // 터치가 되었다고 시리얼모니터에 출력
    Firebase.setInt("Home1/" + date + "/Bathroom/"+detailTime, 1);
    if(Firebase.failed()) {
      Serial.print("setting /number1 failed:");
      Serial.println(Firebase.error());
      return;
    }
    flag1 = 1;
 
  }else if(value1!=1){
    Serial.println("nothing1...");      // 터치가 되지 않았다고 시리얼 모니터에 'nothing...' 출력
    flag1 = 0;
  }


  if(flag2==0 &&value2 == 1){                       // 터치가 되었을 때if(value1 == 1){                       // 터치가 되었을 때
    Serial.println("터치!");           // 터치가 되었다고 시리얼모니터에 출력
    Firebase.setInt("Home1/" + date + "/Bedroom/"+detailTime, 1);
    if(Firebase.failed()) {
      Serial.print("setting /number2 failed:");
      Serial.println(Firebase.error());
      return;
    }
    flag2 = 1;
  }else if(value2 != 1){
    Serial.println("nothing2...");      // 터치가 되지 않았다고 시리얼 모니터에 'nothing...' 출력
    flag2 = 0;
  }


  if(flag3 == 0 && value3 == 1){                       // 터치가 되었을 때if(value1 == 1){                       // 터치가 되었을 때
    Serial.println("터치!");           // 터치가 되었다고 시리얼모니터에 출력
    Firebase.setInt("Home1/" + date + "/Frontdoor/"+detailTime, 1);
    if(Firebase.failed()) {
      Serial.print("setting /number3 failed:");
      Serial.println(Firebase.error());
      return;
    }
    flag3 = 1;
  }else if(value3 != 1){
    Serial.println("nothing3...");      // 터치가 되지 않았다고 시리얼 모니터에 'nothing...' 출력
    flag3 = 0;
  }

  if(flag4 == 0 && value4 == 1){                       // 터치가 되었을 때if(value1 == 1){                       // 터치가 되었을 때
    Serial.println("터치!");           // 터치가 되었다고 시리얼모니터에 출력
    Firebase.setInt("Home1/" + date + "/Refrigerator/"+detailTime, 1);
    if(Firebase.failed()) {
      Serial.print("setting /number4 failed:");
      Serial.println(Firebase.error());
      return;
    }
    flag4 = 1;
  }else if(value4 != 1){
    Serial.println("nothing4...");      // 터치가 되지 않았다고 시리얼 모니터에 'nothing...' 출력
    flag4 = 0;
  }

  delay(1000);  // 1s 대기  
  
  //delay(5);
  
  server.handleClient();
  MDNS.update();
}
