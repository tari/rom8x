@echo off
copy C:\Asm\Tasm\Tasm80.tab /A
copy C:\Asm\Tasm\Tasm32.exe /B
copy C:\Asm\Tasm\ti83plus.inc /A
copy C:\Asm\Tasm\DevPac8X.com /B

Tasm32 -80 -b -a -dprogName="\"83PBE\"" -dpageNum="\"1\"" -dbootPage=$1F template.z80 G83PBE1.bin
Tasm32 -80 -b -a -dprogName="\"83PSE\"" -dpageNum="\"1\"" -dbootPage=$7F template.z80 G83PSE1.bin
Tasm32 -80 -b -a -dprogName="\"84PBE\"" -dpageNum="\"1\"" -dbootPage=$3F template.z80 G84PBE1.bin
Tasm32 -80 -b -a -dprogName="\"84PBE\"" -dpageNum="\"2\"" -dbootPage=$2F template.z80 G84PBE2.bin
Tasm32 -80 -b -a -dprogName="\"84PSE\"" -dpageNum="\"1\"" -dbootPage=$7F template.z80 G84PSE1.bin
Tasm32 -80 -b -a -dprogName="\"84PSE\"" -dpageNum="\"2\"" -dbootPage=$6F template.z80 G84PSE2.bin
Tasm32 -80 -b -a -dprogName="\"84CSE\"" -dpageNum="\"1\"" -dbootPage=$FF template.z80 G84CSE1.bin
Tasm32 -80 -b -a -dprogName="\"84CSE\"" -dpageNum="\"2\"" -dbootPage=$EF template.z80 G84CSE2.bin

DevPac8X G83PBE1
DevPac8X G83PSE1
DevPac8X G84PBE1
DevPac8X G84PBE2
DevPac8X G84PSE1
DevPac8X G84PSE2
DevPac8X G84CSE1
DevPac8X G84CSE2

del Tasm80.tab > NUL
del Tasm32.exe > NUL
del ti83plus.inc > NUL
del DevPac8X.com > NUL

del template.lst
del G83PBE1.bin > NUL
del G83PSE1.bin > NUL
del G84PBE1.bin > NUL
del G84PBE2.bin > NUL
del G84PSE1.bin > NUL
del G84PSE2.bin > NUL
del G84CSE1.bin > NUL
del G84CSE2.bin > NUL