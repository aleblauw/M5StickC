#include <M5StickC.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#define DHTPIN 26 // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define TFT_GREY 0x5AEB

#include "config.h"

String data;
String temp ,hum;
String temp_previous;

DHT dht(DHTPIN, DHTTYPE);

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
  Serial.println("DHTxx test!");
  dht.begin();
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
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
  Serial.println("Failed to read from DHT sensor!");
  return;
  }
  
  // Reading temperature or humidity takes about 250 milliseconds! END
  // Display temp, hum and heatindex
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  M5.Lcd.setTextSize(1);
  float hi = dht.computeHeatIndex(f, h);
  M5.Lcd.println("");

  M5.Lcd.print("Humidity: ");
  M5.Lcd.println(h);
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  M5.Lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
  M5.Lcd.setTextFont(2);
  M5.Lcd.print("Temperature: ");
  M5.Lcd.println(t);
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  M5.Lcd.setTextColor(TFT_GREEN,TFT_BLACK);
  M5.Lcd.setTextFont(2);
  M5.Lcd.print("Heat index: ");
  M5.Lcd.println(hi);
  Serial.print("Heat index: ");
  Serial.print(hi);
  Serial.println(" *F");

  // Display temp, hum and heatindex

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
      if (temp.toInt() > 0){
        data = "Temperature=" + temp + "&Humidity=" + hum + "&SensorId=M5StickC" ;
        Serial.println("Used current Temperature");
      } else {
        temp = temp_previous;
        data = "Temperature=" + temp + "&Humidity=" + hum + "&SensorId=M5StickC" ;
        M5.Lcd.println("Used previous");
        Serial.println("Used previous temperature");
      }
      temp = String(t);
      hum = String(h);
      
      int httpCode = https.POST(data);
  
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
        String response = https.getString();
        Serial.println(httpCode);
        Serial.println(response);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          
          M5.Lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
          M5.Lcd.setTextFont(2);
          Serial.println(response);
          M5.Lcd.println(httpCode);
        }
      } else {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
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
  delay(10000);
}
