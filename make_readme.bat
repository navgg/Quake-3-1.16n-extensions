set /p version=<baseq3\version.txt

cd baseq3\doc

powershell -Command "(gc 1-basic_info.txt) -replace 'Quake III X-Mod 1.16n \d\.(.+)b', 'Quake III X-Mod 1.16n %version%' | Out-File -encoding ASCII 1-basic_info.txt"

copy 1-basic_info.txt+2-comand_list.txt+3-unlagged_commands.txt+4-authors.txt+5-changes.txt ..\..\{X-Mod}readme.txt
copy 5-changes.txt ..\..\changes.txt

cd ..\..