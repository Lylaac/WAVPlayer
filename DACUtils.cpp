#include "DACUtils.h"
#include <Adafruit_TLV320DAC3100.h>
#include <I2S.h>

#define pBCLK  D9
#define pWS    D10
#define pDOUT  D11
#define pRESET 7

void initDAC(Adafruit_TLV320DAC3100& codec, I2S& i2s)
{
  Serial.begin(115200);
  // Hardware reset
  pinMode(pRESET, OUTPUT);
  digitalWrite(pRESET, LOW);
  delay(100);
  digitalWrite(pRESET, HIGH);
  delay(100);

  Serial.println("Initializing codec...");
  if (!codec.begin()) {
    Serial.println("Failed to initialize codec!");
  }
  codec.reset();

  Serial.println("Configuring codec interface...");
  if (!codec.setCodecInterface(TLV320DAC3100_FORMAT_I2S, TLV320DAC3100_DATA_LEN_16)) {
    Serial.println("Failed to configure codec interface!");
  }

  Serial.println("Configuring codec clocks...");
  if (!codec.setCodecClockInput(TLV320DAC3100_CODEC_CLKIN_PLL) ||
      !codec.setPLLClockInput(TLV320DAC3100_PLL_CLKIN_BCLK)) {
    Serial.println("Failed to configure codec clocks!");
  }

  if (!codec.setPLLValues(1, 1, 8, 0)) {
    Serial.println("Failed to configure PLL values!");
  }

  if (!codec.setNDAC(true, 8) || !codec.setMDAC(true, 2) || !codec.setDOSR(128)) {
    Serial.println("Failed to configure DAC dividers!");
  }

  if (!codec.powerPLL(true)) {
    Serial.println("Failed to power up PLL!");
  }

  Serial.println("Configuring DAC path...");
  if (!codec.setDACDataPath(true, true,
                            TLV320_DAC_PATH_NORMAL, TLV320_DAC_PATH_NORMAL,
                            TLV320_VOLUME_STEP_1SAMPLE)) {
    Serial.println("Failed to configure DAC data path!");
  }

  if (!codec.configureAnalogInputs(TLV320_DAC_ROUTE_MIXER, TLV320_DAC_ROUTE_MIXER,
                                   false, false, false, false)) {
    Serial.println("Failed to configure DAC routing!");
  }

  Serial.println("Setting DAC volume...");
  if (!codec.setDACVolumeControl(false, false, TLV320_VOL_INDEPENDENT) ||
      !codec.setChannelVolume(false, 18) || !codec.setChannelVolume(true, 18)) {
    Serial.println("Failed to configure DAC volume control!");
  }

  if (!codec.setChannelVolume(false, -35.0) || !codec.setChannelVolume(true, -35.0)) {
    Serial.println("Failed to set DAC channel volumes!");
  }

  // Headphone output
  codec.configureHeadphoneDriver(true, true, TLV320_HP_COMMON_1_35V, false);
  codec.setHPLVolume(true, 0);
  codec.setHPRVolume(true, 0);
  codec.configureHPL_PGA(9, true);
  codec.configureHPR_PGA(9, true);

  Serial.println("Initializing I2S...");
  i2s.setBCLK(pBCLK);
  i2s.setDATA(pDOUT);
  i2s.setBitsPerSample(16);
}

void changeVolume(Adafruit_TLV320DAC3100& codec, bool up)
{
  float volume;
  if (up)
  {
    volume = codec.getChannelVolume(true) + 1;
  }
  else
  {
    volume = codec.getChannelVolume(true) - 1;
  }
  codec.setChannelVolume(false, volume);
  codec.setChannelVolume(true, volume);
}