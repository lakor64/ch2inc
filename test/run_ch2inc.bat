@echo off

mkdir ch2inc 2>NUL

for %%i in (.\*.h) do (
	echo Parse %%i
    call :h2incfnc %%i
)

goto quit

:h2incfnc
"ch2inc.exe" --only-int-macros --msvc --nologo -d "ch2drvmasm.dll" -p win -b 32 "%1" "ch2inc\%1.inc"
exit /b 0

:quit
