#pragma once
#include "f4se_common/Relocation.h"
#include "f4se/GameForms.h"
#include "f4se/GameReferences.h"
#include "f4se/GameObjects.h"
#include "f4se/NiNodes.h"


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
	void setEquippedScopeZoom();
	float getZoomMultiplier();
}
