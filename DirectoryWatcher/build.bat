IF NOT EXIST "%cd%\build\" (
    mkdir build
) 
set OUTPUT=%cd%\build\

cmake -H"%cd%" -B"%OUTPUT%"

cmake --build "%OUTPUT%"
pause