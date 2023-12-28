#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>


LiquidCrystal_I2C lcd(0x3F,16,2); 
int sleepvalue = 3000;

void setup() {
 
    // LCD init
    lcd.init();
    lcd.clear();         
    lcd.backlight();   
    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
    Serial.begin(115200);
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    lcd.clear();  
    lcd.setCursor(0,0);   
    lcd.print("Anslut t. Wifi:");
    lcd.setCursor(0,1);   
    lcd.print("PepparkakaAP");

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    wm.resetSettings();


    // Define a text box, 50 characters maximum
    WiFiManagerParameter custom_text_box1("my_text1", "JSON endpoint (source)", "https://api.npoint.io/52f3032ade3648cd73d9", 50);
    // Add custom parameter
    wm.addParameter(&custom_text_box1);
    WiFiManagerParameter custom_text_box2("my_text2", "Screen Delay Ms", "5000", 50);
    // Add custom parameter
    wm.addParameter(&custom_text_box2);


    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("PepparkakaAP"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        lcd.clear();  
        lcd.setCursor(0,0);   
        lcd.print("Ansluten");
        lcd.setCursor(0,1);   
        lcd.print(custom_text_box1.getValue());
        // Print custom text box value to serial monitor
        Serial.print("Custom text box entries: ");
        Serial.println(custom_text_box1.getValue());
        Serial.println(custom_text_box2.getValue());
        const char* endpoint = custom_text_box1.getValue();
        const char* delay = custom_text_box2.getValue();
        sleepvalue = atoi(delay);
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
            const char* value3 = doc["line3"];
            Serial.println(value3);
            const char* value4 = doc["line4"];
            Serial.println(value4);

            // Displayed parsed values
            lcd.clear();  
            lcd.setCursor(0,0);   
            lcd.print(value1);
            lcd.setCursor(0,1);   
            lcd.print(value2);
            delay(sleepvalue);
            lcd.clear();  
            lcd.setCursor(0,0);
            lcd.print(value3);
            lcd.setCursor(0,1);
            lcd.print(value4);
   
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
  delay(sleepvalue);
}

