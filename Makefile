TARGETS=stk swm calc
DEBUGFLAGS=$(CFLAGS) -g -Wall -Wextra -pedantic -Werror
RELEASEFLAGS=$(CFLAGS) -O3

all: debug

release:
	@if [ ! -d bin ]; then mkdir bin; fi
	@for target in $(TARGETS); do (CFLAGS="$(RELEASEFLAGS)" $(MAKE) -C $$target); done

debug:
	@if [ ! -d bin ]; then mkdir bin; fi
	@for target in $(TARGETS); do (CFLAGS="$(DEBUGFLAGS)" $(MAKE) -C $$target); done

clean:
	@for target in $(TARGETS); do ($(MAKE) -C $$target clean); done

default: all
