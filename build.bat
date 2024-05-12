@echo off
@set SOURCE=src/main.cpp
@set OUT_EXE=main
@set OUT_DIR=build
@set LIBS=user32.lib gdi32.lib
@set INCLUDES=/Isrc\libs\imgui
@set ARGS=/std:c++20
IF NOT EXIST %OUT_DIR%\ MKDIR %OUT_DIR% 

cl -DNDEBUG=1 -Zi %ARGS% %INCLUDES% %SOURCE% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS%