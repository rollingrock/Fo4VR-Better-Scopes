#include "hook.h"
#include "BetterScopes.h"


RelocAddr<uintptr_t> hookMainLoopFunc(0xd8187e);


typedef void(*_hookedMainLoop)();
RelocAddr<_hookedMainLoop> hookedMainLoop(0xd83ac0);

namespace Hook {

	void hookIt() {

		BetterScopes::update();

		hookedMainLoop();
		return;
	}


	void hookMain() {

		g_branchTrampoline.Write5Call(hookMainLoopFunc.GetUIntPtr(), (uintptr_t)&hookIt);
	}


}




