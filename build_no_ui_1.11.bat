call compile_cgame
cd ..
rem call compile_ui
rem cd ..

set "pakname=pak2aX.pk3"

cd baseq3

..\bin_nt\7za a -tzip -mx9 %pakname% @..\release_noui_listfile.txt

move %pakname% ..\%pakname%

cd ..