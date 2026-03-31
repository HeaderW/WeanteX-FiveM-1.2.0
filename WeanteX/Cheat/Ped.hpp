struct Bones {
	Vector3 head;
	char padding0[0x4]{};
	Vector3 leftFoot;
	char padding1[0x4]{};
	Vector3 rightFoot;
	char padding2[0x4]{};
	Vector3 leftAnkle;
	char padding3[0x4]{};
	Vector3 rightAnkle;
	char padding4[0x4]{};
	Vector3 leftHand;
	char padding5[0x4]{};
	Vector3 rightHand;
	char padding6[0x4]{};
	Vector3 neck;
	char padding7[0x4]{};
	Vector3 hip;
};

enum Bone : int {

	Head,
	LeftFoot,
	RightFoot,
	LeftAnkle,
	RightAnkle,
	LeftHand,
	RightHand,
	Neck,
	Hip,



	Spine0, Spine1, Spine2, Spine3, Spine4,


	LeftShoulder, LeftUpperArm, LeftElbow, LeftForearm, LeftWrist,
	LeftThumb0, LeftThumb1, LeftThumb2, LeftThumb3,
	LeftIndex0, LeftIndex1, LeftIndex2, LeftIndex3,
	LeftMiddle0, LeftMiddle1, LeftMiddle2, LeftMiddle3,
	LeftRing0, LeftRing1, LeftRing2, LeftRing3,
	LeftPinky0, LeftPinky1, LeftPinky2, LeftPinky3,


	RightShoulder, RightUpperArm, RightElbow, RightForearm, RightWrist,
	RightThumb0, RightThumb1, RightThumb2, RightThumb3,
	RightIndex0, RightIndex1, RightIndex2, RightIndex3,
	RightMiddle0, RightMiddle1, RightMiddle2, RightMiddle3,
	RightRing0, RightRing1, RightRing2, RightRing3,
	RightPinky0, RightPinky1, RightPinky2, RightPinky3,


	LeftHip, LeftThigh, LeftKnee, LeftShin, LeftCalfTwist,
	LeftAnkleTwist, LeftToe0, LeftToe1, LeftToe2,


	RightHip, RightThigh, RightKnee, RightShin, RightCalfTwist,
	RightAnkleTwist, RightToe0, RightToe1, RightToe2,


	HeadTop, Forehead, LeftEye, RightEye, LeftEar, RightEar,
	Nose, NoseTop, LeftCheek, RightCheek, UpperLip, LowerLip,
	Jaw, Chin, LeftJaw, RightJaw,


	Chest, UpperChest, LowerChest, Stomach, Pelvis, LeftRib, RightRib,


	LeftCollarbone, RightCollarbone, LeftScapula, RightScapula,
	LeftLatissimus, RightLatissimus, LeftPectoral, RightPectoral,


	LeftQuadriceps, RightQuadriceps, LeftHamstring, RightHamstring,
	LeftCalf, RightCalf, LeftSoleus, RightSoleus,


	LeftHeel, RightHeel, LeftArch, RightArch, LeftBallFoot, RightBallFoot,
	LeftBigToe, RightBigToe, LeftLittleToe, RightLittleToe,


	LeftBicep, RightBicep, LeftTricep, RightTricep, LeftDeltoid, RightDeltoid,


	UpperBack, MiddleBack, LowerBack, LeftTrapezius, RightTrapezius,


	NeckBase, NeckMiddle, NeckTop, LeftSternocleidomastoid, RightSternocleidomastoid,


	LeftFlank, RightFlank, LeftOblique, RightOblique,


	LeftTemple, RightTemple, LeftEyebrow, RightEyebrow,
	LeftNostril, RightNostril, LeftCornerMouth, RightCornerMouth,


