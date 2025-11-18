#include "ffb.h"
#include "shared/line.h"

#pragma push(pack, 1)
struct Sys_Device_FFBIo
{
    char pad[16];
    bool isBoot;        // 16
    char pad2[9];       // 17
    bool resultPowerOn; // 25
    char pad3[1];
    bool requestPowerOn; // 27
    char pad4[129];      // 28
    bool isStandby;      // 157
};
#pragma pop(pack)

void Sys_Device_FFBIo_Update_(Sys_Device_FFBIo *ffbIo)
{

}

void ffb_init()
{
    // TODO: Make it work
    // Line::Hook((void *)0x8398c40, (void *)Sys_Device_FFBIo_Update_);
}