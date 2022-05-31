// Ulova vaha SD card, zápis na ní proveden vždy, data se nejvíce hodí při výpadku spojení a ztráty měřících hodnot

#include <SPI.h>
#include <SD.h>

File myFile;
//int const pinSS = 10;
int pinCS = 53;

void setup()
{
  // i když se pin nepoužívá, musí být definován jako výstup! (větší stabilita přenosu)
  // pinMode(pinSS, OUTPUT);
  // začátek sériové komunikace
  Serial.begin(9600);
  pinMode(pinCS, OUTPUT);
  Serial.print("inicializace SD karty...");

  // pokud není karta dostupná, tak:
  if (!SD.begin())
  {
    Serial.println(" inicializace selhala!");
    while (1);
  }
  Serial.println(" inicializace uspesna!");

  // soubor, který chceme otevřít a psát do něj (FILE_WRITE)
  // se jmenuje test.txt (popřípadě vložte celou cestu k souboru)
  myFile = SD.open("test.txt", FILE_WRITE);

  // pokud se soubor načte a otevře, tak:
  if (myFile)
  {
    Serial.println("Zapisuji do test.txt");
    // zapiš do souboru
    myFile.println("dneska sviti slunce");
    // zavři soubor
    myFile.close();
    Serial.println("uspesne zapsano");
  } 
  // pokud se nepodaří soubor načíst a otevřít, tak:
  else
  {
    Serial.println("soubor se nepodarilo otevrit");
  }

  // přečtění souboru:
  Serial.println("\ncteni ze souboru\n");
  // otevři soubor test.txt pro čtení, mód je defaultně nastavený na FILE_READ
  myFile = SD.open("test.txt");
  // pokud se soubor načte a otevře, tak:
  if (myFile)
  {
    Serial.println("soubor obsahuje: ");

    // čti ze souboru, dokud je co
    while (myFile.available())
    {
      Serial.write(myFile.read());
    }
    // zavři soubor
    myFile.close();
  }
  // pokud se nepodaří soubor načíst a otevřít, tak:
  else
  {
    Serial.println("soubor se nepodarilo otevrit");
  }
}

void loop()
{
  // jelikož chceme, aby kód proběhl jenom jednou, tak zde nic nebude
}
