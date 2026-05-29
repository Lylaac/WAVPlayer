#include <SD.h>
#include "SDUtils.h"

const int _MISO = 16;  // AKA SPI RX
const int _MOSI = 19;  // AKA SPI TX
const int _CS = 17;
const int _SCK = 18;

void initSD()
{
  Serial.begin(115200);

  Serial.println("Initializing SD...");
  // Initialize the SD card
  bool sdInitialized = false;
  if (_MISO == 0 || _MISO == 4 || _MISO == 16) {
    SPI.setRX(_MISO);
    SPI.setTX(_MOSI);
    SPI.setSCK(_SCK);
    sdInitialized = SD.begin(_CS);
  } else if (_MISO == 8 || _MISO == 12) {
    SPI1.setRX(_MISO);
    SPI1.setTX(_MOSI);
    SPI1.setSCK(_SCK);
    sdInitialized = SD.begin(_CS, SPI1);
  } else {
    Serial.println(("ERROR: Unknown SPI Configuration"));
    while (1) {
      delay(1);
    }
  }

  if (!sdInitialized) {
    Serial.println("initialization failed!");
    while (1) {
      delay(1000);
    rp2040.reboot();

    }
  }

  
}