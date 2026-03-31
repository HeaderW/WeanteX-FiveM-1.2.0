#define XorString

bool Vec3Empty(const Vector3& value) {
	return value == Vector3(0.f, 0.f, 0.f);
}

Vector3 Vec3Transform(Vector3* vIn, Matrix* mIn) {
	Vector3 vOut{};
	vOut.x = vIn->x * mIn->_11 + vIn->y * mIn->_21 + vIn->z * mIn->_31 + 1.f * mIn->_41;
	vOut.y = vIn->x * mIn->_12 + vIn->y * mIn->_22 + vIn->z * mIn->_32 + 1.f * mIn->_42;
	vOut.z = vIn->x * mIn->_13 + vIn->y * mIn->_23 + vIn->z * mIn->_33 + 1.f * mIn->_43;
	return vOut;
};

void NormalizeAngles(Vector3& angle) {
	while (angle.x > 89.0f)
		angle.x -= 180.f;

	while (angle.x < -89.0f)
		angle.x += 180.f;

	while (angle.y > 180.f)
		angle.y -= 360.f;

	while (angle.y < -180.f)
		angle.y += 360.f;
}

float GetDistance(Vector3 value1, Vector3 value2) {
	float num = value1.x - value2.x;
	float num2 = value1.y - value2.y;
	float num3 = value1.z - value2.z;
	return sqrt(num * num + num2 * num2 + num3 * num3);
}

Vector3 CalcAngle(Vector3 localCam, Vector3 toPoint) {
	Vector3 vOut{};
	float distance = GetDistance(localCam, toPoint);
	vOut.x = (toPoint.x - localCam.x) / distance;
	vOut.y = (toPoint.y - localCam.y) / distance;
	vOut.z = (toPoint.z - localCam.z) / distance;
	return vOut;
}

bool WorldToScreen(const Matrix& viewMatrix, const Vector3& vWorld, Vector2& vOut) {
    Matrix v = viewMatrix.Transpose();
    Vector4 row2 = Vector4(v._21, v._22, v._23, v._24);
    Vector4 row3 = Vector4(v._31, v._32, v._33, v._34);
    Vector4 row4 = Vector4(v._41, v._42, v._43, v._44);

    Vector3 proj;
    proj.x = (row2.x * vWorld.x) + (row2.y * vWorld.y) + (row2.z * vWorld.z) + row2.w;
    proj.y = (row3.x * vWorld.x) + (row3.y * vWorld.y) + (row3.z * vWorld.z) + row3.w;
    proj.z = (row4.x * vWorld.x) + (row4.y * vWorld.y) + (row4.z * vWorld.z) + row4.w;
    if (proj.z <= 0.1f)
        return false;

    float invZ = 1.0f / proj.z;
    proj.x *= invZ;
    proj.y *= invZ;

    float screenW = static_cast<float>(Game.lpRect.right);
    float screenH = static_cast<float>(Game.lpRect.bottom);
    float halfW = screenW * 0.5f;
    float halfH = screenH * 0.5f;
    vOut.x = halfW + (0.5f * proj.x * screenW);
    vOut.y = halfH - (0.5f * proj.y * screenH);
    return true;
}

