#include <windows.h>

#include "config.h"
#include "hooks.h"
#include "input_state.h"

static HMODULE g_RealVersion = NULL;

#define LOAD_VERSION(name) \
    g_##name = (name##Fn)GetProcAddress(g_RealVersion, #name)

typedef BOOL (WINAPI* GetFileVersionInfoAFn)(LPCSTR, DWORD, DWORD, LPVOID);
typedef BOOL (WINAPI* GetFileVersionInfoByHandleFn)(DWORD, LPCWSTR, DWORD, LPDWORD, LPVOID);
typedef BOOL (WINAPI* GetFileVersionInfoExAFn)(DWORD, LPCSTR, DWORD, DWORD, LPVOID);
typedef BOOL (WINAPI* GetFileVersionInfoExWFn)(DWORD, LPCWSTR, DWORD, DWORD, LPVOID);
typedef DWORD (WINAPI* GetFileVersionInfoSizeAFn)(LPCSTR, LPDWORD);
typedef DWORD (WINAPI* GetFileVersionInfoSizeExAFn)(DWORD, LPCSTR, LPDWORD);
typedef DWORD (WINAPI* GetFileVersionInfoSizeWFn)(LPCWSTR, LPDWORD);
typedef BOOL (WINAPI* GetFileVersionInfoWFn)(LPCWSTR, DWORD, DWORD, LPVOID);
typedef DWORD (WINAPI* VerFindFileAFn)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT, LPSTR, PUINT);
typedef DWORD (WINAPI* VerFindFileWFn)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT, LPWSTR, PUINT);
typedef DWORD (WINAPI* VerInstallFileAFn)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT);
typedef DWORD (WINAPI* VerInstallFileWFn)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT);
typedef DWORD (WINAPI* VerLanguageNameAFn)(DWORD, LPSTR, DWORD);
typedef DWORD (WINAPI* VerLanguageNameWFn)(DWORD, LPWSTR, DWORD);
typedef BOOL (WINAPI* VerQueryValueAFn)(LPCVOID, LPCSTR, LPVOID*, PUINT);
typedef BOOL (WINAPI* VerQueryValueWFn)(LPCVOID, LPCWSTR, LPVOID*, PUINT);

static GetFileVersionInfoAFn g_GetFileVersionInfoA = NULL;
static GetFileVersionInfoByHandleFn g_GetFileVersionInfoByHandle = NULL;
static GetFileVersionInfoExAFn g_GetFileVersionInfoExA = NULL;
static GetFileVersionInfoExWFn g_GetFileVersionInfoExW = NULL;
static GetFileVersionInfoSizeAFn g_GetFileVersionInfoSizeA = NULL;
static GetFileVersionInfoSizeExAFn g_GetFileVersionInfoSizeExA = NULL;
static GetFileVersionInfoSizeWFn g_GetFileVersionInfoSizeW = NULL;
static GetFileVersionInfoWFn g_GetFileVersionInfoW = NULL;
static VerFindFileAFn g_VerFindFileA = NULL;
static VerFindFileWFn g_VerFindFileW = NULL;
static VerInstallFileAFn g_VerInstallFileA = NULL;
static VerInstallFileWFn g_VerInstallFileW = NULL;
static VerLanguageNameAFn g_VerLanguageNameA = NULL;
static VerLanguageNameWFn g_VerLanguageNameW = NULL;
static VerQueryValueAFn g_VerQueryValueA = NULL;
static VerQueryValueWFn g_VerQueryValueW = NULL;

static void LoadRealVersionDll() {
    if (g_RealVersion) {
        return;
    }

    char systemDir[MAX_PATH] = {};
    GetSystemDirectoryA(systemDir, MAX_PATH);
    lstrcatA(systemDir, "\\version.dll");
    g_RealVersion = LoadLibraryA(systemDir);
    if (!g_RealVersion) {
        return;
    }

    LOAD_VERSION(GetFileVersionInfoA);
    LOAD_VERSION(GetFileVersionInfoByHandle);
    LOAD_VERSION(GetFileVersionInfoExA);
    LOAD_VERSION(GetFileVersionInfoExW);
    LOAD_VERSION(GetFileVersionInfoSizeA);
    LOAD_VERSION(GetFileVersionInfoSizeExA);
    LOAD_VERSION(GetFileVersionInfoSizeW);
    LOAD_VERSION(GetFileVersionInfoW);
    LOAD_VERSION(VerFindFileA);
    LOAD_VERSION(VerFindFileW);
    LOAD_VERSION(VerInstallFileA);
    LOAD_VERSION(VerInstallFileW);
    LOAD_VERSION(VerLanguageNameA);
    LOAD_VERSION(VerLanguageNameW);
    LOAD_VERSION(VerQueryValueA);
    LOAD_VERSION(VerQueryValueW);
}

