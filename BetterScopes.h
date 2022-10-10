#pragma once
#include "f4se_common/Relocation.h"
#include "f4se/GameForms.h"
#include "f4se/GameReferences.h"
#include "f4se/GameObjects.h"
#include "f4se/NiNodes.h"
#include "f4se/PluginAPI.h"  // SKSEInterface, PluginInfo

extern PluginHandle g_pluginHandle;
extern F4SEMessagingInterface* g_messaging;

namespace BetterScopes {

	class Reticle {
	public:
		Reticle() : reticleNode(nullptr) {
			rightEye = true;
		};

		Reticle(NiNode* a_node, NiNode* a_body) : reticleNode(a_node), body(a_body) {
			rightEye = true;
		};

		void collimateSight();

		void moveReticle();


	private:
		NiNode* reticleNode;
		NiPoint3 eye2ret;
		NiPoint3 ret2Barrel;
		NiPoint3 offset;
		bool rightEye;
		NiNode* body;
		NiTransform eye;
	};

	void startUp();
	void update();
	void setEquippedScopeZoom(const bool toggleZoom=false);
	float getZoomMultiplier();
	void keepScopeVisible();
	void setZoomToggle(const bool value = true);
	void repositionReticle(const float x, const float z, const float interval);
	void saveReticlePreview();
	void handleStaticGripping(NiNode* weaponNode);
	void restoreWeaponNode(NiNode* weaponNode, NiTransform weapSave);
}
