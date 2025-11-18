#include "camera.h"
#include "shared/line.h"
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define WIDTH 640
#define HEIGHT 480
#define BUFFER_SIZE WIDTH *HEIGHT * 3

#define WIDTH_EVENT 320
#define HEIGHT_EVENT 320
#define BUFFER_SIZE_EVENT WIDTH_EVENT *HEIGHT_EVENT * 3

uint8_t cameraBuffer[BUFFER_SIZE];
uint8_t cameraBufferEvent[BUFFER_SIZE_EVENT];

enum TrimType
{
    TRIM_TYPE_DEFAULT = 0,
    TRIM_TYPE_EVENT
};

TrimType trimType = TRIM_TYPE_DEFAULT;

int Sys_Device_USBCamera_IsBooted()
{
    return 1;
}

int Sys_Device_USBCamera_IsError()
{
    return 0;
}

void Sys_Device_USBCamera_Start()
{
}

void Sys_Device_USBCamera_Stop()
{
}

void *Sys_Device_USBCamera_GetBuffer()
{
    if (trimType == TRIM_TYPE_EVENT)
        return cameraBufferEvent;
    return cameraBuffer;
}

int Sys_Device_USBCamera_GetImageWidth()
{
    if (trimType == TRIM_TYPE_EVENT)
        return WIDTH_EVENT;
    return WIDTH;
}

int Sys_Device_USBCamera_GetImageHeight()
{
    if (trimType == TRIM_TYPE_EVENT)
        return HEIGHT_EVENT;
    return HEIGHT;
}

void Sys_Device_USBCamera_Update()
{
}

void Sys_Device_USBCamera_SetTrimType(TrimType type)
{
    trimType = type;
}

void Sys_Device_USBCamera_Reset() {
    trimType = TRIM_TYPE_DEFAULT;
}

void camera_init()
{
    Line::Hook((void *)0x83d7700, (void *)Sys_Device_USBCamera_IsBooted);
    Line::Hook((void *)0x83d7840, (void *)Sys_Device_USBCamera_IsError);

    Line::Hook((void *)0x83d7750, (void *)Sys_Device_USBCamera_Start);
    Line::Hook((void *)0x83d77a0, (void *)Sys_Device_USBCamera_Stop);

    Line::Hook((void *)0x83d77f0, (void *)Sys_Device_USBCamera_GetBuffer);

    Line::Hook((void *)0x83d7a30, (void *)Sys_Device_USBCamera_GetImageWidth);
    Line::Hook((void *)0x83d7ab0, (void *)Sys_Device_USBCamera_GetImageHeight);

    Line::Hook((void *)0x83d7e50, (void *)Sys_Device_USBCamera_Update);
    
    Line::Hook((void *)0x83d7990, (void *)Sys_Device_USBCamera_SetTrimType);
    
    Line::Hook((void *)0x83d7f60, (void *)Sys_Device_USBCamera_Reset);

    memset(cameraBuffer, 0, BUFFER_SIZE);
    memset(cameraBufferEvent, 0, BUFFER_SIZE_EVENT);

    {
        int width, height, comp;
        unsigned char *imgData = stbi_load("camera.png", &width, &height, &comp, 3);
        if (!imgData)
        {
            printf("camera.png not found\n");
        }
        else
        {
            printf("camera.png width = %d, height = %d, comp = %d\n", width, height, comp);
            if (width != WIDTH || height != HEIGHT)
            {
                printf("needs to be %dx%dpx\n", WIDTH, HEIGHT);
            }
            else if (comp != 3)
            {
                printf("comp isnt 3?\n");
            }
            else
            {
                memcpy(cameraBuffer, imgData, BUFFER_SIZE);
            }
            stbi_image_free(imgData);
        }
    }

    {
        int width, height, comp;
        unsigned char *imgData = stbi_load("camera_event.png", &width, &height, &comp, 3);
        if (!imgData)
        {
            printf("camera_event.png not found\n");
        }
        else
        {
            printf("camera_event.png width = %d, height = %d, comp = %d\n", width, height, comp);
            if (width != WIDTH_EVENT || height != HEIGHT_EVENT)
            {
                printf("needs to be %dx%dpx\n", WIDTH_EVENT, HEIGHT_EVENT);
            }
            else if (comp != 3)
            {
                printf("comp isnt 3?\n");
            }
            else
            {
                memcpy(cameraBufferEvent, imgData, BUFFER_SIZE_EVENT);
            }
            stbi_image_free(imgData);
        }
    }
}