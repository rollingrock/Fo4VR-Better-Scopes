#pragma once

namespace BetterScopes{

	bool loadConfig();
	bool getEyeAlignmentConfig();
	float getEyeOffsetConfig();
	float getScopeZoomScaleConfig();
	float getScopeZoomSpeedConfig();
	float getScopeDetectThreshConfig();
	bool getUseFRIKDynamicGrippingConfig();
	void setUseFRIKDynamicGrippingConfig(bool useGrip);
}
