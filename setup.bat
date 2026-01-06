@echo off

REM Download vcpkg
git clone https://github.com/Microsoft/vcpkg.git

REM Bootstrap vcpkg
call vcpkg\bootstrap-vcpkg.bat

REM Install dependencies
vcpkg\vcpkg install

REM Return to project root
cd ..

echo Setup complete!
pause
