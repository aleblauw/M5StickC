#include <M5StickC.h>
#include <WiFi.h>
//#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
// DS18B20 op pin 26.
#define ONE_WIRE_BUS 26
#define TFT_GREY 0x5AEB

#include "config.h"

OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

String data;
String temp ;

void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

WiFiMulti WiFiMulti;

void setup() {
  M5.begin();
  Serial.begin(9600);
 // Serial.setDebugOutput(true);
  M5.Lcd.setRotation(3);
  Serial.println("DS18b20 test!");
  Serial.println();
  Serial.println();
  Serial.println();
 
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(SSID, WIFIPASSWORD);

  // wait for WiFi connection
  int timer = 0;
  Serial.print("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED) and timer <5) {
    Serial.print(timer);
    timer++;
  }
  if (timer <5){
  Serial.println(" connected");
  M5.Lcd.println(" connected");
  
  }
  else{
  M5.Lcd.setTextColor(TFT_GREEN,TFT_BLACK);
  M5.Lcd.setTextFont(3);
  Serial.println(" disconnected");
  M5.Lcd.println(" disconnected");
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, LOW); //LED ON
       }
  setClock();  
}

void loop() {
  //io.run();
  delay(2000);
  //M5.Lcd.fillScreen(TFT_GREY);
  sensors.requestTemperatures(); 
  temp = sensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" *C  |  ");
  
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  M5.Lcd.setTextSize(1);

  M5.Lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
  M5.Lcd.setTextFont(2);
  M5.Lcd.print("Temperature: ");
  M5.Lcd.println(temp);
  M5.Lcd.print(" *C ");
  M5.Lcd.setTextColor(TFT_GREEN,TFT_BLACK);
  M5.Lcd.setTextFont(2);

  WiFiClientSecure *client = new WiFiClientSecure;
  
  if(client) {
    //client -> setCACert(rootCACertificate);

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
      
      Serial.print("[HTTPS] begin...\n");
      https.begin(*client, POST_URL);   // HTTPS
      https.addHeader("Content-Type", "application/x-www-form-urlencoded");
      https.addHeader("authorization", AUTH_TOKEN);
      Serial.print("[HTTPS] POST...\n");
      temp = String(temp);
      data = "Temperature=" + temp + "&SensorId=M5StickC-2" ;

      int httpCode = https.POST(data);
  
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
        String response = https.getString();
        Serial.println(httpCode);
        Serial.println(response);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          
          M5.Lcd.setTextColor(TFT_GREEN,TFT_BLACK);
          M5.Lcd.setTextFont(2);
          Serial.println(response);
          M5.Lcd.println(httpCode);
          pinMode(M5_LED, OUTPUT);
          digitalWrite(M5_LED, HIGH); //LED OFF
        }
      } else {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        pinMode(M5_LED, OUTPUT);
        digitalWrite(M5_LED, LOW); //LED ON   
      }
  
      https.end();
  
      https.begin(*client, POST_URL_ADAFRUIT);
  
      https.addHeader(ADAFRUIT_HEADER);
      https.addHeader(ADAFRUIT_HEADER2);
      Serial.print("[HTTPS] POST to adafruit...\n");
      // start connection and send HTTP header
      data = "value=" + temp;     
      httpCode = https.POST(data);
      if (httpCode > 0) {
        Serial.printf("[HTTPS] POST ADAFruit... code: %d\n", httpCode);
        String response = https.getString();
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          M5.Lcd.setTextColor(TFT_GREEN,TFT_BLACK);
          M5.Lcd.setTextFont(2);
          Serial.println(response);
          M5.Lcd.println(httpCode);
          pinMode(M5_LED, OUTPUT);
          digitalWrite(M5_LED, HIGH); //LED OFF
        }
      } else {
        Serial.printf("[HTTPS] POST to Adafruit... failed, error: %s\n", https.errorToString(httpCode).c_str());
        pinMode(M5_LED, OUTPUT);
        digitalWrite(M5_LED, LOW); //LED ON   
      }
  
      https.end();
        
      // End extra scoping block
    }
  
    delete client;
  } else {
    Serial.println("Unable to create client");
  }

  Serial.println();
  Serial.println("Waiting 10s before the next round...");
  M5.Lcd.setTextColor(TFT_BLUE,TFT_BLACK);
  M5.Lcd.setTextFont(2);
  M5.Lcd.print("Waiting 10s before the next round...");
  delay(10000);
}
