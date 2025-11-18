#include <cstdio>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "audio.h"
#include "bana.h"
#include "camera.h"
#include "compress.h"
#include "ffb.h"
#include "gl.h"
#include "hasp.h"
#include "input.h"
#include "mq.h"
#include "net.h"
#include "nuinput.h"
#include "window.h"

#include "shared/config.h"
#include "shared/line.h"

#include <glad/gl.h>
#include "touch.h"
#include "nvpm.h"

#define DllExport __declspec(dllexport)

int jmp_fork()
{
    return 1;
}

void Sys_LogMes(int type, char *msg, ...)
{
    switch (type)
    {
    case 1:
    {
        printf("[INF] ");
        break;
    }
    case 2:
    {
        printf("[WRN] ");
        break;
    }
    case 3:
    {
        printf("[SYS] ");
        break;
    }
    case 4:
    {
        printf("[ERR] ");
        break;
    }
    }

    va_list va;
    va_start(va, msg);
    vprintf(msg, va);
    va_end(va);
}

int althman_get_my_pid()
{
    return (int)pthread_self();
}

char *(__cdecl *old_getenv)(const char *name);
char *jmp_getenv(const char *name)
{
    if (strcmp(name, "__NU_SCREEN_WINDOWED") == 0)
        return (char *)"1";
    return old_getenv(name);
}

int send_command(int a1, int a2, int a3, int a4, char *)
{
    return 0;
}

int lumen_debug_printfmt(const char *format, ...)
{
    printf("[LMN] ");

    va_list va;
    va_start(va, format);
    int len = vprintf(format, va);
    va_end(va);
    return len;
}

void(__cdecl *old_glTexParameteri)(GLenum target, GLenum pname, GLenum param);
void jmp_glTexParameteri(GLenum target, GLenum pname, GLenum param)
{
    if (param == GL_CLAMP)
        param = GL_CLAMP_TO_EDGE;
    old_glTexParameteri(target, pname, param);
}

void *lib_cg_so = NULL;
void *lib_cgGL_so = NULL;
void *lib_curl = NULL;

void(__cdecl *old_std_string_assign)(void *a1, void *a2);
void jmp_std_string_assign(void *a1, void *a2)
{
    if (!a2)
        return;
    // check for empty string
    if (*(int *)a2 == 0)
        return;

    old_std_string_assign(a1, a2);
}