	Bone150, Bone151, Bone152, Bone153, Bone154, Bone155, Bone156, Bone157, Bone158, Bone159,
	Bone160, Bone161, Bone162, Bone163, Bone164, Bone165, Bone166, Bone167, Bone168, Bone169,
	Bone170, Bone171, Bone172, Bone173, Bone174, Bone175, Bone176, Bone177, Bone178, Bone179,
	Bone180, Bone181, Bone182, Bone183, Bone184, Bone185, Bone186, Bone187, Bone188, Bone189,
	Bone190, Bone191, Bone192, Bone193, Bone194, Bone195, Bone196, Bone197, Bone198, Bone199,
	Bone200, Bone201, Bone202, Bone203, Bone204, Bone205, Bone206, Bone207, Bone208, Bone209,
	Bone210, Bone211, Bone212, Bone213, Bone214, Bone215, Bone216, Bone217, Bone218, Bone219,
	Bone220, Bone221, Bone222, Bone223, Bone224, Bone225, Bone226, Bone227, Bone228, Bone229,

	BONE_COUNT = 230
};

class Ped {
public:
	uintptr_t pointer;
	uintptr_t playerInfo;
	uintptr_t weaponManager;
	uintptr_t currentWeapon;

	float armor;
	float health;
	Vector3 position;
	Matrix boneMatrix;
	Vector3 boneList[230]{};

	bool GetPlayer(uintptr_t& base) {
		pointer = base;
		return pointer != NULL;
	}

	std::string GetWeaponName() {
		uintptr_t weaponManager = ReadMemory<uintptr_t>(pointer + Offsets.WeaponManager);
		uintptr_t step1 = ReadMemory<uintptr_t>(weaponManager + 0x20);
		return ReadString(ReadMemory<uintptr_t>(step1 + 0x5F0));
	}

	uintptr_t GetWeapon() {
		return ReadMemory<uintptr_t>(ReadMemory<uintptr_t>(ReadMemory<uintptr_t>(pointer + Offsets.WeaponManager) + 0x20) + 0x10);
	}

	int GetId() {
		return ReadMemory<int>(ReadMemory<uint64_t>(pointer + Offsets.PlayerInfo) + Offsets.Id);
	}

	bool IsPlayer() {
		return playerInfo != NULL;
	}

	bool IsDead() {
		return position == Vector3(0.f, 0.f, 0.f);
	}

	bool IsVisible() {
		enum VisibilityFlags : BYTE {
			InvisibleFlag1 = 36,
			InvisibleFlag2 = 0,
			InvisibleFlag3 = 4,
			IntersectMissionEntityAndTrain = 2,
			IntersectPeds1 = 4,
			IntersectVehicles = 10,
			IntersectVegetation = 256,
			FrustumCulling = 512,
			OcclusionCulling = 1024,
			DistanceCulling = 2048
		};

		BYTE VisibilityFlag = ReadMemory<BYTE>(pointer + Offsets.VisibleFlag);
		if (VisibilityFlag == InvisibleFlag1 ||
			VisibilityFlag == InvisibleFlag2 ||
			VisibilityFlag == InvisibleFlag3 ||
			VisibilityFlag == IntersectMissionEntityAndTrain ||
			VisibilityFlag == IntersectPeds1 ||
			VisibilityFlag == IntersectVehicles ||
			VisibilityFlag == IntersectVegetation ||
			(VisibilityFlag & FrustumCulling) ||
			(VisibilityFlag & OcclusionCulling) ||
			(VisibilityFlag & DistanceCulling))
		{
			return false;
		}

		return true;
	}

	bool Update() {
		playerInfo = ReadMemory<uintptr_t>(pointer + Offsets.PlayerInfo);
		currentWeapon = GetWeapon();
		health = ReadMemory<float>(pointer + Offsets.Health);
		position = ReadMemory<Vector3>(pointer + 0x90);
		armor = ReadMemory<float>(pointer + Offsets.Armor);
		boneMatrix = ReadMemory<Matrix>(pointer + 0x60);
		UpdateBones();
		return true;
	}

