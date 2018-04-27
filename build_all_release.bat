call compile_cgame
cd ..
call compile_ui
cd ..

set "pakname={X-Mod}.pk3"

cd baseq3

..\bin_nt\7za a -tzip -mx9 %pakname% @..\release_listfile.txt

move %pakname% ..\%pakname%

cd ..