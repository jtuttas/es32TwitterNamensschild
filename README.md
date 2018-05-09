# ESP32 Interaktives Namensschild mit Twitter Integration
Das Namensschild ist konfigurierbar über eine JSON Datei, die der ESP32 zuvor von einem Server lädt.
```json
{
  "event":"MMBbS Hackaton",
  "subject":"mmbbs/hack",
  "members": [
    "Name1",
    "Name2",
    "Name3"
  ]
}
```
Die Twitter Nachrichten werden mittels eines Python Scripts *twitterMQTT.py* abgebildet als MQTT Topics auf einem MQTT Server gespeichert.

## Verkabelung

Display | ESP32 
--- | --- 
BUSY | D4 
RST | D21 
DC | D22 
CS | D5 
CLK | D18 
DIN | D23 
GND | GND 
3.3V | 3.3V 

## Video
[![ESP32 E-Ink Namensschild](http://img.youtube.com/vi/-tE8FJc4dC4/0.jpg)](http://www.youtube.com/watch?v=-tE8FJc4dC4)