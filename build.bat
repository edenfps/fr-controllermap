@echo off
setlocal

set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if not exist "%VCVARS%" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat"
)
if not exist "%VCVARS%" (
    echo Could not find vcvarsall.bat
    exit /b 1
)

call "%VCVARS%" x86 || exit /b 1

set ROOT=%~dp0
set OUT=%ROOT%Client\version.dll
set BUILD=%ROOT%build

if not exist "%BUILD%" mkdir "%BUILD%"

cl /nologo /LD /O2 /MT /EHsc ^
  /Fe"%OUT%" ^
  /Fo"%BUILD%\\" ^
  /Fd"%BUILD%\\" ^
  "%ROOT%src\config.cpp" ^
  "%ROOT%src\hooks.cpp" ^
  "%ROOT%src\input_state.cpp" ^
  "%ROOT%src\dllmain.cpp" ^
  /link /DEF:"%ROOT%src\version.def" /IMPLIB:"%BUILD%\version.lib" user32.lib xinput.lib kernel32.lib winmm.lib

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Built %OUT%
endlocal