extern "C" {

BOOL WINAPI GetFileVersionInfoA(LPCSTR a, DWORD b, DWORD c, LPVOID d) {
    LoadRealVersionDll();
    return g_GetFileVersionInfoA ? g_GetFileVersionInfoA(a, b, c, d) : FALSE;
}

BOOL WINAPI GetFileVersionInfoByHandle(DWORD a, LPCWSTR b, DWORD c, LPDWORD d, LPVOID e) {
    LoadRealVersionDll();
    return g_GetFileVersionInfoByHandle ? g_GetFileVersionInfoByHandle(a, b, c, d, e) : FALSE;
}

BOOL WINAPI GetFileVersionInfoExA(DWORD a, LPCSTR b, DWORD c, DWORD d, LPVOID e) {
    LoadRealVersionDll();
    return g_GetFileVersionInfoExA ? g_GetFileVersionInfoExA(a, b, c, d, e) : FALSE;
}

BOOL WINAPI GetFileVersionInfoExW(DWORD a, LPCWSTR b, DWORD c, DWORD d, LPVOID e) {
    LoadRealVersionDll();
    return g_GetFileVersionInfoExW ? g_GetFileVersionInfoExW(a, b, c, d, e) : FALSE;
}

DWORD WINAPI GetFileVersionInfoSizeA(LPCSTR a, LPDWORD b) {
    LoadRealVersionDll();
    return g_GetFileVersionInfoSizeA ? g_GetFileVersionInfoSizeA(a, b) : 0;
}

DWORD WINAPI GetFileVersionInfoSizeExA(DWORD a, LPCSTR b, LPDWORD c) {
    LoadRealVersionDll();
    return g_GetFileVersionInfoSizeExA ? g_GetFileVersionInfoSizeExA(a, b, c) : 0;
}

DWORD WINAPI GetFileVersionInfoSizeW(LPCWSTR a, LPDWORD b) {
    LoadRealVersionDll();
    return g_GetFileVersionInfoSizeW ? g_GetFileVersionInfoSizeW(a, b) : 0;
}

BOOL WINAPI GetFileVersionInfoW(LPCWSTR a, DWORD b, DWORD c, LPVOID d) {
    LoadRealVersionDll();
    return g_GetFileVersionInfoW ? g_GetFileVersionInfoW(a, b, c, d) : FALSE;
}

DWORD WINAPI VerFindFileA(DWORD a, LPCSTR b, LPCSTR c, LPCSTR d, LPSTR e, PUINT f, LPSTR g, PUINT h) {
    LoadRealVersionDll();
    return g_VerFindFileA ? g_VerFindFileA(a, b, c, d, e, f, g, h) : 0;
}

DWORD WINAPI VerFindFileW(DWORD a, LPCWSTR b, LPCWSTR c, LPCWSTR d, LPWSTR e, PUINT f, LPWSTR g, PUINT h) {
    LoadRealVersionDll();
    return g_VerFindFileW ? g_VerFindFileW(a, b, c, d, e, f, g, h) : 0;
}

DWORD WINAPI VerInstallFileA(DWORD a, LPCSTR b, LPCSTR c, LPCSTR d, LPCSTR e, LPCSTR f, LPSTR g, PUINT h) {
    LoadRealVersionDll();
    return g_VerInstallFileA ? g_VerInstallFileA(a, b, c, d, e, f, g, h) : 0;
}

DWORD WINAPI VerInstallFileW(DWORD a, LPCWSTR b, LPCWSTR c, LPCWSTR d, LPCWSTR e, LPCWSTR f, LPWSTR g, PUINT h) {
    LoadRealVersionDll();
    return g_VerInstallFileW ? g_VerInstallFileW(a, b, c, d, e, f, g, h) : 0;
}

DWORD WINAPI VerLanguageNameA(DWORD a, LPSTR b, DWORD c) {
    LoadRealVersionDll();
    return g_VerLanguageNameA ? g_VerLanguageNameA(a, b, c) : 0;
}

DWORD WINAPI VerLanguageNameW(DWORD a, LPWSTR b, DWORD c) {
    LoadRealVersionDll();
    return g_VerLanguageNameW ? g_VerLanguageNameW(a, b, c) : 0;
}

BOOL WINAPI VerQueryValueA(LPCVOID a, LPCSTR b, LPVOID* c, PUINT d) {
    LoadRealVersionDll();
    return g_VerQueryValueA ? g_VerQueryValueA(a, b, c, d) : FALSE;
}

BOOL WINAPI VerQueryValueW(LPCVOID a, LPCWSTR b, LPVOID* c, PUINT d) {
    LoadRealVersionDll();
    return g_VerQueryValueW ? g_VerQueryValueW(a, b, c, d) : FALSE;
}

}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(module);
        LoadConfig();
        InstallHooks();
        InitializeInputThread();
        break;
    case DLL_PROCESS_DETACH:
        ShutdownInputThread();
        RemoveHooks();
        if (g_RealVersion) {
            FreeLibrary(g_RealVersion);
            g_RealVersion = NULL;
        }
        break;
    default:
        break;
    }
    return TRUE;
}
