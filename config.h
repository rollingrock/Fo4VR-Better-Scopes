#pragma once

namespace BetterScopes{

	bool loadConfig();
	bool getEyeAlignmentConfig();
	float getEyeOffsetConfig();
	float getScopeZoomScaleConfig();
	float getScopeZoomSpeedConfig();
	float getScopeDetectThreshConfig();
	float getScopeDistanceThreshConfig();
	bool getUseFRIKDynamicGrippingConfig();
	void setUseFRIKDynamicGrippingConfig(bool useGrip);
}
