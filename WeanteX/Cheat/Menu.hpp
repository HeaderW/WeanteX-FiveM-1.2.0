#include "SimpleMath.h"
#include "Fonts.hpp"
#include "Logo.hpp"



struct MenuStruct {
	ImVec2 Pos, Region, Spacing;
	ImVec2 WindowSize = { 900, 600 };
	ImFont* Inter;
	ImFont* Breesh;
	ImFont* InterSmaller;
	ImFont* InterSemiBold;
	ImFont* FontAwesome;
	ImFont* Arial;
	ImFont* Tahoma;
	ImFont* Verdana;
	ImFont* SFProDisplayRegular;
	ImFont* SegoeUI;
	ImFont* TimesNewRoman;
	ImFont* Calibri;
	ImFont* CourierNew;
	ImFont* Consolas;
	ImFont* TrebuchetMS;
	ImFontConfig Config;
	ID3D11ShaderResourceView* Logo;
	ID3D11ShaderResourceView* Chracter;
	DWORD ColorPickerFlags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
	int CurrentTab, SubTab1 = 0, SubTab2 = 0, SubTab3, SubTab4 = 0, SubTab5 = 0, SubTab6 = 0, SubTab7 = 0;
	float BackgroundRounding = 10.f, ChildRounding = 11.f, PageRounding = 4.f, ElementsRounding = 2.f;
	float ButtonHeight = 35.0f;
	float ButtonSpacing = 2.0f;
	float ButtonX = 15.0f;
	float SeparatorOffset = 3.0f;
	float InitialOffsetX = 0;
	int LastTab = -1;
	float TabAlpha = 0.0f;
	float MenuAlpha = 0.0f;
	const float AlphaSpeed = 0.02f;
	

	float MenuScale = 0.0f;
	bool IsAnimating = false;
	bool IsOpening = false;
	float AnimationSpeed = 8.0f;
	bool MenuVisible = false;
	ImColor MainColor, BackgroundColor, HeaderColor, TabColor, SeparatorColor, ChildColor, ChildCapColor, ChildCapstructColor, PageActiveColor, PageActiveIconColor, PageColor, PageTextColor, ElementsColor, ElementsHoverColor, TextColor, TextActiveColor, TextHoverColor, AccentColor, CheckboxMarkColor;
} Menu;

void InitializeMenu(ID3D11Device* pDevice) {
	D3DX11_IMAGE_LOAD_INFO info;
	ID3DX11ThreadPump* pump{ nullptr };

	D3DX11CreateShaderResourceViewFromMemory(pDevice, Logo, sizeof(Logo), &info, pump, &Menu.Logo, 0);

	auto& io = ImGui::GetIO();
	auto& style = ImGui::GetStyle();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig iconsConfig;
	iconsConfig.MergeMode = true;
	iconsConfig.PixelSnapH = true;
	iconsConfig.OversampleH = 3;
	iconsConfig.OversampleV = 3;

	Menu.Inter = io.Fonts->AddFontFromMemoryTTF(Inter, sizeof(Inter), 18.0f, &Menu.Config, io.Fonts->GetGlyphRangesCyrillic());
	Menu.InterSmaller = io.Fonts->AddFontFromMemoryTTF(Inter, sizeof(Inter), 14.0f, &Menu.Config, io.Fonts->GetGlyphRangesCyrillic());
	Menu.FontAwesome = io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesomeData, FontAwesomeDataSize, 18.f, &iconsConfig, iconsRanges);
	Menu.InterSemiBold = io.Fonts->AddFontFromMemoryTTF(EspFont, sizeof(EspFont), 18.0f, &Menu.Config, io.Fonts->GetGlyphRangesCyrillic());
	Menu.Breesh = io.Fonts->AddFontFromMemoryTTF(Breesh, sizeof(Breesh), 18.0f, &Menu.Config, io.Fonts->GetGlyphRangesCyrillic());

	Menu.SFProDisplayRegular = io.Fonts->AddFontFromMemoryTTF(SFProDisplayRegular, sizeof(SFProDisplayRegular), 18.0f, &Menu.Config, io.Fonts->GetGlyphRangesCyrillic());



	Menu.Arial = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\arial.ttf"), 14.0f);
	Menu.Tahoma = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\tahoma.ttf"), 15.0f);
	Menu.Verdana = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\verdana.ttf"), 16.0f);
	Menu.SegoeUI = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\segoeui.ttf"), 17.0f);
	Menu.TimesNewRoman = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\times.ttf"), 18.0f);
	Menu.Calibri = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\calibri.ttf"), 19.0f);
	Menu.CourierNew = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\cour.ttf"), 13.0f);
	Menu.Consolas = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\consola.ttf"), 15.0f);
	Menu.TrebuchetMS = io.Fonts->AddFontFromFileTTF(XorString("C:\\Windows\\Fonts\\trebuc.ttf"), 14.0f);


	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.13f, 0.16f, 0.1f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.09f, 0.11f, 0.1f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.09f, 0.12f, 0.14f, 0.1f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.08f, 0.09f, 0.11f, 0.1f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	style.WindowPadding = ImVec2(0, 0);
	style.ItemSpacing = ImVec2(20, 20);
	style.WindowBorderSize = 0;
	style.ScrollbarSize = 1.f;

	Menu.MainColor = ImColor(223, 32, 77);
	Menu.BackgroundColor = ImColor(31, 34, 42, 10);
	Menu.HeaderColor = ImColor(42, 45, 58, 10);
	Menu.TabColor = ImColor(20, 24, 29, 10);
	Menu.AccentColor = ImColor(49, 49, 49, 10);
	Menu.ElementsHoverColor = ImColor(24, 30, 35, 10);
	Menu.ElementsColor = ImColor(20, 24, 29, 10);
	Menu.CheckboxMarkColor = ImColor(223, 32, 77);
	Menu.ChildColor = ImColor(20, 24, 29, 10);
	Menu.ChildCapColor = ImColor(42, 45, 58, 10);
	Menu.ChildCapstructColor = ImColor(31, 31, 39, 10);
	Menu.PageTextColor = ImColor(140, 140, 140, 10);
	Menu.PageActiveColor = ImColor(31, 34, 42, 10);
	Menu.PageActiveIconColor = ImColor(171, 170, 186, 10);
	Menu.PageColor = ImColor(22, 23, 25, 10);
	Menu.TextActiveColor = ImColor(255, 255, 255, 10);
	Menu.TextHoverColor = ImColor(255, 255, 255, 10);
	Menu.TextColor = ImColor(174, 174, 174, 10);
	Menu.SeparatorColor = ImColor(36, 39, 49, 10);
}


