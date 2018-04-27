==========================================================================================================
========================================Quake III X-Mod 1.16 0.80b========================================
==========================================================================================================

Custom extensions for patching Quake 3 1.16n (beta version)

==========================================================================================================
                                        Installation & Requirements
==========================================================================================================

- requires quake 3 arena version 1.16n
- to install unzip {X-Mod}.pk3 to baseq3 folder of your quake 3 arena installation path

==========================================================================================================
                                                Features
==========================================================================================================

- wide screen fix, supports any ratio, icons not stretching, fov fix for widescreens
- extended ingame ui with short description of many settings
- added advanced settings menu with many hidden game settings
- extended display menu, brightness options, fps selection, primitives etc.
- extended network menu, rate options, added packets, snaps etc. (automatic or manual setting options)
- misc controls added to menu, like kill, screenshot etc.
- colored crossairs, 36 colors in total
- bright cpm skins for enemies and team with custom coloring
- cpm hitsounds based on damage dealt
- draw gun with no bobbing like in cpm
- weapon auto swith after respawn settings
- speedometer options, extended lagometer + ping or display only when packetloss
- display played id on scoreboard, useful for private chatting/following eg. \tell id \follow id
- ping colors, below 50 white, below 100 green, below 250 yellow, below 400 magenta, more than 400 red
- ability to disable chat beep or enemy taunt sounds
- resolving favorite servers by domain name
- integrated unlagged 2.01 (client compatible with noghost911 servers)
- optimization: removed some debug info

==========================================================================================================
                                                Buf fixes
==========================================================================================================

- rewards display fixed if it's more than 10
- cinematics playback fixed for widescreens (only from cinematics menu and singleplayer games)
- colored server names shifting left bug fixed in server browser menu
- sarge/default model in team games
- display cg_shadow 1 when cg_marks 0

==========================================================================================================
                                               Command list
==========================================================================================================

cg_wideScreenFix 0|1|2|3 - fix perspective for widescreen 0: no fixes 1: fix only icons 2: fix only fov 3: fix fov and icons
cg_defaultWeapon 0-9 - default weapon when spawn 0: default 1: gauntlet ...
cg_drawPlayerIDs 0|1 - show player id in scoreboard
cg_centerPrintAlpha 1.0-0 - center print transparency
cg_crosshairColor 0-35 - crosshair color "" - default, 0: black 1: red etc.
cg_drawSpeed 0|1|2 - speedometer 0: off 1: top corner 2: center screen
cg_deadBodyDarken 0|1|2|3 - pm skins becomes gray after death 0: off 1: Just grey 2: BT709 Greyscale 3: Y-Greyscale (PAL/NTSC)
cg_chatSound 1|0 - chat beep sound
cg_noTaunt 0|1 - enemy taunt sound
cg_drawGun 0|1|2 - 0: no gun 1: bobbing gun 2: static gun no bobbing
cg_enemyModel_enabled 0|1 - enemy model on\off
cg_enemyModel "" - forcing enemy model "keel/pm", "thankjr" etc. if it's empty and cg_enemyModel_enambled = 1 then 
		it will get pm skins for current player model eg. player model visor/blue it will make him visor/pm + specified colors
cg_enemyColors "" - "1234" 1-rail 2-head 3-torso 4-legs (colors from 0 to 7 or any letter for extended color table, 
		special symbols ? - color depending on team (1 - red, 4 - blue, 2- ffa), ! - same as ? but in ffa color is random, 
		* - random color 0-7, . - random extended color 0-35)
cg_teamModel "" - same as cg_enemyModel but for team
cg_teamColors "" - same as cg_enemyColors but for team
cg_lagometer 0|1|2|3 - 0 :off 1: default 2: default + ping 3: only when packetloss
cg_hitsounds 0|1|2 - 0: default 1: pro mode hi-low 2: low-hi hp hitsounds
cg_networkAdjustments 0|1|2|3 - 0: off 1: min rate 8000 packets 30 2: packets 40-60 rate min 16000 3: packets 60-100 rate min 25000 packetdup off (snaps = sv_fps or min 40 in all cases if it's on)
cg_drawGun 0|1|2 - 0: no gun 1: default bobbing 2: not bobbing cpm style
cg_drawScoreBox 1|0 - display score box in low right corner
cg_scoreboard 0|1 - scoreboard 0: default large-small 1: always small
cg_drawAccuracy 0|1 - total weapon accuracy
cgx_debug 0|1 - show debug info
cgx_version - show version

==========================================================================================================
                                            Unlagged 2.01 command list
==========================================================================================================

cg_delag 1|0 - If this is set to "1" (the default), your lag with all instant-hit weapons will be compensated for. 
You can also set it for individual weapons. These are the important values:

1 - Everything
2 - Shotgun
4 - Machinegun
8 - Lightning Gun
16 - Railgun
Add the values together. For example, if you want lag compensation for just the lightning gun and the machinegun, you would set cg_delag to "12".

cg_delag_projectileNudge 0..999 - projectile time nudge, set it to your average ping on server may make easier dodging rockets

cg_delag_optimizePrediction 1|0 - optimized prediction, if your CPU bottleneck this may give some serious perfomance improvement

cg_delag_cmdTimeNudge 0..999 - number of milliseconds you would like the server to "nudge" the time of your instant-hit attacks. 
For example, if you feel that the server overcompensates for your ping, you might try setting it to 25. 
That will effectively add 25ms of lag.

==========================================================================================================
                                                     Authors
==========================================================================================================

X-Mod - (c) 2018 NaViGaToR (322)
Unlagged 2.01 - (c) 2006 Neil “haste” Toronto
CPMA - (c) 2000-2010 Challenge World, (c) 2016-2018 The ProMode Team
Quake 3 Arena - (c) 1999-2005 Id Software 

==========================================================================================================
                                                     Changes
==========================================================================================================
0.70b

added 1920x1200 in menu
added credits menu
added scroll in serverinfo
added scroll in driverinfo
added unlag\compability with nemesis servers
added unlag with bma servers
added message if game couldn't load map
max fps limit set to 500

fixed cg_teamModel "" init
fixed static gun when jump
fixed players jittering with timeNudge other than 0 (removed timeNudge extrapolation of unlagged 2.01)
fix black color names (dark gray color used instead black)

0.80b

mod renamed to X-Mod
bright models replaced with high quality models
lagometer moved from advanced to network menu
removed "Gibs" from advanced menu (cg_gibs)
cg_delag_optimizePrediction set to 0 by default
symbol ! in cg_enemyModel for rail uses player's specified color

added generation of "download_map.bat" script in game folder if map not found (opens in browser link to worldspawn to download map)
added cg_sharedConfig 1|0 fixes problem with saving configs after game exit (0: off 1:automatically saves q3config.cfg to baseq3 folder after pressing 'leave arena' or 'exit game', to save manually use \writesharedconfig command)
added command \autorecord to record demo (which do the following "g_syncronousClients 1; record <demoname>; g_syncronousClients 0")
added "Predict items pickup" to network menu (cg_predictitems)
added hints for some network settings

fixed bug when enemy models was not loading (only three colorful lines was visible)
fixed small visual inconsistencies of missle weapons
fixed fov for models in credits & player menu for widescreens and etc.
fixed display of players icons when loading (if more than 8)
fixed handicap display in large scoreboard
fixed win\loss display in large scoreboard
fixed showplayerids toggle in menu