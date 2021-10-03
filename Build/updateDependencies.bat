@echo OFF
setlocal enabledelayedexpansion
set XFONT_COMPILER_PATH=%cd%

rem --------------------------------------------------------------------------------------------------------
rem Set the color of the terminal to blue with yellow text
rem --------------------------------------------------------------------------------------------------------
COLOR 8E
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
powershell write-host -fore Cyan Welcome I am your XFONT COMPILER dependency updater bot, let me get to work...
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
echo.

:DOWNLOAD_DEPENDENCIES
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
powershell write-host -fore White XFONT COMPILER - DOWNLOADING DEPENDENCIES
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
echo.

rem ------------------------------------------------------------
rem XBMP_TOOLS
rem ------------------------------------------------------------
:XBMP_TOOLS
rmdir "../dependencies/xbmp_tools" /S /Q
git clone --recurse-submodules -j8 https://github.com/LIONant-depot/xbmp_tools.git "../dependencies/xbmp_tools"
if %ERRORLEVEL% GEQ 1 goto :ERROR
cd ../dependencies/xbmp_tools/build
if %ERRORLEVEL% GEQ 1 goto :ERROR
call updateDependencies.bat "return"
if %ERRORLEVEL% GEQ 1 goto :ERROR
cd /d %XFONT_COMPILER_PATH%
if %ERRORLEVEL% GEQ 1 goto :ERROR

rem ------------------------------------------------------------
rem MSDFGEN
rem ------------------------------------------------------------
:CRUNCH
rmdir "../dependencies/msdfgen" /S /Q
git clone --recurse-submodules -j8 https://github.com/Chlumsky/msdfgen "../dependencies/msdfgen"
if %ERRORLEVEL% GEQ 1 goto :ERROR

rem ------------------------------------------------------------
rem XRESOURCE_PIPELINE
rem ------------------------------------------------------------
:XRESOURCE_PIPELINE
rmdir "../dependencies/xresource_pipeline" /S /Q
git clone https://github.com/LIONant-depot/xresource_pipeline.git "../dependencies/xresource_pipeline"
if %ERRORLEVEL% GEQ 1 goto :ERROR

rem ------------------------------------------------------------
rem XCORE
rem ------------------------------------------------------------
:XCORE
rmdir "../dependencies/xcore" /S /Q
git clone https://gitlab.com/LIONant/xcore.git "../dependencies/xcore"
if %ERRORLEVEL% GEQ 1 goto :ERROR
cd ../dependencies/xcore/builds
if %ERRORLEVEL% GEQ 1 goto :ERROR
call UpdateDependencies.bat "return"
if %ERRORLEVEL% GEQ 1 goto :ERROR
cd /d %XFONT_COMPILER_PATH%
if %ERRORLEVEL% GEQ 1 goto :ERROR

rem ------------------------------------------------------------
rem DONE
rem ------------------------------------------------------------
:DONE
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
powershell write-host -fore White XFONT COMPILER - DONE!!
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
goto :PAUSE

:ERROR
powershell write-host -fore Red ------------------------------------------------------------------------------------------------------
powershell write-host -fore Red XFONT COMPILER - ERROR!!
powershell write-host -fore Red ------------------------------------------------------------------------------------------------------

:PAUSE
rem if no one give us any parameters then we will pause it at the end, else we are assuming that another batch file called us
if %1.==. pause