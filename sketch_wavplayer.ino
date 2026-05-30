#include <Adafruit_TLV320DAC3100.h>
#include <I2S.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <BackgroundAudio.h>
#include <SD.h>
#include <vector>
#include <algorithm>
#include <string.h>
#include "DACUtils.h"
#include "SDUtils.h"
#include "icons.h"
#include "UIUtils.h"
#include "generalUtils.h"

#define BUTTON_UP 22
#define BUTTON_DOWN 21
#define BUTTON_OK 28
#define BUTTON_BACK 20
#define BUTTON_PAUSE 15


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

byte prevButtonUp = LOW;
byte prevButtonDown = LOW;
byte prevButtonOk = LOW;
byte prevButtonBack = LOW;
byte prevButtonPause = LOW;

unsigned long buttonCooldown = 50; // millis
unsigned long lastUp = 0;
unsigned long lastDown = 0;
unsigned long lastOk = 0;
unsigned long lastBack = 0;
unsigned long lastPause = 0;

String entry;

byte buttonState;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, -1);

Adafruit_TLV320DAC3100 codec;  // Create codec object

I2S dac(OUTPUT);

BackgroundAudioWAVClass<RawDataBuffer<32 * 1024>> BMP(dac);

// The file we're currently playing
File f;

bool fail = false;
bool playingSong = false;
bool playingMode = false;
std::vector<String> queue;
int songIndex = 0;
int currentIndex = 0;
int prevIndex = 0;
String fName;


uint8_t filebuff[1024];

unsigned long scrollSpeed = 250;
unsigned long lastScrollUpdate = 0;

void setup() {

  Serial.begin(115200);
  
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_OK, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUTTON_PAUSE, INPUT_PULLUP);

  initDAC(codec, dac);

  initSD();

  // Start the background player
  BMP.begin();

  initUI(display);  
  
  drawSplash(display);

  delay(5000);

  drawPath(display);
}

void loop() {

  if (!f && playingSong)
  {
    if (songIndex < queue.size() && songIndex > -1)
    {
      songChange();
      BMP.flush();
      f = SD.open(queue[songIndex]);
      f.setTimeout(2000);
      auto p = f.position();
      p = p & ~511; // Ensure on a sector boundary, MP3 will resync appropriately
      f.seek(p);
      displaySong(display, f);
    }
    else
    {
      playingSong = false;
      playingMode = false;
      drawPath(display);
    }
  }

  // Stuff the buffer with as much as it will take, only doing full sector reads for performance
  while (f && BMP.availableForWrite() > 1024) {
    int len = f.read(filebuff, 1024);
    if (len != 1024)
    {
      if (fail)
      {
        fail = false;
        String trackName = getSongName(f);
        String albumName = getAlbum(f);
        String artistName = getArtist(f);
        unsigned long trackLength = getTrackLengthSeconds(f.size());
        f.close();
        File scrobble = SD.open(".SongHistory.csv", FILE_WRITE);
        scrobble.println(artistName + "," + trackName + "," + albumName + "," + String(trackLength));
        scrobble.close();
        songIndex += 1;
      }
      else
      {
        fail = true;
      }
    }
    else
    {
      fail = false;
      BMP.write(filebuff, len); 
    }
  }

  if (checkButton(BUTTON_UP, prevButtonUp, lastUp))
  {
    if (playingMode)
    {
      changeVolume(codec, true);
    }
    else
    {
      lastScrollUpdate = millis();
      updatePosition(display, false);
    }
  }

  if (checkButton(BUTTON_DOWN, prevButtonDown, lastDown))
  {
    if (playingMode)
    {
      changeVolume(codec, false);
    }
    else
    {
      lastScrollUpdate = millis();
      updatePosition(display, true);
    }
  }

  if (checkButton(BUTTON_OK, prevButtonOk, lastOk))
  {
    if (playingMode)
    {
      f.close();
      songIndex += 1;
    }
    else
    {
      lastScrollUpdate = millis();
      if (updatePath(display, true))
      {   
        currentIndex = arrowPlacement+scrollHeight;
        if (queueMenu)
        {
          determineQueueing();
        }
        else if (inPlaylist && currentIndex == 1 && !removingPlaylist)
        {
          playlistAddOrRemove(true);
        }
        else if (inPlaylist && currentIndex == 2 && !removingPlaylist)
        {
          playlistAddOrRemove(false);
        }
        else if (removingPlaylist)
        {
          File playlist = SD.open(playlistPath, O_WRITE);
          for (int i = 0; i < pathList.size(); i++)
          {
            if (i != currentIndex)
            {
              playlist.println(pathList[i].substring(0, pathList[i].length() - 1));
            }
           }
          playlist.close();
          removingPlaylist = false;
          getFolders();
          drawPath(display);
        }
        else if (addingPlaylist)
        {
          File playlist = SD.open(playlistPath, FILE_WRITE);
          playlist.println(currentPath + pathList[currentIndex]);
          playlist.close();
          currentPath = playlistPath;
          currentDepth = 2;
          inPlaylist = true;
          addingPlaylist = false;
          getFolders();
          drawPath(display);
        }
        else
        {
          queueCheck();
        }
      }
    }
  }

  if (checkButton(BUTTON_BACK, prevButtonBack, lastBack))
  {
    if (playingMode)
    {
      f.close();
      songIndex -= 1;
    }
    else if (!removingPlaylist)
    {
      lastScrollUpdate = millis();
      updatePath(display, false);
    }
  }

  if (checkButton(BUTTON_PAUSE, prevButtonPause, lastPause))
  {
    if (BMP.paused() && playingSong)
    {
      lastScrollUpdate = millis();
      BMP.unpause();
      songChange();
      playingMode = true;
    }
    else if (!BMP.paused() && playingSong)
    {
      lastScrollUpdate = millis();
      BMP.pause();
      playingMode = false;
      drawPath(display);
    }
  }

  if (std::find(scrollables.begin(), scrollables.end(), arrowPlacement+scrollHeight) != scrollables.end() + 1 && !playingMode)
  {
    if (scrollPlacement == 0)
    {
      if (millis() - lastScrollUpdate > scrollSpeed * 4)
      {
        lastScrollUpdate = millis();
        scrollPlacement += 1;
        drawPath(display);
      }
    }
    else
    {
      if (millis() - lastScrollUpdate > scrollSpeed)
      {
        lastScrollUpdate = millis();
        scrollPlacement += 1;
        drawPath(display);
      }
    }
  }
  if (playingMode)
  {
    if (millis() - lastScrollUpdate > scrollSpeed)
    {
      lastScrollUpdate = millis();
      displaySong(display, f);
    }
  }
}

