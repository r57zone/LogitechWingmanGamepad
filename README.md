[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/LogitechWingmanArduinoPedals/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/LogitechWingmanArduinoPedals/blob/master/README.RU.md)
← Choose language | Выберите язык

# Logitech WingMan с Arduino педалями
Xbox emulator for Logitech WingMan wheel [with modified pedals](https://github.com/r57zone/XboxExternalPedals) changed to work via Arduino. If you have a regular steering wheel with pedals, then use [x360ce](https://www.x360ce.com/) to emulate an Xbox controller, if there are no pedals, then you can use this project, triggers can be emulated with the buttons on the steering wheel. Powered by [ViGEm](https://github.com/ViGEm) driver.

# Working modes
The first mode works if Arduino pedals are connected.

The front 2 buttons on the steering wheel work like `B` and `A`, the top 2 buttons on the steering wheel work like `Back` and `Start`, and the back buttons work like `X` and `Y`.



The second mode is enabled when Arduino pedals are not connected.

The front 2 buttons of the steering wheel work as triggers for the Xbox gamepad and you can play without pedals. The top 2 buttons work as `Back` and `Start`, and the back buttons as `B` and `A`.

## Setup
1. Install [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Install Microsoft Visual C++ Redistributable 2017 or later.
3. Connect the steering wheel and [external Arduino pedals](https://github.com/r57zone/XboxExternalPedals) (optional).
4. Enter the pedal's COM port number into the `Config.ini` configuration file.
5. Run the program and check the work in the XInputTest program.
6. If necessary, calibrate the steering wheel by entering the data from the program into the configuration file. Data can be displayed by pressing `ALT + F9` and by pressing `ALT + F8`.

## Download
>Version for Windows 10.

**[Download](https://github.com/r57zone/LogitechWingmanArduinoPedals/releases)**

## Feedback
`r57zone[at]gmail.com`