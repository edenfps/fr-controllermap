#include "input_state.h"

#include "config.h"

#include <math.h>
#include <mmsystem.h>
#include <string.h>

InputState g_Input = {};

static HANDLE g_Thread = NULL;
static volatile LONG g_Running = 0;
static int g_InjectedKeyDown[256] = {};
static int g_RmbInjected = 0;
static int g_CameraLookEnabled = 0;
static int g_PrevRightThumb = 0;
static HWND g_GameWindow = NULL;
static float g_SmoothRightX = 0.0f;
static float g_SmoothRightY = 0.0f;
static float g_CameraAccumX = 0.0f;
static float g_CameraAccumY = 0.0f;

static float ApplyDeadzone(float value, float deadzone) {
    if (fabsf(value) < deadzone) {
        return 0.0f;
    }
    float sign = value >= 0.0f ? 1.0f : -1.0f;
    float scaled = (fabsf(value) - deadzone) / (1.0f - deadzone);
    if (scaled > 1.0f) {
        scaled = 1.0f;
    }
    return sign * scaled;
}

static void SetInjectedKey(int vk, int down) {
    if (vk <= 0 || vk >= 256) {
        return;
    }
    if (down) {
        if (g_InjectedKeyDown[vk]) {
            return;
        }
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = (WORD)vk;
        SendInput(1, &input, sizeof(INPUT));
        g_InjectedKeyDown[vk] = 1;
    } else {
        if (!g_InjectedKeyDown[vk]) {
            return;
        }
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = (WORD)vk;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
        g_InjectedKeyDown[vk] = 0;
    }
}

static void SetInjectedRmb(int down) {
    if (down) {
        if (g_RmbInjected) {
            g_Input.syntheticRmb = 1;
            return;
        }
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        g_RmbInjected = 1;
        g_Input.syntheticRmb = 1;
    } else {
        if (!g_RmbInjected) {
            g_Input.syntheticRmb = 0;
            return;
        }
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &input, sizeof(INPUT));
        g_RmbInjected = 0;
        g_Input.syntheticRmb = 0;
    }
}

static HWND GetGameWindow() {
    HWND foreground = GetForegroundWindow();
    if (foreground) {
        g_GameWindow = foreground;
        return foreground;
    }
    return g_GameWindow;
}

static void PostCursorClientPos(HWND hwnd, UINT message) {
    if (!hwnd) {
        return;
    }
    POINT cursor = {};
    GetCursorPos(&cursor);
    ScreenToClient(hwnd, &cursor);
    LPARAM lParam = MAKELPARAM(cursor.x, cursor.y);
    PostMessageA(hwnd, message, MK_RBUTTON, lParam);
}

static void EnterCameraLookMode() {
    HWND hwnd = GetGameWindow();
    (void)hwnd;
    ClipCursor(NULL);

    if (g_Config.requireRightMouseForCamera) {
        SetInjectedRmb(1);
    } else {
        g_Input.syntheticRmb = 1;
    }

    g_CameraAccumX = 0.0f;
    g_CameraAccumY = 0.0f;
    g_SmoothRightX = 0.0f;
    g_SmoothRightY = 0.0f;
}

static void ExitCameraLookMode() {
    HWND hwnd = GetGameWindow();

    if (g_Config.requireRightMouseForCamera) {
        SetInjectedRmb(0);
        if (hwnd) {
            PostCursorClientPos(hwnd, WM_RBUTTONUP);
            PostCursorClientPos(hwnd, WM_MOUSEMOVE);
        }
    } else {
        g_Input.syntheticRmb = 0;
    }

    ReleaseCapture();
    ClipCursor(NULL);

    while (ShowCursor(TRUE) < 0) {
    }

    g_CameraAccumX = 0.0f;
    g_CameraAccumY = 0.0f;
    g_SmoothRightX = 0.0f;
    g_SmoothRightY = 0.0f;
}

