@echo off

setlocal
set "PATH=..\..\bin_nt;%PATH%"

mkdir vm
cd vm

set cc=lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

%cc% ../ui_main.c
@if errorlevel 1 goto quit
%cc% ../ui_cdkey.c
@if errorlevel 1 goto quit
%cc% ../ui_ingame.c
@if errorlevel 1 goto quit
%cc% ../ui_confirm.c
@if errorlevel 1 goto quit
%cc% ../ui_setup.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_misc.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_lib.c
@if errorlevel 1 goto quit
%cc% ../../game/q_math.c
@if errorlevel 1 goto quit
%cc% ../../game/q_shared.c
@if errorlevel 1 goto quit
%cc% ../ui_gameinfo.c
@if errorlevel 1 goto quit
%cc% ../ui_atoms.c
@if errorlevel 1 goto quit
%cc% ../ui_connect.c
@if errorlevel 1 goto quit
%cc% ../ui_controls2.c
@if errorlevel 1 goto quit
%cc% ../ui_demo2.c
@if errorlevel 1 goto quit
%cc% ../ui_mfield.c
@if errorlevel 1 goto quit
%cc% ../ui_credits.c
@if errorlevel 1 goto quit
%cc% ../ui_credits2.c
@if errorlevel 1 goto quit
%cc% ../ui_menu.c
@if errorlevel 1 goto quit
%cc% ../ui_options.c
@if errorlevel 1 goto quit
%cc% ../ui_display.c
@if errorlevel 1 goto quit
%cc% ../ui_sound.c
@if errorlevel 1 goto quit
%cc% ../ui_network.c
@if errorlevel 1 goto quit
%cc% ../ui_playermodel.c
@if errorlevel 1 goto quit
%cc% ../ui_players.c
@if errorlevel 1 goto quit
%cc% ../ui_playersettings.c
@if errorlevel 1 goto quit
%cc% ../ui_preferences.c
@if errorlevel 1 goto quit
%cc% ../ui_preferences2.c
@if errorlevel 1 goto quit
%cc% ../ui_huds.c
@if errorlevel 1 goto quit
%cc% ../ui_qmenu.c
@if errorlevel 1 goto quit
%cc% ../ui_serverinfo.c
@if errorlevel 1 goto quit
%cc% ../ui_servers2.c
@if errorlevel 1 goto quit
%cc% ../ui_sparena.c
@if errorlevel 1 goto quit
%cc% ../ui_specifyserver.c
@if errorlevel 1 goto quit
%cc% ../ui_splevel.c
@if errorlevel 1 goto quit
%cc% ../ui_sppostgame.c
@if errorlevel 1 goto quit
%cc% ../ui_startserver.c
@if errorlevel 1 goto quit
%cc% ../ui_syscalls.c
@if errorlevel 1 goto quit
%cc% ../ui_team.c
@if errorlevel 1 goto quit
%cc% ../ui_video.c
@if errorlevel 1 goto quit
%cc% ../ui_cinematics.c
@if errorlevel 1 goto quit
%cc% ../ui_spskill.c
@if errorlevel 1 goto quit
%cc% ../ui_addbots.c
@if errorlevel 1 goto quit
%cc% ../ui_removebots.c
@if errorlevel 1 goto quit
%cc% ../ui_loadconfig.c
@if errorlevel 1 goto quit
%cc% ../ui_saveconfig.c
@if errorlevel 1 goto quit
%cc% ../ui_teamorders.c
@if errorlevel 1 goto quit
%cc% ../ui_mods.c
@if errorlevel 1 goto quit
%cc% ../uix_extensions.c
@if errorlevel 1 goto quit


q3asm -f ../ui

cd ../../baseq3
mkdir vm
cd ..
move ui\vm\ui.qvm baseq3\vm\ui.qvm

:quit
cd ..
