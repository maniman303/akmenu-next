#include "personaldata.h"
#include "unicode.h"

u16 colors[] = {0x4E0C, 0x0137, 0x0C1F, 0x7E3F, 0x025F, 0x039E, 0x03F5, 0x03E0, 0x1E80, 0x4769, 0x7AE6, 0x7960, 0x4800, 0x6811, 0x741A, 0x481F};

std::string cPersonalData::username() {
    return unicode_to_local_string((u16*)PersonalData->name, PersonalData->nameLen, NULL);
}

u8 cPersonalData::alarmHour() {
    return PersonalData->alarmHour;
}

u8 cPersonalData::alarmMinute() {
    return PersonalData->alarmMinute;
}

u16 cPersonalData::color() {
    u8 theme = std::max((u8)0, std::min((u8)15, PersonalData->theme));
    u16 res = colors[theme];

    return res | BIT(15);
}