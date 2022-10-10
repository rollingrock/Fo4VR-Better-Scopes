#pragma once
#include <vector>

namespace BetterScopes{

	bool loadConfig(const uint64_t msBetweenLoads = 0);
	bool saveConfig();
	bool getEyeAlignmentConfig();
	float getEyeOffsetConfig();
	float getRetOffsetXConfig();
	float getRetOffsetZConfig();
	float getRetIntervalConfig();
	void setRetOffsetXConfig(const float value);
	void setRetOffsetZConfig(const float value);
	float getScopeZoomScaleConfig();
	float getScopeZoomSpeedConfig();
	float getScopeDetectThreshConfig();
	float getScopeDistanceThreshConfig();
	bool getUseFRIKDynamicGrippingConfig();
	void setUseFRIKDynamicGrippingConfig(bool useGrip);
	std::vector<float> getZoomValues();
}
