#include "GameSDK.hpp"
#include "Ped.hpp"
#include <wininet.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <thread>
#include <chrono>
#include <DrawImGui.hpp>
#pragma comment(lib, "wininet.lib")

Ped LocalPlayer;
uintptr_t TPModelInfo = NULL;
uintptr_t TPNavigation = NULL;
Vector3 TPPosition = Vector3(0, 0, 0);
std::vector<Ped> PedList;


std::string ServerIp = "";
std::string ServerPort = "";
std::string FivemFolder = "";
std::string CrashoMetryLocation = "";
std::unordered_map<int, std::string> PlayerIdToName;
std::string PlayersInfo = "";
bool LanGame = false;
bool NamesThreadRunning = false;


struct VehicleData {
	uintptr_t pointer;
	Vector3 position;
	float health;
	bool isValid;
};
std::vector<VehicleData> VehicleList;


std::map<int, bool> friendStatus;

Vector3 EndBulletPos;
Vector3 SilentTargetPos = Vector3(0, 0, 0);

struct PedBarFix {
	int id;
	float health;
	float armor;
};

std::vector<PedBarFix> pedBarFix;



std::string DownloadServerInfo(const std::string& serverIp, int serverPort) {
	std::string result = "";
	std::string url = "http://" + serverIp + ":" + std::to_string(serverPort) + "/players.json";

	HINTERNET hInternet = InternetOpenA("FiveM-ESP", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInternet) {
		HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
		if (hConnect) {
			char buffer[4096];
			DWORD bytesRead;
			while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
				buffer[bytesRead] = '\0';
				result += buffer;
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hInternet);
	}
	return result;
}


void FindServerInfo() {
	if (ServerIp.size() > 2) return;


	if (FivemFolder.empty()) {
		HKEY hKey;
		WCHAR Buffer[MAX_PATH];
		DWORD BufferSize = sizeof(Buffer);
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\FiveM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			if (RegQueryValueExW(hKey, L"Last Run Location", NULL, NULL, (LPBYTE)Buffer, &BufferSize) == ERROR_SUCCESS) {
				std::wstring wPath(Buffer);
				FivemFolder = std::string(wPath.begin(), wPath.end());
				CrashoMetryLocation = FivemFolder + "data\\cache\\crashometry";
			}
			RegCloseKey(hKey);
		}
	}


	if (!CrashoMetryLocation.empty() && std::filesystem::exists(CrashoMetryLocation)) {
		std::ifstream file(CrashoMetryLocation, std::ios::binary);
		if (file.is_open()) {
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();


			size_t pos = content.find("last_server_url");
			if (pos != std::string::npos) {

				std::string urlPart = content.substr(pos + 15, 100);


				std::regex ipPortRegex(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d{1,5}))");
				std::smatch match;
				if (std::regex_search(urlPart, match, ipPortRegex)) {
					ServerIp = match[1].str();
					ServerPort = match[2].str();
				}
			}
		}
	}


	if (ServerIp.size() < 5 || ServerPort.size() < 2) {
		LanGame = true;
	}
}


void ParsePlayersJson(const std::string& jsonStr) {
	PlayerIdToName.clear();

	if (jsonStr.empty()) {
		printf("[DEBUG] JSON string is empty!\n");
		fflush(stdout);
		return;
	}

	printf("[DEBUG] Parsing JSON with multiple patterns...\n");
	fflush(stdout);


	std::vector<std::regex> patterns = {

		std::regex("\\{[^}]*\"id\"\\s*:\\s*(\\d+)[^}]*\"name\"\\s*:\\s*\"([^\"]+)\"[^}]*\\}"),

		std::regex("\\{[^}]*\"name\"\\s*:\\s*\"([^\"]+)\"[^}]*\"id\"\\s*:\\s*(\\d+)[^}]*\\}"),

		std::regex("\"id\"\\s*:\\s*(\\d+).*?\"name\"\\s*:\\s*\"([^\"]+)\""),

		std::regex("\"name\"\\s*:\\s*\"([^\"]+)\".*?\"id\"\\s*:\\s*(\\d+)")
	};

	bool foundAny = false;

	for (size_t patternIndex = 0; patternIndex < patterns.size(); patternIndex++) {
		printf("[DEBUG] Trying pattern %d...\n", (int)patternIndex + 1);
		fflush(stdout);

		std::sregex_iterator iter(jsonStr.begin(), jsonStr.end(), patterns[patternIndex]);
		std::sregex_iterator end;

		int matchCount = 0;
		for (; iter != end; ++iter) {
			std::smatch match = *iter;
			try {
				int playerId;
				std::string playerName;


				if (patternIndex == 1 || patternIndex == 3) {

					playerName = match[1].str();
					playerId = std::stoi(match[2].str());
				}
				else {

					playerId = std::stoi(match[1].str());
					playerName = match[2].str();
				}

				PlayerIdToName[playerId] = playerName;
				matchCount++;
				foundAny = true;

				printf("[DEBUG] Pattern %d matched - ID: %d, Name: %s\n",
					(int)patternIndex + 1, playerId, playerName.c_str());
				fflush(stdout);

			}
			catch (const std::exception& e) {
				printf("[DEBUG] Parse error in pattern %d: %s\n", (int)patternIndex + 1, e.what());
				fflush(stdout);
			}
		}

		printf("[DEBUG] Pattern %d found %d matches\n", (int)patternIndex + 1, matchCount);
		fflush(stdout);


		if (matchCount > 0) {
			printf("[DEBUG] Using pattern %d successfully!\n", (int)patternIndex + 1);
			fflush(stdout);
			break;
		}
	}

	if (!foundAny) {
		printf("[DEBUG] No matches found with any pattern. Raw JSON sample: %s\n",
			jsonStr.substr(0, 200).c_str());
		fflush(stdout);
	}
}


std::string GetServerInfo() {
	if (ServerIp.empty() || LanGame) {
		return "";
	}

	try {
		std::string rawInfo = DownloadServerInfo(ServerIp, std::stoi(ServerPort));
		return rawInfo;
	}
	catch (const std::exception& e) {

	}

	return "";
}







void UpdateNamesThread() {
	NamesThreadRunning = true;

	while (!exitLoop && NamesThreadRunning) {
		if (!LanGame && ServerIp.size() > 5 && ServerPort.size() > 2) {
			PlayersInfo = GetServerInfo();

			if (!PlayersInfo.empty()) {

				printf("[DEBUG] JSON Response: %s\n", PlayersInfo.substr(0, 200).c_str());
				fflush(stdout);


				ParsePlayersJson(PlayersInfo);


				printf("[DEBUG] Found %d players in JSON\n", (int)PlayerIdToName.size());
				for (const auto& pair : PlayerIdToName) {
					printf("[DEBUG] Player ID: %d, Name: %s\n", pair.first, pair.second.c_str());
				}
				fflush(stdout);
			}
			else {
				printf("[DEBUG] Empty JSON response from server\n");
				fflush(stdout);
			}

			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
		else {
			printf("[DEBUG] LanGame: %s, ServerIp: %s, ServerPort: %s\n",
				LanGame ? "true" : "false", ServerIp.c_str(), ServerPort.c_str());
			fflush(stdout);
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	NamesThreadRunning = false;
}

inline std::string GetPedName(Ped& ped) {
	int playerId = ped.GetId();


	printf("[DEBUG] GetPedName called for Player ID: %d\n", playerId);
	fflush(stdout);


	if (!LanGame && !PlayerIdToName.empty()) {
		printf("[DEBUG] Not LAN game, checking PlayerIdToName map (size: %d)\n", (int)PlayerIdToName.size());
		fflush(stdout);

		auto it = PlayerIdToName.find(playerId);
		if (it != PlayerIdToName.end() && !it->second.empty()) {
			printf("[DEBUG] Found name for Player ID %d: %s\n", playerId, it->second.c_str());
			fflush(stdout);
			return it->second;
		}
		else {
			printf("[DEBUG] No name found for Player ID %d in map\n", playerId);
			fflush(stdout);
		}
	}
	else {
		printf("[DEBUG] LAN game or empty PlayerIdToName map. LanGame: %s, Map size: %d\n",
			LanGame ? "true" : "false", (int)PlayerIdToName.size());
		fflush(stdout);
	}


	printf("[DEBUG] Returning fallback name for Player ID: %d\n", playerId);
	fflush(stdout);
	return "Player " + std::to_string(playerId);
}


int GetPlayerUnderCrosshair() {
	Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
	float crosshairX = (float)Game.lpRect.right / 2;
	float crosshairY = (float)Game.lpRect.bottom / 2;
	float minDistance = 50.0f;
	int closestPlayerID = -1;

	for (auto& ped : PedList) {
		if (!ped.Update()) continue;
		if (!ped.IsPlayer()) continue;

		Vector2 screenPos;
		if (!WorldToScreen(viewMatrix, ped.position, screenPos)) continue;


		float distance = sqrt(pow(screenPos.x - crosshairX, 2) + pow(screenPos.y - crosshairY, 2));

		if (distance < minDistance) {
			minDistance = distance;
			closestPlayerID = ped.GetId();
		}
	}

	return closestPlayerID;
}

void AddFriend(int playerID) {
	if (playerID != -1) {
		friendStatus[playerID] = true;
		printf("[FRIEND] Player ID %d added as friend\n", playerID);
		fflush(stdout);
	}
}

void RemoveFriend(int playerID) {
	if (playerID != -1) {
		friendStatus[playerID] = false;
		printf("[FRIEND] Player ID %d removed from friends\n", playerID);
		fflush(stdout);
	}
}

bool IsFriend(int playerID) {
	auto it = friendStatus.find(playerID);
	return (it != friendStatus.end() && it->second);
}

void UpdatePeds() {
	while (!exitLoop) {
		std::vector<Ped> updatedPedList;
		std::vector<VehicleData> updatedVehicleList;

		Game.World = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.GameWorld);
		LocalPlayer.pointer = ReadMemory<uintptr_t>(Game.World + Offsets.LocalPlayer);
		Game.ViewPort = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ViewPort);
		Game.ReplayInterface = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.ReplayInterface);


		uintptr_t entityListPtr = ReadMemory<uintptr_t>(Game.ReplayInterface + 0x18);
		uintptr_t entityList = ReadMemory<uintptr_t>(entityListPtr + 0x100);

		for (int i = 0; i < (int)maxPlayerCount; i++) {
			Ped ped;
			uintptr_t player = ReadMemory<uintptr_t>(entityList + (i * 0x10));
			if (player == LocalPlayer.pointer) {
				continue;
			}
			else if (!ped.GetPlayer(player)) {
				continue;
			}
			else if (!ped.Update()) {
				continue;
			}
			updatedPedList.push_back(ped);
		}


		uintptr_t vehicleInterface = ReadMemory<DWORD64>(Game.ReplayInterface + 0x10);
		if (vehicleInterface) {
			uintptr_t vehicleList = ReadMemory<DWORD64>(vehicleInterface + 0x180);
			if (vehicleList) {
				int vehicleListCount = ReadMemory<int>(vehicleInterface + 0x188);
				if (vehicleListCount > 0 && vehicleListCount <= 300) {
					int maxVehicles = min(vehicleListCount, 30);

					for (int i = 0; i < maxVehicles; ++i) {
						uintptr_t vehicle = ReadMemory<uintptr_t>(vehicleList + (i * 0x10));
						if (!vehicle) continue;

						Vector3 vehiclePos = ReadMemory<Vector3>(vehicle + 0x90);
						if (vehiclePos.x == 0.0f && vehiclePos.y == 0.0f && vehiclePos.z == 0.0f) {
							continue;
						}

						VehicleData vData;
						vData.pointer = vehicle;
						vData.position = vehiclePos;
						vData.health = ReadMemory<float>(vehicle + Offsets.Health);
						vData.isValid = true;

						updatedVehicleList.push_back(vData);
					}
				}
			}
		}

		PedList = updatedPedList;
		VehicleList = updatedVehicleList;
		Sleep(loopDelay);
	}
}

