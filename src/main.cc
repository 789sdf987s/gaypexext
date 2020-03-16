#include "charfix.h"
#include <windows.h>

#include "globals.h"
#include "overlay.h"
#include "console.h"
#include "utils.h"
#include "activation/activation.h"
//#include "activation/activation.h"

GlobalVars* g_Vars;
Driver* g_Drv;

void OfflineOffsets() 
{
    // updated: 3/11/2020
    g_Vars->offsets.lastupdate = 1583943005;
    
    g_Vars->offsets.localPlayer = 0x1dea658;
    g_Vars->offsets.entityList = 0x1883c38;
    g_Vars->offsets.punchAngle = 0x2308;
    g_Vars->offsets.viewMatrix = 0x1b3bd0;
    g_Vars->offsets.viewRender = 0xcb011e0;
    g_Vars->offsets.studioHdr = 0x10e0;

    g_Vars->offsets.viewAngles = 0x23d0;
    g_Vars->offsets.cameraPos = 0x1da8;
    g_Vars->offsets.vecOrigin = 0x14c;
    g_Vars->offsets.absVelocity = 0x140;
    g_Vars->offsets.activeWeapon = 0x1944;
    g_Vars->offsets.boneClass = 0xee0;
    g_Vars->offsets.propName = 0x518;
    g_Vars->offsets.bleedout = 0x2590;
    g_Vars->offsets.lifeState = 0x730;
    g_Vars->offsets.teamNum = 0x3f0;
    g_Vars->offsets.health = 0x3e0;
    g_Vars->offsets.shield = 0x170;
    g_Vars->offsets.flags = 0x98;

    g_Vars->offsets.bulletSpeed = 0x1d2c;
    g_Vars->offsets.bulletGravity = 0x1d34;
}

void OfflineSettings() 
{
    g_Vars->settings.maxfps = 60;
    
    g_Vars->settings.visuals.enabled = true;
    g_Vars->settings.visuals.box = true;
    g_Vars->settings.visuals.health = true;
    g_Vars->settings.visuals.shield = true;
    g_Vars->settings.visuals.showTarget = true;
    g_Vars->settings.visuals.fovCircle = false;
    
    g_Vars->settings.aim.enabled = true;
    g_Vars->settings.aim.aimkey = VK_XBUTTON2;
    g_Vars->settings.aim.maxfov = 10.0f;
    g_Vars->settings.aim.nopunch = true;
    g_Vars->settings.aim.maxdistance = 5000;
    
    g_Vars->settings.aim.smooth = true;
    g_Vars->settings.aim.divider = 200;
    
    g_Vars->settings.aim.gravity = true;
    g_Vars->settings.aim.velocity = true;
    
    g_Vars->settings.aim.teamCheck = true;
    g_Vars->settings.aim.knockCheck = true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{            
    Sleep(1000);
    
    g_Vars = new GlobalVars();
    g_Vars->activated = false;
    g_Vars->shouldExit = false;
    g_Vars->ready = false;

    OfflineOffsets();
    OfflineSettings();

    if (TESTBUILD) 
    {
        Console::WriteLog("Starting in debug mode...");
        Console::WriteLog("Allocating and connecting to system console...");
        AllocConsole();
        freopen_s((FILE**)stdin,  "CONIN$", "r",  stdin);
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); 
        printf("GVAR size: %i", sizeof(GlobalVars));
        
        g_Vars->activated = true;
    } else 
    {
        // TODO: load offsets from server
        g_Vars->activated = Activation::Activate();
    }

    //TODO: remove
    AllocConsole();
    freopen_s((FILE**)stdin,  "CONIN$", "r",  stdin);
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    
    Console::WriteLog("Creating threads...");
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Overlay::Loop, nullptr, 0, nullptr);
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Console::DisplayLoop, nullptr, 0, nullptr);

    Console::WriteLog("Getting program info...");
    int pid = Utils::FindProcess(L"r5apex.exe");
    Console::WriteLog("Game PID is %i", pid);
    uintptr_t baseaddr = Utils::GetBase(pid, "r5apex.exe");
    Console::WriteLog("Game base address is %llx", baseaddr);
    g_Vars->apexBase = baseaddr;

    if (!pid || !baseaddr) 
    {
        g_Vars->shouldExit = true;
        Console::WriteLog("Failed to get process information");
    }
    
    Console::WriteLog("Trying to connect to the driver...");
    g_Drv = new Driver();
    g_Drv->Init(pid);   
    int test = g_Drv->Read<int>(g_Vars->apexBase);
    Console::WriteLog("Test read: %i", test);

    Console::WriteLog("Using offsets from %s", Utils::UnixDate(g_Vars->offsets.lastupdate).c_str());

    if (!g_Vars->shouldExit)
        g_Vars->ready = true;

    while (!g_Vars->shouldExit) 
    {
        Sleep(1000);
    }

    g_Vars->ready = false;
    Console::WriteLog("Exit signal received. Exiting.");
    Sleep(5000);

    return 0;
}