#include <stdint.h>
#define MIDI_CC 1
#define MIDI_NOTE 2
#define MIDI_NOTE_OFF 3

typedef struct {
	uint8_t type;
	uint32_t control;
	uint32_t value;
} MidiEvent;

int open_midi();
void close_midi();
int poll_midi(MidiEvent *event);
void write_midi_cc(uint8_t control, uint8_t value);
