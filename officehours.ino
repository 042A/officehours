// Setup

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// DATA STORE : https://www.npoint.io/docs/52f3032ade3648cd73d9
// Svenska tecken finns med i LCD-char sheeten men ej enligt standardencoding. 
// https://forum.arduino.cc/t/i2c-lcd-2x16-svenska-tecken-hur/626674/5

// LCD I2C SETUP, ADRESS, CHAR LEN, ROWS
LiquidCrystal_I2C lcd(0x3F,16,2); 

// NETWORK
const char* ssid = "NETGEAR";
const char* password = "pass47bb";

void setup() {

  // LCD init
  lcd.init();
  lcd.clear();         
  lcd.backlight();   
  
  // Print a message on both lines of the LCD.
  lcd.setCursor(0,0);  
  lcd.print("Row 1");
  lcd.setCursor(0,1);   
  lcd.print("Row 2");

  // Serial init
  Serial.begin(115200);
  Serial.println("...");
  Serial.println();
  Serial.println();

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

}


void loop() {

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    // Ignore SSL certificate validation
    client->setInsecure();
    //create an HTTPClient instance
    HTTPClient https;
    
    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://api.npoint.io/52f3032ade3648cd73d9")) {  // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);

          // Parse JSON
          if(httpCode == 200)
          {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            if (error)
            return;
            
            
            const char* value1 = doc["line1"];
            Serial.println(value1);
            const char* value2 = doc["line2"];
            Serial.println(value2);

            // JsonObject messages = doc["screens"[0]];  //get lightstates obj
            // Serial.println(messages);
            deserializeJson(doc, "[1,2,3]");

            // extract the values
            JsonArray array = doc.as<JsonArray>();
            for(JsonVariant v : array) {
                Serial.println(v.as<int>());
            }

            // Displayed parsed values
            lcd.clear();  
            lcd.setCursor(0,0);   
            lcd.print(value1);
            lcd.setCursor(0,1);   
            lcd.print(value2);
            delay(4000);
            lcd.clear();  
            lcd.setCursor(4,0);
            lcd.print("Hardcoded msg");
            lcd.setCursor(1,1);
            lcd.print("w.padding:");
   
          }
          else
          {
            Serial.println("Error in response");
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  Serial.println();
  Serial.println("Waiting 5 sec before the next round...");
  delay(5000);
}


  