
struct GameStruct {
	DWORD pID;
	HANDLE hProcess;
	HWND hWnd;
	LPCSTR lpClassName = "grcWindow";
	std::string Version;
	RECT lpRect;
	POINT lpPoint;
	uintptr_t World;
	uintptr_t ViewPort;
	uintptr_t ReplayInterface;
}Game;

struct OffsetsStruct {
	uintptr_t GameBase;
	uintptr_t CitizenPlayernamesBase;
	uintptr_t GameWorld;
	uintptr_t FreecanPatch;
	uintptr_t ReplayInterface;
	uintptr_t ViewPort;
	uintptr_t LocalPlayer;
	uintptr_t Camera;
	uintptr_t BlipList;
	uintptr_t PlayerInfo;
	uintptr_t Id;
	uintptr_t Health;
	uintptr_t MaxHealth;
	uintptr_t Armor;
	uintptr_t WeaponManager;
	uintptr_t BoneList;
	uintptr_t Silent;
	uintptr_t Waypoint;
	uintptr_t Vehicle;
	uintptr_t VisibleFlag;
} Offsets;

DWORD FindGame() {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return 0;
	}
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hSnapshot, &pe32)) {
		do {

			int len = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, NULL, 0, NULL, NULL);
			std::string processName;
			if (len > 0) {
				processName.resize(len - 1);
				WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, &processName[0], len, NULL, NULL);
			}

			if (processName.find(XorString("Process.exe")) != std::string::npos) {
				CloseHandle(hSnapshot);
				return pe32.th32ProcessID;
			}
		} while (Process32Next(hSnapshot, &pe32));
	}
	CloseHandle(hSnapshot);
	return 0;
}

std::string versions[] = { "3570", "3407", "3323", "3258", "3095", "2944", "2802", "2699" };


char* GetModuleName() {
	char pModule[128]{};
	GetModuleBaseNameA(Game.hProcess, nullptr, pModule, sizeof(pModule));
	return pModule;
}

MODULEINFO GetModuleInfo(const std::string moduleName) {
	DWORD cb;
	HMODULE hMods[256]{};
	MODULEINFO modInfo{};
	if (EnumProcessModules(Game.hProcess, hMods, sizeof(hMods), &cb)) {
		for (unsigned int i = 0; i < (cb / sizeof(HMODULE)); i++) {
			char szModName[MAX_PATH];
			if (GetModuleBaseNameA(Game.hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char))) {
				if (moduleName == szModName) {
					GetModuleInformation(Game.hProcess, hMods[i], &modInfo, sizeof(modInfo));
					break;
				}
			}
		}
	}
	return modInfo;
}

uintptr_t FindPattern(const std::vector<uint8_t>& read_data, const std::string pattern, int offset, int extra) {
	std::vector<uint8_t> bytes;
	std::istringstream patternStream(pattern);
	std::string byteStr;

	while (patternStream >> byteStr) {
		if (byteStr == "?" || byteStr == "??")
			bytes.push_back(0);
		else
			bytes.push_back(static_cast<uint8_t>(strtol(byteStr.c_str(), nullptr, 16)));
	}

	for (size_t i = 1000000; i < read_data.size(); ++i) {
		bool patternMatch = true;
		for (size_t j = 0; j < bytes.size(); ++j) {
			if (bytes[j] != 0 && read_data[i + j] != bytes[j]) {
				patternMatch = false;
				break;
			}
		}

		if (patternMatch) {
			uintptr_t patternAddress = Offsets.GameBase + i;
			int32_t of;
			ReadProcessMemory(Game.hProcess, reinterpret_cast<LPCVOID>(patternAddress + offset), &of, sizeof(of), nullptr);
			uintptr_t result = patternAddress + of + extra;
			bytes.clear();
			return (result - Offsets.GameBase);
		}
	}
}

template <class t>
t ReadMemory2(uintptr_t address) {
	t read;
	ReadProcessMemory(Game.hProcess, (LPVOID)address, &read, sizeof(t), NULL);
	return read;
}

