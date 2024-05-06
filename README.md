[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/LogitechWingmanArduinoPedals/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/LogitechWingmanArduinoPedals/blob/master/README.RU.md)
← Choose language | Выберите язык

# Logitech WingMan Xbox Gamepad
Xbox gamepad emulator for Logitech WingMan steering wheel (will probably fit other steering wheels as well). Supports [external modified pedals](https://github.com/r57zone/XboxExternalPedals).

# Working modes
There are 4 operating modes of the program:
* Steering wheel + standard pedals;
* Steering wheel + external pedals (DInput or Arduino)
* Steering wheel without pedals, instead of them there are buttons on the steering wheel.

Buttons can be changed in the profile file. You can switch them in the program. You can also set the default mode and profile in the `Config.ini` configuration file.

## Setup
1. Install [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Install Microsoft Visual C++ Redistributable 2017 or later.
3. Connect the steering wheel and standard pedals or [external modified pedals](https://github.com/r57zone/XboxExternalPedals).
4. If you use non-standard pedals, you can enable it in the configuration file by entering the number of the COM port of the pedals or by turning on the search for DInput pedals.
5. Run the program and check the work in the `XInputTest` program.
6. If necessary, calibrate the steering wheel by entering the data from the program into the configuration file. Data can be displayed by pressing `ALT + F9`.

## Download
>Version for Windows 10, 11.

**[Download](https://github.com/r57zone/LogitechWingmanArduinoPedals/releases)**

## Feedback
`r57zone[at]gmail.com`