call compile_cgame
cd ..
rem call compile_ui
rem cd ..

set "pakname={X-Mod}.pk3"

cd baseq3

..\bin_nt\7za a -tzip -mx9 %pakname% @..\bin_nt\listfile_noui.txt

move %pakname% ..\%pakname%

cd ..