#include <string.h>
#include <vector>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <algorithm>
#include <SD.h>
#include "UIUtils.h"
#include "icons.h"
#include "SDUtils.h"
#include "generalUtils.h"

std::vector<String> pathList;
std::vector<int> scrollables;
String playlistPath;
String currentPath = "/";
int currentDepth = 0;
int arrowPlacement = 0;
int scrollHeight = 0;
int scrollPlacement = 0;

int trackScroll = 0;
int albumScroll = 0;
int artistScroll = 0;

int trackScrollCounter = 4;
int albumScrollCounter = 4;
int artistScrollCounter = 4;

bool inPlaylist = false;
bool addingPlaylist = false;
bool removingPlaylist = false;

bool queueMenu = false;


void initUI(Adafruit_SSD1306& display)
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  getFolders();
}

void drawSplash(Adafruit_SSD1306& display)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.drawRoundRect(0, 0, 128, 32, 8, WHITE);
  display.setCursor(42, 12);
  display.println("WELCOME!");

  display.display();
}

bool updatePath(Adafruit_SSD1306& display, bool down)
{
  scrollPlacement = 0;
  bool returner = false;
  if (down && currentDepth == 2)
  {
    returner = true;
  }
  else if (down && currentDepth < 2)
  {
    currentPath += pathList[arrowPlacement + scrollHeight] + "/";
    currentDepth += 1;
    getFolders();
  }
  else if (currentDepth > 0 && down == false)
  {
    if (currentDepth == 2)
    {
      currentPath = currentPath.substring(0,currentPath.substring(0, currentPath.lastIndexOf('/')).lastIndexOf('/'));
    }
    else
    {
      currentPath = "/";
    }
    currentDepth -= 1;
    getFolders();
  }

  if (!returner)
  {
    arrowPlacement = 0;
    scrollHeight = 0;
  }

  drawPath(display);

  return returner;
}

void drawPath(Adafruit_SSD1306& display)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  for (int i = scrollHeight; i < scrollHeight + 5; i++)
  {
    String lineName = pathList[i];

    if (lineName.substring(0, 2) == "AA")
    {
      lineName = lineName.substring(2, lineName.length());
    }

    if (lineName.substring(lineName.length() - 3, lineName.length()) == "txt")
    {
      lineName = lineName.substring(0, lineName.length() - 4);
    }

    if (inPlaylist && (i > 2 || removingPlaylist))
    {
      lineName = lineName.substring(lineName.lastIndexOf("/") + 1, lineName.length() - 1);
    }

    int height = i - scrollHeight;
    if (i >= pathList.size())
    {
      break;
    }

    switch(currentDepth)
    {
      case 0:
        if (lineName == "Playlists")
        {
          display.drawBitmap(0,8*height, playlistIcon, 7, 7, SSD1306_WHITE);
        }
        else
        {
          display.drawBitmap(0,8*height, artistIcon, 7, 7, SSD1306_WHITE);
        }
        break;
      case 1:
        display.drawBitmap(0,8*height, albumIcon, 7, 7, SSD1306_WHITE);
        break;
      case 2:
        if (queueMenu)
        {
          switch (i)
          {
            case 0:
              display.drawBitmap(0,8*height, queueIcon, 7, 7, SSD1306_WHITE);
              break;
            case 1:
              display.drawBitmap(0,8*height, playIcon, 7, 7, SSD1306_WHITE);
              break;
            case 2:
              display.drawBitmap(0,8*height, cancelIcon, 7, 7, SSD1306_WHITE);
              break;
          }
        }
        else
        {
          if (lineName != "Shuffle" && lineName != "Add song" && lineName != "Remove song")
          {
            display.drawBitmap(0,8*height, trackIcon, 7, 7, SSD1306_WHITE);
            lineName = lineName.substring(3, lineName.length() - 4);
          }
          else if (lineName == "Shuffle")
          {
            display.drawBitmap(0,8*height, shuffleIcon, 7, 7, SSD1306_WHITE);
          }
          else if (lineName == "Add song")
          {
            display.drawBitmap(0,8*height, addPlaylistIcon, 7, 7, SSD1306_WHITE);
          }
          else
          {
            display.drawBitmap(0,8*height, removePlaylistIcon, 7, 7, SSD1306_WHITE);
          }
        }
        break;
    }

    display.setCursor(9, 8*height);

    if (lineName.length() <= 18)
    {
      display.print(lineName);
    }
    else if (i == scrollHeight + arrowPlacement)
    {
      if (scrollPlacement > lineName.length() - 15)
      {
        scrollPlacement = 0;
      }

      display.print(lineName.substring(scrollPlacement, 18 + scrollPlacement));
    }
    else
    {
      display.print(lineName.substring(0, 18));
    }

  }

  display.drawBitmap(121, 8*arrowPlacement, arrowPointer, 7, 7, SSD1306_WHITE);

  display.display();
}

