#define _GNU_SOURCE
#include "midi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <portmidi.h>
#include <porttime.h>

static PmStream* midi_in;
static PmStream* midi_out;
static uint8_t out_cc_state[256];

static int get_midi_device_id(int output) {
	int device_id;
	
	if(output) {
		device_id = Pm_GetDefaultOutputDeviceID();
	} else {
		device_id = Pm_GetDefaultInputDeviceID();
	}
	
	for(int i = 0; i < Pm_CountDevices(); i++) {
		const PmDeviceInfo *di = Pm_GetDeviceInfo(i);
		if(!di) continue;
		if(output) {
			if(strcasestr(di->name, "nanokontrol") && di->output) device_id = i;
		} else {
			if(strcasestr(di->name, "nanokontrol") && di->input) device_id = i;
		}
	}
	const PmDeviceInfo *di = Pm_GetDeviceInfo(device_id);
	if(di) fprintf(stderr, "%s MIDI device: %s\n", (output ? "output" : "input"), di->name);
	return device_id;
}

int open_midi() {
	int err;
	Pm_Initialize();
	Pt_Start(1, 0, 0);
	if((err = Pm_OpenInput(&midi_in, get_midi_device_id(0), NULL, 100, Pt_Time, NULL))) {
		fprintf(stderr, "MIDI input error: %s\n", Pm_GetErrorText(err));
		return err;
	}
	
	if((err = Pm_OpenOutput(&midi_out, get_midi_device_id(1), NULL, 0, Pt_Time, NULL, 0))) {
		fprintf(stderr, "MIDI output error: %s\n", Pm_GetErrorText(err));
		return err;
	}
	
	Pm_SetFilter(midi_in, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);
	memset(out_cc_state, 0, 256);
	return 0;
}

void close_midi() {
	if(midi_in) {
		Pm_Close(midi_in);
		midi_in = NULL;
	}
	if(midi_out) {
		Pm_Close(midi_out);
		midi_out = NULL;
	}
}

int poll_midi(MidiEvent *event) {
	static PmEvent buffer;
	int length;
	
	if(!midi_in) return 0;
	int status = Pm_Poll(midi_in);
    if(!status) return 0;
    length = Pm_Read(midi_in, &buffer, 1);
    if (length <= 0) return 0;
    switch(Pm_MessageStatus(buffer.message)) {
		case 0xB0:
			event->type = MIDI_CC;
			break;
		case 0x9D:
			event->type = MIDI_NOTE;
			break;
		case 0x8D:
			event->type = MIDI_NOTE_OFF;
			break;
		default:
			fprintf(stderr, "midi: %08x\n", buffer.message);
			event->type = Pm_MessageStatus(buffer.message);
			break;
	}
			
	event->control = Pm_MessageData1(buffer.message);
	event->value = Pm_MessageData2(buffer.message);	
	return 1;
}

void write_midi_cc(uint8_t control, uint8_t value) {
	if(out_cc_state[control] == value) return;
	out_cc_state[control] = value;
	if(!midi_out) return;
	Pm_WriteShort(midi_out, 0, Pm_Message(0xB0, control, value));
	
}
