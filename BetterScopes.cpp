#include "BetterScopes.h"
#include "openvr/openvr.h"
#include "matrix.h"
#include "utils.h"
#include "Quaternion.h"
#include "playerNodes.h"

#include "f4se/GameCamera.h"

//#include "api/PapyrusVRAPI.h"
//#include "api/VRManagerAPI.h"

namespace BetterScopes {

	PlayerNodes* pn;

	NiPoint3 initialLocal = NiPoint3(0,0,0);

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

			//vrhook = RequestOpenVRHookManagerObject();
			
			return true;
		});


		_MESSAGE("Finished setting up for game load");
	}

	
	// this function runs in the main loop so will fire every frame
	void update() {

		if (!(*g_player)->unkF0) {
			return;
		}

		if (!(*g_player)->unkF0->rootNode) {
			return;
		}

		if (!(*g_player)->firstPersonSkeleton) {
			return;
		}

		pn = (PlayerNodes*)((char*)(*g_player) + 0x6e0);

		NiNode* body = (*g_player)->firstPersonSkeleton;
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

		reticle->setAlignment();
		reticle->moveReticle();


		delete reticle;
		return;
	}

	void Reticle::setAlignment() {
		updateTransformsDown(reticleNode, true);
		NiNode* camera = (*g_playerCamera)->cameraNode;

		eye = camera->m_worldTransform;

		NiPoint3 eyeloc = NiPoint3(0,0,0);

		float eyeoffset = 2.3f;   // offset to the right eye from the player camera

		eyeloc.x = eyeoffset;

		// move the eye world position by the eye offset
		eyeloc = eye.rot * eyeloc;

		eye.pos += eyeloc;

		// get vector from the eye to the center of the reticle
		eye2ret = reticleNode->m_worldTransform.pos - eye.pos;
		
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

	}


	void Reticle::moveReticle() {
		NiPoint3 newLocal = offset + reticleNode->m_localTransform.pos;

		reticleNode->m_localTransform.pos = newLocal;

		updateTransformsDown(reticleNode, true);

	}


}