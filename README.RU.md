[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/LogitechWingmanArduinoPedals/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/LogitechWingmanArduinoPedals/blob/master/README.RU.md)

# Logitech WingMan Xbox Gamepad
Эмулятор Xbox геймпада для руля Logitech WingMan (вероятно подойдет и к другим рулям). Поддерживаются [внешние модифицированные педали](https://github.com/r57zone/XboxExternalPedals). Работает на базе драйвера [ViGEm](https://github.com/ViGEm).

# Режимы работы
Есть 4 режима работы программы:
* Руль + стандартные педали;
* Руль + внешние педали (DInput или Arduino)
* Руль без педалей, вместо них кнопки на руле.

Кнопки можно переностроить в файле профилей. Переключить их можно в программе. Также можно установить режим и профиль по умолчанию в конфигурационном файле `Config.ini`.

## Настройка
1. Установите [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Установите Microsoft Visual C++ Redistributable 2017 или новее.
3. Подключите руль и стандартные педали или [внешние модифицированные педали](https://github.com/r57zone/XboxExternalPedals).
4. Если вы используете нестандартные педали, то включить можно в конфигурационном файле введя номер COM-порта педалей или включив поиск DInput педалей.
5. Запустите программу и проверьте работу в программе `XInputTest`.
6. При необходимости откалибруйте руль, введя данные из программы в конфигурационный файл. Данные можно вывести по нажатию `ALT + F9`.

## Загрузка
>Версия для Windows 10, 11.

**[Загрузить](https://github.com/r57zone/LogitechWingmanArduinoPedals/releases)**

## Обратная связь
`r57zone[собака]gmail.com`