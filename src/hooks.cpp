#include "hooks.h"

#include "config.h"
#include "input_state.h"

#include <string.h>

typedef SHORT (WINAPI* GetAsyncKeyStateFn)(int vKey);
typedef SHORT (WINAPI* GetKeyStateFn)(int vKey);
typedef BOOL (WINAPI* GetKeyboardStateFn)(PBYTE lpKeyState);

static GetAsyncKeyStateFn g_OriginalGetAsyncKeyState = NULL;
static GetKeyStateFn g_OriginalGetKeyState = NULL;
static GetKeyboardStateFn g_OriginalGetKeyboardState = NULL;

static BYTE* g_PatchedAsyncSite = NULL;
static BYTE* g_PatchedKeySite = NULL;
static BYTE g_AsyncBackup[5] = {};
static BYTE g_KeyBackup[5] = {};

static SHORT WINAPI HookedGetAsyncKeyState(int vKey) {
    if (g_Config.enabled && g_Input.controllerConnected) {
        if (vKey == VK_RBUTTON && g_Input.syntheticRmb) {
            return (SHORT)0x8001;
        }
    }
    return g_OriginalGetAsyncKeyState(vKey);
}

static SHORT WINAPI HookedGetKeyState(int vKey) {
    if (g_Config.enabled && g_Input.controllerConnected) {
        if (vKey == VK_RBUTTON && g_Input.syntheticRmb) {
            return (SHORT)0x8000;
        }
    }
    return g_OriginalGetKeyState(vKey);
}

static BOOL WINAPI HookedGetKeyboardState(PBYTE lpKeyState) {
    BOOL result = g_OriginalGetKeyboardState(lpKeyState);
    if (result && g_Config.enabled && g_Input.controllerConnected && lpKeyState) {
        if (g_Input.syntheticRmb) {
            lpKeyState[VK_RBUTTON] = (BYTE)(lpKeyState[VK_RBUTTON] | 0x80);
        }
    }
    return result;
}

static FARPROC ResolveImport(HMODULE module, const char* importName) {
    HMODULE user32 = GetModuleHandleA("USER32.dll");
    if (!user32) {
        return NULL;
    }
    return GetProcAddress(user32, importName);
}

static BOOL PatchImport(HMODULE module, const char* importName, FARPROC replacement, FARPROC* originalOut) {
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)module;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }

    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)module + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        return FALSE;
    }

    IMAGE_DATA_DIRECTORY importDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!importDir.VirtualAddress) {
        return FALSE;
    }

    PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)module + importDir.VirtualAddress);
    for (; importDesc->Name; ++importDesc) {
        const char* dllName = (const char*)module + importDesc->Name;
        if (_stricmp(dllName, "USER32.dll") != 0) {
            continue;
        }

        PIMAGE_THUNK_DATA originalThunk = (PIMAGE_THUNK_DATA)((BYTE*)module + importDesc->OriginalFirstThunk);
        PIMAGE_THUNK_DATA firstThunk = (PIMAGE_THUNK_DATA)((BYTE*)module + importDesc->FirstThunk);
        for (; originalThunk->u1.AddressOfData; ++originalThunk, ++firstThunk) {
            if (IMAGE_SNAP_BY_ORDINAL(originalThunk->u1.Ordinal)) {
                continue;
            }

            PIMAGE_IMPORT_BY_NAME importByName =
                (PIMAGE_IMPORT_BY_NAME)((BYTE*)module + originalThunk->u1.AddressOfData);
            if (strcmp((const char*)importByName->Name, importName) != 0) {
                continue;
            }

            DWORD oldProtect = 0;
            if (!VirtualProtect(&firstThunk->u1.Function, sizeof(FARPROC), PAGE_READWRITE, &oldProtect)) {
                return FALSE;
            }

            if (originalOut && !*originalOut) {
                *originalOut = (FARPROC)firstThunk->u1.Function;
            }
            firstThunk->u1.Function = (ULONG_PTR)replacement;
            VirtualProtect(&firstThunk->u1.Function, sizeof(FARPROC), oldProtect, &oldProtect);
            return TRUE;
        }
    }

    return FALSE;
}

void InstallHooks() {
    HMODULE game = GetModuleHandleA(NULL);
    if (!game) {
        return;
    }

    g_OriginalGetAsyncKeyState = (GetAsyncKeyStateFn)ResolveImport(game, "GetAsyncKeyState");
    g_OriginalGetKeyState = (GetKeyStateFn)ResolveImport(game, "GetKeyState");
    g_OriginalGetKeyboardState = (GetKeyboardStateFn)ResolveImport(game, "GetKeyboardState");

    PatchImport(game, "GetAsyncKeyState", (FARPROC)HookedGetAsyncKeyState, (FARPROC*)&g_OriginalGetAsyncKeyState);
    PatchImport(game, "GetKeyState", (FARPROC)HookedGetKeyState, (FARPROC*)&g_OriginalGetKeyState);
    PatchImport(game, "GetKeyboardState", (FARPROC)HookedGetKeyboardState, (FARPROC*)&g_OriginalGetKeyboardState);

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)game;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)game + dos->e_lfanew);
    BYTE* base = (BYTE*)game;
    DWORD imageSize = nt->OptionalHeader.SizeOfImage;

    struct PatchPair {
        FARPROC* original;
        FARPROC hook;
    } pairs[] = {
        {(FARPROC*)&g_OriginalGetAsyncKeyState, (FARPROC)HookedGetAsyncKeyState},
        {(FARPROC*)&g_OriginalGetKeyState, (FARPROC)HookedGetKeyState},
        {(FARPROC*)&g_OriginalGetKeyboardState, (FARPROC)HookedGetKeyboardState},
    };

    for (DWORD offset = 0; offset + sizeof(FARPROC) <= imageSize; offset += sizeof(FARPROC)) {
        FARPROC* slot = (FARPROC*)(base + offset);
        for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
            if (*pairs[i].original && *slot == *pairs[i].original) {
                DWORD oldProtect = 0;
                if (VirtualProtect(slot, sizeof(FARPROC), PAGE_READWRITE, &oldProtect)) {
                    *slot = pairs[i].hook;
                    VirtualProtect(slot, sizeof(FARPROC), oldProtect, &oldProtect);
                }
            }
        }
    }
}

void RemoveHooks() {
    (void)g_PatchedAsyncSite;
    (void)g_PatchedKeySite;
    (void)g_AsyncBackup;
    (void)g_KeyBackup;
}
