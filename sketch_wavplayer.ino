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
      if (inPlaylist)
      {
        f = SD.open(queue[songIndex].substring(0, queue[songIndex].length()-1));
      }
      else
      {
        f = SD.open(queue[songIndex]);
      }
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
        f.close();
        File scrobble = SD.open(".SongHistory.csv", FILE_WRITE);
        scrobble.println(artistName + "," + trackName + "," + albumName);
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
      BMP.write(filebuff, len); 
    }
  }



  if (millis() - lastUp > buttonCooldown)
  {
    buttonState = digitalRead(BUTTON_UP);
    if (buttonState != prevButtonUp) {
      lastUp = millis();
      prevButtonUp = buttonState;
      if (buttonState == LOW) {
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
      
    }
  }

  if (millis() - lastDown > buttonCooldown)
  {
    buttonState = digitalRead(BUTTON_DOWN);
    if (buttonState != prevButtonDown) {
      lastDown = millis();
      prevButtonDown = buttonState;
      if (buttonState == LOW) {
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
    }
  }

  if (millis() - lastOk > buttonCooldown)
  {
    buttonState = digitalRead(BUTTON_OK);
    if (buttonState != prevButtonOk) {
      lastOk = millis();
      prevButtonOk = buttonState;
      if (buttonState == LOW) {
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
              queueMenu = false;
              getFolders();
              switch (currentIndex)
              {
                case 0:
                  playingMode = true;
                  BMP.unpause();
                  if (inPlaylist)
                  {
                    queue.emplace(queue.begin() + songIndex + 1, pathList[prevIndex]);
                  }
                  else
                  {
                    queue.emplace(queue.begin() + songIndex + 1, currentPath + pathList[prevIndex]);
                  }
                  break;
                case 1:
                  if (inPlaylist)
                  {
                    songIndex = prevIndex - 3;
                  }
                  else
                  {
                    songIndex = prevIndex - 1;
                  }
                  BMP.unpause();
                  playingSong = true;
                  playingMode = true;
                  queue.clear();
                  if (inPlaylist)
                  {
                    for (int i = 3; i < pathList.size(); i++)
                    {
                      queue.push_back(pathList[i]);
                    }
                  }
                  else
                  {
                    for (int i = 1; i < pathList.size(); i++)
                    {
                      queue.push_back(currentPath + pathList[i]);
                    }
                  }
                  f.close();
                  break;
              }
              drawPath(display);
            }
            else if (inPlaylist && currentIndex == 1 && !removingPlaylist)
            {
              arrowPlacement = 0;
              scrollHeight = 0;
              playlistPath = currentPath;
              currentPath = "/";
              addingPlaylist = true;
              inPlaylist = false;
              currentDepth = 0;
              getFolders();
              drawPath(display);
            }
            else if (inPlaylist && currentIndex == 2 && !removingPlaylist)
            {
              arrowPlacement = 0;
              scrollHeight = 0;
              playlistPath = currentPath;
              removingPlaylist = true;
              getFolders();
              drawPath(display);
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
            else if (inPlaylist)
            {
              if (playingSong && currentIndex > 2)
              {
                prevIndex = currentIndex;
                queueOption(display);
              }
              else
              {
                queue.clear();
                BMP.unpause();
                playingSong = true;
                playingMode = true;
                songIndex = currentIndex - 3;
                for (int i = 3; i < pathList.size(); i++)
                {
                  queue.push_back(pathList[i]);
                }
                f.close();
                if (currentIndex == 0)
                {
                  songIndex = 0;
                  random_shuffle(queue.begin(), queue.end());
                }
              }
            }
            else
            {
              if (playingSong && currentIndex != 0)
              {
                prevIndex = currentIndex;
                queueOption(display);
              }
              else
              {
                songIndex = currentIndex - 1;
                BMP.unpause();
                playingSong = true;
                playingMode = true;
                queue.clear();
                for (int i = 1; i < pathList.size(); i++)
                {
                  queue.push_back(currentPath + pathList[i]);
                }
                f.close();
                if (currentIndex == 0)
                {
                  songIndex = 0;
                  random_shuffle(queue.begin(), queue.end());
                }
              }
            }
          }
          
        }
      }
    }
  }

  if (millis() - lastBack > buttonCooldown)
  {
    buttonState = digitalRead(BUTTON_BACK);
    if (buttonState != prevButtonBack) {
      lastBack = millis();
      prevButtonBack = buttonState;
      if (buttonState == LOW) {
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
    }
  }

  if (millis() - lastPause > buttonCooldown)
  {
    buttonState = digitalRead(BUTTON_PAUSE);
    if (buttonState != prevButtonPause) {
      lastPause = millis();
      prevButtonPause = buttonState;
      if (buttonState == LOW) {
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
  

  // Could do things like draw a UI on a LCD, check buttons, etc. here on each loop
  // Just be sure to feed the MP3 raw data at a BPS matching the recording mode
  // (i.e. a 128Kb MP3 only needs 128Kb (16K byte) of reads every second).  Don't disable
  // IRQs for long, either, as the audio DMA and calculations re driven off of them.
  //
  // If you don't send data fast enough (underflow) or you stop sending data completely,
  // the audio will silence and continue when more data is available.
}
