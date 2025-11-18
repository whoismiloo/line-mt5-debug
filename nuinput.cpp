#include "nuinput.h"
#include <string.h>
#include "shared/line.h"
#include <windows.h>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <toml.hpp>
#include <iostream>

bool controllerEnabled = false;
int controllerRepeatDelay = 250;
float controllerDeadzone = 0.05f;
bool mouseEnabled = false;
bool mouseRightClickEnabled = false;

#pragma pack(push, 1)
struct NuInputStateHeader
{
    int index;
    int state;
    int deviceType;
    int deviceSubType;
};

struct NuInputStateController
{
    int press;           // 0
    int release;         // 4
    int on;              // 8
    int off;             // 12
    int repeat;          // 16
    int flags;           // 20
    uint16_t analog[16]; // 24
};

struct NuInputStateKeyboard
{
    int modifier;
    uint16_t key[32];
    uint16_t character[32];
};

struct NuInputStateMouse
{
    int press;
    int release;
    int on;
    int off;
    int doubleClick;
    int value[4];
};

struct NuInputStateJoystickStick
{
    int x;
    int y;
    int z;
    int rx;
    int ry;
    int rz;
    int slider[2];
};
struct NuInputStateJoystick
{
    int press;
    int release;
    int on;
    int off;
    int repeat;
    int flags;
    NuInputStateJoystickStick stick[2];
    int pov; // 104
    int buttons[4];
};

struct NuInputState
{
    NuInputStateHeader header;
    union
    {
        NuInputStateController controller;
        NuInputStateKeyboard keyboard;
        NuInputStateMouse mouse;
        NuInputStateJoystick joystick;
    };
};
#pragma pack(pop)

NuInputState nuInputStates[9]; // Controller (main input), Controller (for flash key input?), Unused, Unused, Keyboard, Unused, Unused, Unused, Mouse
NuInputState *nuInputStateController = &nuInputStates[0];
NuInputState *nuInputStateKeyboard = &nuInputStates[4];
NuInputState *nuInputStateMouse = &nuInputStates[8];

std::queue<SDL_Event> eventQueue = {};
std::mutex eventQueueLock;

std::unordered_map<int, std::chrono::steady_clock::time_point> repeatKeys = {};

int16_t applyAxisDeadzone(int16_t value) {
    float prog = (float)value / (float)INT16_MAX;
    if (prog < controllerDeadzone)
        return 0;
    return value;
}

