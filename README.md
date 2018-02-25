# Quake 3 1.16 extensions
Custom extensions for patching Quake 3 1.16n

Still under development, beta version

###### Features
- wide screen fix, supports any ratio, icons not stretching
- extended ingame ui with short description of many settings
- added advanced settings menu with many hidden game settings
- extended display menu, brightness options, fps selection, primitives etc.
- extended network menu, rate options, added packets, snaps etc. (automatic or manual setting options)
- misc controls added to menu, like kill, screenshot etc.
- colored crossairs, 36 colors in total
- bright cpm skins for enemies and team with custom coloring
- cpm hitsounds based on damage dealt cg_hitsounds
- draw gun with no bobbing like in cpm cg_drawGun 2
- weapon auto swith after respawn settings
- speedometer options, extended lagometer + ping or display only when packetloss
- display played id on scoreboard, useful for private chatting/following eg. \tell id \follow id
- ping colors, below 50 white, below 100 green, below 250 yellow, below 400 magenta, more than 400 red
- ability to disable chat beep or enemy taunt sounds
- resolving favorite servers by domain name
- rewards display fixed if it's more than 10
- colored server names shifting left bug fixed in server browser menu
- optimization: removed some debug info

###### Command list

- `cg_wideScreenFix 0|1|2|3` - fix perspective for widescreen 0: no fixes 1: fix only icons 2: fix only fov 3: fix fov and icons
- `cg_defaultWeapon 0-9` - default weapon when spawn 0: default 1: gauntlet ...
- `cg_drawPlayerIDs 0|1` - show player id in scoreboard	
- `cg_centerPrintAlpha 1.0-0` - center print transparency
- `cg_crosshairColor 0-35` - crosshair color 0
- `cg_drawSpeed 0|1|2` - speedometer 0: off 1: top corner 2: center screen
- `cg_deadBodyDarken 0|1|2|3` - pm skins becomes gray after death 0: off 1: Just grey 2: BT709 Greyscale 3: Y-Greyscale (PAL/NTSC)
- `cg_chatSound 1|0` - chat beep sound
- `cg_noTaunt 0|1` - enemy taunt sound
- `cg_drawGun 0|1|2` - 0: no gun 1: bobbing gun 2: static gun no bobbing
- `cg_enemyModel_enabled 0|1` - enemy model on\off
- `cg_enemyModel ""` - forcing enemy model "keel/pm", "thankjr" etc. if it's empty and cg_enemyModel_enambled = 1 then it will get pm skins for current player model eg. player model visor/blue it will make him visor/pm + specified colors
- `cg_enemyColors ""` - "1234" 1-rail 2-head 3-torso 4-legs (colors from 0 to 7 or any letter for extended color table, special symbols  `?` - color depending on team (1 - red, 4 - blue, 2- ffa), `!` - same as ? but in ffa color is random, `*` - random color 0-7, `.` - random extended color 0-35)	
- `cg_teamModel ""` - same as cg_enemyModel but for team
- `cg_teamColors ""` - same as cg_enemyColors but for team
- `cg_lagometer 0|1|2|3` - 0 :off 1: default 2: default + ping 3: only when packetloss
- `cg_hitsounds 0|1|2` - 0: default 1: pro mode hi-low 2: low-hi hp hitsounds
- `cg_networkAdjustments 0|1|2` - off, 1: packets 40-60 rate min 8000, 2: packets 60+ rate min 25000 packetdup off (snaps = sv_fps or min 40 in both cases)
- `cg_drawGun 0|1|2` - 0: no gun 1: default bobbing 2: not bobbing cpm style
- `cg_drawScoreBox 1|0` - display score box in low right corner 
- `cg_scoreboard 0|1` - scoreboard 0: default large-small 1: always small
- `cg_sharedConfig 0|1` - in development
- `cgx_debug 0|1|2` - show debug info
- `cgx_version` - show version
