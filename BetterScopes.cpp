#include "BetterScopes.h"




namespace BetterScopes {


	void startUp() {

		g_modAttachmentMap->ForEach([&](ObjectModMiscPair* item) {
			if (item->key->modContainer.dataSize > 0) {
				int elements = item->key->modContainer.dataSize / sizeof(BGSMod::Attachment::Mod::Data);

				for (auto i = 0; i < elements; i++) {
					if (item->key->modContainer.data[i].target == 48) {
						item->key->modContainer.data[i].value.i.v1 = 0;
						item->key->modContainer.data[i].value.i.v2 = 0;
					}
				}
			}
			
			return true;
		});


		_MESSAGE("Finished setting up for game load");
	}


	void update() {

	}



}