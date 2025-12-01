#include "engine/GameManager.h"
#include "engine/config/config.hpp"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    if (g_Config.readConfig("CONFIG.txt"))
    {
        if (g_Config.game.system.debugMode)
        {
            AllocConsole();

            FILE* fp;
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONOUT$", "w", stderr);
            freopen_s(&fp, "CONIN$", "r", stdin);

            std::cout << "Debug console initialized.\n";
        }
        GameManager game;
        game.run();
    }
    else
    {
        exit(0);
    }
    return 0;
}