# minesweeper-console
A console version of Minesweeper

## Images
![image](https://user-images.githubusercontent.com/98898166/177020655-7002c989-2a68-482f-9e79-0e64b961a299.png)

## Compatibility

- This is compatible with Windows. This program relies on libraries such as `Windows.h` and `conio.h`, which are not portable.
- Consoles that support ANSI Escape Codes are required. cmd on Windows does not support this, instead, use the Terminal application.

## Setup

- A compiled binary can be found for some releases in the attachments of a release.
- If there is no compiled attachment, compile the code yourself by first downloading this repository and compiling using `g++ Minesweeper.cpp`

## Controls

- Use WASD to navigate your cursor
- Use E to select a space. If you select a bomb, you lose.
- Use Q to flag a space. If all bombs are flagged, you win.
- In most consoles, you can use ctrl + scroll wheel to zoom in and out. This may be helpful since the board is small otherwise.

## How to play

Standard Minesweeper rules apply.

Look at this webpage if you do not know how to play Minesweeper: https://minesweepergame.com/strategy/how-to-play-minesweeper.php
