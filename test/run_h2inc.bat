@echo off

mkdir h2inc 2>NUL

for %%i in (.\*.h) do (
    call :h2incfnc %%i
)

goto quit

:h2incfnc
h2inc.exe /WIN32 /nologo /Fa"h2inc\%1.inc" "%1"
exit /b 0

:quit
