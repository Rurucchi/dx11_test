:: doc: https://learn.microsoft.com/en-us/windows/win32/direct3dtools/dx-graphics-tools-fxc-using

:: fxc /Od /Zi /T vs_5_1 /E vs /Fo triangle.vs.fxc triangle.hlsl
:: fxc /Od /Zi /T ps_5_1 /E ps /Fo triangle.ps.fxc triangle.hlsl


@set OUT_DIR=build

::  ----------------- add shader compiling here

:: TRIANGLE
@set SOURCE=src/render/shader/triangle.hlsl

::ps
@set OUT_FXC=triangle.ps.fxc
@set ENTRY=ps
@set PROFILE=ps_5_0

fxc /T %PROFILE% /E %ENTRY% /Fo %OUT_DIR%/%OUT_FXC% %SOURCE%

::vs
@set OUT_FXC=triangle.vs.fxc
@set ENTRY=vs
@set PROFILE=vs_5_0

fxc /T %PROFILE% /E %ENTRY% /Fo %OUT_DIR%/%OUT_FXC% %SOURCE%