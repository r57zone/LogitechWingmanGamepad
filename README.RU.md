[![EN](https://user-images.githubusercontent.com/9499881/33184537-7be87e86-d096-11e7-89bb-f3286f752bc6.png)](https://github.com/r57zone/LogitechWingmanArduinoPedals/) 
[![RU](https://user-images.githubusercontent.com/9499881/27683795-5b0fbac6-5cd8-11e7-929c-057833e01fb1.png)](https://github.com/r57zone/LogitechWingmanArduinoPedals/blob/master/README.RU.md)

# Logitech WingMan с Arduino педалями
Xbox эмулятор для руля Logitech WingMan, [с модифицироваными педалями](https://github.com/r57zone/XboxExternalPedals), переделанными на работу через Arduino. Если у вас обычный руль, с педалями, то используйте [x360ce](https://www.x360ce.com/) для эмуляции Xbox контроллера, если педалей нет, то можно использовать этот проект, тригеры можно эмулировать кнопками на руле. Работает на базе драйвера [ViGEm](https://github.com/ViGEm).

# Режимы работы
Первый режим работает, если Arduino педали подключены.

Передние 2 кнопки руля работают как `B` и `A`, верхние 2 кнопки руля работают как `Назад` и `Старт`, а задние работают как `X` и `Y`.



Второй режим работает включается, если Arduino педали не подключены.

Передние 2 кнопки руля работают как триггеры Xbox геймпада и вы можете играть без педалей. Верхние 2 кнопки работают как `Назад` и `Старт`, а задние как `B` и `A`.

## Настройка
1. Установите [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases).
2. Установите Microsoft Visual C++ Redistributable 2017 или новее.
3. Подключите руль и [внешние Arduino педали](https://github.com/r57zone/XboxExternalPedals) (опционально).
4. Введите номер COM-порта педалей, в конфигурационный файл `Config.ini`.
5. Запустите программу и проверьте работу в программе XInputTest.
6. При необходимости откалибруйте руль, введя данные из программы в конфигурационный файл. Данные можно вывести по нажатию `ALT + F9` и по нажатию `ALT + F8`.

## Загрузка
>Версия для Windows 10.

**[Загрузить](https://github.com/r57zone/LogitechWingmanArduinoPedals/releases)**

## Обратная связь
`r57zone[собака]gmail.com`