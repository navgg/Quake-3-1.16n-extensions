call build_all_release.bat
call make_readme.bat

set /p version=<%cd%\baseq3\version.txt
set "zipname=X-Mod-v%version%.zip"

(
echo {X-Mod}.pk3
echo {X-Mod}readme.txt
)>list_tmp

bin_nt\7za a -tzip -mx9 -sdel %zipname% @list_tmp
del list_tmp