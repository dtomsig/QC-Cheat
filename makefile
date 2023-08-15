#### BUILD ALL ####
build: dll_cheat external_cheat 



#### PICK DLL OR EXTERNAL. WILL MAKE A RECURSIVE CALL WITH CORRECT FLAG SET. #####
dll_cheat: 
   nmake dll DEFINE_FLAG="/D DLL_CHEAT"

external_cheat:
    nmake external_exe DEFINE_FLAG="/D EXTERNAL_CHEAT"

external_cheat_debug: 
    nmake external_exe DEFINE_FLAG="/D EXTERNAL_CHEAT /D IMG_DEBUG"



#### FINAL PRODUCT RECIPES ####
dll: aimbot chams client library offsets os_util
    link /DLL /out:.\build\inject.dll aimbot.obj chams.obj client.obj offsets.obj \
                                   os_util.obj Advapi32.lib Comdlg32.lib D3DCompiler.lib d3d11.lib \
                                   Gdi32.lib User32.lib lib\imgui\imgui.lib \
                                   lib\minhook\libMinHook.x64.lib lib\opencv2\opencv_world470.lib\
    
external_exe:  aimbot client gui offsets library os_util  
    cl aimbot.obj client.obj gui.obj offsets.obj os_util.obj Advapi32.lib Comdlg32.lib d3d11.lib \
       User32.lib lib\imgui\imgui.lib lib\opencv2\*.lib /link \
       /out:.\build\cheat.exe /SUBSYSTEM:WINDOWS /nologo
 


#### OBJECTS FOR DLL AND EXTERNAL CHEATS ####
aimbot:
    cl /c aimbot.cpp /EHsc /nologo /I .\include $(DEFINE_FLAG)
    
chams:
    cl /c chams.cpp /EHsc /nologo /I .\include
    
client:
    cl /c client.cpp /EHsc /nologo /I .\include $(DEFINE_FLAG)
    
esp:
    cl /c esp.cpp /EHsc /nologo
	
gui:
    cl /c gui.cpp /EHsc /nologo /I .\include
    
library:
    cl /Fo.\lib\imgui\ /c include\imgui\*.cpp /nologo 
    lib .\lib\imgui\*.obj /out:.\lib\imgui\imgui.lib
    del .\lib\imgui\*.obj

offsets:
    cl /c offsets.cpp /EHsc /nologo

os_util:
    cl /c os_util.cpp /EHsc /nologo

#### CLEAN ####
clean:
    del *.exe *.obj lib\imgui\*.lib build\*.exe build\*.dll