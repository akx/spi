LDLIBS := -lportmidi -lm
CFLAGS := -Wall -Wextra -Wno-unused-parameter -g -std=c99

spi: midi.c spi.c screen.c libdot3k.a
