# QC-Cheat
Quake Champions Wallhack + Aimbot

**
Introduction:**

This game hacking stuff is crazy difficult. Quake Champions is on the lower end of the difficulty and I spent tons of hours on this. It was pretty 
fun to get it working. The code isn't perfect especially in chams.cpp which I largely got from someone else. However, I cannot understate how difficult
this whole process is.

I have had requests from people to make this public so here it is.



**How to Run:
**
1. See the makefile. I used the nmake program on windows to build the cheat. You should install the appropriate
   cl.exe or Microsoft Compiler program.

2. opencv_world740.dll must be added to system path in order to run.

3. After compiling, a inject.dll will appear in the build folder. Use extreme injector (use the close program after injection option to avoid QC anti-cheat).


Currently, the wallhack uses Direct X hooking. I got the source from unknowncheats (nSeven) and have upgraded it. The aimbot is color hitscan using opencv. Currently, 
the aimbot uses an average to find a target.  So it really only works well for one target. I have tested blob detection. That works better and I will 
incorporate the code at a later point. It can handle multiple targets on screen.




The current weapon is determined by a pointer chain in offsets.cpp that is current as of 08/15/2013. This pointer chain may not work at a later point in time.


I am currently upgrading the aimbot to use the view matrix instead of mouse_event to lock on target. Currently, only the LG/MG works well with the aimbot. The move_mouse function in 
aimbot.cpp only works well for tracking weapons. Page down must be pressed in order to toggle the aimbot.

If I get time, I will update the aimbot to use the view matrix so flick weapons like rail can be supported. I have had success in editing the view matrix but
it's a long process. 



**
Video: **

This is a clip from my "night of terror". 