static void ReleaseAllInjectedKeys() {
    for (int vk = 0; vk < 256; ++vk) {
        SetInjectedKey(vk, 0);
    }
    if (g_CameraLookEnabled) {
        g_CameraLookEnabled = 0;
        ExitCameraLookMode();
    } else {
        SetInjectedRmb(0);
        ReleaseCapture();
        ClipCursor(NULL);
    }
    g_PrevRightThumb = 0;
}

static void UpdateCameraLookToggle(const XINPUT_GAMEPAD& pad) {
    const int pressed = (pad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
    if (pressed && !g_PrevRightThumb) {
        g_CameraLookEnabled = !g_CameraLookEnabled;
        if (g_CameraLookEnabled) {
            EnterCameraLookMode();
        } else {
            ExitCameraLookMode();
        }
    }
    g_PrevRightThumb = pressed;
}

static int RoundAccum(float* value) {
    if (*value >= 0.0f) {
        int rounded = (int)(*value + 0.5f);
        *value -= (float)rounded;
        return rounded;
    }

    int rounded = (int)(*value - 0.5f);
    *value -= (float)rounded;
    return rounded;
}

static void ApplyCamera(float rightX, float rightY, float deltaSeconds) {
    if (!g_CameraLookEnabled) {
        return;
    }

    if (deltaSeconds <= 0.0f || deltaSeconds > 0.1f) {
        deltaSeconds = 0.004f;
    }

    const float smoothing = expf(-18.0f * deltaSeconds);
    g_SmoothRightX = g_SmoothRightX * smoothing + rightX * (1.0f - smoothing);
    g_SmoothRightY = g_SmoothRightY * smoothing + rightY * (1.0f - smoothing);

    float pixelsPerSecond = g_Config.cameraSensitivity * 720.0f;
    g_CameraAccumX += g_SmoothRightX * pixelsPerSecond * deltaSeconds;
    float deltaY = g_SmoothRightY * pixelsPerSecond * deltaSeconds;
    if (g_Config.cameraInvertY > 0.5f) {
        deltaY = -deltaY;
    }
    g_CameraAccumY += deltaY;

    int moveX = RoundAccum(&g_CameraAccumX);
    int moveY = RoundAccum(&g_CameraAccumY);
    if (moveX == 0 && moveY == 0) {
        return;
    }

    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = moveX;
    input.mi.dy = moveY;
    SendInput(1, &input, sizeof(INPUT));
}

static void WriteAnalogMovement(float forward, float strafe) {
    if (!g_Config.useAnalogMovementMemory) {
        return;
    }
    if (g_Config.moveForwardAddr) {
        *(float*)(uintptr_t)g_Config.moveForwardAddr = forward;
    }
    if (g_Config.strafeAddr) {
        *(float*)(uintptr_t)g_Config.strafeAddr = strafe;
    }
    if (g_Config.moveSpeedScaleAddr) {
        float mag = sqrtf(forward * forward + strafe * strafe);
        if (mag > 1.0f) {
            mag = 1.0f;
        }
        *(float*)(uintptr_t)g_Config.moveSpeedScaleAddr = mag;
    }
}

static void ApplyMovement(float forward, float strafe) {
    WriteAnalogMovement(forward, strafe);

    if (g_Config.useAnalogMovementMemory &&
        (g_Config.moveForwardAddr || g_Config.strafeAddr)) {
        return;
    }

    const float threshold = 0.20f;
    SetInjectedKey(g_Config.vkMoveForward, forward > threshold);
    SetInjectedKey(g_Config.vkMoveBackward, forward < -threshold);
    SetInjectedKey(g_Config.vkStrafeLeft, strafe < -threshold);
    SetInjectedKey(g_Config.vkStrafeRight, strafe > threshold);

    float mag = sqrtf(forward * forward + strafe * strafe);
    SetInjectedKey(VK_SHIFT, mag > 0.01f && mag < 0.55f);
}

static void ApplyButtons(const XINPUT_GAMEPAD& pad) {
    SetInjectedKey(g_Config.vkJump, (pad.wButtons & XINPUT_GAMEPAD_A) != 0);
    SetInjectedKey(g_Config.vkInteract, (pad.wButtons & XINPUT_GAMEPAD_X) != 0);
    SetInjectedKey(g_Config.vkTabTarget, (pad.wButtons & XINPUT_GAMEPAD_Y) != 0);
    SetInjectedKey(g_Config.vkAutoRun, (pad.wButtons & XINPUT_GAMEPAD_B) != 0);
    SetInjectedKey(g_Config.vkAltActionBar, (pad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
    SetInjectedKey(g_Config.vkDockInventory, (pad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);
    SetInjectedKey(g_Config.vkDockAtlas, (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0);
    SetInjectedKey(g_Config.vkDockCharacter, (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
    SetInjectedKey(g_Config.vkDockJobs, (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
    SetInjectedKey(g_Config.vkDockQuests, (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
    SetInjectedKey(VK_ESCAPE, (pad.wButtons & XINPUT_GAMEPAD_START) != 0);
    SetInjectedKey(g_Config.vkActionBar[0], pad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    SetInjectedKey(g_Config.vkActionBar[1], pad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    SetInjectedKey(g_Config.vkActionBar[8], (pad.wButtons & XINPUT_GAMEPAD_BACK) != 0);
}

static DWORD WINAPI InputThreadProc(LPVOID) {
    timeBeginPeriod(1);
    DWORD lastTick = timeGetTime();

    while (InterlockedCompareExchange(&g_Running, 0, 0)) {
        DWORD now = timeGetTime();
        float deltaSeconds = (now - lastTick) / 1000.0f;
        lastTick = now;

        XINPUT_STATE state = {};
        if (XInputGetState((DWORD)g_Config.controllerIndex, &state) == ERROR_SUCCESS) {
            g_Input.controllerConnected = 1;
            g_Input.raw = state;

            SHORT lx = state.Gamepad.sThumbLX;
            SHORT ly = state.Gamepad.sThumbLY;
            SHORT rx = state.Gamepad.sThumbRX;
            SHORT ry = state.Gamepad.sThumbRY;

            g_Input.leftX = ApplyDeadzone(lx / 32767.0f, g_Config.moveDeadzone);
            g_Input.leftY = ApplyDeadzone(ly / 32767.0f, g_Config.moveDeadzone);
            g_Input.rightX = ApplyDeadzone(rx / 32767.0f, g_Config.deadzone);
            g_Input.rightY = ApplyDeadzone(-(ry / 32767.0f), g_Config.deadzone);
            g_Input.leftMag = sqrtf(g_Input.leftX * g_Input.leftX + g_Input.leftY * g_Input.leftY);
            g_Input.rightMag = sqrtf(g_Input.rightX * g_Input.rightX + g_Input.rightY * g_Input.rightY);

            ApplyMovement(g_Input.leftY, g_Input.leftX);
            ApplyButtons(state.Gamepad);
            UpdateCameraLookToggle(state.Gamepad);
            ApplyCamera(g_Input.rightX, g_Input.rightY, deltaSeconds);
        } else {
            if (g_Input.controllerConnected) {
                ReleaseAllInjectedKeys();
                g_SmoothRightX = 0.0f;
                g_SmoothRightY = 0.0f;
                g_CameraAccumX = 0.0f;
                g_CameraAccumY = 0.0f;
            }
            g_Input.controllerConnected = 0;
            memset(&g_Input.raw, 0, sizeof(g_Input.raw));
        }

        Sleep(1);
    }

    ReleaseAllInjectedKeys();
    timeEndPeriod(1);
    return 0;
}

void InitializeInputThread() {
    if (!g_Config.enabled) {
        return;
    }
    InterlockedExchange(&g_Running, 1);
    g_Thread = CreateThread(NULL, 0, InputThreadProc, NULL, 0, NULL);
}

void ShutdownInputThread() {
    InterlockedExchange(&g_Running, 0);
    if (g_Thread) {
        WaitForSingleObject(g_Thread, 2000);
        CloseHandle(g_Thread);
        g_Thread = NULL;
    }
}

void UpdateSyntheticKeys() {
}
