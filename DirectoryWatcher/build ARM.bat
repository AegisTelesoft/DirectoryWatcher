IF NOT EXIST "%cd%\build\" (
    mkdir build
) 
set OUTPUT=%cd%\build\

C:\Android\sdk\cmake\3.6.3155560\bin\cmake.exe -H"C:\Users\%username%\Desktop\DW\DirectoryWatcher\DirectoryWatcher" -B"C:\Users\%username%\Desktop\DW\DirectoryWatcher\DirectoryWatcher\build" -G"Android Gradle - Ninja" -DANDROID_ABI=armeabi -DANDROID_NDK="C:\Android\sdk\ndk-bundle" -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="C:\Users\%username%\Desktop\DW\DirectoryWatcher\DirectoryWatcher\build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=C:\Android\sdk\cmake\3.6.3155560\bin\ninja.exe -DCMAKE_TOOLCHAIN_FILE=C:\Android\sdk\ndk-bundle\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=15 -DANDROID_TOOLCHAIN=clang

cmake --build "%cd%\build\"

pause