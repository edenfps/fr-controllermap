#pragma once

#include <windows.h>
#include <XInput.h>

struct InputState {
    float leftX;
    float leftY;
    float rightX;
    float rightY;
    float leftMag;
    float rightMag;
    int buttons[32];
    int syntheticKeys[256];
    int syntheticRmb;
    int controllerConnected;
    XINPUT_STATE raw;
};

extern InputState g_Input;

void InitializeInputThread();
void ShutdownInputThread();
void UpdateSyntheticKeys();
