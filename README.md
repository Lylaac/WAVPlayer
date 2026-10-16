This is a project to allow a Raspberry Pi Pico 2 to play music from an SD card using the Adafruit TLV320 DAC, a 128x32 SSD1306 screen and a Pimoroni Lipo Amigo Pro for power management and charging. Currently this code is complete but lacking comments and the physical status of the project is on hold as I learn CAD in fusion360.
The code was written in the Arduino IDE! 
The project has a fully functioning UI with the ability to create playlists, shuffle playlists or albums, add songs to queue and also log what songs have been listened to to a CSV file so that they can then be uploaded to last.fm .
To use this yourself you need to format the SD card as follows:
Root needs a playlist folder and folders for each artist. Each artist folder then must contain album folders, which then contain the WAV files (they must be WAV format) with this name scheme: [track number formatted as a 2 digit number, so first on album is 01, 10th is 10] [track name].wav. Examples:

01 Hell's Bells.wav
10 Living For The Weekend.wav

They MUST be formatted as so or the display will not work correctly.

Photos of the project in progress can be found below, as well as a breadboard schematic:
<img width="3264" height="2448" alt="PXL_20261016_163902407" src="https://github.com/user-attachments/assets/157778ec-df04-46d7-9dce-ede5bc6d8e1d" />
(An Image of the full project on the breadboard)

<img width="2448" height="3264" alt="PXL_20261016_163952217" src="https://github.com/user-attachments/assets/559df82c-da9a-4cfe-b655-0e90d0544a6b" />
(An image of the welcome screen that flashes when the device is turned on)

<img width="4208" height="2368" alt="IMG_20261016_164754005_HDR" src="https://github.com/user-attachments/assets/5a6f1ad6-beab-48e1-91bf-92ec54d05cf7" />
(An image of the main screen with access to playlists and artists)

<img width="4208" height="2368" alt="IMG_20261016_164820398" src="https://github.com/user-attachments/assets/fe2307f4-bb13-413c-aaa0-ace7ae0dded5" />
(An image of the UI when on an album)

<img width="4208" height="2368" alt="IMG_20261016_164712678" src="https://github.com/user-attachments/assets/995c35ed-8c43-4b19-8ae9-bffe9cb585ea" />
(An image of the UI when a song is playing)

<img width="4208" height="2368" alt="IMG_20261016_164747276" src="https://github.com/user-attachments/assets/1016af45-aa41-4265-95a3-dec504394a60" />
(An image of the playlist UI)

<img width="903" height="529" alt="image" src="https://github.com/user-attachments/assets/2d79029b-14f6-40f7-a768-8b99318c6eea" />
(An image of the schematic of the breadboard in Fritzing, not including the Pimoroni Lipo Amigo Pro, which is connected to ground and pin 39 VSYS of the Pi Pico 2.)
