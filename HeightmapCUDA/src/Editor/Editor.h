#ifndef _EDITOR_H
#define _EDITOR_H

namespace Settings 
{
    struct MapleStartupFile;
};

// Assumes ImGui Frame in Platform & Graphics has began
void EditorEntry(struct AssetManager *asset_manager);

// Returns true when submit is pressed
bool EditorEngineSetup(Settings::MapleStartupFile *project);
bool EditorProjectSetup(struct AssetProject *project);

#endif //_EDITOR_H