inline float EaseOutCubic(float t) {
    return 1.0f - pow(1.0f - t, 3.0f);
}

#include "Items.hpp"
namespace Cheats {
	namespace MenuUtils {
		int MenuKey = VK_INSERT;

		bool StreamProof = false;
		bool OutlineEnabled = true;
		bool DynamicFOV = false;
	}

	namespace AimAssist {
		bool OnlyVisible = false;
		bool IgnorePed = true;
		bool IgnoreDeath = true;
		static int SelectedMode = 0;
		const char* ModesAimAssist[] = { "Aimbot", "Silent", "T.Bot" };

		namespace Aimbot {
			bool Enabled = false;
			bool DrawFov = false;
			ImColor Color = ImColor(255, 255, 255);
			const char* Type[7] = { "Head", "Neck", "Chest", "Right Arm", "Left Arm", "Right Leg", "Left Leg" };

			int SelectedType = 0;
			int Key;
			float Fov = 50;
			float Smooth = 5;
			float Distance = 300;
		}

		namespace Silent {
			bool Enabled = false;
			bool RandomTarget = false;
			bool ClosestBone = false;
			bool DrawFov = false;
			ImColor Color = ImColor(255, 255, 255);
			const char* Type[7] = { "Head", "Neck", "Chest", "Right Arm", "Left Arm", "Right Leg", "Left Leg" };

			bool AddFried = false;

			int SelectedType = 0;
			bool Pslient = false;
			int Key;
			float Fov = 50;
			float Smooth = 5;
			float Distance = 300;

			bool DrawTarget = false;
			ImColor TargetColor = ImColor(255, 0, 0);

			bool DrawSilentLine = false;
			ImColor SilentLineColor = ImColor(255, 255, 255);


			int AddFriendKey = 0;
			std::map<int, bool> friendList;
		}

		namespace Triggerbot {
			bool Enabled = false;
			bool DrawFov = false;
			ImColor Color = ImColor(255, 255, 255);
			const char* Type[7] = { "Head", "Neck", "Chest", "RightHand", "LeftHand", "RightAnkle", "LeftAnkle" };

			int SelectedType = 0;
			int Key;
			int Fov = 50;
			int Smooth = 5;
			int Delay = 1;
			int CrosshairTolerance = 5;
			int Distance = 300;
		}
	}

	namespace Players {
		bool OnlyVisible = false;
		bool IgnorePed = true;
		bool IgnoreDeath = true;
		float Distance = 300;


		bool ShowNpcEsp = false;

		namespace DrawSkeleton {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawHeadDot {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawId {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawName {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawBox {
			bool Enabled = false;
			const char* Type[2]{ "2D", "Corner" };
			int SelectedType = 0;
			ImColor Color = ImColor(255, 255, 255);
			float Size = 0.75f;
			bool UseCustomGradient = false;
			ImColor BoxGradientTopColor = ImColor(0, 0, 255, 255);
			ImColor BoxGradientBottomColor = ImColor(255, 0, 0, 255);
			bool GradientEnabled = false;
			float GradientIntensity = 0.0f;
		}

