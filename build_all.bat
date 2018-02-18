call compile_cgame
cd ..
call compile_ui
cd ..

set "pakname=pak2X.pk3"

bin_nt\7za a -tzip -mx9 %pakname% %CD%\baseq3\*