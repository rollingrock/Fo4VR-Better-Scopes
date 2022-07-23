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
PluginHandle g_pluginHandle = kPluginHandle_Invalid;
F4SEMessagingInterface* g_messaging = NULL;

namespace BetterScopes {

	PlayerNodes* pn;

	NiPoint3 initialLocal = NiPoint3(0,0,0);

	float scopeZoom = 0.0f;
	bool lookingThroughScope = false;

	bool isLoaded = false;

	bool stickyLookScope = false;

	float lastZoom = 0.f;
	std::vector<float> zoomValues;
	bool zoomTogglePressed = false;
	NiPoint3 previewOffset = NiPoint3(0, 0, 0);

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
		zoomValues = getZoomValues();

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
		loadConfig(5000); // only load every 5 seconds

		pn = (PlayerNodes*)((char*)pc + 0x6e0);

		NiNode* body = pc->firstPersonSkeleton;
		Reticle* reticle = nullptr;
		NiAVObject* scopeRet = nullptr;

		static BSFixedString reticleNodeName = "ReticleNode";
		static BSFixedString weapNodeName = "Weapon";
		static BSFixedString weapOffNodeName = "WeaponOffset";

		// in case i start putting holsters with frik attached to the body make sure only looking at scopes below the weapon node
		NiAVObject* weap = body->GetObjectByName(&weapNodeName);
		if (weap) {
			NiPoint3 offset = NiPoint3(-0.94, 0, 0);
			NiAVObject* weapOffset = weap->GetObjectByName(&weapOffNodeName);

			if (weapOffset) {
				offset.x -= weapOffset->m_localTransform.pos.y;
				offset.y -= -2.099;
			}
			weap->m_localTransform.pos += offset;
			scopeRet = weap->GetObjectByName(&reticleNodeName);
		}
		else {
			return;
		}

		if (scopeRet) {
			updateTransformsDown(weap->GetAsNiNode(), true);
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

		if (stickyLookScope && zoomTogglePressed) {
			setEquippedScopeZoom(true);
			zoomTogglePressed = false;
		}else{
			setEquippedScopeZoom(false);
		}

		delete reticle;

		return;
	}

	// Native function that takes the 1st person skeleton weapon node and calculates the skeleton from upperarm down based off the offsetNode
	void update1stPersonArm(PlayerCharacter* pc, NiNode** weapon, NiNode** offsetNode) {
		using func_t = decltype(&update1stPersonArm);
		RelocAddr<func_t> func(0xef6280);

		return func(pc, weapon, offsetNode);
	}

	void set1stPersonArm(NiNode* weapon, NiNode* offsetNode) {

		NiNode** wp = &weapon;
		NiNode** op = &offsetNode;

		update1stPersonArm(*g_player, wp, op);
	}

	void handleStaticGripping(NiNode* weaponNode) {

		NiNode* offsetNode = pn->primaryWeaponOffsetNOde;
		Matrix44 w;
		w.data[0][0] = -0.120;
		w.data[1][0] = 0.987;
		w.data[2][0] = 0.108;
		w.data[0][1] = 0.991;
		w.data[1][1] = 0.112;
		w.data[2][1] = 0.077;
		w.data[0][2] = 0.064;
		w.data[1][2] = 0.116;
		w.data[2][2] = -0.991;


		weaponNode->m_localTransform.rot = w.make43();

		weaponNode->m_localTransform.pos = NiPoint3(6.389, -2.099, -3.133);

		weaponNode->IncRef();
		set1stPersonArm(weaponNode, offsetNode);
	}

	void restoreWeaponNode(NiNode* weaponNode, NiTransform weapSave) {
		weaponNode->m_localTransform = weapSave;
	}