namespace Draw {
	void Always() {
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		if (Cheats::AimAssist::Aimbot::DrawFov) {
			drawList->AddCircle(ImVec2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f), Cheats::AimAssist::Aimbot::Fov, Cheats::AimAssist::Aimbot::Color, 100, 1.0f);
		}

		if (Cheats::AimAssist::Silent::DrawFov) {
			ImVec2 centerPos = ImVec2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
			float fov_radius = Cheats::AimAssist::Silent::Fov;


			if (Cheats::MenuUtils::DynamicFOV && !PedList.empty()) {

				float closestDistance = FLT_MAX;
				bool foundTarget = false;

				for (auto& ped : PedList) {
					if (!ped.Update() || ped.IsDead()) continue;

					float distance = GetDistance(ped.position, LocalPlayer.position);
					if (distance < closestDistance && distance <= Cheats::AimAssist::Silent::Distance) {
						closestDistance = distance;
						foundTarget = true;
					}
				}

				if (foundTarget) {

					fov_radius = std::clamp(500.0f / closestDistance, 2.0f, 70.0f);
				}
			}

			drawList->AddCircle(centerPos, fov_radius, Cheats::AimAssist::Silent::Color, 100, 1.0f);
		}

		if (Cheats::AimAssist::Triggerbot::DrawFov) {
			drawList->AddCircle(ImVec2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f), Cheats::AimAssist::Triggerbot::Fov, Cheats::AimAssist::Triggerbot::Color, 100, 1.0f);
		}


		if (Cheats::AimAssist::Silent::DrawTarget && !Vec3Empty(SilentTargetPos)) {

			Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
			Vector2 targetScreenPos;

			if (WorldToScreen(viewMatrix, SilentTargetPos, targetScreenPos)) {
				float circleRadius = 5.0f;
				ImColor targetColor = Cheats::AimAssist::Silent::TargetColor;


				drawList->AddCircle(ImVec2(targetScreenPos.x, targetScreenPos.y), circleRadius, targetColor, 12, 3.0f);
			}
		}


		if (Cheats::AimAssist::Silent::DrawSilentLine && !Vec3Empty(SilentTargetPos)) {

			Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
			Vector2 targetScreenPos;

			if (WorldToScreen(viewMatrix, SilentTargetPos, targetScreenPos)) {

				ImVec2 crosshairPos = ImVec2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f);
				ImVec2 targetPos = ImVec2(targetScreenPos.x, targetScreenPos.y);

				ImColor lineColor = Cheats::AimAssist::Silent::SilentLineColor;


				if (Cheats::MenuUtils::OutlineEnabled) {
					drawList->AddLine(crosshairPos, targetPos, ImColor(0, 0, 0, 255), 3.0f);
				}


				drawList->AddLine(crosshairPos, targetPos, lineColor, 1.5f);
			}
		}


		if (Cheats::PeekAssist::Enabled && Cheats::PeekAssist::HasMarkedPosition && Cheats::PeekAssist::ShowMarker) {
			Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
			Vector2 markerScreenPos;

			if (WorldToScreen(viewMatrix, Cheats::PeekAssist::MarkedPosition, markerScreenPos)) {

				float markerRadius = 15.0f;
				ImColor markerColor = ImColor(0, 255, 0, 200);


				drawList->AddCircleFilled(ImVec2(markerScreenPos.x, markerScreenPos.y),
					markerRadius, markerColor);


				drawList->AddCircle(ImVec2(markerScreenPos.x, markerScreenPos.y),
					markerRadius, ImColor(0, 0, 0, 255), 16, 3.0f);


				drawList->AddCircleFilled(ImVec2(markerScreenPos.x, markerScreenPos.y),
					4.0f, ImColor(255, 255, 255, 255));


				static float animTime = 0.0f;
				animTime += ImGui::GetIO().DeltaTime * 2.0f;
				if (animTime > 6.28f) animTime = 0.0f;

				float pulseRadius = markerRadius + 5.0f + sin(animTime) * 3.0f;
				ImColor pulseColor = ImColor(0, 255, 0, (int)(100 + sin(animTime) * 50));
				drawList->AddCircle(ImVec2(markerScreenPos.x, markerScreenPos.y),
					pulseRadius, pulseColor, 16, 2.0f);
			}
		}
	}

	void Esp() {
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();


		Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);

		if (!LocalPlayer.Update()) {
			return;
		}


		int maxPedsToRender = (int)PedList.size();


		for (int i = 0; i < maxPedsToRender; i++) {
			auto& ped = PedList[i];
			if (!ped.Update()) {
				continue;
			}

			float pDistance = GetDistance(ped.position, LocalPlayer.position);
			if (pDistance >= Cheats::Players::Distance) {
				continue;
			}
			if (Cheats::Players::IgnorePed && !ped.IsPlayer()) {
				continue;
			}

			if (Cheats::Players::IgnoreDeath && ped.IsDead()) {
				continue;
			}

			if (Cheats::Players::OnlyVisible && !ped.IsVisible()) {
				continue;
			}

			int pedID = ped.GetId();
			Vector2 pBase{}, pHead{}, pNeck{}, pLeftFoot{}, pRightFoot{};
			if (!WorldToScreen(viewMatrix, ped.position, pBase) ||
				!WorldToScreen(viewMatrix, ped.boneList[Head], pHead) ||
				!WorldToScreen(viewMatrix, ped.boneList[Neck], pNeck) ||
				!WorldToScreen(viewMatrix, ped.boneList[LeftFoot], pLeftFoot) ||
				!WorldToScreen(viewMatrix, ped.boneList[RightFoot], pRightFoot)) {
				continue;
			}

			float HeadToNeck = pNeck.y - pHead.y;
			float pTop = pHead.y - (HeadToNeck * 2.5f);
			float pBottom = (pLeftFoot.y > pRightFoot.y ? pLeftFoot.y : pRightFoot.y) * 1.001f;
			float pHeight = pBottom - pTop;
			float pWidth = pHeight / 3.5f;
			float bScale = pWidth / 1.5f;
			float reducedWidth = pWidth * Cheats::Players::DrawBox::Size;
			ImVec2 pos(pBase.x, pBottom + 2.0f);
			float spacing = 2.0f;
			float baseSize = 15.0f;
			float scaledSize = baseSize;
			if (pDistance > 100.0f) {
				scaledSize = baseSize * (100.0f / pDistance);
				if (scaledSize < 8.0f) {
					scaledSize = 8.0f;
				}
			}


			if (Cheats::Players::DrawSkeleton::Enabled) {
				ImColor color = Cheats::Players::DrawSkeleton::Color;


				if (IsFriend(pedID)) {
					color = ImColor(0, 255, 0, 255);
				}
				else if (!ped.IsVisible()) {
					color = ImColor(255, 255, 255);
				}

				Vector2 screenHeadPos;
				for (int j = 0; j < 5; j++) {
					Vector3 skeletonList[][2] = {
						{ ped.boneList[Neck], ped.boneList[Hip] },
						{ ped.boneList[Neck], ped.boneList[LeftHand] },
						{ ped.boneList[Neck], ped.boneList[RightHand] },
						{ ped.boneList[Hip], ped.boneList[LeftFoot] },
						{ ped.boneList[Hip], ped.boneList[RightFoot] }
					};

					Vector2 ScreenB1, ScreenB2;
					if (Vec3Empty(skeletonList[j][0]) || Vec3Empty(skeletonList[j][1])) {
						break;
					}
					else if (!WorldToScreen(viewMatrix, skeletonList[j][0], ScreenB1) || !WorldToScreen(viewMatrix, skeletonList[j][1], ScreenB2)) {
						break;
					}

					if (Cheats::MenuUtils::OutlineEnabled) {
						DrawLine(drawList, ImVec2(ScreenB1.x, ScreenB1.y), ImVec2(ScreenB2.x, ScreenB2.y), ImColor(0, 0, 0, 255), 3);
					}
					DrawLine(drawList, ImVec2(ScreenB1.x, ScreenB1.y), ImVec2(ScreenB2.x, ScreenB2.y), color, 1);
				}
			}


			if (Cheats::Players::DrawHeadDot::Enabled) {
				ImColor color = Cheats::Players::DrawHeadDot::Color;


				if (IsFriend(pedID)) {
					color = ImColor(0, 255, 0, 255);
				}
				else if (!ped.IsVisible()) {
					color = ImColor(255, 255, 255);
				}


				float dotRadius = 8.0f;


				if (pDistance > 100.0f) {
					dotRadius = 8.0f * (100.0f / pDistance);
					if (dotRadius < 3.0f) {
						dotRadius = 3.0f;
					}
				}


				if (Cheats::MenuUtils::OutlineEnabled) {

					drawList->AddCircle(ImVec2(pHead.x, pHead.y), dotRadius, ImColor(0, 0, 0, 255), 16, 3.0f);
				}


				drawList->AddCircle(ImVec2(pHead.x, pHead.y), dotRadius, color, 16, 1.0f);
			}

			if (Cheats::Players::DrawBox::Enabled) {
				ImColor color = Cheats::Players::DrawBox::Color;
				ImVec4 mainColor = color.Value;

				auto DrawBoxWithGradient = [&](bool isCornerBox) {
					if (Cheats::Players::DrawBox::GradientEnabled) {
						ImColor topColor, bottomColor;
						if (Cheats::Players::DrawBox::UseCustomGradient) {
							topColor = Cheats::Players::DrawBox::BoxGradientTopColor;
							topColor.Value.w *= Cheats::Players::DrawBox::GradientIntensity;
							bottomColor = Cheats::Players::DrawBox::BoxGradientBottomColor;
							bottomColor.Value.w *= Cheats::Players::DrawBox::GradientIntensity;
						}
						else {
							float intensity = Cheats::Players::DrawBox::GradientIntensity;
							topColor = ImColor(mainColor.x * 0.6f, mainColor.y * 0.6f, mainColor.z * 0.6f, 0.25f * intensity);
							bottomColor = ImColor(mainColor.x * 0.1f, mainColor.y * 0.1f, mainColor.z * 0.1f, 0.35f * intensity);
						}

						drawList->AddRectFilledMultiColor(ImVec2(pBase.x - reducedWidth + 1, pTop + 1), ImVec2(pBase.x + reducedWidth - 1, pBottom - 1), ImGui::ColorConvertFloat4ToU32(topColor), ImGui::ColorConvertFloat4ToU32(topColor), ImGui::ColorConvertFloat4ToU32(bottomColor), ImGui::ColorConvertFloat4ToU32(bottomColor));
					}

					if (!isCornerBox) {
						DrawLineOutline(ImVec2(pBase.x - reducedWidth, pTop), ImVec2(pBase.x + reducedWidth, pTop), color, 1.f);
						DrawLineOutline(ImVec2(pBase.x - reducedWidth, pTop), ImVec2(pBase.x - reducedWidth, pBottom), color, 1.f);
						DrawLineOutline(ImVec2(pBase.x + reducedWidth, pTop), ImVec2(pBase.x + reducedWidth, pBottom), color, 1.f);
						DrawLineOutline(ImVec2(pBase.x - reducedWidth, pBottom), ImVec2(pBase.x + reducedWidth, pBottom), color, 1.f);
					}
					else {
						DrawLineOutline(ImVec2((pBase.x - reducedWidth), pTop), ImVec2((pBase.x - reducedWidth) + bScale, pTop), color, 1.f);
						DrawLineOutline(ImVec2((pBase.x + reducedWidth), pTop), ImVec2((pBase.x + reducedWidth) - bScale, pTop), color, 1.f);
						DrawLineOutline(ImVec2(pBase.x - reducedWidth, pTop), ImVec2(pBase.x - reducedWidth, pTop + bScale), color, 1.f);
						DrawLineOutline(ImVec2(pBase.x - reducedWidth, pBottom), ImVec2(pBase.x - reducedWidth, pBottom - bScale), color, 1.f);
						DrawLineOutline(ImVec2(pBase.x + reducedWidth, pTop), ImVec2(pBase.x + reducedWidth, pTop + bScale), color, 1.f);
						DrawLineOutline(ImVec2(pBase.x + reducedWidth, pBottom), ImVec2(pBase.x + reducedWidth, pBottom - bScale), color, 1.f);
						DrawLineOutline(ImVec2((pBase.x - reducedWidth), pBottom), ImVec2((pBase.x - reducedWidth) + bScale, pBottom), color, 1.f);
						DrawLineOutline(ImVec2((pBase.x + reducedWidth), pBottom), ImVec2((pBase.x + reducedWidth) - bScale, pBottom), color, 1.f);
					}
					};

				if (Cheats::Players::DrawBox::SelectedType == 0) {
					DrawBoxWithGradient(false);
				}
				else if (Cheats::Players::DrawBox::SelectedType == 1) {
					DrawBoxWithGradient(true);
				}
			}

			if (Cheats::Players::DrawLine::Enabled) {
				ImColor color = Cheats::Players::DrawLine::Color;
				ImVec2 startPos, endPos;
				ImGuiIO& io = ImGui::GetIO();
				if (Cheats::Players::DrawLine::SelectedType == 0) {
					startPos.x = pBase.x + reducedWidth;
					startPos.y = pTop;
					endPos.x = Game.lpRect.right / 2.f;
					endPos.y = 0;
				}
				if (Cheats::Players::DrawLine::SelectedType == 1) {
					startPos.x = pBase.x + reducedWidth;
					startPos.y = pTop;
					endPos.x = io.DisplaySize.x / 2.0f;
					endPos.y = io.DisplaySize.y / 2.0f;
				}
				if (Cheats::Players::DrawLine::SelectedType == 2) {
					startPos.x = pBase.x;
					startPos.y = pBottom;
					endPos.x = io.DisplaySize.x / 2.0f;
					endPos.y = io.DisplaySize.y;
				}
				DrawLineOutline(startPos, endPos, color, 1.f);
			}

			if (Cheats::Players::DrawId::Enabled) {
				ImColor color = Cheats::Players::DrawId::Color;
				if (!ped.IsVisible()) {
					color = DarkenColor(color, 0.3f);
				}

				std::string pedId = std::to_string(pedID);
				if (!pedId.empty()) {
					ImGui::PushFont(FrameWork::Assets::InterMedium12);
					ImVec2 textSize = ImGui::CalcTextSize(pedId.c_str());
					ImVec2 drawPos = ImVec2(pBase.x - textSize.x / 2.0f, pTop - 27.0f);

					ImGui::GetBackgroundDrawList()->AddText(drawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, color.Value.w), pedId.c_str());
					ImGui::GetBackgroundDrawList()->AddText(drawPos, color, pedId.c_str());
					ImGui::PopFont();
				}
			}

			if (Cheats::Players::DrawName::Enabled) {
				ImColor color = Cheats::Players::DrawName::Color;
				if (!ped.IsVisible()) {
					color = DarkenColor(color, 0.3f);
				}

				std::string pedName = GetPedName(ped);
				if (!ped.IsPlayer()) {
					pedName = XorString("NPC");
				}

				if (!pedName.empty()) {
					ImGui::PushFont(FrameWork::Assets::InterMedium12);
					ImVec2 textSize = ImGui::CalcTextSize(pedName.c_str());
					ImVec2 drawPos = ImVec2(pBase.x - textSize.x / 2.0f, pTop - 15.0f);

					ImGui::GetBackgroundDrawList()->AddText(drawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, color.Value.w), pedName.c_str());
					ImGui::GetBackgroundDrawList()->AddText(drawPos, color, pedName.c_str());
					ImGui::PopFont();
				}
			}

			if (Cheats::Players::DrawHealth::Enabled || Cheats::Players::DrawArmor::Enabled) {
				PedBarFix fix;
				fix.id = pedID;
				fix.health = ped.health;
				fix.armor = ped.armor;
				bool exists = false;
				for (auto& item : pedBarFix) {
					if (item.id == pedID) {
						if (ped.health > 0) {
							item.health = ped.health;
						}
						else {
							ped.health = item.health;
						}

						if (ped.armor > 0) {
							item.armor = ped.armor;
						}
						else {

							item.armor = 0;
							ped.armor = 0;
						}
						exists = true;
						break;
					}
				}
				if (!exists) {
					pedBarFix.push_back(fix);
				}
			}

			if (Cheats::Players::DrawHealth::Enabled) {
				float healthRatio = (float(ped.health) - 100.0f) / 100.0f;
				if (healthRatio < 0.0f) healthRatio = 0.0f;
				if (healthRatio > 1.0f) healthRatio = 1.0f;

				float barWidth, barHeight;
				ImVec2 barPos, barEnd;


				if (Cheats::Players::DrawHealth::SelectedPosition == 0 || Cheats::Players::DrawHealth::SelectedPosition == 1) {
					barWidth = (pBase.x + reducedWidth) - (pBase.x - reducedWidth);
					barHeight = (pDistance < 90.0f) ? 4.0f : 2.0f;

					if (Cheats::Players::DrawHealth::SelectedPosition == 0) {
						barPos = ImVec2(pBase.x - reducedWidth, pos.y);
					}
					else {
						barPos = ImVec2(pBase.x - reducedWidth, pTop - barHeight - 2.0f);
					}
					barEnd = ImVec2(barPos.x + barWidth, barPos.y + barHeight);
				}
				else {
					barWidth = (pDistance < 90.0f) ? 4.0f : 2.0f;
					barHeight = pBottom - pTop;

					if (Cheats::Players::DrawHealth::SelectedPosition == 2) {

						barPos = ImVec2(pBase.x - reducedWidth - 2.0f - barWidth, pTop);
					}
					else {
						barPos = ImVec2(pBase.x + reducedWidth + 2.0f, pTop);
					}
					barEnd = ImVec2(barPos.x + barWidth, barPos.y + barHeight);
				}

				ImU32 bgColor = IM_COL32(0, 0, 0, 255);
				ImU32 borderColor = IM_COL32(0, 0, 0, 255);
				ImVec4 colLow(1.0f, 0.0f, 0.0f, 1.0f);
				ImVec4 colHigh(0.0f, 1.0f, 0.0f, 1.0f);

				drawList->AddRectFilled(barPos, barEnd, bgColor);

				if (Cheats::Players::DrawHealth::SelectedPosition == 0 || Cheats::Players::DrawHealth::SelectedPosition == 1) {
					float filledWidth = barWidth * healthRatio;
					ImVec4 currentColor(colLow.x + (colHigh.x - colLow.x) * healthRatio, colLow.y + (colHigh.y - colLow.y) * healthRatio, colLow.z + (colHigh.z - colLow.z) * healthRatio, 1.0f);
					drawList->AddRectFilled(ImVec2(barPos.x, barPos.y), ImVec2(barPos.x + filledWidth, barPos.y + barHeight), ImGui::ColorConvertFloat4ToU32(currentColor));
				}
				else {
					float filledHeight = barHeight * healthRatio;
					ImVec4 currentColor(colLow.x + (colHigh.x - colLow.x) * healthRatio, colLow.y + (colHigh.y - colLow.y) * healthRatio, colLow.z + (colHigh.z - colLow.z) * healthRatio, 1.0f);
					drawList->AddRectFilled(ImVec2(barPos.x, barEnd.y - filledHeight), ImVec2(barEnd.x, barEnd.y), ImGui::ColorConvertFloat4ToU32(currentColor));
				}

				drawList->AddRect(barPos, barEnd, borderColor);

				if (Cheats::Players::DrawHealth::SelectedPosition == 0) {
					pos.y += barHeight + 2.0f;
				}
			}

			if (Cheats::Players::DrawArmor::Enabled && ped.armor > 0.0f) {
				float armorRatio = ped.armor / 100.0f;
				if (armorRatio < 0.0f) armorRatio = 0.0f;
				if (armorRatio > 1.0f) armorRatio = 1.0f;

				float barWidth, barHeight;
				ImVec2 barPos, barEnd;


				if (Cheats::Players::DrawArmor::SelectedPosition == 2 || Cheats::Players::DrawArmor::SelectedPosition == 3) {
					barWidth = (pBase.x + reducedWidth) - (pBase.x - reducedWidth);
					barHeight = (pDistance < 90.0f) ? 4.0f : 2.0f;

					if (Cheats::Players::DrawArmor::SelectedPosition == 2) {
						barPos = ImVec2(pBase.x - reducedWidth, pTop - barHeight - 2.0f);
					}
					else {
						barPos = ImVec2(pBase.x - reducedWidth, pos.y);
					}
					barEnd = ImVec2(barPos.x + barWidth, barPos.y + barHeight);
				}
				else {
					barWidth = (pDistance < 90.0f) ? 4.0f : 2.0f;
					barHeight = pBottom - pTop;

					if (Cheats::Players::DrawArmor::SelectedPosition == 0) {

						if (Cheats::Players::DrawHealth::Enabled && Cheats::Players::DrawHealth::SelectedPosition == 2) {

							float healthBarWidth = (pDistance < 90.0f) ? 4.0f : 2.0f;
							float spacing = 2.0f;
							barPos = ImVec2(pBase.x - reducedWidth - 2.0f - healthBarWidth - spacing - barWidth, pTop);
						}
						else {

							barPos = ImVec2(pBase.x - reducedWidth - 2.0f - barWidth, pTop);
						}
					}
					else {

						float healthBarWidth = (pDistance < 90.0f) ? 4.0f : 2.0f;
						float spacing = 1.0f;
						barPos = ImVec2(pBase.x + reducedWidth + 2.0f + healthBarWidth + spacing, pTop);
					}
					barEnd = ImVec2(barPos.x + barWidth, barPos.y + barHeight);
				}


				ImU32 bgColor = IM_COL32(0, 0, 0, 255);
				ImU32 borderColor = IM_COL32(0, 0, 0, 255);

				drawList->AddRectFilled(barPos, barEnd, bgColor);


				ImVec4 armorColor(0.416f, 0.475f, 0.612f, 1.0f);

				if (Cheats::Players::DrawArmor::SelectedPosition == 2 || Cheats::Players::DrawArmor::SelectedPosition == 3) {
					float filledWidth = barWidth * armorRatio;
					drawList->AddRectFilled(ImVec2(barPos.x, barPos.y), ImVec2(barPos.x + filledWidth, barPos.y + barHeight), ImGui::ColorConvertFloat4ToU32(armorColor));
				}
				else {
					float filledHeight = barHeight * armorRatio;
					drawList->AddRectFilled(ImVec2(barPos.x, barEnd.y - filledHeight), ImVec2(barEnd.x, barEnd.y), ImGui::ColorConvertFloat4ToU32(armorColor));
				}


				drawList->AddRect(barPos, barEnd, borderColor);

				if (Cheats::Players::DrawArmor::SelectedPosition == 3) {
					pos.y += barHeight + 2.0f;
				}
			}

			if (Cheats::Players::DrawWeaponName::Enabled) {
				ImColor color = Cheats::Players::DrawWeaponName::Color;
				if (!ped.IsVisible()) {
					color = DarkenColor(color, 0.3f);
				}
				std::string weaponName = ped.GetWeaponName();
				if (!weaponName.empty())
				{
					ImGui::PushFont(FrameWork::Assets::InterMedium12);
					ImVec2 textSize = ImGui::CalcTextSize(weaponName.c_str());
					ImVec2 drawPos = ImVec2(pBase.x - textSize.x / 2.0f, pos.y);

					ImGui::GetBackgroundDrawList()->AddText(drawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, color.Value.w), weaponName.c_str());
					ImGui::GetBackgroundDrawList()->AddText(drawPos, color, weaponName.c_str());
					ImGui::PopFont();
					pos.y += textSize.y + 1.0f;
				}
			}

			if (Cheats::Players::DrawDistance::Enabled) {
				ImColor color = Cheats::Players::DrawDistance::Color;
				if (!ped.IsVisible()) {
					color = DarkenColor(color, 0.3f);
				}
				std::string dataText = "[" + std::to_string((int)pDistance) + "m]";

				ImGui::PushFont(FrameWork::Assets::InterMedium12);
				ImVec2 textSize = ImGui::CalcTextSize(dataText.c_str());
				ImVec2 drawPos = ImVec2(pBase.x - textSize.x / 2.0f, pos.y);

				ImGui::GetBackgroundDrawList()->AddText(drawPos + ImVec2(1, 1), ImColor(0.f, 0.f, 0.f, color.Value.w), dataText.c_str());
				ImGui::GetBackgroundDrawList()->AddText(drawPos, color, dataText.c_str());
				ImGui::PopFont();
			}


			if (Cheats::Players::DirectionEsp::Enabled) {
				ImColor color = ConvertToImColor((float*)&Cheats::Players::DirectionEsp::Color);
				if (!ped.IsVisible()) {
					color = DarkenColor(color, 0.3f);
				}
				ImVec2 screenCenter = ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2);
				ImVec2 directionToPed = ImVec2(pBase.x - screenCenter.x, pBase.y - screenCenter.y);
				float length = sqrt(directionToPed.x * directionToPed.x + directionToPed.y * directionToPed.y);
				directionToPed.x /= length;
				directionToPed.y /= length;

				static float animationTime = 0.0f;
				animationTime += ImGui::GetIO().DeltaTime * 0.10f;
				if (animationTime > 1.0f)
					animationTime = 0.0f;

				float arrowDistanceFromCenter = 100.0f + sin(animationTime * 3.14f) * 10.0f;
				ImVec2 arrowPos = ImVec2(screenCenter.x + directionToPed.x * arrowDistanceFromCenter, screenCenter.y + directionToPed.y * arrowDistanceFromCenter);
				ImVec2 arrowEndPos = ImVec2(screenCenter.x + directionToPed.x * (arrowDistanceFromCenter + 20.0f), screenCenter.y + directionToPed.y * (arrowDistanceFromCenter + 20.0f));
				ImColor animatedColor = color;
				animatedColor.Value.w = 0.8f + sin(animationTime * 6.28f) * 0.2f;
				DrawArrowTriangleOutlined(drawList, arrowPos, arrowEndPos, animatedColor, 2.0f);
			}
		}
	}

	void Vehicle() {
		if (!Cheats::Vehicle::Enabled) {
			return;
		}


		Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);

		if (!LocalPlayer.Update()) {
			return;
		}

		ImDrawList* drawList = ImGui::GetBackgroundDrawList();


		uintptr_t vehicleInterface = ReadMemory<DWORD64>(Game.ReplayInterface + 0x10);
		if (!vehicleInterface) return;

		uintptr_t vehicleList = ReadMemory<DWORD64>(vehicleInterface + 0x180);
		if (!vehicleList) return;

		int vehicleListCount = ReadMemory<int>(vehicleInterface + 0x188);
		if (vehicleListCount <= 0 || vehicleListCount > 300) return;


		int maxVehicles = min(vehicleListCount, 30);

		for (int i = 0; i < maxVehicles; ++i) {
			uintptr_t vehicle = ReadMemory<uintptr_t>(vehicleList + (i * 0x10));
			if (!vehicle) continue;

			Vector3 vehiclePos = ReadMemory<Vector3>(vehicle + 0x90);
			if (vehiclePos.x == 0.0f && vehiclePos.y == 0.0f && vehiclePos.z == 0.0f) {
				continue;
			}

			Vector2 vehicleLocation;
			if (!WorldToScreen(viewMatrix, vehiclePos, vehicleLocation)) {
				continue;
			}


			if (vehicleLocation.x < 0 || vehicleLocation.y < 0 ||
				vehicleLocation.x > Game.lpRect.right || vehicleLocation.y > Game.lpRect.bottom) {
				continue;
			}


			float vDistance = GetDistance(vehiclePos, LocalPlayer.position);
			if (vDistance >= Cheats::Vehicle::Distance) {
				continue;
			}


			bool isLocalVehicle = (vDistance < 3.0f);
			if (isLocalVehicle && !Cheats::Vehicle::DrawLocalVehicle) {
				continue;
			}
			if (!isLocalVehicle && !Cheats::Vehicle::DrawEnemyVehicle) {
				continue;
			}


			if (Cheats::Vehicle::VehicleHealth) {
				float vehicleHealth = ReadMemory<float>(vehicle + Offsets.Health);
				if (vehicleHealth > 0.0f) {
					float vehicleMaxHealth = 1000.0f;
					float healthRatio = vehicleHealth / vehicleMaxHealth;
					if (healthRatio < 0.0f) healthRatio = 0.0f;
					if (healthRatio > 1.0f) healthRatio = 1.0f;

					float barWidth = 50.0f;
					float barHeight = 6.0f;
					float filledWidth = barWidth * healthRatio;

					ImVec2 barPos(vehicleLocation.x - barWidth / 2, vehicleLocation.y - 25);
					ImVec2 barEnd(barPos.x + barWidth, barPos.y + barHeight);

					ImU32 bgColor = IM_COL32(0, 0, 0, 255);
					ImU32 borderColor = IM_COL32(0, 0, 0, 255);

					drawList->AddRectFilled(barPos, barEnd, bgColor);
					if (filledWidth > 0) {
						drawList->AddRectFilled(ImVec2(barPos.x, barPos.y),
							ImVec2(barPos.x + filledWidth, barPos.y + barHeight),
							Cheats::Vehicle::HealthBarColor);
					}
					drawList->AddRect(barPos, barEnd, borderColor);
				}
			}


			if (Cheats::Vehicle::VehicleEspShowDistance) {
				ImFont* pFonts = Menu.Tahoma;
				ImGui::PushFont(pFonts);
				std::string distanceText = "[" + std::to_string((int)vDistance) + "m]";

				ImVec2 textPos(vehicleLocation.x, vehicleLocation.y + 15);
				DrawOutlinedText(drawList, pFonts, distanceText, textPos + ImVec2(1, 1), 13.f, ImColor(0, 0, 0, 150), true);
				DrawOutlinedText(drawList, pFonts, distanceText, textPos, 13.f, Cheats::Vehicle::DistanceColor, true);

				ImGui::PopFont();
			}


			if (Cheats::Vehicle::VehicleEspSnapline) {
				ImVec2 screenCenter(Game.lpRect.right / 2.f, Game.lpRect.bottom);
				drawList->AddLine(screenCenter, ImVec2(vehicleLocation.x, vehicleLocation.y),
					Cheats::Vehicle::SnaplineColor, 1.0f);
			}


			if (Cheats::Vehicle::VehicleMarker) {
				ImVec2 markerPos(vehicleLocation.x, vehicleLocation.y + 40);
				drawList->AddCircle(markerPos, 5, Cheats::Vehicle::MarkerOuterColor, 12, 2.0f);
				drawList->AddCircleFilled(markerPos, 3, Cheats::Vehicle::MarkerInnerColor);
			}
		}
	}

}

