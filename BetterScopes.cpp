#include "BetterScopes.h"
#include "openvr/openvr.h"
#include "matrix.h"
#include "utils.h"




namespace BetterScopes {


	void startUp() {

		// on startup go through every mod attachment in the game and if you find bHasScope (target = 48) then
		// set the boolean to false.    this will stop the scope overlay native to the game to fire

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