	void UpdateBones() {

		Bones bones = ReadMemory<Bones>(pointer + Offsets.BoneList);
		boneList[Head] = Vec3Transform(&bones.head, &boneMatrix);
		boneList[LeftFoot] = Vec3Transform(&bones.leftFoot, &boneMatrix);
		boneList[RightFoot] = Vec3Transform(&bones.rightFoot, &boneMatrix);
		boneList[LeftAnkle] = Vec3Transform(&bones.leftAnkle, &boneMatrix);
		boneList[RightAnkle] = Vec3Transform(&bones.rightAnkle, &boneMatrix);
		boneList[LeftHand] = Vec3Transform(&bones.leftHand, &boneMatrix);
		boneList[RightHand] = Vec3Transform(&bones.rightHand, &boneMatrix);
		boneList[Neck] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[Hip] = Vec3Transform(&bones.hip, &boneMatrix);





		for (int i = Spine0; i <= Spine4; i++) {
			float factor = (float)(i - Spine0) / 4.0f;
			boneList[i] = LerpVector3(boneList[Hip], boneList[Neck], factor);
		}


		for (int i = LeftShoulder; i <= LeftForearm; i++) {
			float factor = (float)(i - LeftShoulder) / 3.0f;
			boneList[i] = LerpVector3(boneList[Neck], boneList[LeftHand], factor);
		}


		for (int i = RightShoulder; i <= RightForearm; i++) {
			float factor = (float)(i - RightShoulder) / 3.0f;
			boneList[i] = LerpVector3(boneList[Neck], boneList[RightHand], factor);
		}


		boneList[LeftHip] = LerpVector3(boneList[Hip], boneList[LeftFoot], 0.1f);
		boneList[LeftThigh] = LerpVector3(boneList[Hip], boneList[LeftFoot], 0.35f);
		boneList[LeftKnee] = LerpVector3(boneList[Hip], boneList[LeftFoot], 0.55f);
		boneList[LeftShin] = LerpVector3(boneList[Hip], boneList[LeftFoot], 0.75f);
		boneList[LeftCalfTwist] = LerpVector3(boneList[Hip], boneList[LeftFoot], 0.82f);
		boneList[LeftAnkleTwist] = LerpVector3(boneList[Hip], boneList[LeftFoot], 0.9f);


		boneList[RightHip] = LerpVector3(boneList[Hip], boneList[RightFoot], 0.1f);
		boneList[RightThigh] = LerpVector3(boneList[Hip], boneList[RightFoot], 0.35f);
		boneList[RightKnee] = LerpVector3(boneList[Hip], boneList[RightFoot], 0.55f);
		boneList[RightShin] = LerpVector3(boneList[Hip], boneList[RightFoot], 0.75f);
		boneList[RightCalfTwist] = LerpVector3(boneList[Hip], boneList[RightFoot], 0.82f);
		boneList[RightAnkleTwist] = LerpVector3(boneList[Hip], boneList[RightFoot], 0.9f);


		boneList[LeftQuadriceps] = LerpVector3(boneList[LeftHip], boneList[LeftKnee], 0.3f);
		boneList[LeftHamstring] = LerpVector3(boneList[LeftHip], boneList[LeftKnee], 0.7f);
		boneList[LeftCalf] = LerpVector3(boneList[LeftKnee], boneList[LeftAnkle], 0.5f);
		boneList[LeftSoleus] = LerpVector3(boneList[LeftKnee], boneList[LeftAnkle], 0.7f);

		boneList[RightQuadriceps] = LerpVector3(boneList[RightHip], boneList[RightKnee], 0.3f);
		boneList[RightHamstring] = LerpVector3(boneList[RightHip], boneList[RightKnee], 0.7f);
		boneList[RightCalf] = LerpVector3(boneList[RightKnee], boneList[RightAnkle], 0.5f);
		boneList[RightSoleus] = LerpVector3(boneList[RightKnee], boneList[RightAnkle], 0.7f);


		boneList[LeftHeel] = LerpVector3(boneList[LeftAnkle], boneList[LeftFoot], 0.2f);
		boneList[LeftArch] = LerpVector3(boneList[LeftAnkle], boneList[LeftFoot], 0.5f);
		boneList[LeftBallFoot] = LerpVector3(boneList[LeftAnkle], boneList[LeftFoot], 0.8f);
		boneList[LeftBigToe] = AddOffset(boneList[LeftFoot], 0.0f, 0.05f, 0.0f);
		boneList[LeftLittleToe] = AddOffset(boneList[LeftFoot], 0.03f, 0.03f, 0.0f);

		boneList[RightHeel] = LerpVector3(boneList[RightAnkle], boneList[RightFoot], 0.2f);
		boneList[RightArch] = LerpVector3(boneList[RightAnkle], boneList[RightFoot], 0.5f);
		boneList[RightBallFoot] = LerpVector3(boneList[RightAnkle], boneList[RightFoot], 0.8f);
		boneList[RightBigToe] = AddOffset(boneList[RightFoot], 0.0f, 0.05f, 0.0f);
		boneList[RightLittleToe] = AddOffset(boneList[RightFoot], -0.03f, 0.03f, 0.0f);


		GenerateFingerBones(LeftThumb0, LeftThumb3, boneList[LeftHand]);
		GenerateFingerBones(LeftIndex0, LeftIndex3, boneList[LeftHand]);
		GenerateFingerBones(LeftMiddle0, LeftMiddle3, boneList[LeftHand]);
		GenerateFingerBones(LeftRing0, LeftRing3, boneList[LeftHand]);
		GenerateFingerBones(LeftPinky0, LeftPinky3, boneList[LeftHand]);

		GenerateFingerBones(RightThumb0, RightThumb3, boneList[RightHand]);
		GenerateFingerBones(RightIndex0, RightIndex3, boneList[RightHand]);
		GenerateFingerBones(RightMiddle0, RightMiddle3, boneList[RightHand]);
		GenerateFingerBones(RightRing0, RightRing3, boneList[RightHand]);
		GenerateFingerBones(RightPinky0, RightPinky3, boneList[RightHand]);


		GenerateToeBones(LeftToe0, LeftToe2, boneList[LeftFoot]);
		GenerateToeBones(RightToe0, RightToe2, boneList[RightFoot]);


		boneList[HeadTop] = LerpVector3(boneList[Head], boneList[Neck], -0.2f);
		boneList[Forehead] = LerpVector3(boneList[Head], boneList[Neck], 0.1f);
		boneList[LeftEye] = AddOffset(boneList[Head], -0.05f, 0.03f, 0.0f);
		boneList[RightEye] = AddOffset(boneList[Head], 0.05f, 0.03f, 0.0f);
		boneList[Nose] = AddOffset(boneList[Head], 0.0f, 0.02f, 0.08f);
		boneList[Jaw] = LerpVector3(boneList[Head], boneList[Neck], 0.6f);


		boneList[Chest] = LerpVector3(boneList[Neck], boneList[Hip], 0.3f);
		boneList[UpperChest] = LerpVector3(boneList[Neck], boneList[Hip], 0.2f);
		boneList[LowerChest] = LerpVector3(boneList[Neck], boneList[Hip], 0.4f);


		for (int i = 50; i < BONE_COUNT; i++) {
			if (Vec3Empty(boneList[i])) {
				boneList[i] = InterpolateBone(i);
			}
		}
	}

