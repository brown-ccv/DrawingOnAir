@echo off


REM Copy this script and edit the following line to start your own
REM unix-style script from a .bat file.

set PROGNAME=kuchora/apps/forceServer/run cit304


set USER=%USERNAME%
set CYGWIN=tty notitle glob ntea ntsec smbntsec check_case:adjust
set PATH=%PATH%:/usr/X11R6/bin:.:/bin
set DISPLAY=:0
set ARCH=cygwin

IF EXIST Y:\u\%USER% (
  set HOME=Y:\u\%USER%
) 


REM Check for Cygwin installed in a non-default location
IF EXIST F:\cygwin\bin (
  set CYGWINBIN=F:\cygwin\bin
)
IF EXIST D:\cygwin\bin (
  set CYGWINBIN=D:\cygwin\bin
)
IF EXIST C:\cygwin\bin (
  set CYGWINBIN=C:\cygwin\bin
)

REM Check for $G
IF EXIST F:\gfx\tools\WIN32\bin (
  set GBIN=/cygdrive/f/gfx/tools/WIN32/bin
  set GSRC=/cygdrive/f/gfx/tools/WIN32/src
)
IF EXIST D:\gfx\tools\WIN32\bin (
  set GBIN=/cygdrive/d/gfx/tools/WIN32/bin
  set GSRC=/cygdrive/d/gfx/tools/WIN32/src
)
IF EXIST C:\gfx\tools\WIN32\bin (
  set GBIN=/cygdrive/c/gfx/tools/WIN32/bin
  set GSRC=/cygdrive/c/gfx/tools/WIN32/src
)

%CYGWINBIN%\tcsh.exe -c "%GSRC%/%PROGNAME%