void RestoreSilent() {
	std::vector<uint8_t> ReWriteTable =
	{
		0xF3, 0x41, 0x0F, 0x10, 0x19,
		0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04,
		0xF3, 0x41, 0x0F, 0x10, 0x51, 0x08
	};

	WriteBytes(Offsets.GameBase + Offsets.Silent, &ReWriteTable[0], ReWriteTable.size());
	std::vector<uint8_t> AngleReWriteTable =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00
	};
	WriteBytes(Offsets.GameBase + 0x34E, &AngleReWriteTable[0], AngleReWriteTable.size());
}

void ApplySilent() {
	static uint64_t HandleBulletAddress = Offsets.GameBase + Offsets.Silent;
	static uint64_t AllocPtr = Offsets.GameBase + 0x34E;

	auto CalculateRelativeOffset = [](uint64_t CurrentAddress, uint64_t TargetAddress, int Offset = 5) {
		intptr_t RelativeOffset = static_cast<intptr_t>(TargetAddress - (CurrentAddress + Offset));
		return static_cast<uint32_t>(RelativeOffset);
		};

	union
	{
		float f;
		uint32_t i;
	} EndPosX, EndPosY, EndPosZ;

	EndPosX.f = EndBulletPos.x;
	EndPosY.f = EndBulletPos.y;
	EndPosZ.f = EndBulletPos.z;

	{
		std::vector<uint8_t> ReWriteTable =
		{
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		uint32_t JmpOffset = CalculateRelativeOffset(HandleBulletAddress, AllocPtr);
		ReWriteTable[1] = static_cast<uint8_t>(JmpOffset & 0xFF);
		ReWriteTable[2] = static_cast<uint8_t>((JmpOffset >> 8) & 0xFF);
		ReWriteTable[3] = static_cast<uint8_t>((JmpOffset >> 16) & 0xFF);
		ReWriteTable[4] = static_cast<uint8_t>((JmpOffset >> 24) & 0xFF);
		WriteBytes(HandleBulletAddress, &ReWriteTable[0], ReWriteTable.size());
	}

	{
		uintptr_t currentAddress = (uintptr_t)AllocPtr;
		uintptr_t targetAddress = (uintptr_t)(HandleBulletAddress);
		intptr_t relativeOffset = static_cast<intptr_t>(targetAddress - (currentAddress + 28));
		uint32_t jmpOffset = static_cast<uint32_t>(relativeOffset);

		std::vector<uint8_t> ReWriteTable =
		{
			0x41, 0xC7, 0x01, static_cast<uint8_t>(EndPosX.i), static_cast<uint8_t>(EndPosX.i >> 8), static_cast<uint8_t>(EndPosX.i >> 16), static_cast<uint8_t>(EndPosX.i >> 24),
			0x41, 0xC7, 0x41, 0x04, static_cast<uint8_t>(EndPosY.i), static_cast<uint8_t>(EndPosY.i >> 8), static_cast<uint8_t>(EndPosY.i >> 16), static_cast<uint8_t>(EndPosY.i >> 24),
			0x41, 0xC7, 0x41, 0x08, static_cast<uint8_t>(EndPosZ.i), static_cast<uint8_t>(EndPosZ.i >> 8), static_cast<uint8_t>(EndPosZ.i >> 16), static_cast<uint8_t>(EndPosZ.i >> 24),
			0xF3, 0x41, 0x0F, 0x10, 0x19,
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		ReWriteTable[29] = static_cast<uint8_t>(jmpOffset & 0xFF);
		ReWriteTable[30] = static_cast<uint8_t>((jmpOffset >> 8) & 0xFF);
		ReWriteTable[31] = static_cast<uint8_t>((jmpOffset >> 16) & 0xFF);
		ReWriteTable[32] = static_cast<uint8_t>((jmpOffset >> 24) & 0xFF);
		WriteBytes(AllocPtr, &ReWriteTable[0], ReWriteTable.size());
	}
}

Ped FindBestTarget(int aimFov) {
	Ped bestTarget;
	float minFov = 9999.f;
	for (auto& ped : PedList) {
		if (!LocalPlayer.Update()) {
			break;
		}

		if (!ped.Update()) {
			continue;
		}

		float pDistance = GetDistance(ped.position, LocalPlayer.position);
		if (pDistance >= Cheats::AimAssist::Aimbot::Distance) {
			continue;
		}

		if (Cheats::AimAssist::OnlyVisible && !ped.IsVisible()) {
			continue;
		}

		if (Cheats::AimAssist::IgnorePed && !ped.IsPlayer()) {
			continue;
		}

		if (Cheats::AimAssist::IgnoreDeath && ped.IsDead()) {
			continue;
		}

		int pedID = ped.GetId();
		if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID]) {
			continue;
		}

		Vector2 screenPosition;
		Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
		if (Cheats::AimAssist::Aimbot::SelectedType == 0) {
			if (!WorldToScreen(viewMatrix, ped.boneList[Head], screenPosition)) {
				continue;
			}
		}
		else if (Cheats::AimAssist::Aimbot::SelectedType == 1) {
			if (!WorldToScreen(viewMatrix, ped.boneList[Neck], screenPosition)) {
				continue;
			}
		}
		else if (Cheats::AimAssist::Aimbot::SelectedType == 2) {
			if (!WorldToScreen(viewMatrix, ped.boneList[Hip], screenPosition)) {
				continue;
			}
		}
		else if (Cheats::AimAssist::Aimbot::SelectedType == 3) {
			if (!WorldToScreen(viewMatrix, ped.boneList[RightHand], screenPosition)) {
				continue;
			}
		}
		else if (Cheats::AimAssist::Aimbot::SelectedType == 4) {
			if (!WorldToScreen(viewMatrix, ped.boneList[LeftHand], screenPosition)) {
				continue;
			}
		}
		else if (Cheats::AimAssist::Aimbot::SelectedType == 5) {
			if (!WorldToScreen(viewMatrix, ped.boneList[RightAnkle], screenPosition)) {
				continue;
			}
		}
		else if (Cheats::AimAssist::Aimbot::SelectedType == 6) {
			if (!WorldToScreen(viewMatrix, ped.boneList[LeftAnkle], screenPosition)) {
				continue;
			}
		}
		float fov = abs((Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f) - screenPosition).Length());
		if (fov < aimFov) {
			if (fov < minFov) {
				bestTarget = ped;
				minFov = fov;
				continue;
			}
		}
	}
	return bestTarget;
}

