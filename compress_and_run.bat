set "gamedir=C:\Games\Quake 3 Arena 1.16n\"
set "pakname=pak2X.pk3"

bin_nt\7za a -tzip -mx9 %pakname% %CD%\baseq3\vm

move "%pakname%" "%gamedir%baseq3\%pakname%"

START /D "%gamedir%" /MAX "" quake3.exe +devmap q3dm1 +set cg_debugEvents 1 +set cg_debugDelag 1 +set cg_showMiss 1 +set cg_drawBBox 1
rem +cg_debuganim 1
rem +devmap q3dm1