void updatePosition(Adafruit_SSD1306& display, bool increment)
{
  scrollPlacement = 0;
  if (increment)
  {
    if (scrollHeight + arrowPlacement < pathList.size() - 1)
    {
      if (arrowPlacement < 3)
      {
        arrowPlacement += 1;
      }
      else
      {
        scrollHeight += 1;
      }
    }
  }
  else
  {
    arrowPlacement -= 1;
    if (arrowPlacement < 0)
    {
      arrowPlacement = 0;
      scrollHeight -= 1;
      if (scrollHeight < 0)
      {
        scrollHeight = 0;
      }
    }
  }
  drawPath(display);
}

void getFolders()
{
  int counter;
  pathList.clear();
  scrollables.clear(); 
  if (currentPath.substring(currentPath.length() - 4, currentPath.length() - 1) == "txt")
  {
    counter = 3;
    inPlaylist = true;
    File playlist = SD.open(currentPath);
    while (playlist.available())
    {
      String name = playlist.readStringUntil('\n');
      pathList.push_back(name);
      if (name.length() > 18)
      {
        scrollables.push_back((counter));
      }
      counter += 1;
    }
    if (!removingPlaylist)
    {
      pathList.emplace(pathList.begin(), "Remove song");
      pathList.emplace(pathList.begin(), "Add song");
    }
    playlist.close();
  }
  else
  {
    counter = 1;
    inPlaylist = false;
    File root = SD.open(currentPath);
    File entry;
      
    while (true)
    {
      entry = root.openNextFile();
      if (!entry)
      {
        break;
      }
      if (entry.name()[0] == '.') {
        // Ignore . and ..
        continue;
      }

      String name = entry.name();

      if (addingPlaylist && name == "AAPlaylists")
      {
        continue;
      }
      if (name.length() > 18)
      {
        scrollables.push_back((counter));
      }

      pathList.push_back(name);

      entry.close();
      counter += 1;
    }
    root.close();

    sort(pathList.begin(), pathList.end());
  }

  if (currentDepth == 2 && !addingPlaylist && !removingPlaylist)
  {
    pathList.emplace(pathList.begin(), "Shuffle");
  }
  
}

void displaySong(Adafruit_SSD1306& display, File track)
{
  String trackName = getSongName(track);
  String albumName = getAlbum(track);
  String artistName = getArtist(track);

  String fullMinLength = String(getTrackLengthMinutes(track.size()));
  String fullSecLength = String(getTrackRemainderSeconds(track.size()));

  String currentMinLength = String(getTrackLengthMinutes(track.position()));
  String currentSecLength = String(getTrackRemainderSeconds(track.position()));

  String timeStamp = currentMinLength + ":" + currentSecLength + "/" + fullMinLength + ":" + fullSecLength;

  display.clearDisplay();

  display.setCursor(128-timeStamp.length()*6, 0);

  display.print(timeStamp);

  display.drawBitmap(0,8, trackIcon, 7, 7, SSD1306_WHITE);
  display.drawBitmap(0,16, albumIcon, 7, 7, SSD1306_WHITE);
  display.drawBitmap(0,24, artistIcon, 7, 7, SSD1306_WHITE);

  
  display.setCursor(0, 0);
  display.println("Now Playing:");

  display.setCursor(9, 8);
  scrollingSongDisplay(display, trackName, trackScroll, trackScrollCounter);
  display.setCursor(9, 16);
  scrollingSongDisplay(display, albumName, albumScroll, albumScrollCounter);
  display.setCursor(9, 24);
  scrollingSongDisplay(display, artistName, artistScroll, artistScrollCounter);

  display.display();
}

void scrollingSongDisplay(Adafruit_SSD1306& display, String line, int& scrollType, int& scrollCounter)
{
  if (line.length() <= 19)
  {
    display.print(line);
  }
  else
  {
    display.print(line.substring(scrollType, 19 + scrollType));
    scrollType += 1;
  }

  if (scrollType > line.length() - 16)
  {
    scrollType = 0;
    scrollCounter = 4;
  }

  if (scrollCounter > 0)
  {
    scrollType = 0;
    scrollCounter -= 1;
  }

  display.println();
}

void songChange()
{
  trackScroll = 0;
  albumScroll = 0;
  artistScroll = 0;

  trackScrollCounter = 4;
  albumScrollCounter = 4;
  artistScrollCounter = 4;  
}

void queueOption(Adafruit_SSD1306& display)
{
  arrowPlacement = 0;
  scrollHeight = 0;
  queueMenu = true;
  pathList.clear();
  pathList.push_back("Queue Song");
  pathList.push_back("Play Song");
  pathList.push_back("Cancel");
  drawPath(display);
}