void SetAim() {
	while (!exitLoop) {
		if (Cheats::AimAssist::Aimbot::Enabled) {
			Ped target = FindBestTarget(Cheats::AimAssist::Aimbot::Fov);
			if (GetAsyncKeyState(Cheats::AimAssist::Aimbot::Key) & 0x8000) {
				if (!Vec3Empty(target.boneList[Head])) {
					uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
					Vector3 viewAngle = ReadMemory<Vector3>(camera + 0x3D0);
					Vector3 cameraPosition = ReadMemory<Vector3>(camera + 0x60);
					Vector3 angle;

					if (Cheats::AimAssist::Aimbot::SelectedType == 0) {
						if (Vec3Empty(target.boneList[Head])) {
							continue;
						}
						angle = CalcAngle(cameraPosition, target.boneList[Head]);
					}
					else if (Cheats::AimAssist::Aimbot::SelectedType == 1) {
						if (Vec3Empty(target.boneList[Neck])) {
							continue;
						}
						angle = CalcAngle(cameraPosition, target.boneList[Neck]);
					}
					else if (Cheats::AimAssist::Aimbot::SelectedType == 2) {
						if (Vec3Empty(target.boneList[Hip])) {
							continue;
						}
						angle = CalcAngle(cameraPosition, target.boneList[Hip]);
					}
					else if (Cheats::AimAssist::Aimbot::SelectedType == 3) {
						if (Vec3Empty(target.boneList[RightHand])) {
							continue;
						}
						angle = CalcAngle(cameraPosition, target.boneList[RightHand]);
					}
					else if (Cheats::AimAssist::Aimbot::SelectedType == 4) {
						if (Vec3Empty(target.boneList[LeftHand])) {
							continue;
						}
						angle = CalcAngle(cameraPosition, target.boneList[LeftHand]);
					}
					else if (Cheats::AimAssist::Aimbot::SelectedType == 5) {

						if (Vec3Empty(target.boneList[RightAnkle])) {
							continue;
						}
						angle = CalcAngle(cameraPosition, target.boneList[RightAnkle]);
					}
					else if (Cheats::AimAssist::Aimbot::SelectedType == 6) {
						if (Vec3Empty(target.boneList[LeftAnkle])) {
							continue;
						}
						angle = CalcAngle(cameraPosition, target.boneList[LeftAnkle]);

					}
					NormalizeAngles(angle);
					Vector3 delta = angle - viewAngle;
					NormalizeAngles(delta);
					Vector3 writeAngle = viewAngle + (Cheats::AimAssist::Aimbot::Smooth ? delta / Cheats::AimAssist::Aimbot::Smooth : delta);
					NormalizeAngles(writeAngle);
					if (!Vec3Empty(writeAngle)) {

						Vector3 currentViewAngle = ReadMemory<Vector3>(camera + 0x3D0);

						if (currentViewAngle.x != writeAngle.x || currentViewAngle.y != writeAngle.y || currentViewAngle.z != writeAngle.z) {
							WriteMemory<Vector3>(camera + 0x3D0, writeAngle);
						}
					}
				}
			}
		}

		if (Cheats::AimAssist::Silent::Enabled) {
			static bool initialized = false;
			if (!initialized) {
				srand(static_cast<unsigned>(time(0)));
				initialized = true;
			}

			HANDLE x9c;
			std::vector<int> boneTypes = { Hip, Hip, Neck, Hip, Hip, Neck, Hip, Hip, Neck, Hip, Hip, Neck };
			int randomIndex = rand() % boneTypes.size();
			Ped target;
			float minFov = 9999.f;
			for (auto& ped : PedList) {
				if (!LocalPlayer.Update()) {
					break;
				}

				if (!ped.Update()) {
					continue;
				}

				float pDistance = GetDistance(ped.position, LocalPlayer.position);
				if (pDistance >= Cheats::AimAssist::Silent::Distance) {
					continue;
				}

				if (Cheats::AimAssist::OnlyVisible && !ped.IsVisible()) {
					continue;
				}

				if (Cheats::AimAssist::IgnorePed && !ped.IsPlayer()) {
					continue;
				}

				if (Cheats::AimAssist::IgnoreDeath && ped.IsDead()) {
					continue;
				}

				int pedID = ped.GetId();
				if (friendStatus.find(pedID) != friendStatus.end() && friendStatus[pedID]) {
					continue;
				}

				Vector2 screenPosition;
				Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
				if (Cheats::AimAssist::Silent::SelectedType == 0) {
					if (!WorldToScreen(viewMatrix, ped.boneList[Head], screenPosition)) {
						continue;
					}
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 1) {
					if (!WorldToScreen(viewMatrix, ped.boneList[Neck], screenPosition)) {
						continue;
					}
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 2) {
					if (!WorldToScreen(viewMatrix, ped.boneList[Hip], screenPosition)) {
						continue;
					}
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 3) {
					if (!WorldToScreen(viewMatrix, ped.boneList[RightHand], screenPosition)) {
						continue;
					}
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 4) {
					if (!WorldToScreen(viewMatrix, ped.boneList[LeftHand], screenPosition)) {
						continue;
					}
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 5) {
					if (!WorldToScreen(viewMatrix, ped.boneList[RightAnkle], screenPosition)) {
						continue;
					}
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 6) {
					if (!WorldToScreen(viewMatrix, ped.boneList[LeftAnkle], screenPosition)) {
						continue;
					}
				}
				float fov = abs((Vector2(Game.lpRect.right / 2.f, Game.lpRect.bottom / 2.f) - screenPosition).Length());
				if (fov < Cheats::AimAssist::Silent::Fov) {
					if (fov < minFov) {
						target = ped;
						minFov = fov;
						continue;
					}
				}
			}
			Vector3 SlientType;
			uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
			Vector3 viewAngle = ReadMemory<Vector3>(camera + 0x3D0);
			Vector3 cameraPosition = ReadMemory<Vector3>(camera + 0x60);

			if (Cheats::AimAssist::Silent::ClosestBone) {

				Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
				float crosshairX = (float)Game.lpRect.right / 2.0f;
				float crosshairY = (float)Game.lpRect.bottom / 2.0f;

				float minDistance = 9999.0f;
				int closestBoneIndex = Head;


				for (int i = 0; i < BONE_COUNT; i++) {

					if (Vec3Empty(target.boneList[i])) continue;

					Vector2 boneScreenPos;
					if (WorldToScreen(viewMatrix, target.boneList[i], boneScreenPos)) {
						float distance = sqrt(pow(boneScreenPos.x - crosshairX, 2) + pow(boneScreenPos.y - crosshairY, 2));


						if (distance < minDistance) {
							minDistance = distance;
							closestBoneIndex = i;
						}
					}
				}

				SlientType = target.boneList[closestBoneIndex];



			}
			else if (Cheats::AimAssist::Silent::RandomTarget) {
				SlientType = target.boneList[boneTypes[randomIndex]];
			}
			else {
				if (Cheats::AimAssist::Silent::SelectedType == 0) {
					SlientType = target.boneList[Head];
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 1) {
					SlientType = target.boneList[Neck];
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 2) {
					SlientType = target.boneList[Hip];
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 3) {
					SlientType = target.boneList[RightHand];
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 4) {
					SlientType = target.boneList[LeftHand];
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 5) {
					SlientType = target.boneList[RightAnkle];
				}
				else if (Cheats::AimAssist::Silent::SelectedType == 6) {
					SlientType = target.boneList[LeftAnkle];
				}
			}


			float distanceToTarget = GetDistance(LocalPlayer.position, target.position);
			if (distanceToTarget < 5.0f) {
				SlientType = target.boneList[boneTypes[randomIndex]];
			}
			if (Vec3Empty(SlientType)) {
				SilentTargetPos = Vector3(0, 0, 0);
				CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, NULL, NULL);
			}
			else {
				Vector3 angle = CalcAngle(cameraPosition, SlientType);
				Vector3 delta = angle - viewAngle;
				NormalizeAngles(delta);
				float fovDistance = sqrtf(delta.x * delta.x + delta.y * delta.y);
				if (fovDistance <= Cheats::AimAssist::Silent::Fov) {
					if (GetAsyncKeyState(Cheats::AimAssist::Silent::Key) & 0x8000) {
						EndBulletPos = SlientType;
						SilentTargetPos = SlientType;
						if (Cheats::AimAssist::Silent::Pslient) {
							x9c = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, 0, NULL);
							x9c = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ApplySilent, NULL, 0, NULL);
							x9c = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, 0, NULL);
							CloseHandle(x9c);
							CloseHandle(x9c);
							CloseHandle(x9c);
						}
						else {
							x9c = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ApplySilent, NULL, 0, NULL);
							CloseHandle(x9c);
						}
					}
					else {
						SilentTargetPos = Vector3(0, 0, 0);
						x9c = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, 0, NULL);
						CloseHandle(x9c);
					}
				}
				else {
					SilentTargetPos = Vector3(0, 0, 0);
					x9c = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RestoreSilent, NULL, 0, NULL);
					CloseHandle(x9c);
				}
			}

		}
		Sleep(1);
	}
}