static std::string GetWeaponName(DWORD hash) {
    switch (hash) {
    case 0x92A27487: return XorString("Dagger");
    case 0x958A4A8F: return XorString("Bat");
    case 0xF9E6AA4B: return XorString("Bottle");
    case 0x84BD7BFD: return XorString("Crowbar");
    case 0x8BB05FD7: return XorString("Flashlight");
    case 0x440E4788: return XorString("GolfClub");
    case 0x4E875F73: return XorString("Hammer");
    case 0xF9DCBF2D: return XorString("Hatchet");
    case 0xD8DF3C3C: return XorString("Knuckle");
    case 0x99B507EA: return XorString("Knife");
    case 0xDD5DF8D9: return XorString("Machete");
    case 0xDFE37640: return XorString("Switchblade");
    case 0x678B81B1: return XorString("Nightstick");
    case 0x19044EE0: return XorString("Wrench");
    case 0xCD274149: return XorString("BattleAxe");
    case 0x94117305: return XorString("PoolCue");
    case 0x3813FC08: return XorString("StoneHatchet");
    case 0x1B06D571: return XorString("Pistol");
    case 0xBFE256D4: return XorString("Pistol Mk2");
    case 0x5EF9FEC4: return XorString("Combat Pistol");
    case 0x22D8FE39: return XorString("APPistol");
    case 0x3656C8C1: return XorString("Stun Gun");
    case 0x99AEEB3B: return XorString("Pistol 50");
    case 0xBFD21232: return XorString("SNS Pistol");
    case 0x88374054: return XorString("SNS Pistol Mk2");
    case 0xD205520E: return XorString("Heavy Pistol");
    case 0x83839C4: return XorString("Vintage Pistol");
    case 0x47757124: return XorString("Flare Gun");
    case 0xDC4DB296: return XorString("Marksman Pistol");
    case 0xC1B3C3D1: return XorString("Revolver");
    case 0xCB96392F: return XorString("Revolver Mk2");
    case 0x97EA20B8: return XorString("Double Action");
    case 0xAF3696A1: return XorString("Ray Pistol");
    case 0x2B5EF5EC: return XorString("Ceramic Pistol");
    case 0x917F6C8C: return XorString("Navy Revolver");
    case 0x13532244: return XorString("Micro SMG");
    case 0x2BE6766B: return XorString("SMG");
    case 0x78A97CD0: return XorString("SMG Mk2");
    case 0xEFE7E2DF: return XorString("Assault SMG");
    case 0xA3D4D34: return XorString("Combat PDW");
    case 0xDB1AA450: return XorString("Machine Pistol");
    case 0xBD248B55: return XorString("Mini SMG");
    case 0x476BF155: return XorString("Ray Carbine");
    case 0x1D073A89: return XorString("Pump Shotgun");
    case 0x555AF99A: return XorString("Pump Shotgun Mk2");
    case 0x7846A318: return XorString("Sawnoff Shotgun");
    case 0xE284C527: return XorString("Assault Shotgun");
    case 0x9D61E50F: return XorString("Bullpup Shotgun");
    case 0xA89CB99E: return XorString("Musket");
    case 0x3AABBBAA: return XorString("Heavy Shotgun");
    case 0xBFEFFF6D: return XorString("Assault Rifle");
    case 0x394F415C: return XorString("Assault Rifle Mk2");
    case 0x83BF0278: return XorString("Carbine Rifle");
    case 0xFAD1F1C9: return XorString("Carbine Rifle Mk2");
    case 0xAF113F99: return XorString("Advanced Rifle");
    case 0xC0A3098D: return XorString("Special Carbine");
    case 0x969C3D67: return XorString("Special Carbine Mk2");
    case 0x7F229F94: return XorString("Bullpup Rifle");
    case 0x84D6FAFD: return XorString("Bullpup Rifle Mk2");
    case 0x624FE830: return XorString("Compact Rifle");
    case 0x9D07F764: return XorString("MG");
    case 0x7FD62962: return XorString("Combat MG");
    case 0xDBBD7280: return XorString("Combat MG Mk2");
    case 0x61012683: return XorString("Gusenberg");
    case 0x5FC3C11: return XorString("Sniper Rifle");
    case 0xC472FE2: return XorString("Heavy Sniper");
    case 0xA914799: return XorString("Heavy Sniper Mk2");
    case 0xC734385A: return XorString("Marksman Rifle");
    case 0x6A6C02E0: return XorString("Marksman Rifle Mk2");
    case 0xB1CA77B1: return XorString("RPG");
    case 0xA284510B: return XorString("Grenade Launcher");
    case 0x42BF8A85: return XorString("Minigun");
    case 0x6D544C99: return XorString("Railgun");
    case 0x63AB0442: return XorString("Homing Launcher");
    case 0x781FE4A: return XorString("Compact Grenade Launcher");
    case 0x93E220BD: return XorString("Grenade");
    case 0x24B17070: return XorString("Molotov");
    case 0x2C3731D9: return XorString("Sticky Bomb");
    case 0xAB564B93: return XorString("Proximity Mine");
    case 0xBA45E8B8: return XorString("Pipe Bomb");
    case 0x34A67B97: return XorString("Petrol Can");
    case 0xFBAB5776: return XorString("Parachute");
    case 0x60EC506: return XorString("Fire Extinguisher");
    default: return XorString("Unknown");
    }
}