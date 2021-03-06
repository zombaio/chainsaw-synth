#include "VoiceManager.h"

VoiceManager::VoiceManager(){ }
VoiceManager::~VoiceManager(){ }

void VoiceManager::process(Buffer *buf, Parameters *p){
	for(int i = 0; i < NUM_VOICE; i++){
		if(voice[i].isActive()){
			voice[i].process(buf, p);	
		}
	}
	// Let's make it bit quieter
	for(int i = 0; i < buf->size; i++){
		buf->dataL[i] *= 0.5;
		buf->dataR[i] *= 0.5;
	}

}

void VoiceManager::note(Note n){
	for(int i = 0; i < NUM_VOICE; i++){
		if(n.type == Note::NOTE_ON){
			if(!voice[i].isActive()){
				voice[i].note(n);
				return;
			}
		} else {
			if(voice[i].isActive() && voice[i].getNote() == n.note){
				voice[i].note(n);
			}
		}
	}
	// FIXME: No free voices?
}

