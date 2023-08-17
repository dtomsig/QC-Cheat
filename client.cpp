#include <Windows.h>
#include <iostream>
#include <string>
#include "aimbot.hpp"
#include "chams.hpp"
#include "esp.hpp"
#include "gui.hpp"
#include "minhook/MinHook.h" 
#include "os_util.hpp"
#include "offsets.hpp"

#define GUI_TGL    0001
#define AIMBOT_TGL 0010
#define ESP_TGL    0100
#define CHAMS_TGL  1000

#ifdef EXTERNAL_CHEAT
HANDLE game_handle;
#endif 

uintptr_t base_address;
static int AIMBOT_STATE, CHAMS_STATE, ESP_STATE, GUI_STATE;

int cheat_init();
DWORD __stdcall run_cheat_loop(LPVOID);
void set_error_state();


#ifdef EXTERNAL_CHEAT

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE h_instance, PSTR, INT cmd_show)
{
    if(AllocConsole() == 0)
        set_error_state();
    
    if(AttachConsole(GetCurrentProcessId() == 0))
        set_error_state();
        
	if(freopen("CONIN$", "r", stdin) == NULL)
        set_error_state();
	if(freopen("CONOUT$", "w", stdout) == NULL)
        set_error_state();
    if(freopen("CONOUT$", "w", stderr) == NULL)
        set_error_state();
    if(cheat_init() == 0)
        set_error_state();
    run_cheat_loop(NULL);
}


void set_error_state()
{
    std::cerr << "Error detected " << std::endl;
    while(1);
}

#endif



#ifdef DLL_CHEAT

BOOL __stdcall DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{ 
	switch (dwReason)
	{
        case DLL_PROCESS_ATTACH:
            cheat_init();        
            DisableThreadLibraryCalls(hModule);
            CreateThread(NULL, 0, install_chams, NULL, 0, NULL);
            CreateThread(NULL, 20971520, run_cheat_loop, NULL, 0, NULL);
            break;
     
        case DLL_PROCESS_DETACH:
            CreateThread(NULL, 0, disable_chams, NULL, 0, NULL);
            break;
    }
	return TRUE;
}

#endif



int cheat_init()
{
	DWORD process_id;
    
    find_process_id(process_id, _base_executable);
    
    if(process_id == 0)
        return 0;
    
    // For external cheats, an open handle to the process is required.
    #ifdef EXTERNAL_CHEAT
    game_handle = OpenProcess(PROCESS_VM_READ, 0, process_id);
    
    if(game_handle == NULL)
        return 0;
    #endif

    base_address = get_mdle_begin_addr(process_id, _base_module);
    return 1;
}



DWORD __stdcall run_cheat_loop(LPVOID)
{
    while(true)
    {
        if(GetAsyncKeyState(VK_END) & 0x0001)
            GUI_STATE = GUI_STATE ^ GUI_TGL;
        
        if((GetAsyncKeyState(VK_NEXT) & 0x0001))
        {
            AIMBOT_STATE = AIMBOT_STATE ^ AIMBOT_TGL;
            
            //Actions taken as aimbot is turned on.
            if(AIMBOT_STATE == 0010)
                init_aimbot();
            else
                destroy_aimbot();
        }
        
        if((GetAsyncKeyState(VK_PRIOR) & 0x0001))
            CHAMS_STATE = CHAMS_STATE ^ CHAMS_TGL;
            

        switch(CHAMS_STATE | ESP_STATE | AIMBOT_STATE | GUI_STATE)
        {
            case 0010:
                scan_set_weapon();
                scan_set_target();
                adj_if_firing();
                break;
        }
    }
}