	void setEquippedScopeZoom(const bool toggleZoom) {
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
			weapData;
			const auto maxZoom = weapData->zoomData->zoomData.fovMult;
			scopeZoom = lastZoom > 0 && lastZoom <= maxZoom ? lastZoom : maxZoom;

			if (zoomValues.size() > 1 && toggleZoom) {
				lastZoom = scopeZoom;
				const auto itr = std::find(zoomValues.begin(), zoomValues.end(), lastZoom);
				int startIndex = 0;
				if (itr != zoomValues.end()) {
					startIndex = std::distance(zoomValues.begin(), itr);
					_MESSAGE("LastZoom found: %0.2lf at %d", lastZoom, startIndex);
				}
				else {
					_MESSAGE("LastZoom not found: %0.2lf", lastZoom);
				}
				for (int i = startIndex + 1; i != startIndex; i = (i + 1) % zoomValues.size())
					if (maxZoom > 0 && zoomValues[i] > 0 && maxZoom >= zoomValues[i]) {
						scopeZoom = zoomValues[i];
						_DMESSAGE("Found zoom %0.2lf at index %d below maxZoom. Setting zoom to %0.2lf", zoomValues[i], i, scopeZoom);
						break;
					}
					else {
						_DMESSAGE("Ignoring unreachable zoom %0.2lf at index %d: weapon max at %0.2lf", zoomValues[i], i, scopeZoom);
						continue;
					}
			}
			lastZoom = scopeZoom;
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

		NiAVObject* sNorm = pc->firstPersonSkeleton->GetObjectByName(&scopeNormalName);
		NiAVObject* sAim = pc->firstPersonSkeleton->GetObjectByName(&scopeAimingName);

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
		// apply reticle adjustments
		if (!(previewOffset.x == 0 && previewOffset.z == 0)){ // if previewOffset has value, we're in preview mode
			offset.x += previewOffset.x;
			offset.z += previewOffset.z;
		}else { // use ini data
			offset.x += getRetOffsetXConfig(); // left/right
			offset.z += getRetOffsetZConfig(); // up/down
		}

		lookingThroughScope = dot > getScopeDetectThreshConfig() ? true : false;

		lookingThroughScope = dotHMD > (getScopeDetectThreshConfig() - 0.02) ? lookingThroughScope : false;   // widen hmd to scope threshold slightly to allow for some margin off the center of the eye

		lookingThroughScope = eyelen <= getScopeDistanceThreshConfig() ? lookingThroughScope : false;   // require scope to be closer to eye for zoom effect

		if (stickyLookScope != lookingThroughScope) {
			g_messaging->Dispatch(g_pluginHandle, 15, (void*)lookingThroughScope, sizeof(bool), "F4VRBody");
			stickyLookScope = lookingThroughScope;
		}	

	}


	void Reticle::moveReticle() {
		if (!lookingThroughScope) {
			return;
		}

		NiPoint3 newLocal = offset + reticleNode->m_localTransform.pos;

		reticleNode->m_localTransform.pos = newLocal;

		updateTransformsDown(reticleNode, true);

	}

	void setZoomToggle(const bool value) {
		zoomTogglePressed = value;
	}

	void repositionReticle(const float x, const float z, const float interval) {
		if (x == 0.f && z == 0.f) { // treat 0,0 entry as a reset
			previewOffset.x = 0.f;
			previewOffset.z = 0.f;
			setRetOffsetXConfig(0.f);
			setRetOffsetZConfig(0.f);
			return;
		}
		else if (previewOffset.x == 0.f && previewOffset.z == 0.f) { // if previewOffset empty, load with config info.
			previewOffset.x = getRetOffsetXConfig();
			previewOffset.z = getRetOffsetZConfig();
		}
		previewOffset.x += x * interval;
		previewOffset.z += z * interval;
	}

	void saveReticlePreview() {
		setRetOffsetXConfig(previewOffset.x);
		setRetOffsetZConfig(previewOffset.z);
		previewOffset.x = 0.f;
		previewOffset.z = 0.f;
		saveConfig();
		loadConfig();
	}
}