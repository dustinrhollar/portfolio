#ifndef _ENGINE_CORE_SETTINGS_H
#define _ENGINE_CORE_SETTINGS_H

namespace Settings
{
    // Specifies the active window
    // The game is only active in Game mode
    // CreateStartup: create the engine startup file
    // CreateProject: create a new project file 
    enum class EngineWindowMode : u8
    {
        Game,
        CreateStartup,
        CreateProject,
    };
    
    // NOTE(Dustin): Maybe put in a better place?
    struct MapleStartupFile
    {
        char       dir[2048];
        char       name[512];       // startup project name
    };
    
    file_global EngineWindowMode g_window_mode = EngineWindowMode::Game;
};


#endif //_ENGINE_CORE_SETTINGS_H
