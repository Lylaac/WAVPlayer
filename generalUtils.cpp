#include <string.h>
#include <SD.h>
#include "generalUtils.h"

int byteRate = 176400;

String getAlbum(File track)
{
  String fName = track.fullName();
  return fName.substring(fName.indexOf("/", fName.indexOf("/") + 1)+1, fName.lastIndexOf("/"));
}

String getSongName(File track)
{
  String fName = track.fullName();
  return fName.substring(fName.lastIndexOf("/") + 4, fName.length() - 4);
}

String getArtist(File track)
{
  String fName = track.fullName();
  return fName.substring(1, fName.lastIndexOf("/", fName.lastIndexOf("/") - 1));
}

int getTrackRemainderSeconds(unsigned long size)
{
  int length = size / byteRate;
  int lengthMins = getTrackLengthMinutes(size);
  length -= (lengthMins * 60);
  return length;
}

int getTrackLengthMinutes(unsigned long size)
{
  int length = getTrackLengthSeconds(size) / 60;
  return length;
}

float getTrackLengthSeconds(unsigned long size)
{
  float length = size / byteRate;
  return length;
}