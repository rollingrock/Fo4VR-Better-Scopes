#pragma once

namespace BetterScopes{

	bool loadConfig(const uint64_t msBetweenLoads = 0);
	bool getEyeAlignmentConfig();
	float getEyeOffsetConfig();
	float getScopeZoomScaleConfig();
	float getScopeZoomSpeedConfig();
	float getScopeDetectThreshConfig();
	float getScopeDistanceThreshConfig();
	bool getUseFRIKDynamicGrippingConfig();
	void setUseFRIKDynamicGrippingConfig(bool useGrip);
}
