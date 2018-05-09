
// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

// mapping suggestion from Waveshare SPI e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion from Waveshare SPI e-Paper to generic ESP8266
// BUSY -> GPIO4, RST -> GPIO2, DC -> GPIO0, CS -> GPIO15, CLK -> GPIO14, DIN -> GPIO13, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// new mapping suggestion for STM32F1, e.g. STM32F103C8T6 "BluePill"
// BUSY -> A1, RST -> A2, DC -> A3, CS-> A4, CLK -> A5, DIN -> A7

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <GxEPD.h>
// GDEH029A1
#include <GxGDEH029A1/GxGDEH029A1.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <BitmapGraphics.h>
#include <ArduinoJson.h>

float tempC = 0;

//GxIO_Class io(SPI, SS, 0, 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 2, uint8_t busy = 4);
//GxEPD_Class display(io);

GxIO_Class io(SPI, SS, 22, 21);
GxEPD_Class display(io, 16, 4);

const char *ssid = "FRITZ!Box Fon WLAN 7390";
const char *password = "geheim";
const char *mqtt_server = "service.joerg-tuttas.de";
String web_serverconfig = "http://service.joerg-tuttas.de/eink.json";
WiFiClient espClient;
PubSubClient client(espClient);
String event;
String member;
String subject;
String teilnehmer[100];
const int BUTTON_PIN = 0;
int currentName=0;
int numerOfMembers=0;
const char *e;

void showPartialMemberUpdate()
{
    const char *name = "FreeSansBold9pt7b";
    const GFXfont *f = &FreeSansBold9pt7b;

    uint16_t box_x = 102;
    uint16_t box_y = 0;
    uint16_t box_w = 193;
    uint16_t box_h = 76;
    uint16_t cursor_y = box_y + 10;

    display.setRotation(135);
    display.setFont(f);
    display.setTextColor(GxEPD_BLACK);

    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y+20);
    display.print(event);
    display.setCursor(box_x, cursor_y+50);
    display.print(member);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void showPartialTwitterUpdate(String msg, String from)
{
    const char *name = "FreeMon9pt7b";
    const GFXfont *f = &FreeMono9pt7b;

    uint16_t box_x = 35;
    uint16_t box_y = 83;
    uint16_t box_w = 240;
    uint16_t box_h = 40;
    uint16_t cursor_y = box_y + 10;

    display.setRotation(135);
    display.setFont(f);
    display.setTextColor(GxEPD_BLACK);

    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y);
    display.print(msg.substring(0,21));
    display.setCursor(box_x, cursor_y+14);
    display.print(msg.substring(21,42));
    display.setCursor(box_x, cursor_y+28);
    display.print(msg.substring(42));
    
    display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(payload);
    String msg = root["text"];
    String from = root["username"];
    showPartialTwitterUpdate(msg,from);
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {           
            const char* t;
            t=subject.c_str();
            client.subscribe(t);
            Serial.print("connected and subscribe to:");
            // ... and resubscribe
            Serial.println(subject);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup()
{

    Serial.begin(115200);
    delay(500);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.println("setup");
    display.init();
    Serial.println("display Init");
    display.drawExampleBitmap(gImage_splash, 0, 0, 128, 296, GxEPD_BLACK);
    display.update();
    Serial.write("\r\nConnect to WLAN");
    WiFi.begin(ssid, password);
    Serial.println();
    Serial.println("Waiting for connection and IP Address from DHCP");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(2000);
        Serial.print(".");
    }
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    display.drawExampleBitmap(gImage_gui, 0, 0, 128, 296, GxEPD_BLACK);
    display.update();

    display.drawExampleBitmap(gImage_gui, sizeof(gImage_gui), GxEPD::bm_default | GxEPD::bm_partial_update);

    // Laden der Konfiguration vom Server
    HTTPClient http;
    http.begin(web_serverconfig); //Specify the URL
    int httpCode = http.GET();    //Make the request

    if (httpCode > 0)
    { //Check for the returning code

        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(payload);
        String s = root["event"];  
        event = s;
        Serial.print("event gelesen:");
        Serial.println(event);

        String s2 = root["subject"];
        subject = s2;
        Serial.print("subject gelesen:");
        Serial.println(subject);

        JsonArray& myArray = root["members"];
        numerOfMembers=myArray.size();
        for (int i=0;i<numerOfMembers;i++) {
            String t = root["members"][i];
            teilnehmer[i]=t;
        }
        member = teilnehmer[0];
        Serial.print("members gelesen:");
        Serial.println(member);
    }

    else
    {
        event = "undefined";
        subject="undefined";
        member = "N.N.";
        Serial.println("Error on HTTP request");
    }
    showPartialMemberUpdate();
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    
    if (digitalRead(BUTTON_PIN) == LOW)
    { // Check if button has been pressed
        while (digitalRead(BUTTON_PIN) == LOW); // Wait for button to be released
        Serial.println("Change Name");
        currentName++;
        if (currentName>=numerOfMembers) {
            currentName=0;
        }
        member = teilnehmer[currentName];
        Serial.print("members gelesen:");
        Serial.println(member);
        showPartialMemberUpdate();
    }
    
    client.loop();
}
