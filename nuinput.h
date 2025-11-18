#pragma once
#include <SDL2/SDL.h>

void nuinput_sdl(const SDL_Event& event);

bool nuinput_dlopen(const char *lib_name, void **result);
bool nuinput_dlsym(void *handle, const char *symbol, void **result);

void nuinput_init();