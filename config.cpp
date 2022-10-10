#pragma once
#include "config.h"
#include "include/SimpleIni.h"
#include <chrono>
#include <sstream>
#include <vector>
using namespace std::chrono;


namespace BetterScopes {

	bool c_rightEyeDominant;
	float c_eyeOffset;
	float c_retOffsetX;
	float c_retOffsetZ;
	float c_retInterval;
	float c_scopeZoomScale;
	float c_scopeZoomSpeed;
	float c_scopeDetectThresh;
	float c_scopeDistanceThresh;
	bool c_useFRIKDynamicGripping;
	std::string c_zoomValues{};
	uint64_t lastLoadTime = 0;
	CSimpleIniA ini;

	bool loadConfig(const uint64_t msBetweenLoads) {
		const std::uint64_t now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		if (now - lastLoadTime < msBetweenLoads)
			return false;
		SI_Error rc = ini.LoadFile(".\\Data\\F4SE\\plugins\\BetterScopes.ini");

		if (rc < 0) {
			_MESSAGE("ERROR: cannot read BetterScopes.ini");
			return false;
		}

		c_rightEyeDominant = ini.GetBoolValue("BetterScopes", "rightEyeDominant", true);
		c_eyeOffset = (float)ini.GetDoubleValue("BetterScopes", "eyeOffset", 2.3f);
		c_retOffsetX = (float)ini.GetDoubleValue("BetterScopes", "retOffsetX", 0.23f);
		c_retOffsetZ = (float)ini.GetDoubleValue("BetterScopes", "retOffsetZ", -0.22f);
		c_retInterval = (float)ini.GetDoubleValue("BetterScopes", "retMoveInterval", 0.075f);
		c_scopeZoomScale = (float)ini.GetDoubleValue("BetterScopes", "scopeZoomScale", 1.0f);
		c_scopeZoomSpeed = (float)ini.GetDoubleValue("BetterScopes", "scopeZoomSpeed", 0.5f);
		c_scopeDetectThresh = (float)ini.GetDoubleValue("BetterScopes", "lookScopeDetectThreshold", 0.99f);
		c_scopeDistanceThresh = (float)ini.GetDoubleValue("BetterScopes", "lookScopeDistanceThreshold ", 20.00f);
		c_useFRIKDynamicGripping = ini.GetBoolValue("BetterScopes", "UseFRIKDynamicGripping", true);
		c_zoomValues = ini.GetValue("BetterScopes", "ZoomValues", "2.5,4.0,8.0");

		lastLoadTime = now;
		return true;
	}

	bool saveConfig() {
		const std::uint64_t now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		ini.SetValue("BetterScopes", "retOffsetX", std::to_string(c_retOffsetX).c_str());
		ini.SetValue("BetterScopes", "retOffsetZ", std::to_string(c_retOffsetZ).c_str());
	
		SI_Error rc = ini.SaveFile(".\\Data\\F4SE\\plugins\\BetterScopes.ini");
		if (rc < 0) {
			_MESSAGE("ERROR: cannot save BetterScopes.ini");
			return false;
		}
		lastLoadTime = now;
		_MESSAGE("Saved BetterScopes.ini");
		return true;
	}

	bool getEyeAlignmentConfig() {

		return c_rightEyeDominant;
	}

	float getEyeOffsetConfig() {
		return c_eyeOffset;
	}

	float getRetOffsetXConfig()
	{
		return c_retOffsetX;
	}

	float getRetOffsetZConfig()
	{
		return c_retOffsetZ;
	}

	float getRetIntervalConfig()
	{
		return c_retInterval;
	}
	
	void setRetOffsetXConfig(const float value)
	{
		c_retOffsetX = value;
	}

	void setRetOffsetZConfig(const float value)
	{
		c_retOffsetZ = value;
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

	std::vector<float> getZoomValues() {
		std::string item;
		std::stringstream ss(c_zoomValues);
		std::vector<float> result{};
		const char delim = ',';
		while (std::getline(ss, item, delim)) {
			if (item[0] == '\0')
				continue;
			result.push_back(std::stod(item));
		}

		return result;
	}
}