bool IsTargetInCrosshair(const Vector2& screenPosition) {
	const float crosshairX = (float)Game.lpRect.right / 2;
	const float crosshairY = (float)Game.lpRect.bottom / 2;
	return (abs(screenPosition.x - crosshairX) <= Cheats::AimAssist::Triggerbot::CrosshairTolerance && abs(screenPosition.y - crosshairY) <= Cheats::AimAssist::Triggerbot::CrosshairTolerance);
}

void TriggerBot() {
	while (exitLoop == false) {
		if (Cheats::AimAssist::Triggerbot::Enabled && (GetAsyncKeyState(Cheats::AimAssist::Triggerbot::Key) & 0x8000)) {
			Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
			Ped target = FindBestTarget(Cheats::AimAssist::Triggerbot::Fov);
			Vector3 TriggerBottype;
			if (Cheats::AimAssist::Triggerbot::SelectedType == 0) {
				TriggerBottype = target.boneList[Head];
			}
			else if (Cheats::AimAssist::Triggerbot::SelectedType == 1) {
				TriggerBottype = target.boneList[Neck];
			}
			else if (Cheats::AimAssist::Triggerbot::SelectedType == 2) {
				TriggerBottype = target.boneList[Hip];

			}
			else if (Cheats::AimAssist::Triggerbot::SelectedType == 3) {
				TriggerBottype = target.boneList[RightHand];
			}
			else if (Cheats::AimAssist::Triggerbot::SelectedType == 4) {
				TriggerBottype = target.boneList[LeftHand];
			}
			else if (Cheats::AimAssist::Triggerbot::SelectedType == 5) {
				TriggerBottype = target.boneList[RightAnkle];
			}
			else if (Cheats::AimAssist::Triggerbot::SelectedType == 6) {
				TriggerBottype = target.boneList[LeftAnkle];
			}
			if (!Vec3Empty(TriggerBottype)) {
				Vector2 screenPosition;
				if (WorldToScreen(viewMatrix, TriggerBottype, screenPosition)) {
					if (IsTargetInCrosshair(screenPosition)) {
						mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
						Sleep(Cheats::AimAssist::Triggerbot::Delay);
						mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
					}
				}
			}
		}
		Sleep(loopDelay);
	}
}

