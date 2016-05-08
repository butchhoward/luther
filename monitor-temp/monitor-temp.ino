/**
 * Workshop example for periodically sending temperature data.
 *
 * Visit https://www.losant.com/kit for full instructions.
 *
 * Copyright (c) 2016 Losant IoT. All rights reserved.
 * https://www.losant.com
 */

#include <ESP8266WiFi.h>
#include <Losant.h>

/*
 * Fetch definitions of 
const char* WIFI_SSID
const char* WIFI_PASS
 */
#include "credentials_wifi.h"

/* 
 *  fetch definitions of
const char* LOSANT_DEVICE_ID 
const char* LOSANT_ACCESS_KEY
const char* LOSANT_ACCESS_SECRET
*/
#include "credentials_losant.h"


const int BUTTON_PIN = 14;
const int LED_PIN = 12;
const int ONBOARD_RED_LED_PIN = 0;
const int ONBOARD_BLUE_LED_PIN = 2;

bool ledState = false;

WiFiClientSecure wifiClient;

LosantDevice device(LOSANT_DEVICE_ID);

namespace {
  double currentTempF = 0.0;
  double currentTempC = 0.0;
  int tempSum = 0;
  int tempCount = 0;
  
  int buttonState = 0;
  int timeSinceLastRead = 0;
  double tempAtLastTweet = 0.0;
  int timeSinceLastTweet = 0;
  const int AUTOMATIC_TWEET_INTERVAL_MILLSECONDS = 12 * 60 * 60 * 1000;
  const int DATA_REPORT_INTERVAL_MILLISECONDS = 12 * 1000; 

  boolean timeToTweetAgain() {
    boolean timeToTweet = false;

    if (timeSinceLastTweet > AUTOMATIC_TWEET_INTERVAL_MILLSECONDS) {
      timeToTweet = true;
    }
    else {
      double tempDiff = currentTempF - tempAtLastTweet;
      if ( tempDiff > 5.0 || tempDiff < -5.0 ) {
        timeToTweet = true;
      }
    }

    return timeToTweet;
  }
  
  double calculateCelciusFromRaw(double raw) {
    // The tmp36 documentation requires the -0.5 offset, but during
    // testing while attached to the Feather, all tmp36 sensors
    // required a -0.52 offset for better accuracy.
    double degreesC = (((raw / 1024.0) * 2.0) - 0.57) * 100.0;
    return degreesC;
  }
  
  double convertCtoF(double degreesC) {
      double degreesF = degreesC * 1.8 + 32;
      return degreesF;
  }

  void saveCurrentTemp() {
    double averageRaw = (double)tempSum / (double)tempCount;
    currentTempC = calculateCelciusFromRaw(averageRaw);
    currentTempF = convertCtoF(currentTempC);
  }
  
  void placeTemperatureDataInState( JsonObject& root) {
    root["tempC"] = currentTempC;
    root["tempF"] = currentTempF;
  }
  
  void sendState( JsonObject& root) {
    root.printTo(Serial);
    Serial.println(";");
    device.sendState(root);
  }

  void flashOnBoardLED(int ledPin) {
    const int FLASH_DELAY = 50;
    digitalWrite(ledPin, LOW);
    delay(FLASH_DELAY);
    digitalWrite(ledPin, HIGH);
    delay(FLASH_DELAY);
  }

  void flashOnBoardRedLED() {
    flashOnBoardLED(ONBOARD_RED_LED_PIN);
  }

  void flashOnBoardBlueLED() {
    flashOnBoardLED(ONBOARD_BLUE_LED_PIN);
  }

  void sendTweetSignal() {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["button"] = true;
    placeTemperatureDataInState( root );
    sendState(root);
    timeSinceLastTweet = 0;
    tempAtLastTweet = currentTempF;
    Serial.println("Tweet requested");
    flashOnBoardBlueLED();
  }

  void buttonPressed() {
    Serial.println("Button Pressed!");
    sendTweetSignal();
  }
  
  void reportTemp() {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    placeTemperatureDataInState( root );
    sendState(root);
  }

  void ledOn() {
    digitalWrite(LED_PIN, HIGH);
  }
  void ledOff() {
    digitalWrite(LED_PIN, LOW);
  }
  void toggle() {
    Serial.print("Toggling LED.");
    ledState = !ledState;
    ledState ? ledOn() : ledOff();
    Serial.println( ledState ? "ON" : "OFF" );
  }

  void reportData() {
    tempSum += analogRead(A0);
    tempCount++;

    if(timeSinceLastRead > DATA_REPORT_INTERVAL_MILLISECONDS) {
      saveCurrentTemp();
      reportTemp();
      flashOnBoardRedLED();
  
      timeSinceLastRead = 0;
      tempSum = 0;
      tempCount = 0;
    }
  }

  void pauseForATime() {
    delay(100);
    timeSinceLastRead += 100;
    timeSinceLastTweet += 100;
  }

  void handleButtonPress() {
    int currentRead = digitalRead(BUTTON_PIN);
    if(currentRead != buttonState) {
      buttonState = currentRead;
      if(buttonState) {
        buttonPressed();
      }
    }
  }

  void sendPeriodicTweet() {
    if( timeToTweetAgain() ) {
      sendTweetSignal();
    }
  }

  boolean isDisconnectedFromWifi() {
    boolean isDisconnected = (WiFi.status() != WL_CONNECTED);
    if(isDisconnected) {
      Serial.println("Disconnected from WiFi");
    }
    return isDisconnected;
  }

  boolean isDisconnectedFromMQTT() {
    boolean isDisconnected = !device.connected();
    if(isDisconnected) {
      Serial.println("Disconnected from MQTT");
      Serial.println(device.mqttClient.state());
    }
    return isDisconnected;
  }

  void connectToWIFI() {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
  
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  void connectToLosant() {
    Serial.println();
    Serial.print("Connecting to Losant...");
  
    device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);
  
    while(!device.connected()) {
      delay(500);
      Serial.print(".");
    }
  }
  
};

void handleCommand(LosantCommand *command) {
  Serial.print("Command received: ");
  Serial.println(command->name);

  if(strcmp(command->name, "toggle") == 0) {
    toggle();
  }
}

void connect() {
  if (ledState) {
    ledOff();
    delay(1000);
  }
  ledOn();
  connectToWIFI();
  connectToLosant();
  Serial.println("Connected!");
  Serial.println("This device is now ready for use!");
  ledOff();
  ledState = false;
}

void setup() {
  Serial.begin(115200);

  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the workshop that's running to
  // appear on each upload.
  delay(2000);

  Serial.println();
  Serial.println("Running Workshop 3 Firmware.");

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(ONBOARD_RED_LED_PIN, OUTPUT);
  pinMode(ONBOARD_BLUE_LED_PIN, OUTPUT);
  device.onCommand(&handleCommand);
  connect();
}

void loop() {
  if(isDisconnectedFromWifi() || isDisconnectedFromMQTT()) {
    connect();
  }
  device.loop();

  reportData();
  handleButtonPress();
  sendPeriodicTweet();
  pauseForATime();
}

