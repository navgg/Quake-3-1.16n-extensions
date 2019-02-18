call compile_cgame
call compile_ui

set "pakname={X-Mod}.pk3"

cd baseq3

..\bin_nt\7za a -tzip -mx5 %pakname% @..\bin_nt\listfile.txt

move %pakname% ..\%pakname%

cd ..