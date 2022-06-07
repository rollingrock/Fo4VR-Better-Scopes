#pragma once

#include "f4se/GameReferences.h"
#include "f4se/NiNodes.h"
#include "openvr/openvr.h"

#include "matrix.h"

namespace BetterScopes {

	 float vec3_len(NiPoint3 v1);
	 NiPoint3 vec3_norm(NiPoint3 v1);

	 float vec3_dot(NiPoint3 v1, NiPoint3 v2);
	 
	 NiPoint3 vec3_cross(NiPoint3 v1, NiPoint3 v2);

	 float vec3_det(NiPoint3 v1, NiPoint3 v2, NiPoint3 n);

	 float degrees_to_rads(float deg);
	 float rads_to_degrees(float deg);

	 NiPoint3 rotateXY(NiPoint3 vec, float angle);
	 NiPoint3 pitchVec(NiPoint3 vec, float angle);

	 NiMatrix43 getRotationAxisAngle(NiPoint3 axis, float theta);

	 void updateTransforms(NiNode* node);

	 void updateTransformsDown(NiNode* nde, bool updateSelf);

	 bool getLeftHandedMode();

	 NiNode* getChildNode(const char* nodeName, NiNode* nde);
	 NiNode* get1stChildNode(const char* nodeName, NiNode* nde);

	 Matrix44 HMD43MatrixToNiMatrix(vr::HmdMatrix34_t hmdMat);
	 Matrix44 HMD44MatrixToNiMatrix(vr::HmdMatrix44_t hmdMat);

}