	void GenerateFingerBones(int startBone, int endBone, const Vector3& handPos) {
		int boneCount = endBone - startBone + 1;
		for (int i = 0; i < boneCount; i++) {
			float factor = (float)i / (boneCount - 1);

			boneList[startBone + i] = AddOffset(handPos,
				(factor * 0.1f) - 0.05f,
				factor * 0.15f,
				0.0f);
		}
	}

	void GenerateToeBones(int startBone, int endBone, const Vector3& footPos) {
		int boneCount = endBone - startBone + 1;
		for (int i = 0; i < boneCount; i++) {
			float factor = (float)i / (boneCount - 1);

			boneList[startBone + i] = AddOffset(footPos,
				0.0f,
				factor * 0.08f,
				0.0f);
		}
	}

	Vector3 AddOffset(const Vector3& base, float x, float y, float z) {
		return Vector3(base.x + x, base.y + y, base.z + z);
	}


	Vector3 LerpVector3(const Vector3& a, const Vector3& b, float t) {
		return Vector3(
			a.x + t * (b.x - a.x),
			a.y + t * (b.y - a.y),
			a.z + t * (b.z - a.z)
		);
	}


	bool Vec3Empty(const Vector3& v) {
		return (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f);
	}

	Vector3 InterpolateBone(int boneIndex) {

		if (boneIndex < 50) return boneList[Head];
		else if (boneIndex < 100) return boneList[Chest];
		else if (boneIndex < 150) return boneList[Hip];
		else return boneList[LeftFoot];
	}
};