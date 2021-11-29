# Flare Farming Bot

Referenced in https://gamehacking.academy/lesson/4/4.

A hack for Flare version 1.12 that will automatically move the player towards the nearest enemy and then attack until the enemy is dead.

It accomplishes this through a thread that constantly compares the current player's X and Y position to an enemy's position. Depending on the position, the mouse cursor is then set to a certain angle and SendInput is used to send a mouse down event to the game.

After injecting this hack, go in game and hold down the "M" key. Your character will then begin running toward the closest enemy and begin attacking them.

This must be injected into the Flare process to work. One way to do this is to use a DLL injector. Another way is to enable AppInit_DLLs in the registry.