void TeleportObject(uintptr_t Object, uintptr_t Navigation, uintptr_t ModelInfo, Vector3 Position, Vector3 VisualPosition, bool Stop) {
	float BackupMagic = 0.f;
	if (Stop) {
		BackupMagic = ReadMemory<float>(ModelInfo + 0x2C);
		WriteMemory(ModelInfo + 0x2C, 0.f);
	}

	WriteMemory(Object + 0x90, VisualPosition);
	WriteMemory(Navigation + 0x50, Position);
	if (Stop) {
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
		WriteMemory(ModelInfo + 0x2C, BackupMagic);
	}
}

Vector3 GetPositionByID(int targetID) {
	for (auto& ped : PedList) {
		if (!ped.Update()) continue;
		if (ped.GetId() == targetID) {
			return ped.position;
		}
	}
}

void AddPlayerList() {
	while (true) {
		if (!LocalPlayer.Update()) return;

		for (auto& ped : PedList) {
			if (!ped.Update()) continue;
			if (Cheats::Players::IgnorePed && !ped.IsPlayer()) continue;

			float distance = GetDistance(ped.position, LocalPlayer.position);
			if (distance > 300.0f) continue;

			int pedID = ped.GetId();


			if (friendStatus.find(pedID) == friendStatus.end()) {
				friendStatus[pedID] = false;
			}
		}

		Sleep(loopDelay);
	}
}

