#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ControllerConfig g_Config = {};

static int ReadInt(const char* value, int fallback) {
    if (!value || !*value) {
        return fallback;
    }
    return atoi(value);
}

static float ReadFloat(const char* value, float fallback) {
    if (!value || !*value) {
        return fallback;
    }
    return (float)atof(value);
}

static unsigned int ReadHex(const char* value, unsigned int fallback) {
    if (!value || !*value) {
        return fallback;
    }
    return (unsigned int)strtoul(value, NULL, 0);
}

static void ReadPath(char* out, size_t outSize, const char* section, const char* key, const char* fileName) {
    GetPrivateProfileStringA(section, key, "", out, (DWORD)outSize, fileName);
}

void LoadConfig() {
    char iniPath[MAX_PATH] = {};
    char value[256] = {};

    GetModuleFileNameA(NULL, iniPath, MAX_PATH);
    char* slash = strrchr(iniPath, '\\');
    if (slash) {
        *(slash + 1) = '\0';
    }
    lstrcatA(iniPath, "FRController.ini");

    GetPrivateProfileStringA("General", "Enabled", "1", value, sizeof(value), iniPath);
    g_Config.enabled = ReadInt(value, 1);

    GetPrivateProfileStringA("General", "ControllerIndex", "0", value, sizeof(value), iniPath);
    g_Config.controllerIndex = ReadInt(value, 0);

    GetPrivateProfileStringA("General", "Deadzone", "0.20", value, sizeof(value), iniPath);
    g_Config.deadzone = ReadFloat(value, 0.20f);

    GetPrivateProfileStringA("General", "MoveDeadzone", "0.12", value, sizeof(value), iniPath);
    g_Config.moveDeadzone = ReadFloat(value, 0.12f);

    GetPrivateProfileStringA("Camera", "Sensitivity", "18.0", value, sizeof(value), iniPath);
    g_Config.cameraSensitivity = ReadFloat(value, 18.0f);

    GetPrivateProfileStringA("Camera", "InvertY", "0", value, sizeof(value), iniPath);
    g_Config.cameraInvertY = ReadFloat(value, 0.0f);

    GetPrivateProfileStringA("Camera", "RequireRightMouseButton", "1", value, sizeof(value), iniPath);
    g_Config.requireRightMouseForCamera = ReadInt(value, 1);

    GetPrivateProfileStringA("AnalogMovement", "UseMemory", "0", value, sizeof(value), iniPath);
    g_Config.useAnalogMovementMemory = ReadInt(value, 0);

    GetPrivateProfileStringA("AnalogMovement", "MoveForwardAddress", "0", value, sizeof(value), iniPath);
    g_Config.moveForwardAddr = ReadHex(value, 0);

    GetPrivateProfileStringA("AnalogMovement", "StrafeAddress", "0", value, sizeof(value), iniPath);
    g_Config.strafeAddr = ReadHex(value, 0);

    GetPrivateProfileStringA("AnalogMovement", "SpeedScaleAddress", "0", value, sizeof(value), iniPath);
    g_Config.moveSpeedScaleAddr = ReadHex(value, 0);

    g_Config.vkMoveForward = 'W';
    g_Config.vkMoveBackward = 'S';
    g_Config.vkStrafeLeft = 'A';
    g_Config.vkStrafeRight = 'D';
    g_Config.vkJump = VK_SPACE;
    g_Config.vkInteract = 'E';
    g_Config.vkTabTarget = VK_TAB;
    g_Config.vkAutoRun = VK_NUMLOCK;
    g_Config.vkDockAtlas = 'M';
    g_Config.vkDockCharacter = 'C';
    g_Config.vkDockJobs = 'J';
    g_Config.vkDockQuests = 'L';
    g_Config.vkDockInventory = 'I';
    g_Config.vkAltActionBar = VK_OEM_3;
    for (int i = 0; i < 9; ++i) {
        g_Config.vkActionBar[i] = '1' + i;
    }

    ReadPath(value, sizeof(value), "Bindings", "MoveForward", iniPath);
    if (value[0]) g_Config.vkMoveForward = ReadInt(value, g_Config.vkMoveForward);
    ReadPath(value, sizeof(value), "Bindings", "MoveBackward", iniPath);
    if (value[0]) g_Config.vkMoveBackward = ReadInt(value, g_Config.vkMoveBackward);
    ReadPath(value, sizeof(value), "Bindings", "StrafeLeft", iniPath);
    if (value[0]) g_Config.vkStrafeLeft = ReadInt(value, g_Config.vkStrafeLeft);
    ReadPath(value, sizeof(value), "Bindings", "StrafeRight", iniPath);
    if (value[0]) g_Config.vkStrafeRight = ReadInt(value, g_Config.vkStrafeRight);
    ReadPath(value, sizeof(value), "Bindings", "Jump", iniPath);
    if (value[0]) g_Config.vkJump = ReadInt(value, g_Config.vkJump);
    ReadPath(value, sizeof(value), "Bindings", "Interact", iniPath);
    if (value[0]) g_Config.vkInteract = ReadInt(value, g_Config.vkInteract);
    ReadPath(value, sizeof(value), "Bindings", "TabTarget", iniPath);
    if (value[0]) g_Config.vkTabTarget = ReadInt(value, g_Config.vkTabTarget);
    ReadPath(value, sizeof(value), "Bindings", "AutoRun", iniPath);
    if (value[0]) g_Config.vkAutoRun = ReadInt(value, g_Config.vkAutoRun);
    ReadPath(value, sizeof(value), "Bindings", "DockAtlas", iniPath);
    if (value[0]) g_Config.vkDockAtlas = ReadInt(value, g_Config.vkDockAtlas);
    ReadPath(value, sizeof(value), "Bindings", "DockCharacter", iniPath);
    if (value[0]) g_Config.vkDockCharacter = ReadInt(value, g_Config.vkDockCharacter);
    ReadPath(value, sizeof(value), "Bindings", "DockJobs", iniPath);
    if (value[0]) g_Config.vkDockJobs = ReadInt(value, g_Config.vkDockJobs);
    ReadPath(value, sizeof(value), "Bindings", "DockQuests", iniPath);
    if (value[0]) g_Config.vkDockQuests = ReadInt(value, g_Config.vkDockQuests);
    ReadPath(value, sizeof(value), "Bindings", "DockInventory", iniPath);
    if (value[0]) g_Config.vkDockInventory = ReadInt(value, g_Config.vkDockInventory);
    ReadPath(value, sizeof(value), "Bindings", "AltActionBar", iniPath);
    if (value[0]) g_Config.vkAltActionBar = ReadInt(value, g_Config.vkAltActionBar);

    for (int i = 0; i < 9; ++i) {
        char keyName[32];
        wsprintfA(keyName, "ActionBar%d", i + 1);
        ReadPath(value, sizeof(value), "Bindings", keyName, iniPath);
        if (value[0]) {
            g_Config.vkActionBar[i] = ReadInt(value, g_Config.vkActionBar[i]);
        }
    }
}
