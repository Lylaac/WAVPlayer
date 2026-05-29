#include <Adafruit_TLV320DAC3100.h>
#include <I2S.h>

#ifndef DACUtils_H

#define DACUtils_H

void initDAC(Adafruit_TLV320DAC3100& codec, I2S& i2s);

void changeVolume(Adafruit_TLV320DAC3100& codec, bool up);


#endif