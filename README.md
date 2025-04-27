# supercool.ch8
a chip-8 ~~emulator~~ interpreter written in c (using raylib for graphics)

# current progress
passes tests 1-6 from [Timendus](https://github.com/Timendus/chip8-test-suite), targeting chip-8 only (not schip/xo-chip)

no audio

# screenshots
![image](https://github.com/user-attachments/assets/2c793abe-ebd7-470e-a59b-35c62250c135)
![image](https://github.com/user-attachments/assets/38fc9494-71be-4b5a-916f-17d047339d85)
![image](https://github.com/user-attachments/assets/4be12f51-f7cf-4e2d-8f11-7fedd939abf6)

# build & run
1. install visual studio code
2. git clone this repo
3. open the project folder in visual studio code
4. press F5 to run in debug mode

# usage
when run the program executes the rom which it has read. to change the rom, recompile and edit the filename in the call to `fopen()` in main.c.

to enable a quirk, define the corresponding flag (found near the top of main.c)
```C
#define Q_VF_RESET // turn on VF reset quirk
#define Q_MEMORY // turn on FX65 / FX55 index register quirk
#define Q_DISPLAY_WAIT // turn on display wait quirk
#define Q_CLIPPING // turn on sprite clipping quirk
#define Q_SHIFTING // turn on 8XY6/8XYE VX <= VY quirk
#define Q_JUMPING // turn on BNNN jump to VX + NNN instruction quirk
```
