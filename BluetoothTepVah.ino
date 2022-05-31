// OLED displej přes I2C 128x32 znaků
// řadič SSD1306

// připojení knihovny U8glib
#include "U8glib.h"

// inicializace OLED displeje z knihovny U8glib
U8GLIB_SSD1306_128X32 mujOled(U8G_I2C_OPT_NONE);
int vypnout = 130;
String zprava;

////////////////////////////////////////////////////////////////////
// Teploměr a vlhkoměr DHT11/22
// připojení knihovny DHT
#include "DHT.h"
// nastavení čísla pinu s připojeným DHT senzorem
#define pinDHT 5

// odkomentování správného typu čidla
#define typDHT11 DHT11     // DHT 11

// inicializace DHT senzoru s nastaveným pinem a typem senzoru
DHT mojeDHT(pinDHT, typDHT11);
float tep;
float vlh;
///////////////////////////////////////////////////////////////////

// SD karta kod :)

////////////////////////////////////////////////////////////////
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//pins:
const int HX711_dout = 6; //mcu > HX711 dout pin
const int HX711_sck = 7; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

float vaha1;
const int calVal_eepromAdress = 0;
const int tareOffsetVal_eepromAdress = 4;
unsigned long t = 0;

///////////////////////////////////////////////////////////
// Arduino Bluetooth modul HC-05
// nastavení propojovacích pinů Bluetooth a LED diody
#define RX 9
#define TX 8
// připojení knihovny SoftwareSerial
#include <SoftwareSerial.h>
// inicializace Bluetooth modulu z knihovny SoftwareSerial
SoftwareSerial bluetooth(TX, RX);
////////////////////////////////////////////////

void setup(void) {
  Serial.begin(9600); 
  bluetooth.begin(9600);
  // zapnutí komunikace s teploměrem DHT
  mojeDHT.begin();

  LoadCell.begin();
  //LoadCell.setReverseOutput();
  float calibrationValue; // calibration value (příklad "Kalibrace.ino")

#if defined(ESP8266)|| defined(ESP32)
  EEPROM.begin(512);
#endif
  EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  long tare_offset = 0;
  EEPROM.get(tareOffsetVal_eepromAdress, tare_offset);
  LoadCell.setTareOffset(tare_offset);
  boolean _tare = false; //set this to false as the value has been resored from eeprom

  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
}

void loopvaha(){
  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (true) {
      vaha1 = LoadCell.getData();
      vaha1 = vaha1 * 2 / 1000;
      Serial.print("Load_cell output val: ");
      Serial.println(vaha1);
      newDataReady = 0;
      t = millis();
      delay(100);
    }
  }
}

void loopteplomer(){
  // pomocí funkcí readTemperature a readHumidity načteme
  // do proměnných tep a vlh informace o teplotě a vlhkosti,
  // čtení trvá cca 250 ms
  tep = mojeDHT.readTemperature();
  vlh = mojeDHT.readHumidity();
  // kontrola, jestli jsou načtené hodnoty čísla pomocí funkce isnan
  if (isnan(tep) || isnan(vlh)) {
    // při chybném čtení vypiš hlášku
    Serial.println("Chyba při čtení z DHT senzoru!");
  } else {
    Serial.print("Teplota: "); 
    Serial.print(tep);
    Serial.print(" stupnu Celsia, ");
    Serial.print("vlhkost: "); 
    Serial.print(vlh);
    Serial.println("  %");
  }
  // pauza pro přehlednější výpis
  delay(100);
  }

void loop(void) { // Nastavená smyčka, poté Arduinko zobrazuje již naměřené hodnoty, průměr není potřeba dělat, váha kolísa +- 2g
   if(vypnout > 100) {
    
    loopvaha();
    delay(100);

    vypnout--;
    }
   if(vypnout < 101 && vypnout > 80) {
    
    delay(100);
    loopteplomer();

    vypnout--;
    }
    else {
  // proměnná pro ukládání dat z Bluetooth modulu
  byte BluetoothData;
  // kontrola Bluetooth komunikace, pokud je dostupná nová
  // zpráva, tak nám tato funkce vrátí počet jejích znaků
  if (bluetooth.available() > 0) {
    // načtení prvního znaku ve frontě do proměnné
    BluetoothData=bluetooth.read();
    // dekódování přijatého znaku
    switch (BluetoothData) {
      // každý case obsahuje dekódování jednoho znaku
      case '0':
        bluetooth.print("Tep: ");
        bluetooth.print(tep);
        bluetooth.print(" Vlh: ");
        bluetooth.println(vlh);
        bluetooth.print("UL-8: ");
        bluetooth.print(vaha1);
        bluetooth.print(" kg");
        break;
      case '1':
        bluetooth.println("UL-8: ");
        bluetooth.println(vaha1);
        break;
      case 'a':
        // v případě přejetí znaku 'a' vypíšeme
        // čas od spuštění Arduina
        bluetooth.print("Cas od spusteni Arduina: ");
        bluetooth.print(millis()/1000);
        bluetooth.println(" vterin.");
        break;
      case 'k':
        // možnost kalibrovat váhu pomocí připojení Bluetooth a zadaní "reset" hodnot. !(Zatím prakticky neotestováno)!
        
        // zde je ukázka načtení většího počtu informací,
        // po přijetí znaku 'b' tedy postupně tiskneme 
        // další znaky poslané ve zprávě
        bluetooth.print("Kalibrace bude dodelana :) ");
        BluetoothData=bluetooth.read();
        // v této smyčce zůstáváme do té doby,
        // dokud jsou nějaké znaky ve frontě
        while (bluetooth.available() > 0) {
          bluetooth.write(BluetoothData);
          // krátká pauza mezi načítáním znaků
          delay(10);
          BluetoothData=bluetooth.read();
        }
        // EEPROM.get(calVal_eepromAdress, BluetoothData); Možnost dodelat kalibraci pomoci Bluetooth, propsani do dalsiho zapnuti :)
        bluetooth.println();
        break;
      case '\r':
        // přesun na začátek řádku - znak CR
        break;
      case '\n':
        // odřádkování - znak LF
        break;
      default:
        // v případě přijetí ostatních znaků
        // vytiskneme informaci o neznámé zprávě
        bluetooth.println("Neznamy prikaz.");
    }
  }
  // krátká pauza mezi kontrolami komunikace Bluetooth modulu
  }
}
