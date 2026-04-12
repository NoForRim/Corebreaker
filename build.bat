@echo off
:: Добавляем скачанный компилятор в список путей Windows для этого окна
set PATH=C:\mingw64\bin;%PATH%

:: Проверка: видит ли скрипт компилятор (выведет версию в консоль)
g++ --version

:: Компиляция
g++ ^
game/main.cpp ^
game/Game_Objects/Game_Object.cpp ^
game/Game_Objects/Player/Player.cpp ^
game/Game_Objects/Projectiles/Bullet.cpp ^
game/Game_Objects/Enemies/Mobs/Dummy.cpp ^
game/Game_Objects/Enemies/Mobs/Crawling_Dummy.cpp ^
game/Game_Objects/Enemies/Mobs/Stalker.cpp ^
game/World/Room.cpp ^
game/engine.cpp ^
-o bin/program.exe ^
-static-libgcc ^
-static-libstdc++ ^
-std=c++17 ^
-Iallegro/include ^
-Lallegro/lib ^
-lallegro_ttf ^
-lallegro_font ^
-lallegro_image ^
-lallegro_primitives ^
-lallegro ^
-lallegro_main 

:: Если компиляция успешна — запускаем
if %ERRORLEVEL% EQU 0 (
    echo [ OK ]: Sborka zavershena! Zapusk...
    cd bin
    program.exe
) else (
    echo [ FAIL ]: Oshibka pri sborke.
    pause
)