extern "C"
{
    DllExport void OnInitialize(int version, void **func_tables)
    {
        printf("Plugin OnInitialize\n");

        Line::Init(func_tables);

        config.load();

        SDL_SetMainReady();

        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
        SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");

        if (config.inputBackground)
        {
            SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
        }
        else
        {
            SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "0");
        }
        SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT, "0");

        if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_HAPTIC) != 0)
        {
            printf("Can't initialize SDL2: %s\n", SDL_GetError());
            return;
        }

        SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

        SDL_GameControllerEventState(SDL_ENABLE);
        SDL_JoystickEventState(SDL_ENABLE);

        printf("Initialized SDL2\n");

        window_init();

        lib_curl = dlopen("libcurl.dll", RTLD_NOW);
        if (!lib_curl)
            printf("can't load libcurl.dll\n");

        lib_cg_so = dlopen("cg.dll", RTLD_NOW);
        if (!lib_cg_so)
            printf("can't load cg.dll\n");

        lib_cgGL_so = dlopen("cgGL.dll", RTLD_NOW);
        if (!lib_cgGL_so)
            printf("can't load cgGL.dll\n");
    }

    DllExport void OnPreExecute(const char *lib_name, uintptr_t base_address)
    {
        printf("Plugin OnPreExecute %s\n", lib_name);

        if (strcmp(lib_name, "WMN4i_attract_aging") == 0)
        {
            if (config.bana)
                bana_init();
            if (config.camera)
                camera_init();
            compress_init();
            ffb_init();
            hasp_init(config.terminal ? "267681069420" : "267680069420");
            if (config.input)
                input_init();
            net_init();
            nuinput_init();
            if (config.touch)
                touch_init();

            Line::Hook((void *)0x80a18c0, (void *)Sys_LogMes);
            Line::Hook((void *)0x80586c0, (void *)jmp_fork);
            Line::Hook((void *)0x8d2a510, (void *)althman_get_my_pid);
            Line::Hook((void *)0x8059570, (void *)jmp_getenv, (void **)&old_getenv);

            Line::Hook((void *)0x8059970, (void *)jmp_glTexParameteri, (void **)&old_glTexParameteri);
            
            Line::Hook((void *)0x8d2a2e0, (void *)send_command);

            // mucha
            // mucha5.local
            Line::Patch((void *)0x8fa2fa8, {0x6D, 0x75, 0x63, 0x68, 0x61, 0x35, 0x2E, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x00});
            Line::Patch((void *)0x8f98ca7, {0x41, 0x56, 0x4B, 0x6F, 0x6E, 0x74, 0x6F, 0x6C, 0x33, 0x38, 0x38, 0x00});

            Line::Hook((void *)0x8de5c30, (void *)lumen_debug_printfmt);

            if (!config.terminalProxy)
            {
                if (config.terminal)
                {
                    // Disable proxy because it locks the game
                    Line::Patch((void *)0x80a3542, {0x90,
                                                    0x90,
                                                    0x90,
                                                    0x90,
                                                    0x90});
                    // dont set rpc to 0.0.0.0
                    Line::Patch((void *)0x80a4368, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
                }

                // disable using terminal proxy
                Line::Patch((void *)0x80a4379, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
            }
            
            void *lib_gs = dlopen("gspatchdebug.dll", RTLD_NOW);
            if (!lib_gs)
                printf("can't load gspatchdebug.dll\n");
        }
    }

    DllExport bool OnDlOpen(const char *lib_name, void **result)
    {
        printf("OnDlOpen %s\n", lib_name);

        if (audio_dlopen(lib_name, result))
            return true;
        if (nuinput_dlopen(lib_name, result))
            return true;

        *result = NULL;
        return true;
    }

    DllExport bool OnDlSym(void *handle, const char *symbol, void **result)
    {
        printf("OnDlSym %s\n", symbol);

        if (audio_dlsym(handle, symbol, result))
            return true;
        if (nuinput_dlsym(handle, symbol, result))
            return true;

        return false;
    }

    DllExport bool OnResolveSymbol(const char *symbol, void **result)
    {
        if (mq_resolve(symbol, result))
            return true;

        if (strstr(symbol, "curl_") == symbol)
        {
            *result = (void *)dlsym(lib_curl, symbol);
            return true;
        }

        if (strstr(symbol, "cg") == symbol)
        {
            void *res = (void *)dlsym(lib_cg_so, symbol);
            if (res)
            {
                *result = res;
                return true;
            }

            res = (void *)dlsym(lib_cgGL_so, symbol);
            if (res)
            {
                *result = res;
                return true;
            }
        }

        void *res = gl_resolve(symbol);
        if (res)
        {
            *result = res;
            return true;
        }
        
        res = nvpm_resolve(symbol);
        if (res)
        {
            *result = res;
            return true;
        }

        return false;
    }

    // Skibidi rizz fix
    int __cdecl __mingw_vsscanf(const char *_Str, const char *Format, va_list argp)
    {
        return vsscanf(_Str, Format, argp);
    }

    int __cdecl __mingw_vsnprintf(char *_DstBuf, size_t _MaxCount, const char *_Format,
                                  va_list _ArgList)
    {
        return vsnprintf(_DstBuf, _MaxCount, _Format, _ArgList);
    }

    int __cdecl __mingw_vsprintf(char *a1, const char *a2, va_list a3)
    {
        return vsprintf(a1, a2, a3);
    }
};
