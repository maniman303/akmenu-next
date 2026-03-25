#pragma once

#include <nds/ndstypes.h>
#include <string>
#include <memory>
#include "../language.h"
#include "../ui/msgbox.h"
#include "../taskworker.h"

#define LOADER_NOT_FOUND_TITLE LANG("loader", "not found")
#define LOADER_NOT_FOUND_MESSAGE LANG("loader", "not found message")

// static inline void printLoaderNotFound(std::string loaderPath) {
//     akui::cMessageBox::showModal(LANG("loader", "not found"), loaderPath, MB_OK);
// }

// static inline void printError(std::string errorMsg) {
//     akui::cMessageBox::showModal(LANG("loader", "error"), errorMsg, MB_OK);
// }

class Launcher : public TaskWorker {
  public:
    Launcher();
    virtual ~Launcher() = default;

    virtual std::unique_ptr<TaskWorker> task() const = 0;
    void launchRom(std::string romPath, std::string savePath, u32 flags, u32 cheatOffset, u32 cheatSize, bool hb);

  protected:
    void showModalOk(const std::string& title, const std::string& content);

    u16 _iter;
    std::string _romPath;
    std::string _savePath;
    u32 _flags;
    u32 _cheatOffset;
    u32 _cheatSize;
    bool _hb;
};
