#include <string.h>
#include <SD.h>

#ifndef generalUtils_H

#define generalUtils_H

String getAlbum(File track);

String getSongName(File track);

String getArtist(File track);

int getTrackRemainderSeconds(unsigned long size);

int getTrackLengthMinutes(unsigned long size);

float getTrackLengthSeconds(unsigned long size);

#endif