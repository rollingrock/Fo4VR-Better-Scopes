#include "config.h"
#include "include/SimpleIni.h"


namespace BetterScopes {

	bool c_rightEyeDominant;
	float c_eyeOffset;
	float c_scopeZoomScale;
	float c_scopeZoomSpeed;
	float c_scopeDetectThresh;
	float c_scopeDistanceThresh;
	bool c_useFRIKDynamicGripping;

	bool loadConfig() {
		CSimpleIniA ini;
		SI_Error rc = ini.LoadFile(".\\Data\\F4SE\\plugins\\BetterScopes.ini");

		if (rc < 0) {
			_MESSAGE("ERROR: cannot read BetterScopes.ini");
			return false;
		}

		c_rightEyeDominant = ini.GetBoolValue("BetterScopes", "rightEyeDominant", true);
		c_eyeOffset = (float)ini.GetDoubleValue("BetterScopes", "eyeOffset", 2.3f);
		c_scopeZoomScale = (float)ini.GetDoubleValue("BetterScopes", "scopeZoomScale", 1.0f);
		c_scopeZoomSpeed = (float)ini.GetDoubleValue("BetterScopes", "scopeZoomSpeed", 0.5f);
		c_scopeDetectThresh = (float)ini.GetDoubleValue("BetterScopes", "lookScopeDetectThreshold", 0.99f);
		c_scopeDistanceThresh = (float)ini.GetDoubleValue("BetterScopes", "lookScopeDistanceThreshold ", 20.00f);
		c_useFRIKDynamicGripping = ini.GetBoolValue("BetterScopes", "UseFRIKDynamicGripping", true);

		return true;
	}


	bool getEyeAlignmentConfig() {

		return c_rightEyeDominant;
	}

	float getEyeOffsetConfig() {
		return c_eyeOffset;
	}

	float getScopeZoomScaleConfig() {
		return c_scopeZoomScale;
	}

	float getScopeZoomSpeedConfig() {
		return c_scopeZoomSpeed;
	}

	float getScopeDetectThreshConfig() {
		return c_scopeDetectThresh;
	}

	float getScopeDistanceThreshConfig() {
		return c_scopeDistanceThresh;
	}

	bool getUseFRIKDynamicGrippingConfig() {
		return c_useFRIKDynamicGripping;
	}

	void setUseFRIKDynamicGrippingConfig(bool useGrip) {
		c_useFRIKDynamicGripping = useGrip;
	}
}
