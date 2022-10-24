# StudioTVPlayer
A simple TV studio player with SDI, NDI and MPEG TS outputs for Windows
## Features:
* Few players on a single screen, controlled using mouse and keyboard shortcuts
* Almost every file format can be played - it uses FFmpeg under the hood
* Uses Blackmagic Design Decklink cards as SDI output and input
* Can output to [NDI](https://www.ndi.tv) and MPEG-TS streams
* Can play files with alpha channel (transparency), with key & fill on separate SDI outputs of supporting Decklink cards (i.e. Duo 2, 8K Pro, Extreme 4K)
* Watched folders with clip thumbnails
* Drag & drop files to rundown from watched folder or directly from Windows Explorer
* Audio level adjustment for playing file (+/- 15 dB)
* Multiple outputs for single player
* Time/Timecode input and output on RP-188 VANC stream or overlay on video (can be set for single output)

## Prerequisits
* [C++ for Visual Studio 2019 redistribuable](https://aka.ms/vs/17/release/vc_redist.x64.exe)
* [NDI Runtime](http://new.tk/NDIRedistV4Apple) - if you want to use NDI output
* Decklink drivers, if such a card will be in use.

## Screenshot
![UI](https://user-images.githubusercontent.com/1919742/196053982-079e425a-5c35-4b5a-926b-1060f7fcfde4.png)
