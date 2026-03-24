#include "Launcher.h"
#include "../taskcruncher.h"
#include "../ui/msgbox.h"

Launcher::Launcher() {
    _iter = 0;
    _romPath = "";
    _savePath = "";
    _flags = 0;
    _cheatOffset = 0;
    _cheatSize = 0;
    _hb = false;
}

void Launcher::launchRom(std::string romPath, std::string savePath, u32 flags, u32 cheatOffset, u32 cheatSize, bool hb) {
    _iter = 0;
    _romPath = romPath;
    _savePath = savePath;
    _flags = flags;
    _cheatOffset = cheatOffset;
    _cheatSize = cheatSize;
    _hb = hb;

    taskCruncher().push(this->task());
    taskCruncher().process();
}

void Launcher::showModalOk(const std::string& title, const std::string& content) {
    akui::cMessageBox::showModal(title, content, MB_OK);
}