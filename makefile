#### BUILD ALL ####
build: dll_cheat 



#### PICK DLL OR DEBUG. WILL MAKE A RECURSIVE CALL WITH CORRECT FLAG SET. #####
dll_cheat: 
   nmake dll

dll_cheat_debug: 
    nmake dll DEFINE_FLAG="/D IMG_DEBUG"
    

#### FINAL PRODUCT RECIPES ####
dll: aimbot chams client library offsets os_util
    link /DLL /out:.\build\inject.dll aimbot.obj chams.obj client.obj offsets.obj \
                                   os_util.obj Advapi32.lib Comdlg32.lib D3DCompiler.lib d3d11.lib \
                                   Gdi32.lib User32.lib lib\imgui\imgui.lib \
                                   lib\minhook\libMinHook.x64.lib lib\opencv2\opencv_world470.lib\


#### OBJECTS FOR DLL CHEAT####
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