#include "logger.h"
#include "stringtool.h"

bool isInit = false;

void cLogger::init() {
    FILE* file = fopen("flashcart.log", "w");
    if (file) {
        std::string header = "AKMenu Mako\n";
        fwrite(header.c_str(), header.length(), 1, file);
        fclose(file);
        isInit = true;
    }
}

void cLogger::info(std::string message) {
    if (!isInit) {
        return;
    }

    FILE* file = fopen("flashcart.log", "a");
    if (file) {
        std::string content = formatString("[info] %s\n", message.c_str());
        fwrite(content.c_str(), content.length(), 1, file);
        fclose(file);
    }
}