@echo off
(
echo Function FormatTime(secs^)
echo Dim t, a
echo secs = Int(secs^)
echo a = Array(CStr(Right("00" ^& Int(secs / 3600^) Mod 24, 2^)^), CStr(Right("00" ^& Int(secs / 60^) Mod 60, 2^)^), CStr(Right("00" ^& secs Mod 60, 2^)^)^)
echo FormatTime = Join(a, ":"^)
echo End Function

echo dim dl_url, tmpName
echo dl_url = WScript.Arguments(0^)
echo dl_folder = WScript.Arguments(1^)
echo tmpName = WScript.Arguments(2^)

echo dim xHttp: Set xHttp = createobject("Microsoft.XMLHTTP"^)

echo xHttp.Open "HEAD", dl_url, False
echo xHttp.Send

echo if xHttp.Status = 200 then
echo dim size, sizeMb, hcd, pk3name
echo dim fs: set fs = CreateObject("Scripting.FileSystemObject"^)

echo hcd = xHttp.getResponseHeader("Content-Disposition"^)
echo size = xHttp.getResponseHeader("Content-Length"^)
echo pk3name = Trim(Replace(Mid(hcd, InStrRev(hcd,"filename="^) + 10^), """", " "^)^)
echo sizeMb = Round(size / 1024 / 1024, 2^)

echo WScript.Echo "Found: " ^& pk3name ^& " " ^& sizeMb ^& " Mb"

echo if fs.FileExists(dl_folder ^& "\" ^& pk3name^) then
echo WScript.Echo "You have this map already"
echo WScript.Quit 0
echo end if

echo WScript.Echo "Approx time:"
echo WScript.Echo "1MBps speed (125Kb/s) ~ " ^& FormatTime(sizeMb / 0.125^)
echo WScript.Echo "10MBps speed (1.22Mb/s) ~ " ^& FormatTime(sizeMb / 1.250^)
echo WScript.Echo ""
echo WScript.Echo "downloading..."

echo xHttp.Open "GET", dl_url, False
echo xHttp.Send

echo dim bStrm: Set bStrm = createobject("Adodb.Stream"^)

echo with bStrm
echo     .type = 1
echo     .open
echo     .write xHttp.responseBody
echo     .savetofile tmpName, 2
echo     .close
echo end with

echo Set xHttp = nothing

echo fs.MoveFile tmpName, dl_folder ^& "\" ^& pk3name

echo WScript.Echo "done"

echo elseif xHttp.Status = 404 then
echo WScript.Echo "Map no found"
echo WScript.Quit 1
echo else 
echo WScript.Echo "Http status: " ^& xHttp.Status
echo WScript.Quit 1
echo end if
)>tmp.vbs

mode 80, 25 & color 0A
title {X-Mod} map downloader

set "map=%s"
set "mod=%s"
set "tmp=%s"
set "host=%s"
set "page=%s"
set "pars=%s"

echo Attempting to download map %%map%%
cscript //nologo tmp.vbs "http://%%host%%%%page%%" %%mod%% %%tmp%%
set "res=%%errorlevel%%"
del tmp.vbs

if %%res%%==1 (
	echo.
	echo Press any key to open %%host%% to find map
	pause>nul
	del "%%~f0" & explorer "http://%%host%%"
) else (
	echo.
	echo Press any key to start Quake III Arena
	pause>nul
	del "%%~f0" & quake3.exe %%pars%%
)