void playSong(int index, bool shuffle = true)
{
  if (inPlaylist)
  {
    songIndex = index - 3;
  }
  else
  {
    songIndex = index - 1;
  }
  BMP.unpause();
  playingSong = true;
  playingMode = true;
  queue.clear();
  int startIndex = 1;
  if (inPlaylist)
  {
    startIndex = 3;
  }
  for (int i = startIndex; i < pathList.size(); i++)
  {
    getEntry(i);
    queue.push_back(entry);
  }
  f.close();
  if (index == 0 && shuffle)
  {
    songIndex = 0;
    random_shuffle(queue.begin(), queue.end());
  }
}

bool checkButton(pin_size_t button, byte& prevButtonState, unsigned long& lastUse)
{
  if (millis() - lastUse > buttonCooldown)
  {
    buttonState = digitalRead(button);
    if (buttonState != prevButtonState) {
      lastUse = millis();
      prevButtonState = buttonState;
      if (buttonState == LOW) {
        return true;
      }
    }
  }
  return false;
}

void determineQueueing()
{
  queueMenu = false;
  getFolders();
  switch (currentIndex)
  {
    case 0:
    {
      auto placementIndex = queue.begin() + songIndex + 1;
      getEntry(prevIndex);
      queue.emplace(placementIndex, entry);
      break;
    }
    case 1:
      playSong(prevIndex, false);
      break;
  }
  arrowPlacement = 0;
  scrollHeight = 0;
  drawPath(display);
}

String cleanEntry(String entry)
{
  return entry.substring(0, entry.length()-1);
}

void getEntry(int index)
{
  if (inPlaylist)
  {
    entry = cleanEntry(pathList[index]);
  }
  else
  {
    entry = currentPath + pathList[index];
  }
}

void queueCheck()
{
  int checkIndex = 0;
  if (inPlaylist)
  {
    checkIndex = 2;
  }
  if (playingSong && currentIndex > checkIndex)
  {
    prevIndex = currentIndex;
    queueOption(display);
  }
  else
  {
    playSong(currentIndex);
  }
}

void playlistAddOrRemove(bool add)
{
  arrowPlacement = 0;
  scrollHeight = 0;
  playlistPath = currentPath;

  if (add)
  {
    currentPath = "/";
    addingPlaylist = true;
    inPlaylist = false;
    currentDepth = 0;
  }
  else
  {
    removingPlaylist = true;
  }
  getFolders();
  drawPath(display);

}
