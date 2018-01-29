CC = gcc

CFLAGS = -Wall -Wextra -pedantic -O2 -std=c99
DEBUG_CFLAGS = -fstack-protector-all -g -D U_DBG

LD_FLAGS = -lusb

STRIP = strip

OUTFILE = ckcc

ifdef K70
	CFLAGS += -DKB_K70
endif

default: ckcc

debug: debug_ckcc

# -----------------------------------------------------------------------------
# Compilation targets
ckcc:
	$(CC) ckcc.c -o $(OUTFILE) $(CFLAGS) $(LD_FLAGS)
	$(STRIP) $(OUTFILE)

debug_ckcc:
	$(CC) ckcc.c -o $(OUTFILE) $(CFLAGS) $(LD_FLAGS) $(DEBUG_CFLAGS)

clean:
	rm $(OUTFILE)