bool ReadOffsets() {
	bool findVersion = false;
	while (true) {

		Game.Version = XorString("3570");
		{
			Offsets.GameWorld = 0x25EC580;
			Offsets.ReplayInterface = 0x1FB0418;
			Offsets.ViewPort = 0x2058BA0;
			Offsets.Camera = 0x2059778;
			Offsets.BlipList = 0x2061870;
			Offsets.LocalPlayer = 0x8;
			Offsets.FreecanPatch = 0x27D653;
			Offsets.PlayerInfo = 0x10A8;
			Offsets.Id = 0xE8;
			Offsets.Health = 0x280;
			Offsets.MaxHealth = 0x284;
			Offsets.Armor = 0x150C;
			Offsets.WeaponManager = 0x10B8;
			Offsets.BoneList = 0x410;
			Offsets.Silent = 0x102D550;
			Offsets.Vehicle = 0x0D10;
			Offsets.VisibleFlag = 0x145C;
			Offsets.Waypoint = 0x2047D50;
			auto GameWorld = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
			auto LocalPlayer = ReadMemory2<uintptr_t>(Game.World + Offsets.LocalPlayer);
			auto GameViewPort = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			auto GameReplayInterface = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			if (GameWorld != 0 && GameViewPort != 0 && GameReplayInterface != 0) {
				findVersion = true;
				break;
			}
		}

		Game.Version = XorString("3407");
		{
			Offsets.GameWorld = 0x25D7108;
			Offsets.ReplayInterface = 0x1F9A9D8;
			Offsets.ViewPort = 0x20431C0;
			Offsets.Camera = 0x2043DF8;
			Offsets.BlipList = 0x20440C8;
			Offsets.LocalPlayer = 0x8;
			Offsets.PlayerInfo = 0x10A8;
			Offsets.FreecanPatch = 0x27BD43;
			Offsets.Id = 0xE8;
			Offsets.Health = 0x280;
			Offsets.MaxHealth = 0x284;
			Offsets.Armor = 0x150C;
			Offsets.WeaponManager = 0x10B8;
			Offsets.BoneList = 0x410;
			Offsets.Silent = 0x102FF89;
			Offsets.Vehicle = 0x0D10;
			Offsets.VisibleFlag = 0x145C;
			Offsets.Waypoint = 0x2047D50;
			auto GameWorld = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
			auto LocalPlayer = ReadMemory2<uintptr_t>(Game.World + Offsets.LocalPlayer);
			auto GameViewPort = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			auto GameReplayInterface = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			if (GameWorld != 0 && GameViewPort != 0 && GameReplayInterface != 0) {
				findVersion = true;
				break;
			}
		}

		Game.Version = XorString("3323");
		{
			Offsets.GameWorld = 0x25C15B0;
			Offsets.ReplayInterface = 0x1F85458;
			Offsets.ViewPort = 0x202DC50;
			Offsets.Camera = 0x202E878;
			Offsets.BlipList = 0x2002888;
			Offsets.LocalPlayer = 0x8;
			Offsets.FreecanPatch = 0x27B7B3;
			Offsets.PlayerInfo = 0x10A8;
			Offsets.Id = 0xE8;
			Offsets.Health = 0x280;
			Offsets.MaxHealth = 0x284;
			Offsets.Armor = 0x150C;
			Offsets.WeaponManager = 0x10B8;
			Offsets.BoneList = 0x410;
			Offsets.Silent = 0x1026CAD;
			Offsets.Vehicle = 0x0D10;
			Offsets.VisibleFlag = 0x145C;
			Offsets.Waypoint = 0x2022DE0;
			auto GameWorld = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
			auto LocalPlayer = ReadMemory2<uintptr_t>(Game.World + Offsets.LocalPlayer);
			auto GameViewPort = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			auto GameReplayInterface = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			if (GameWorld != 0 && GameViewPort != 0 && GameReplayInterface != 0) {
				findVersion = true;
				break;
			}
		}


		Game.Version = XorString("3258");
		{
			Offsets.GameWorld = 0x25B14B0;
			Offsets.ReplayInterface = 0x1FBD4F0;
			Offsets.ViewPort = 0x201DBA0;
			Offsets.Camera = 0x201E7D0;
			Offsets.BlipList = 0x2002FA0;
			Offsets.LocalPlayer = 0x8;
			Offsets.PlayerInfo = 0x10A8;
			Offsets.Id = 0xE8;
			Offsets.Health = 0x280;
			Offsets.FreecanPatch = 0x27B367;
			Offsets.MaxHealth = 0x284;
			Offsets.Armor = 0x150C;
			Offsets.WeaponManager = 0x10B8;
			Offsets.BoneList = 0x410;
			Offsets.Silent = 0x101A65D;
			Offsets.Vehicle = 0x0D10;
			Offsets.VisibleFlag = 0x145C;
			Offsets.Waypoint = 0x2023400;
			auto GameWorld = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
			auto LocalPlayer = ReadMemory2<uintptr_t>(Game.World + Offsets.LocalPlayer);
			auto GameViewPort = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			auto GameReplayInterface = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			if (GameWorld != 0 && GameViewPort != 0 && GameReplayInterface != 0) {
				findVersion = true;
				break;
			}
		}

		Game.Version = XorString("3095");
		{
			Offsets.GameWorld = 0x2593320;
			Offsets.ReplayInterface = 0x1F58B58;
			Offsets.ViewPort = 0x20019E0;
			Offsets.Camera = 0x20025B8;
			Offsets.BlipList = 0x2002888;
			Offsets.LocalPlayer = 0x8;
			Offsets.PlayerInfo = 0x10A8;
			Offsets.FreecanPatch = 0x27B367;
			Offsets.Id = 0xE8;
			Offsets.Health = 0x280;
			Offsets.MaxHealth = 0x284;
			Offsets.Armor = 0x150C;
			Offsets.WeaponManager = 0x10B8;
			Offsets.BoneList = 0x410;
			Offsets.Silent = 0x100F5A4;
			Offsets.Vehicle = 0x0D10;
			Offsets.VisibleFlag = 0x145C;
			Offsets.Waypoint = 0x2002FA0;
			auto GameWorld = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
			auto LocalPlayer = ReadMemory2<uintptr_t>(Game.World + Offsets.LocalPlayer);
			auto GameViewPort = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			auto GameReplayInterface = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			if (GameWorld != 0 && GameViewPort != 0 && GameReplayInterface != 0) {
				findVersion = true;
				break;
			}
		}

		Game.Version = XorString("2944");
		{
			Offsets.GameWorld = 0x257BEA0;
			Offsets.ReplayInterface = 0x1F42068;
			Offsets.ViewPort = 0x1FEAAC0;
			Offsets.Camera = 0x1FEB968;
			Offsets.BlipList = 0x1FEB968;
			Offsets.LocalPlayer = 0x8;
			Offsets.PlayerInfo = 0x10A8;
			Offsets.Id = 0xE8;
			Offsets.Health = 0x280;
			Offsets.MaxHealth = 0x284;
			Offsets.Armor = 0x150C;
			Offsets.WeaponManager = 0x10B8;
			Offsets.BoneList = 0x410;
			Offsets.Silent = 0x1003F80;
			Offsets.Vehicle = 0x0D10;
			Offsets.VisibleFlag = 0x145C;
			Offsets.Waypoint = 0x1FF3130;
			auto GameWorld = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
			auto LocalPlayer = ReadMemory2<uintptr_t>(Game.World + Offsets.LocalPlayer);
			auto GameViewPort = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			auto GameReplayInterface = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			if (GameWorld != 0 && GameViewPort != 0 && GameReplayInterface != 0) {
				findVersion = true;
				break;
			}
		}

		Game.Version = XorString("2802");
		{
			Offsets.GameWorld = 0x254D448;
			Offsets.ReplayInterface = 0x1F5B820;
			Offsets.ViewPort = 0x1FBC100;
			Offsets.Camera = 0x1FBCCD8;
			Offsets.BlipList = 0x1FBCFA8;
			Offsets.LocalPlayer = 0x8;
			Offsets.PlayerInfo = 0x10A8;
			Offsets.Id = 0xE8;
			Offsets.Health = 0x280;
			Offsets.MaxHealth = 0x284;
			Offsets.Armor = 0x150C;
			Offsets.WeaponManager = 0x10B8;
			Offsets.BoneList = 0x410;
			Offsets.Silent = 0xFF716C;
			Offsets.Vehicle = 0x0D10;
			Offsets.VisibleFlag = 0x145C;
			Offsets.Waypoint = 0x1FBD6E0;
			auto GameWorld = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
			auto LocalPlayer = ReadMemory2<uintptr_t>(Game.World + Offsets.LocalPlayer);
			auto GameViewPort = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			auto GameReplayInterface = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			if (GameWorld != 0 && GameViewPort != 0 && GameReplayInterface != 0) {
				findVersion = true;
				break;
			}
		}

		Game.Version = XorString("2699");
		{
			Offsets.GameWorld = 0x26684D8;
			Offsets.ReplayInterface = 0x20304C8;
			Offsets.ViewPort = 0x20D8C90;
			Offsets.Camera = 0x20D9868;
			Offsets.BlipList = 0x20D9B38;
			Offsets.LocalPlayer = 0x8;
			Offsets.PlayerInfo = 0x10C8;
			Offsets.Id = 0x88;
			Offsets.Health = 0x280;
			Offsets.MaxHealth = 0x2A0;
			Offsets.Armor = 0x1530;
			Offsets.WeaponManager = 0x10D8;
			Offsets.BoneList = 0x430;
			Offsets.Silent = 0xFF9D90;
			Offsets.Vehicle = 0xD30;
			Offsets.VisibleFlag = 0x147C;
			Offsets.Waypoint = 0x20E1420;
			auto GameWorld = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
			auto LocalPlayer = ReadMemory2<uintptr_t>(Game.World + Offsets.LocalPlayer);
			auto GameViewPort = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
			auto GameReplayInterface = ReadMemory2<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);
			if (GameWorld != 0 && GameViewPort != 0 && GameReplayInterface != 0) {
				findVersion = true;
				break;
			}
		}
	}

	if (!findVersion) {
		Game.Version = "NULL";
		mjLib::Logger::WriteLog(XorString("Failed to retrieve game version!"), mjLib::Logger::LogLevel::LOG_ERROR);
		mjLib::Console::ExitError();
		return false;
	}
	return true;
}

std::string GetGamePath() {
	WCHAR processPath[MAX_PATH] = { 0 };
	DWORD size = MAX_PATH;
	if (!QueryFullProcessImageNameW(Game.hProcess, 0, processPath, &size)) {
		return "";
	}


	int len = WideCharToMultiByte(CP_UTF8, 0, processPath, -1, NULL, 0, NULL, NULL);
	std::string fullPath;
	if (len > 0) {
		fullPath.resize(len - 1);
		WideCharToMultiByte(CP_UTF8, 0, processPath, -1, &fullPath[0], len, NULL, NULL);
	}
	std::string targetPath = XorString("FiveM.app\\");
	size_t position = fullPath.find(targetPath);
	if (position != std::string::npos) {
		return fullPath.substr(0, position + targetPath.size());
	}
	return fullPath;
}