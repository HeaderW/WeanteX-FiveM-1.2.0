



typedef NTSTATUS(NTAPI* tNtWriteVirtualMemory)(HANDLE, PVOID, PVOID, ULONG, PULONG);
tNtWriteVirtualMemory oNtWriteVirtualMemory = nullptr;

template <class t>
t ReadMemory(uintptr_t address) {
    t read;
    ReadProcessMemory(Game.hProcess, (LPVOID)address, &read, sizeof(t), NULL);
    return read;
}

template <class T>
void WriteMemory(uintptr_t address, T value) {
    WriteProcessMemory(Game.hProcess, (LPVOID)address, &value, sizeof(T), NULL);
}

#include <Windows.h>
#include <string>

std::string ReadString( uintptr_t Addr) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    int bytesRead = 0;

    while (bytesRead < bufferSize) {
        char character = 0;
        SIZE_T bytesReadNow = 0;

        if (!ReadProcessMemory(Game.hProcess, (LPCVOID)(Addr + bytesRead), &character, sizeof(char), &bytesReadNow) || bytesReadNow == 0) {
            return "";
        }

        buffer[bytesRead] = character;
        if (character == '\0') {
            break;
        }
        bytesRead++;
    }

    if (bytesRead == bufferSize) {
        return "";
    }

    return std::string(buffer);
}



void WriteBytesNt(uintptr_t address, uint8_t* patch, size_t size) {
    ULONG bytesWritten = 0;
    if (!oNtWriteVirtualMemory) {
        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (hNtdll) {
            oNtWriteVirtualMemory = (tNtWriteVirtualMemory)GetProcAddress(hNtdll, "NtWriteVirtualMemory");
        }
    }

    if (oNtWriteVirtualMemory) {
        oNtWriteVirtualMemory(Game.hProcess, reinterpret_cast<PVOID>(address), patch, static_cast<ULONG>(size), &bytesWritten);
    }
}

void WriteBytes(uintptr_t address, uint8_t* patch, size_t size) {
    if (!oNtWriteVirtualMemory) {
        WriteBytesNt(address, patch, size);
    }
    else {
        SIZE_T bytesWritten;
        oNtWriteVirtualMemory(Game.hProcess, reinterpret_cast<PVOID>(address), patch, static_cast<ULONG>(size), (PULONG)&bytesWritten);
    }
}

std::string GetProcessName(DWORD pID) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pID);
    if (hProcess == nullptr) {
        return "";
    }

    char processName[MAX_PATH];
    DWORD dwSize = MAX_PATH;
    if (QueryFullProcessImageNameA(hProcess, 0, processName, &dwSize)) {
        std::string fullPath(processName);
        size_t pos = fullPath.find_last_of("\\/");
        if (pos != std::string::npos) {
            fullPath = fullPath.substr(pos + 1);
        }
        CloseHandle(hProcess);
        return fullPath;
    }
    CloseHandle(hProcess);
    return "";
}

uintptr_t GetBaseAddress() {
    if (Game.pID == 0) {
        return 0;
    }

    HANDLE moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, Game.pID);
    if (moduleSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(moduleEntry);
    if (!Module32First(moduleSnapshot, &moduleEntry)) {
        CloseHandle(moduleSnapshot);
        return 0;
    }

    do {

        int len = WideCharToMultiByte(CP_UTF8, 0, moduleEntry.szModule, -1, NULL, 0, NULL, NULL);
        std::string currentModuleName;
        if (len > 0) {
            currentModuleName.resize(len - 1);
            WideCharToMultiByte(CP_UTF8, 0, moduleEntry.szModule, -1, &currentModuleName[0], len, NULL, NULL);
        }
        
        if (currentModuleName.compare(GetProcessName(Game.pID)) == 0) {
            uintptr_t baseAddress = (uintptr_t)moduleEntry.modBaseAddr;
            CloseHandle(moduleSnapshot);
            return baseAddress;
        }
    } while (Module32Next(moduleSnapshot, &moduleEntry));
    CloseHandle(moduleSnapshot);
    return 0;
}

uintptr_t GetBaseAddress(const std::string& moduleName) {
    if (Game.pID == 0) {
        return 0;
    }

    HANDLE moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, Game.pID);
    if (moduleSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(moduleEntry);
    if (!Module32First(moduleSnapshot, &moduleEntry)) {
        CloseHandle(moduleSnapshot);
        return 0;
    }

    do {

        int len = WideCharToMultiByte(CP_UTF8, 0, moduleEntry.szModule, -1, NULL, 0, NULL, NULL);
        std::string currentModuleName;
        if (len > 0) {
            currentModuleName.resize(len - 1);
            WideCharToMultiByte(CP_UTF8, 0, moduleEntry.szModule, -1, &currentModuleName[0], len, NULL, NULL);
        }
        
        if (currentModuleName.compare(moduleName) == 0) {
            uintptr_t baseAddress = (uintptr_t)moduleEntry.modBaseAddr;
            CloseHandle(moduleSnapshot);
            return baseAddress;
        }
    } while (Module32Next(moduleSnapshot, &moduleEntry));
    CloseHandle(moduleSnapshot);
    return 0;
}