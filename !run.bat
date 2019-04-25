@echo off
rem
call compile_cgame.bat
rem call compile_ui.bat

rem 
set "gamedir=C:\Games\Quake 3 Arena 1.16n\"
rem set "gamedir=C:\Games\Quake 3 Arena 1.16n clean\"
rem set "gamedir=C:\Games\Quake\Quake 3 Arena 1.16n Vanilla\Vanilla Quake III Arena 1.16n\"

rem set "gamedir=D:\Games\Quake 3\"
rem set "gamedir=D:\Games\Quake3Gracz\"

set "pakname={X-Mod}.pk3"
bin_nt\7za a -tzip -mx0 "%gamedir%baseq3\%pakname%" %CD%\baseq3\vm

set "sv=quake3.exe +set sv_pure 0 +set sv_maxclients 32 +set r_fullscreen 0"
set "rb=+addbot anarki 3 r +addbot mynx 3 r +addbot lucy 3 r +addbot bones 3 r"
set "bb=+addbot anarki 3 b +addbot mynx 3 b +addbot lucy 3 b +addbot bones 3 b"

rem start /D "%gamedir%" "" %sv% +connect localhost:27960
rem start /D "%gamedir%" "" %sv% +connect sodmod.ml:27964 +set developer 1
rem start /D "%gamedir%" "" %sv% +nointro
rem start /D "%gamedir%" "" %sv% +set g_gametype 0 +devmap q3dm1 +addbot anarki 1 +addbot mynx 1
rem start /D "%gamedir%" "" %sv% +set g_gametype 1 +devmap q3dm1 +addbot anarki 1
rem 
start /D "%gamedir%" "" %sv% +set g_gametype 3 +devmap q3dm6 %rb% %rb% %bb% %bb% %fs% +set bot_minplayers 10
rem start /D "%gamedir%" "" %sv% +set g_gametype 4 +devmap 13box %rb% %bb%

rem +cg_debuganim 1
rem +devmap q3dm1
rem +set cg_debugEvents 1 +set cg_debugDelag 1 +set cg_showMiss 1 +set cg_drawBBox 1 %fs%