void NoClip(Vector3 CameraPos) {
	if (!LocalPlayer.pointer)
		return;

	if (!TPModelInfo)
		return;

	static float MagicValue = 0.0f;
	static bool Restoring = false;
	Vector3 NewPosition = TPPosition;

	if (!Cheats::NoClip::Enabled) {
		if (Restoring) {
			WriteMemory<float>(TPModelInfo + 0x2C, MagicValue);
		}
		Restoring = false;
		return;
	}

	if (!Restoring) {
		MagicValue = ReadMemory<float>(TPModelInfo + 0x2C);
		WriteMemory<float>(TPModelInfo + 0x2C, 0.f);
		Restoring = true;
	}

	auto Speed = static_cast<float>(Cheats::NoClip::Speed) / 100.f;

	if (GetAsyncKeyState(VK_SHIFT)) {
		Speed *= 4.f;
	}

	if (GetAsyncKeyState('W')) {
		NewPosition.x += (CameraPos * Speed).x;
		NewPosition.y += (CameraPos * Speed).y;
		NewPosition.z += (CameraPos * Speed).z;
	}

	if (GetAsyncKeyState('S')) {
		NewPosition.x -= (CameraPos * Speed).x;
		NewPosition.y -= (CameraPos * Speed).y;
		NewPosition.z -= (CameraPos * Speed).z;
	}

	Vector3 Right = CameraPos.Cross(Vector3(0, 0, 1));
	if (GetAsyncKeyState('A')) {
		NewPosition.x -= (Right * Speed).x;
		NewPosition.y -= (Right * Speed).y;
		NewPosition.z -= (Right * Speed).z;
	}

	if (GetAsyncKeyState('D')) {
		NewPosition.x += (Right * Speed).x;
		NewPosition.y += (Right * Speed).y;
		NewPosition.z += (Right * Speed).z;
	}

	if (GetAsyncKeyState(VK_SPACE)) {
		NewPosition.z += (Speed);
	}

	if (GetAsyncKeyState(VK_LCONTROL)) {
		NewPosition.z -= (Speed);
	}

	TeleportObject(LocalPlayer.pointer, TPNavigation, 0, NewPosition, NewPosition, false);
}


inline void FreeCam()
{
	static bool patch_applied = false;
	static float savedModelValue1 = 0.0f;
	static float savedModelValue2 = 0.0f;
	static bool valuesBackedUp = false;

	if (Cheats::FreeCam::Enabled) {
		uintptr_t ModelInfo = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x20);
		if (ModelInfo && !valuesBackedUp)
		{

			savedModelValue1 = ReadMemory<float>(ModelInfo + 0x20);
			savedModelValue2 = ReadMemory<float>(ModelInfo + 0x2C);
			valuesBackedUp = true;


			WriteMemory<float>(ModelInfo + 0x20, 0.f);
			WriteMemory<float>(ModelInfo + 0x2C, 0.f);
		}

		if (!patch_applied)
		{

			if (Offsets.FreecanPatch != 0) {
				WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreecanPatch, 0x100FF39090909090);
				WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreecanPatch + 0xB, 0x100FF39090909090);
				WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreecanPatch + 0x16, 0x100FF39090909090);
			}
			patch_applied = true;
		}


		uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
		if (!camera)
			return;

		Vector3 CameraPos = ReadMemory<Vector3>(camera + 0x60);
		Vector3 ViewAngles = ReadMemory<Vector3>(camera + 0x3D0);

		float cam_speed = Cheats::FreeCam::Speed;

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			cam_speed *= 5.f;
		}

		Vector3 NewPos = CameraPos;

		if (GetAsyncKeyState('W') & 0x8000)
		{
			NewPos.x += ViewAngles.x * cam_speed;
			NewPos.y += ViewAngles.y * cam_speed;
			NewPos.z += ViewAngles.z * cam_speed;
		}
		if (GetAsyncKeyState('S') & 0x8000)
		{
			NewPos.x -= ViewAngles.x * cam_speed;
			NewPos.y -= ViewAngles.y * cam_speed;
			NewPos.z -= ViewAngles.z * cam_speed;
		}

		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			NewPos.z += cam_speed;
		}
		if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
		{
			NewPos.z -= cam_speed;
		}

		WriteMemory<Vector3>(camera + 0x60, NewPos);
	}
	else
	{
		if (patch_applied)
		{

			if (Offsets.FreecanPatch != 0) {
				WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreecanPatch, 0x100FF36047110FF3);
				WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreecanPatch + 0xB, 0x100FF3644F110FF3);
				WriteMemory<uintptr_t>(Offsets.GameBase + Offsets.FreecanPatch + 0x16, 0x110FF36847110FF3);
			}
			patch_applied = false;
		}


		if (valuesBackedUp) {
			uintptr_t ModelInfo = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x20);
			if (ModelInfo) {
				WriteMemory<float>(ModelInfo + 0x20, savedModelValue1);
				WriteMemory<float>(ModelInfo + 0x2C, savedModelValue2);
			}
			valuesBackedUp = false;
		}
	}
}

