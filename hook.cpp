#include "hook.h"
#include "utils.h"
#include "config.h"
#include "BetterScopes.h"

#include <math.h>

RelocAddr<uintptr_t> hookMainLoopFunc(0xd83ebc);
RelocAddr<uintptr_t> hookLateCulling(0xd84ee4);
RelocAddr<uintptr_t> hookEyeFOVAdjust(0x2804c26);

typedef void(*_hookedMainLoop)(uint64_t rcx);
RelocAddr<_hookedMainLoop> hookedMainLoop(0x1ba7a80);

typedef void(*_hookedLateCulling)(uint64_t rcx);
RelocAddr<_hookedLateCulling> hookedLateCulling(0xf10ed0);

typedef void(*_hookedEyeFOVAdjustFunc)(uintptr_t camera, uintptr_t adjust, UINT eye);
RelocAddr<_hookedEyeFOVAdjustFunc> hookedEyeFOVAdjustFunc(0x1c2bf60);

bool dir = true;

namespace Hook {

	void hookIt(uint64_t rcx) {
		BetterScopes::update();
		hookedMainLoop(rcx);
		return;
	}
	
	void hookCulling(uint64_t rcx) {
		BetterScopes::keepScopeVisible();
		hookedLateCulling(rcx);
		return;
	}

	struct eyeFOV {
		float left;
		float right;
		float bottom;
		float top;
	};

	void hookEyeFOV(uintptr_t camera, uintptr_t adjust, UINT eye) {

		eyeFOV* fov = (eyeFOV*)adjust;

		float fovScale = BetterScopes::getScopeZoomScaleConfig();
		float fovMult = BetterScopes::getZoomMultiplier() * fovScale;
		float fovSpeed = BetterScopes::getScopeZoomSpeedConfig();

		static float fovAdjust = 0.0f;

		if (fovAdjust < fovMult) {
			fovAdjust += fovSpeed;
		}
		else if (fovAdjust > 0.0f) {
			fovAdjust -= fovSpeed;
		}
		
		if (fovAdjust < 0.0f) {
			// never zoom out
			fovAdjust = 0.0f;
		}

		float leftDeg = BetterScopes::rads_to_degrees(atan(fov->left));
		float rightDeg = BetterScopes::rads_to_degrees(atan(fov->right));
		float botDeg = BetterScopes::rads_to_degrees(atan(fov->bottom));
		float topDeg = BetterScopes::rads_to_degrees(atan(fov->top));

		leftDeg += fovAdjust;
		rightDeg -= fovAdjust;
		topDeg += fovAdjust;
		botDeg -= fovAdjust;

		fov->left = tanf(BetterScopes::degrees_to_rads(leftDeg));
		fov->right = tanf(BetterScopes::degrees_to_rads(rightDeg));
		fov->top = tanf(BetterScopes::degrees_to_rads(topDeg));
		fov->bottom = tanf(BetterScopes::degrees_to_rads(botDeg));

		hookedEyeFOVAdjustFunc(camera, adjust, eye);
	}


	void hookMain() {

		g_branchTrampoline.Write5Call(hookMainLoopFunc.GetUIntPtr(), (uintptr_t)&hookIt);
		g_branchTrampoline.Write5Call(hookLateCulling.GetUIntPtr(), (uintptr_t)&hookCulling);
		g_branchTrampoline.Write5Call(hookEyeFOVAdjust.GetUIntPtr(), (uintptr_t)&hookEyeFOV);
	}


}