		namespace DrawLine {
			bool Enabled = false;
			const char* Type[3]{ "Top", "Center", "Bottom" };
			int SelectedType = 0;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawDistance {
			bool Enabled = false;
			bool StyleBg = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DrawHealth {
			bool Enabled = false;
			const char* Position[4]{ "Bottom", "Top", "Left", "Right" };
			int SelectedPosition = 3;
		}

		namespace DrawArmor {
			bool Enabled = false;
			const char* Position[4]{ "Left", "Right", "Top", "Bottom" };
			int SelectedPosition = 0;
		}

		namespace DrawWeaponName {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}

		namespace DirectionEsp {
			bool Enabled = false;
			ImColor Color = ImColor(255, 255, 255);
		}
	}



	namespace Vehicle {
		ImColor HealthBarColor = ImColor(0, 255, 0, 255);
		ImColor DistanceColor = ImColor(255, 255, 255, 255);
		ImColor SnaplineColor = ImColor(255, 255, 255, 255);
		ImColor MarkerOuterColor = ImColor(0, 0, 0, 100);
		ImColor MarkerInnerColor = ImColor(255, 0, 0, 255);

		bool Enabled = false;
		bool DrawLocalVehicle = true;
		bool DrawEnemyVehicle = true;
		bool VehicleHealth = false;
		bool VehicleEspShowDistance = false;
		bool VehicleEspSnapline = false;
		bool VehicleMarker = false;
		float Distance = 300;
		bool Fix;
		int FixKey;
	}

	namespace Crosshairs {
		bool Enabled = false;
		int SelectedType = 0;
		const char* Type[10]{ "Type 1", "Type 2", "Type 3", "Type 4", "Type 5", "Type 6", "Type 7", "Type 8", "Type 9", "Type 10" };
		int Size = 10;
		ImColor Color = ImColor(255, 255, 255);
	}

	namespace Exploit {
		bool HealthBoost = false;
		float HealthBoostValue = 200.0f;
		int HealthBoostKey;
		bool godMode = false;
		int godModeKey;

		bool ArmorBoost = false;
		float ArmorBoostValue = 20.0f;
		int ArmorBoostKey;

		bool InfiniteAmmo = false;
		bool NoRecoil = false;
		bool NoSpread = false;
		bool NoReload = false;
		bool NoRange = false;
		bool ReloadAmmo = false;
		float ReloadValue = 1;
		int ReloadAmmoKey;

		bool DamageBoost = false;
		float DamageBoostValue = 100.0f;
		bool SafeDamageBoost = false;
		float SafeDamageBoostAmount = 100.f;

		bool Shaking = false;
		int ShakingKey = 0;
		float ShakingSpeed = 5.f;
		float ShakingAmount = 0.5f;


		bool StrafeBoost = false;

		namespace StrafeMacro {
			inline bool IsRunning = false;
			inline int Key = 0;
			inline int Delay = 90;
			inline int SelectedPreset = 0;
			inline const char* Presets[] = { "DSAW", "SDAW", "WASD", "WDSA" };
			inline const std::vector<std::vector<WORD>> KeySequences = {
				{ 0x44, 0x53, 0x41, 0x57 },
				{ 0x53, 0x44, 0x41, 0x57 },
				{ 0x57, 0x41, 0x53, 0x44 },
				{ 0x57, 0x44, 0x53, 0x41 }
			};
		}
	}

	namespace NoClip {
		bool Enabled = false;
		int Speed;
		int Key;
	}

	namespace FreeCam {
		bool Enabled = false;
		float Speed = 1.0f;
		int Key = 0;
		bool CollisionDisabled = false;
		bool KeyToggleActive = false;
	}

	namespace PeekAssist {
		bool Enabled = false;
		int Key = 0;
		Vector3 MarkedPosition = Vector3(0, 0, 0);
		bool HasMarkedPosition = false;
		bool ShowMarker = true;
		ImColor MarkerColor = ImColor(0, 255, 0, 255);
		float MarkerSize = 8.0f;
		float MaxDistance = 20.0f;
	}

}


void ApplyLegitConfig() {

	Cheats::Players::DrawSkeleton::Enabled = true;


	Cheats::Players::DrawDistance::Enabled = true;


	Cheats::Players::DrawWeaponName::Enabled = true;


	Cheats::Players::DrawHealth::Enabled = true;


	Cheats::Players::DrawArmor::Enabled = true;


	Cheats::Players::DrawName::Enabled = true;


	Cheats::Players::DrawBox::Enabled = true;


	Cheats::AimAssist::Silent::Enabled = true;


	Cheats::AimAssist::Silent::Key = VK_RBUTTON;


	Cheats::AimAssist::Silent::Fov = 18.0f;


	Cheats::AimAssist::Silent::ClosestBone = true;
}

static int SelectedItemVehicle = -1;
static char SearchBufferVehicle[128] = "";
std::vector<std::string> VehicleNames = {};

static std::vector<int> playerIDs;
static std::vector<int> newPlayerIDs;
static std::vector<int> oldPlayerIDs = playerIDs;
static std::vector<std::string> playerNames;

static int selectedPlayerID = -1;
static int selectedItemPlayer = -1;
static char searchBuffer[128] = "";
static bool Teleport = false;
