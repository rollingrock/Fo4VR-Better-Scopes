#include "config.h"
#include "include/SimpleIni.h"


namespace BetterScopes {

	bool c_rightEyeDominant;
	float c_eyeOffset;

	bool loadConfig() {
		CSimpleIniA ini;
		SI_Error rc = ini.LoadFile(".\\Data\\F4SE\\plugins\\BetterScopes.ini");

		if (rc < 0) {
			_MESSAGE("ERROR: cannot read BetterScopes.ini");
			return false;
		}

		c_rightEyeDominant = ini.GetBoolValue("BetterScopes", "rightEyeDominant", true);
		c_eyeOffset = (float)ini.GetDoubleValue("BetterScopes", "eyeOffset", 2.3f);

		return true;
	}


	bool getEyeAlignmentConfig() {

		return c_rightEyeDominant;
	}

	float getEyeOffsetConfig() {
		return c_eyeOffset;
	}
}
