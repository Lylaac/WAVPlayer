#include <Adafruit_SSD1306.h>
#include <string.h>
#include <vector>

extern String currentPath;
extern String playlistPath;
extern int arrowPlacement;
extern int scrollHeight;
extern std::vector<String> pathList;
extern std::vector<int> scrollables;
extern int currentDepth;
extern int scrollPlacement;

extern bool inPlaylist;
extern bool addingPlaylist;
extern bool removingPlaylist;

extern bool queueMenu;

#ifndef UIUtils_H

#define UIUtils_H




void initUI(Adafruit_SSD1306& display);

void drawSplash(Adafruit_SSD1306& display);

bool updatePath(Adafruit_SSD1306& display, bool down);

void drawPath(Adafruit_SSD1306& display);

void updatePosition(Adafruit_SSD1306& display, bool increment);

void getFolders();

void displaySong(Adafruit_SSD1306& display, File track);

void scrollingSongDisplay(Adafruit_SSD1306& display, String line, int& scrollType, int& scrollCounter);

void songChange();

void queueOption(Adafruit_SSD1306& display);


#endif