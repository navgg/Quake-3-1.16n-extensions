set "gamedir=C:\Games\Quake 3 Arena 1.16n\"
set "pakname=pak2X.pk3"

bin_nt\7za a -tzip -mx9 %pakname% %CD%\baseq3\vm

move "%pakname%" "%gamedir%baseq3\%pakname%"

START /D "%gamedir%" /MAX "" quake3.exe 
rem +devmap q3dm1