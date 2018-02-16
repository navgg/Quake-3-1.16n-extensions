# Quake-3-1.16-extensions
Custom extensions for patching Quake 3 1.16n

Still under development, alpha version

###### Command list

- `cgx_wideScreenFix 1|0` - fix perspective for widescreen
- `cgx_defaultWeapon 0-9` - default weapon when spawn 0: default 1: gauntlet ...
- `cgx_drawPlayerIDs 0|1` - show player id in scoreboard	
- `cg_centerPrintAlpha 1.0-0` - center print transparency
- `cg_crosshairColor 0-35` - crosshair color 0
- `cg_drawSpeed 0|1|2` - speedmeter 0: off 1: top corner 2: center screen
- `cg_deadBodyDarken 0|1|2|3` - pm skins becomes gray after death 0: off 1: Just grey 2: BT709 Greyscale 3: Y-Greyscale (PAL/NTSC)
- `cg_chatSound 1|0` - chat beep sound
- `cg_noTaunt 0|1` - enemy taunt sound
- `cg_drawGun 0|1|2` - 0: no gun 1: bobbing gun 2: static gun
- `cg_enemyModel_enabled 0|1` - enemy model on\off
- `cg_enemyModel ""` - forcing enemy model "keel/pm", "thankjr" etc.
- `cg_enemyColors ""` - "1234" 1-rail 2-head 3-torso 4-legs (colors from 0 to 7, special symbols ? - color depending on team, ! - same as ? but in ffa color is random, * - random color)	
- `cg_lagometer 0|1|2` - 0: off 1: netgraph 2: netgraph + client ping
- `cg_hitsounds 0|1|2` - 0: default 1: pro mode hi-low 2: low-hi hp hitsounds

ping colors, below 50 white, below 100 green, below 200 yellow, below 350 magenta, more than 350 red
