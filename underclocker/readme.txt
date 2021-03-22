gamecpu (prx to change cpu speed within a game)
version 0.3

***

This PRX file adds the ability to change the CPU & bus speed of your PSP whilst a game is running, by pressing a selectable button combination.
The module will switch between the speeds 111/50MHz, 166/80MHz, 222/111MHz, 266/133MHz, and 333/166MHz.
It can also automatically underclock the PSP to save power.
After changing the speed, it is displayed at the bottom of the screen for a few seconds.

This method of printing on top of the screen is not guaranteed between different games, however it appears to work fine with GTA, although it is a little flickery.

CHANGELOG
0.2a --> 0.3
- added analog stick support to auto underclocking
--- analog stick movement is now detected, and will prevent it underclocking automatically
--- additionally, if you move the analog while it is underclocked, it will go back to the original clock speed as with the other buttons
- rewritten screen blit function 
--- now supports games that use 16 bit and 32 bit graphics modes
--- flickering reduced
--- font improved
- added a configuration option to set default cpu and bus speed for the start of a game
-- see gamecpu_cfg.txt for more info

0.2 --> 0.2a
~ fixed configuration
--- can now use button names rather than their hex codes in the configuration file (see gamecpu_cfg.txt)

0.1 --> 0.2
- added automatic underclocking
--- this feature will automatically underclock the PSP's processor to 100mhz (bus speed 50mhz) after a period of inactivty which you can set in the configuration file. can be switched on or off, as described below
- added configuration
--- check gamecpu_cfg.txt, which goes into the root of your memory stick. the prx will not work correctly if this file is missing.
--- you can configure the trigger buttons for cpu change, the auto underclock (on/off), and the auto underclock timeout
--- READ THE INSTRUCTIONS CAREFULLY in gamecpu_cfg.txt or you will get it wrong.

INSTALLATION

- under 3.XX OE
-- 1) copy gamecpu.prx to the folder ms0:/seplugins (create this folder on the root of your memstick if it doesnt exist)
-- 2) open, or create the file "game.txt" inside this folder along with gamecpu.prx
-- 3) add the following line: "ms0:/seplugins/gamecpu.prx"
-- 4) save the file
-- 5) switch off the PSP
-- 6) whilst holding the R trigger, switch on the PSP
-- 7) navigate to "plugins" on the recovery menu and press X to enable gamecpu.prx which should be listed. If not, start from step 1.

- under devhook
-- 1) copy gamecpu.prx to ms0:/dh/kd
-- 2) go into ms0:/dh/xxx/f0/kd/pspbtcnf_game_dh.txt
---- (where xxx = your emulated firmware, e.g. 271, 282, etc.)
-- 3) find the line "ms0:/dh/kd/devhook.prx"  OR  "/kd/devhook.prx #DEVHOOK"
-- 4) after this line, add the line "ms0:/dh/kd/gamecpu.prx"

- under both
-- copy gamecpu_cfg.txt to the root of your memory stick
-- make sure it is set up correctly

The plugin is tested and confirmed working under 3.10 OE-A2