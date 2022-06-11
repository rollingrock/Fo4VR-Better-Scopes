#include "BetterScopes.h"
#include "openvr/openvr.h"
#include "matrix.h"
#include "utils.h"
#include "Quaternion.h"
#include "playerNodes.h"
#include "config.h"

#include "f4se/GameCamera.h"
#include "f4se/GameSettings.h"

#include <algorithm>

//#include "api/PapyrusVRAPI.h"
//#include "api/VRManagerAPI.h"

namespace BetterScopes {

	PlayerNodes* pn;

	NiPoint3 initialLocal = NiPoint3(0,0,0);

	float scopeZoom = 0.0f;
	bool lookingThroughScope = false;

	bool isLoaded = false;

	void startUp() {

		// on startup go through every mod attachment in the game and if you find bHasScope (target = 48) then
		// set the boolean to false.    this will stop the scope overlay native to the game to fire

		g_modAttachmentMap->ForEach([&](ObjectModMiscPair* item) {
			if (item->key->modContainer.dataSize > 0) {
				int elements = item->key->modContainer.dataSize / sizeof(BGSMod::Attachment::Mod::Data);

				for (auto i = 0; i < elements; i++) {
					if (item->key->modContainer.data[i].target == 48) {
						item->key->modContainer.data[i].value.i.v1 = 0;
						item->key->modContainer.data[i].value.i.v2 = 0;
					}
				}
			}

			return true;
		});

		if (!loadConfig()) {
			_MESSAGE("FAILED TO LOAD CONFIG INI");
		}

		isLoaded = true;

		_MESSAGE("Finished setting up for game load");
	}

	
	// this function runs in the main loop so will fire every frame
	void update() {
		if (!isLoaded) {
			return;
		}

		if (!(*g_player)) {
			return;
		}

		PlayerCharacter* pc = (*g_player);

		if (!pc->unkF0) {
			return;
		}

		if (!pc->unkF0->rootNode) {
			return;
		}

		if (!pc->firstPersonSkeleton) {
			return;
		}

		pn = (PlayerNodes*)((char*)pc + 0x6e0);

		NiNode* body = pc->firstPersonSkeleton;
		Reticle* reticle = nullptr;
		NiAVObject* scopeRet = nullptr;

		static BSFixedString reticleNodeName = "ReticleNode";
		static BSFixedString weapNodeName = "Weapon";

		// in case i start putting holsters with frik attached to the body make sure only looking at scopes below the weapon node
		NiAVObject* weap = body->GetObjectByName(&weapNodeName);
		if (weap) {
			scopeRet = weap->GetObjectByName(&reticleNodeName);
		}
		else {
			return;
		}

		if (scopeRet) {
			reticle = new Reticle(scopeRet->GetAsNiNode(), body);
			if (vec3_len(initialLocal) == 0.0) {
				initialLocal = scopeRet->m_localTransform.pos;
			}
			else {
				scopeRet->m_localTransform.pos = initialLocal;
			}
		}
		else {
			// nothing to do with the scope so exit
			initialLocal = NiPoint3(0, 0, 0);
			return;
		}

		reticle->collimateSight();
		reticle->moveReticle();

		setEquippedScopeZoom();

		delete reticle;


		Setting* set = GetINISetting("bForceUseCustomFOV:VR");

		Setting* set2 = GetINISetting("iCustomFOVEyeIndex:VR");


		return;
	}

	void setEquippedScopeZoom() {
		PlayerCharacter* pc = *g_player;

		TESObjectWEAP* weap = (TESObjectWEAP*)pc->middleProcess->unk08->equipData->item;

		if (!weap) {
			return;
		}

		if (weap->GetFormType() != FormType::kFormType_WEAP) {
			scopeZoom = 0.0f;
			return;
		}

		if (!pc->actorState.IsWeaponDrawn() || !lookingThroughScope) {
			scopeZoom = 0.0f;
			return;
		}

		TESObjectWEAP::InstanceData* weapData = (TESObjectWEAP::InstanceData*)pc->middleProcess->unk08->equipData->instanceData;

		if (weapData) {
			scopeZoom = weapData->zoomData->zoomData.fovMult;
		}

	}

	float getZoomMultiplier() {
		return scopeZoom;
	}

	void keepScopeVisible() {
		PlayerCharacter* pc = *g_player;

		if (!pc->actorState.IsWeaponDrawn()) {
			return;
		}

		static BSFixedString scopeNormalName = "ScopeNormal";
		static BSFixedString scopeAimingName = "ScopeAiming";

		NiNode* sNorm = pc->firstPersonSkeleton->GetObjectByName(&scopeNormalName)->GetAsNiNode();
		NiNode* sAim = pc->firstPersonSkeleton->GetObjectByName(&scopeAimingName)->GetAsNiNode();

		if (!sNorm || !sAim) {
			return;
		}

		sNorm->flags |= 0x1;
		sAim->flags &= 0xFFFFFFFFFFFFFFFE;

	}
	
	void Reticle::collimateSight() {
		updateTransformsDown(reticleNode, true);   // reset reticle node back to default position
		NiNode* camera = (*g_playerCamera)->cameraNode;

		eye = camera->m_worldTransform;

		NiPoint3 eyeloc = NiPoint3(0, 0, 0);

		float eyeoffset = getEyeAlignmentConfig() ? getEyeOffsetConfig() : getEyeOffsetConfig() * -1;   // offset to the eye from the player camera

		eyeloc.x = eyeoffset;

		// move the eye world position by the eye offset
		eyeloc = eye.rot * eyeloc;

		eye.pos += eyeloc;

		// get vector from the eye to the center of the reticle
		eye2ret = reticleNode->m_worldTransform.pos - eye.pos;

		NiPoint3 hmdFwd = vec3_norm(pn->HmdNode->m_worldTransform.rot * NiPoint3(0, 1, 0));

		float dotHMD = vec3_dot(hmdFwd, vec3_norm(eye2ret));

		// use the length of this vector later to scale the calculated unit vector offset to the eye plane
		float eyelen = vec3_len(eye2ret);

		// vector pointing straight down the barrel 
		ret2Barrel = vec3_norm(reticleNode->m_localTransform.pos);

		// make eye2ret local to reticle node
		eye2ret = reticleNode->m_parent->m_worldTransform.rot.Transpose() * eye2ret;

		eye2ret = vec3_norm(eye2ret);

		// will need this for later to turn off moving the reticle once not looking down the scope
		float dot = vec3_dot(eye2ret, ret2Barrel);

		// reticle offset will be the difference between the eye2ret and ret2barrel vectors
		// this effect will project out to infinity.   in the future will add project to range option

		offset = ret2Barrel - eye2ret;
		offset.y = 0;   // do not want to move the reticle forward or backwards from the player
		offset *= eyelen - 1;  // subtract by 1 since offset was calculated 1 unit down the barrel

		lookingThroughScope = dot > getScopeDetectThreshConfig() ? true : false;

		lookingThroughScope = dotHMD > (getScopeDetectThreshConfig() - 0.02) ? lookingThroughScope : false;   // widen hmd to scope threshold slightly to allow for some margin off the center of the eye

	}


	void Reticle::moveReticle() {
		if (!lookingThroughScope) {
			return;
		}

		NiPoint3 newLocal = offset + reticleNode->m_localTransform.pos;

		reticleNode->m_localTransform.pos = newLocal;

		updateTransformsDown(reticleNode, true);

	}


}