void nuInputUpdate()
{
    auto controller = &nuInputStateController[0].controller;
    controller->press = 0;
    controller->repeat = 0;
    controller->release = 0;

    auto mouse = &nuInputStateMouse->mouse;
    mouse->press = 0;
    mouse->release = 0;
    memset(mouse->value, 0, sizeof(mouse->value));

    eventQueueLock.lock();

    while (eventQueue.size() > 0)
    {
        SDL_Event event = eventQueue.front();
        eventQueue.pop();

        switch (event.type)
        {
        case SDL_KEYUP:
        case SDL_KEYDOWN:
        {
            bool down = event.type == SDL_KEYDOWN;

            int index = 0;
            switch (event.key.keysym.sym)
            {
            case SDLK_m:
            {
                index = 0x800;
                break;
            }
            }

            if (index != 0)
            {
                if (down)
                {
                    repeatKeys.insert_or_assign(index, std::chrono::steady_clock::now());

                    // If it wasnt down before
                    if (!(controller->on & index))
                    {
                        // We pressed bro.
                        controller->press |= index;

                        // Too lazy to add a repeat handler so just spam lol
                        controller->repeat |= index;

                        controller->on |= index;
                    }
                }
                else
                {
                    auto repeatKey = repeatKeys.find(index);
                    if (repeatKey != repeatKeys.end())
                        repeatKeys.erase(repeatKey);

                    // If it was down before
                    if (controller->on & index)
                    {
                        // Add release flag
                        controller->release |= index;

                        controller->on &= ~index;
                    }
                }
            }

            break;
        }

        case SDL_CONTROLLERAXISMOTION:
        {
            std::vector<uint8_t> axisIndex = {};

            int buttonIndex = 0;
            switch (event.caxis.axis)
            {
            case SDL_CONTROLLER_AXIS_LEFTY:
            {
                axisIndex = {0, 1};
                break;
            }
            case SDL_CONTROLLER_AXIS_LEFTX:
            {
                axisIndex = {3, 2};
                break;
            }
            case SDL_CONTROLLER_AXIS_RIGHTY:
            {
                axisIndex = {4, 5};
                break;
            }
            case SDL_CONTROLLER_AXIS_RIGHTX:
            {
                axisIndex = {7, 6};
                break;
            }
            case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            {
                buttonIndex = 0x10;
                break;
            }
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            {
                buttonIndex = 0x20;
                break;
            }
            }

            if (!axisIndex.empty())
            {
                if (event.caxis.value > 0)
                {
                    controller->analog[axisIndex[0]] = applyAxisDeadzone(event.caxis.value);
                    controller->analog[axisIndex[1]] = 0;
                }
                else if (event.caxis.value < 0)
                {
                    controller->analog[axisIndex[0]] = 0;
                    controller->analog[axisIndex[1]] = applyAxisDeadzone(SDL_clamp(abs(event.caxis.value), 0, INT16_MAX));
                }
                else
                {
                    controller->analog[axisIndex[0]] = 0;
                    controller->analog[axisIndex[1]] = 0;
                }
            }

            if (buttonIndex != 0)
            {
                bool down = event.caxis.value > 16384;
                if (down)
                {
                    repeatKeys.insert_or_assign(buttonIndex, std::chrono::steady_clock::now());

                    // If it wasnt down before
                    if (!(controller->on & buttonIndex))
                    {
                        // We pressed bro.
                        controller->press |= buttonIndex;

                        // Too lazy to add a repeat handler so just spam lol
                        controller->repeat |= buttonIndex;

                        controller->on |= buttonIndex;
                    }
                }
                else
                {
                    auto repeatKey = repeatKeys.find(buttonIndex);
                    if (repeatKey != repeatKeys.end())
                        repeatKeys.erase(repeatKey);

                    // If it was down before
                    if (controller->on & buttonIndex)
                    {
                        // Add release flag
                        controller->release |= buttonIndex;

                        controller->on &= ~buttonIndex;
                    }
                }
            }

            break;
        }
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERBUTTONDOWN:
        {
            bool down = event.type == SDL_CONTROLLERBUTTONDOWN;

            int index = 0;
            switch (event.cbutton.button)
            {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
            {
                index = 0x1000;
                break;
            }
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            {
                index = 0x4000;
                break;
            }
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            {
                index = 0x2000;
                break;
            }
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            {
                index = 0x8000;
                break;
            }
            case SDL_CONTROLLER_BUTTON_Y:
            {
                index = 0x01;
                break;
            }
            case SDL_CONTROLLER_BUTTON_B:
            {
                index = 0x02;
                break;
            }
            case SDL_CONTROLLER_BUTTON_A:
            {
                index = 0x04;
                break;
            }
            case SDL_CONTROLLER_BUTTON_X:
            {
                index = 0x08;
                break;
            }

            case SDL_CONTROLLER_BUTTON_BACK:
            {
                index = 0x200;
                break;
            }
            case SDL_CONTROLLER_BUTTON_START:
            {
                index = 0x100;
                break;
            }

            // L1, R1
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            {
                index = 0x40;
                break;
            }
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            {
                index = 0x80;
                break;
            }

            // L2, R2
            /*case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            {
                index = 0x10;
                break;
            }
            case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            {
                index = 0x20;
                break;
            }*/

            // LEFT STICK, RIGHT STICK
            case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            {
                index = 0x400;
                break;
            }
            case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            {
                index = 0x800;
                break;
            }
            }

            if (index != 0)
            {
                if (down)
                {
                    repeatKeys.insert_or_assign(index, std::chrono::steady_clock::now());

                    // If it wasnt down before
                    if (!(controller->on & index))
                    {
                        // We pressed bro.
                        controller->press |= index;

                        // Too lazy to add a repeat handler so just spam lol
                        controller->repeat |= index;

                        controller->on |= index;
                    }
                }
                else
                {
                    auto repeatKey = repeatKeys.find(index);
                    if (repeatKey != repeatKeys.end())
                        repeatKeys.erase(repeatKey);

                    // If it was down before
                    if (controller->on & index)
                    {
                        // Add release flag
                        controller->release |= index;

                        controller->on &= ~index;
                    }
                }
            }

            break;
        }

        case SDL_MOUSEMOTION:
        {
            mouse->value[0] = event.motion.xrel;
            mouse->value[1] = -event.motion.yrel;
            break;
        }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        {
            bool down = event.type == SDL_MOUSEBUTTONDOWN;

            int index = 0;
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
            {
                index = 0x01;
                break;
            }
            case SDL_BUTTON_RIGHT:
            {
                if (mouseRightClickEnabled)
                    index = 0x02;
                break;
            }
            }

            if (index != 0)
            {
                if (down)
                {
                    // If it wasnt down before
                    if (!(mouse->on & index))
                    {
                        // We pressed bro.
                        mouse->press |= index;

                        mouse->on |= index;
                    }
                }
                else
                {
                    // If it was down before
                    if (mouse->on & index)
                    {
                        // Add release flag
                        mouse->release |= index;

                        mouse->on &= ~index;
                    }
                }
            }

            break;
        }
        }
    }

    // Handle repeat keys, too lazy to add a start timer of 500ms (i think thats how the game handles it)
    for (const auto &[index, start] : repeatKeys)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (elapsed < controllerRepeatDelay)
            continue;
        repeatKeys.insert_or_assign(index, now);

        controller->repeat |= index;
    }

    eventQueueLock.unlock();
}

