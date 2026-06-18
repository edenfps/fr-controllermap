#pragma once

#include <windows.h>

struct ControllerConfig {
    int enabled;
    int controllerIndex;
    float deadzone;
    float moveDeadzone;
    float cameraSensitivity;
    float cameraInvertY;
    int requireRightMouseForCamera;
    int useAnalogMovementMemory;
    unsigned int moveForwardAddr;
    unsigned int strafeAddr;
    unsigned int moveSpeedScaleAddr;
    int vkMoveForward;
    int vkMoveBackward;
    int vkStrafeLeft;
    int vkStrafeRight;
    int vkJump;
    int vkInteract;
    int vkTabTarget;
    int vkAutoRun;
    int vkActionBar[9];
    int vkDockAtlas;
    int vkDockCharacter;
    int vkDockJobs;
    int vkDockQuests;
    int vkDockInventory;
    int vkAltActionBar;
};

extern ControllerConfig g_Config;

void LoadConfig();