void Exploits() {
	static bool lastState = false;
	static bool lastAddFriendState = false;
	static bool lastPeekAssistState = false;
	static bool lastFreeCamState = false;

	while (!exitLoop) {

		uintptr_t weaponManager = ReadMemory<uintptr_t>(LocalPlayer.pointer + Offsets.WeaponManager);
		uintptr_t weaponinfo = ReadMemory<uintptr_t>(weaponManager + 0x20);

		TPModelInfo = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x20);
		TPPosition = ReadMemory<Vector3>(LocalPlayer.pointer + 0x90);
		TPNavigation = ReadMemory<uintptr_t>(LocalPlayer.pointer + 0x30);

		uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.BlipList);
		uintptr_t camera2 = ReadMemory<uintptr_t>(camera + 0x3C0);
		Vector3 camerapos = ReadMemory<Vector3>(camera2 + 0x40);
		NoClip(camerapos);


		if (GetAsyncKeyState(Cheats::AimAssist::Silent::AddFriendKey) & 0x8000) {
			if (!lastAddFriendState) {
				lastAddFriendState = true;
				int playerUnderCrosshair = GetPlayerUnderCrosshair();
				if (playerUnderCrosshair != -1) {

					if (IsFriend(playerUnderCrosshair)) {
						RemoveFriend(playerUnderCrosshair);
					}
					else {

						AddFriend(playerUnderCrosshair);
					}
				}
			}
		}
		else {
			lastAddFriendState = false;
		}


		if (Cheats::PeekAssist::Enabled && (GetAsyncKeyState(Cheats::PeekAssist::Key) & 0x8000)) {
			if (!lastPeekAssistState) {
				lastPeekAssistState = true;

				if (!Cheats::PeekAssist::HasMarkedPosition) {

					Matrix viewMatrix = ReadMemory<Matrix>(Game.ViewPort + 0x24C);
					uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
					Vector3 cameraPosition = ReadMemory<Vector3>(camera + 0x60);


					float screenCenterX = (float)Game.lpRect.right / 2.0f;
					float screenCenterY = (float)Game.lpRect.bottom / 2.0f;


					Vector3 forward;
					forward.x = viewMatrix._31;
					forward.y = viewMatrix._32;
					forward.z = viewMatrix._33;


					float length = sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
					if (length > 0.0f) {
						forward.x /= length;
						forward.y /= length;
						forward.z /= length;
					}


					Vector3 targetPosition = cameraPosition;
					float maxDistance = 15.0f;
					bool foundGround = false;

					for (float distance = 2.0f; distance <= maxDistance; distance += 0.5f) {
						Vector3 testPos;
						testPos.x = cameraPosition.x + forward.x * distance;
						testPos.y = cameraPosition.y + forward.y * distance;
						testPos.z = cameraPosition.z + forward.z * distance;


						if (testPos.z <= cameraPosition.z + 1.0f) {
							targetPosition = testPos;
							foundGround = true;
							break;
						}
					}


					if (!foundGround) {
						targetPosition.x = cameraPosition.x + forward.x * 10.0f;
						targetPosition.y = cameraPosition.y + forward.y * 10.0f;
						targetPosition.z = cameraPosition.z - 2.0f;
					}


					float distance = GetDistance(cameraPosition, targetPosition);
					if (distance <= Cheats::PeekAssist::MaxDistance) {
						Cheats::PeekAssist::MarkedPosition = targetPosition;
						Cheats::PeekAssist::HasMarkedPosition = true;

						printf("[PEEK ASSIST] Position marked at: %.2f, %.2f, %.2f (Distance: %.1fm)\n",
							targetPosition.x, targetPosition.y, targetPosition.z, distance);
						fflush(stdout);
					}
					else {
						printf("[PEEK ASSIST] Target too far (%.1fm > %.1fm)\n", distance, Cheats::PeekAssist::MaxDistance);
						fflush(stdout);
					}
				}
				else {

					TeleportObject(LocalPlayer.pointer, TPNavigation, TPModelInfo,
						Cheats::PeekAssist::MarkedPosition, Cheats::PeekAssist::MarkedPosition, true);


					Cheats::PeekAssist::HasMarkedPosition = false;
					Cheats::PeekAssist::MarkedPosition = Vector3(0, 0, 0);

					printf("[PEEK ASSIST] Teleported to marked position\n");
					fflush(stdout);
				}
			}
		}
		else {
			lastPeekAssistState = false;
		}


		if (GetAsyncKeyState(Cheats::FreeCam::Key) & 0x8000) {
			if (!lastFreeCamState) {
				lastFreeCamState = true;
				Cheats::FreeCam::Enabled = !Cheats::FreeCam::Enabled;
			}
		}
		else {
			lastFreeCamState = false;
		}


		FreeCam();


		if (Cheats::Exploit::Shaking && (GetAsyncKeyState(Cheats::Exploit::ShakingKey) & 0x8000)) {
			uintptr_t camera = ReadMemory<uintptr_t>(Offsets.GameBase + Offsets.Camera);
			if (camera) {
				Vector3 viewAngle = ReadMemory<Vector3>(camera + 0x3D0);

				static float shakeTimer = 0.0f;
				shakeTimer += 0.1f;


				float shakeOffset = sin(shakeTimer * Cheats::Exploit::ShakingSpeed) * (Cheats::Exploit::ShakingAmount / 100.0f);

				Vector3 newAngle = viewAngle;
				newAngle.y += shakeOffset;

				NormalizeAngles(newAngle);
				WriteMemory<Vector3>(camera + 0x3D0, newAngle);
			}
		}


		static bool strafeRunning = false;
		static std::chrono::steady_clock::time_point lastStrafeTime = std::chrono::steady_clock::now();

		if (Cheats::Exploit::StrafeBoost && Cheats::Exploit::StrafeMacro::Key > 0 && (GetAsyncKeyState(Cheats::Exploit::StrafeMacro::Key) & 0x8000)) {
			if (!strafeRunning) {
				Beep(800, 50);
				strafeRunning = true;
				lastStrafeTime = std::chrono::steady_clock::now();
			}


			auto currentTime = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastStrafeTime).count();

			if (elapsed >= Cheats::Exploit::StrafeMacro::Delay) {
				const auto& sequence = Cheats::Exploit::StrafeMacro::KeySequences[Cheats::Exploit::StrafeMacro::SelectedPreset];
				static int currentKeyIndex = 0;


				WORD key = sequence[currentKeyIndex];
				UINT scanCode = MapVirtualKey(key, MAPVK_VK_TO_VSC);

				keybd_event(key, scanCode, 0, 0);
				Sleep(1);
				keybd_event(key, scanCode, KEYEVENTF_KEYUP, 0);


				currentKeyIndex = (currentKeyIndex + 1) % sequence.size();
				lastStrafeTime = currentTime;
			}
		}
		else {
			strafeRunning = false;
		}

		if (Cheats::Exploit::godMode && (GetAsyncKeyState(Cheats::Exploit::godModeKey) & 0x8000)) {
			if (!lastState) {
				Beep(1000, 100);
				lastState = true;
			}

			int currentGodMode = ReadMemory<int>(LocalPlayer.pointer + 0x189);
			if (currentGodMode != 1) {
				WriteMemory<int>(LocalPlayer.pointer + 0x189, 1);
			}
		}
		else {
			if (lastState) {
				Beep(500, 100);
				lastState = false;
			}

			int currentGodMode = ReadMemory<int>(LocalPlayer.pointer + 0x189);
			if (currentGodMode != 0) {
				WriteMemory<int>(LocalPlayer.pointer + 0x189, 0);
			}
		}

		if (Cheats::Exploit::HealthBoost) {
			if (GetAsyncKeyState(Cheats::Exploit::HealthBoostKey) & 0x8000) {

				float currentHealth = ReadMemory<float>(LocalPlayer.pointer + 0x280);
				if (currentHealth < static_cast<float>(Cheats::Exploit::HealthBoostValue)) {
					WriteMemory<float>(LocalPlayer.pointer + 0x280, static_cast<float>(Cheats::Exploit::HealthBoostValue));
				}
			}
		}

		if (Cheats::Exploit::ArmorBoost) {
			if (GetAsyncKeyState(Cheats::Exploit::ArmorBoostKey) & 0x8000) {

				float ArmorCheck = ReadMemory<float>(LocalPlayer.pointer + Offsets.Armor);
				if (ArmorCheck < 100) {

					Sleep(1);
					float newArmorValue = ArmorCheck + static_cast<float>(Cheats::Exploit::ArmorBoostValue);
					if (newArmorValue > 100) {
						newArmorValue = 100;
					}

					float currentArmor = ReadMemory<float>(LocalPlayer.pointer + Offsets.Armor);
					if (currentArmor == ArmorCheck) {
						WriteMemory<float>(LocalPlayer.pointer + Offsets.Armor, newArmorValue);
					}
				}
			}
		}

		if (Cheats::Exploit::InfiniteAmmo) {
			uintptr_t AmmoInfo = ReadMemory<uintptr_t>(weaponinfo + 0x60);
			uintptr_t AmmoCount = ReadMemory<uintptr_t>(AmmoInfo + 0x8);
			uintptr_t AmmoCount2 = ReadMemory<uintptr_t>(AmmoCount + 0x0);
			WriteMemory<float>(AmmoCount2 + 0x18, 30);
		}

		if (Cheats::Exploit::NoRecoil) {
			WriteMemory<float>(weaponinfo + 0x2F4, 0.f);
		}

		if (Cheats::Exploit::NoSpread) {
			WriteMemory<float>(weaponinfo + 0x84, 0.0f);
		}

		if (Cheats::Exploit::NoReload) {
			WriteMemory<float>(weaponinfo + 0x134, 1000);
		}

		if (Cheats::Exploit::NoRange) {
			WriteMemory<float>(weaponinfo + 0x28C, 1000.f);
		}

		if (Cheats::Exploit::ReloadAmmo) {
			if (GetAsyncKeyState(Cheats::Exploit::ReloadAmmoKey) & 0x8000) {
				uintptr_t AmmoInfo = ReadMemory<uintptr_t>(weaponinfo + 0x60);
				uintptr_t AmmoCount = ReadMemory<uintptr_t>(AmmoInfo + 0x8);
				uintptr_t AmmoCount2 = ReadMemory<uintptr_t>(AmmoCount + 0x0);
				WriteMemory<float>(AmmoCount2 + 0x18, Cheats::Exploit::ReloadValue);
			}
		}


		static uintptr_t lastWeaponInfo = 0;
		static float originalDamage = 0;
		static uint32_t lastSafeDamageVal = 0;
		static bool safeDamageActive = false;

		if (Cheats::Exploit::DamageBoost) {
			if (weaponinfo != lastWeaponInfo) {
				originalDamage = ReadMemory<float>(weaponinfo + 0xB0);
				lastWeaponInfo = weaponinfo;
			}
			WriteMemory<float>(weaponinfo + 0xB0, Cheats::Exploit::DamageBoostValue);
		}
		else if (lastWeaponInfo == weaponinfo && originalDamage != 0) {
			WriteMemory<float>(weaponinfo + 0xB0, originalDamage);
		}


		if (Cheats::Exploit::SafeDamageBoost) {
			uint32_t safeDamageVal = static_cast<uint32_t>(Cheats::Exploit::SafeDamageBoostAmount);
			if (weaponinfo != lastWeaponInfo || safeDamageVal != lastSafeDamageVal) {
				WriteMemory<uint32_t>(weaponinfo + 0x120, safeDamageVal);
				lastSafeDamageVal = safeDamageVal;
				lastWeaponInfo = weaponinfo;
				safeDamageActive = true;
			}
			else if (safeDamageActive) {
				uint32_t currentDamage = ReadMemory<uint32_t>(weaponinfo + 0x120);
				if (currentDamage != safeDamageVal) {
					WriteMemory<uint32_t>(weaponinfo + 0x120, safeDamageVal);
				}
			}
		}
		else if (safeDamageActive && lastWeaponInfo == weaponinfo) {
			WriteMemory<uint32_t>(weaponinfo + 0x120, 1);
			safeDamageActive = false;
		}


		if (Cheats::Vehicle::Fix) {
			if (GetAsyncKeyState(Cheats::Vehicle::FixKey) & 0x8000) {
				uintptr_t vehiclePtr = ReadMemory<uintptr_t>(LocalPlayer.pointer + Offsets.Vehicle);
				if (vehiclePtr) {
					WriteMemory<uint8_t>(vehiclePtr + 0x972, 0x17);
				}
			}
		}

		Sleep(5);
	}
}
