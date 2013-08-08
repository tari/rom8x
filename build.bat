@echo off

call :make 83PBE 1 1F
call :make 83PSE 1 7F
call :make 84PBE 1 3F
call :make 84PBE 2 2F
call :make 84PSE 1 7F
call :make 84PSE 2 6F
call :make 84CSE 1 FF
call :make 84CSE 2 EF

:make
setlocal
set NAME=-DprogName="%1"
set NUM=-DpageNum="%2"
set PAGE=-DbootPage=0%3h

mkdir %1
set OUT=%1/G%1%2.8xp
echo %OUT%
spasm %NAME% %NUM% %PAGE% template.z80 %OUT%

endlocal
goto :eof