void nuInputGetState(int device, NuInputState *a1)
{
    switch (device)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    {
        if (controllerEnabled)
        {
            memcpy(a1, &nuInputStateController[device], sizeof(NuInputStateHeader) + sizeof(NuInputStateController));
        }
        else
        {
            memset(a1, 0, sizeof(NuInputStateHeader) + sizeof(NuInputStateController));
        }
        break;
    }
    case 4:
    case 5:
    case 6:
    case 7:
    {
        memcpy(a1, &nuInputStateKeyboard[device - 4], sizeof(NuInputStateHeader) + sizeof(NuInputStateKeyboard));
        break;
    }
    case 8:
    {
        if (mouseEnabled)
        {
            memcpy(a1, nuInputStateMouse, sizeof(NuInputStateHeader) + sizeof(NuInputStateMouse));
        }
        else
        {
            memset(a1, 0, sizeof(NuInputStateHeader) + sizeof(NuInputStateMouse));
        }
        break;
    }
    }
}

void nuinput_sdl(const SDL_Event &event)
{
    eventQueueLock.lock();
    eventQueue.push(event);
    eventQueueLock.unlock();
}

int jmp_alin_init()
{
    return 0;
}

void jmp_alin_term()
{
}

int jmp_alin_pad(int a1, void *a2)
{
    return 0;
}

int jmp_alin_keyboard(int a1, void *a2)
{
    return 0;
}

int jmp_alin_mouse(int a1, void *a2)
{
    return 0;
}

std::unordered_map<const char *, void *> INPUT_FUNC_STUBS = {
    {"alin_init", (void *)jmp_alin_init},
    {"alin_term", (void *)jmp_alin_term},
    {"alin_pad", (void *)jmp_alin_pad},
    {"alin_keyboard", (void *)jmp_alin_keyboard},
    {"alin_mouse", (void *)jmp_alin_mouse}};

#define NUINPUT_DL_ID 521312

bool nuinput_dlopen(const char *lib_name, void **result)
{
    if (strcmp(lib_name, "./alin.dll") == 0)
    {
        *result = (void *)NUINPUT_DL_ID;
        return true;
    }
    return false;
}
bool nuinput_dlsym(void *handle, const char *symbol, void **result)
{
    if ((int)handle != NUINPUT_DL_ID)
        return false;
    for (auto &it : INPUT_FUNC_STUBS)
    {
        if (strcmp(it.first, symbol) == 0)
        {
            *result = it.second;
            return true;
        }
    }
    *result = NULL;
    return true;
}

void nuinput_init()
{
    memset(nuInputStates, 0, sizeof(nuInputStates));
    for (int i = 0; i < 4; i++)
    {
        nuInputStateController[i].header.index = i;
        nuInputStateController[i].header.state = 0;
        nuInputStateController[i].header.deviceType = 1;
        nuInputStateController[i].header.deviceSubType = 0;
    }

    for (int i = 0; i < 4; i++)
    {
        nuInputStateKeyboard[i].header.index = 4 + i;
        nuInputStateKeyboard[i].header.state = 0;
        nuInputStateKeyboard[i].header.deviceType = 2;
        nuInputStateKeyboard[i].header.deviceSubType = 0;
    }

    nuInputStateMouse->header.index = 8;
    nuInputStateMouse->header.state = 0;
    nuInputStateMouse->header.deviceType = 4;
    nuInputStateMouse->header.deviceSubType = 0;

    Line::Hook((void *)0x8ce21f0, (void *)nuInputUpdate);
    Line::Hook((void *)0x8ce22b0, (void *)nuInputGetState);

    toml::table config;
    try
    {
        config = toml::parse_file("nuinput.toml");

        auto controllerConfig = config["controller"];
        if (controllerConfig)
        {
            controllerEnabled = controllerConfig["enabled"].is_boolean() ? controllerConfig["enabled"].as_boolean()->get() : false;
            controllerRepeatDelay = controllerConfig["repeat_delay"].is_integer() ? controllerConfig["repeat_delay"].as_integer()->get() : 250;

            auto deadzone = controllerConfig["deadzone"];
            if (deadzone.is_floating_point()) {
                controllerDeadzone = controllerConfig["deadzone"].as_floating_point()->get();
            } else if (deadzone.is_integer()) {
                controllerDeadzone = controllerConfig["deadzone"].as_integer()->get();
            }
        }

        auto mouseConfig = config["mouse"];
        if (mouseConfig)
        {
            mouseEnabled = mouseConfig["enabled"].is_boolean() ? mouseConfig["enabled"].as_boolean()->get() : false;
            mouseRightClickEnabled = mouseConfig["right_click"].is_boolean() ? mouseConfig["right_click"].as_boolean()->get() : false;
        }
    }
    catch (const toml::parse_error &err)
    {
        printf("Error parsing nuinput.toml: %s\n", err.what());
    }

    printf("nuinput\ncontroller\nenabled: %d\nrepeat delay: %d\ndeadzone: %f\n\nmouse\nenabled: %d\nright click enabled: %d\n\n", controllerEnabled, controllerRepeatDelay, controllerDeadzone, mouseEnabled, mouseRightClickEnabled);
}