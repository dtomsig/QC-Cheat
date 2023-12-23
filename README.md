# QC-Cheat
Quake Champions Wallhack + Aimbot

# Video

This is a clip from my "night of terror". 

![](https://github.com/dtomsig/QC-Cheat/blob/main/Sample%20Run.gif)

# Introduction:

This game hacking stuff is crazy difficult. Quake Champions is on the lower end of the difficulty and I spent tons of hours on this. It was pretty 
fun to get it working. The code isn't perfect especially in chams.cpp which I largely got from someone else. However, I cannot understate how difficult
this whole process is. This is not for the faint of heart.

I have had requests from people to make this public so here it is. I'm not going to give out compiled binaries. I'm here for the intellectual challenge
and I'm not here to provide a cheating service.



# How to Run:

1. See the makefile. I used the nmake program on windows to build the cheat. You should install the appropriate
   cl.exe or Microsoft Compiler program. Just run the command "nmake". 

2. opencv_world740.dll must be added to system path in order to run.

3. After compiling, a inject.dll will appear in the build folder. Use extreme injector (use the close program after injection option to avoid QC anti-cheat).

4. Fire key must be set to num2 in game. 

5. Game must be in fullscreen mode. FOV must be set to 120 and resolution must be 16:9. I only tested the cheat on 1920x1080 resolution.

6. Set player models to magenta and remove the player arrow indicator above the model.

<br/>
Currently, the wallhack uses Direct X hooking. I got the source from unknowncheats (nSeven) and have upgraded it (chams.cpp). The aimbot is color hitscan using OpenCV.
Currently, the aimbot uses an average to find a target.  So it really only works well for one target. I have tested blob detection. That works better and I will 
incorporate the code at a later point. It can handle multiple targets on the screen.
<br/>
<br/>
The current weapon is determined by a pointer chain in offsets.cpp that is current as of 08/15/2013. This pointer chain may not work at a later point in time.
I am currently upgrading the aimbot to use the view matrix instead of mouse_event to lock on target.  My plan is to replace it with the
rotate_view_screen_offset when I get time.

<br/>
<br/>
Currently, only the LG/MG works well with the aimbot. The move_mouse function in 
aimbot.cpp only works well for tracking weapons. Page down must be pressed in order to toggle the aimbot. 
<br/>
<br/>
If I get time, I will update the aimbot to use the view matrix so flick weapons like rail can be supported. I have had success in editing the view matrix but
it's a long process. 

<br/>

# UPDATE: 08/31/2013

I have replaced move_mouse with rotate_view_screen_offset. I figured out the view matrix for quake champions. I have added a pointer chain in offsets.hpp
to get to the view matrix. Flick weapons like the rail gun can now be used.

If I get more time, I will look at adding a pointer chain to get vertical and horizontal FOV to support those FOV's.

