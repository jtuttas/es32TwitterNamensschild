
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
#include "esp_wpa2.h"
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
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds / \ \ \ \
#define TIME_TO_SLEEP 30 / Time ESP32 will go to sleep (in seconds) */

//GxIO_Class io(SPI, SS, 0, 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 2, uint8_t busy = 4);
//GxEPD_Class display(io);

GxIO_Class io(SPI, SS, 22, 21);
GxEPD_Class display(io, 16, 4);

// TODO: Hier WLAN Zugangsdaten anpassen
#define EAP_ID "tuttas"
#define EAP_USERNAME "tuttas"
#define EAP_PASSWORD "geheim"

static const char* ssid = "MMBBS-Intern";
//const char *ssid = "FRITZ!Box Fon WLAN 7390";
static const char *username = "tuttas";
const char *password = "geheim";
const char *mqtt_server = "service.joerg-tuttas.de";
String web_serverconfig = "http://service.joerg-tuttas.de/eink.json";
WiFiClient espClient;
PubSubClient client(espClient);
RTC_RODATA_ATTR char json[1023];
const int BUTTON_PIN = 0;
RTC_DATA_ATTR int currentName = 0;
RTC_DATA_ATTR int numerOfMembers = 0;
RTC_DATA_ATTR int bootCount = 0;
const char *e;
static const uint8_t LED_BUILTIN = 2;
String event;
String subject;
String member;
String teilnehmer[10];

void showPartialMemberUpdate()
{
    const char *name = "FreeSansBold9pt7b";
    const GFXfont *f = &FreeSansBold9pt7b;

    uint16_t box_x = 102;
    uint16_t box_y = 0;
    uint16_t box_w = 193;
    uint16_t box_h = 70;
    uint16_t cursor_y = box_y + 10;

    display.setRotation(135);
    display.setFont(f);
    display.setTextColor(GxEPD_BLACK);

    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y + 20);
    display.print(event);
    display.setCursor(box_x, cursor_y + 50);
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
    display.print(msg.substring(0, 21));
    display.setCursor(box_x, cursor_y + 14);
    display.print(msg.substring(21, 42));
    display.setCursor(box_x, cursor_y + 28);
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
    showPartialTwitterUpdate(msg, from);
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
            const char *t;
            t = subject.c_str();
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

void readFromJson(String payload)
{
    StaticJsonBuffer<1023> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(payload);
    String s = root["event"];
    event = s;
    Serial.print("event gelesen:");
    Serial.println(event);

    String s2 = root["subject"];
    subject = s2;
    Serial.print("subject gelesen:");
    Serial.println(subject);

    JsonArray &myArray = root["members"];
    numerOfMembers = myArray.size();
    for (int i = 0; i < numerOfMembers; i++)
    {
        String t = root["members"][i];
        teilnehmer[i] = t;
        Serial.print("Member:");
        Serial.println(t);
    }
    member = teilnehmer[currentName];
}

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    Serial.begin(115200);
    delay(500);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("setup");
    //Increment boot number and print it every reboot
    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));
    display.init();
    Serial.println("display Init");
    //display.drawExampleBitmap(gImage_splash, 0, 0, 128, 296, GxEPD_BLACK);
    //display.update();
    Serial.write("\r\nConnect to WLAN");

    // TODO: WPA2 enterprise magic starts here
    
    WiFi.disconnect(true);      
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); 
    esp_wifi_sta_wpa2_ent_enable(&config);
    WiFi.begin(ssid);

    // WPA2 enterprise magic ends here
    
    // TODO: Normale WLAN Verbindung
    //WiFi.begin(ssid, password);

    Serial.println();
    Serial.println("Waiting for connection and IP Address from DHCP");
    int count = 0;
    while (WiFi.status() != WL_CONNECTED && count <= 20)
    {
        delay(1000);
        Serial.print(".");
        if (count % 2 == 0)
        {
            digitalWrite(LED_BUILTIN, LOW);
        }
        else
        {
            digitalWrite(LED_BUILTIN, HIGH);
        }
        count++;
        if (count == 20)
        {
            Serial.println("Can't connect to WLAN, go to sleep!");
            if (bootCount == 1)
            {
                bootCount = 0;
            }
            esp_sleep_enable_timer_wakeup(5 * uS_TO_S_FACTOR);
            esp_deep_sleep_start();
        }
    }
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    digitalWrite(LED_BUILTIN, HIGH);

    if (bootCount == 1)
    {
        display.drawExampleBitmap(gImage_gui, sizeof(gImage_gui), GxEPD::bm_default | GxEPD::bm_partial_update);
        display.drawExampleBitmap(gImage_gui, 0, 0, 128, 296, GxEPD_BLACK);
        display.update();
        Serial.println("Rebuild Image");

        // Laden der Konfiguration vom Server
        HTTPClient http;
        http.begin(web_serverconfig); //Specify the URL
        int httpCode = http.GET();    //Make the request

        if (httpCode > 0)
        { //Check for the returning code

            String payload = http.getString();
            Serial.println(httpCode);
            Serial.println(payload);
            payload.toCharArray(json, payload.length() + 1);

            readFromJson(payload);
        }

        else
        {
            event = "undefined";
            subject = "undefined";
            member = "N.N.";
            Serial.println("Error on HTTP request");
        }
        showPartialMemberUpdate();
    }
    else
    {
        Serial.println("Habe Daten:");
        Serial.println(json);

        readFromJson(String(json));
    }

    int loops = 10;
    if (bootCount == 1)
    {
        loops = 100;
    }

    for (int i = 0; i < loops; i++)
    {
        if (digitalRead(BUTTON_PIN) == LOW)
        { // Check if button has been pressed
            while (digitalRead(BUTTON_PIN) == LOW)
                ; // Wait for button to be released
            Serial.println("Change Name");
            currentName++;
            if (currentName >= numerOfMembers)
            {
                currentName = 0;
            }
            member = teilnehmer[currentName];
            Serial.print("members gelesen:");
            Serial.println(member);
            showPartialMemberUpdate();
        }
        if (!client.connected())
        {
            reconnect();
        }
        client.loop();
        delay(100);
    }
    Serial.println("go to sleep");
    digitalWrite(LED_BUILTIN, HIGH);
    esp_sleep_enable_timer_wakeup(60 * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void loop()
{
}
