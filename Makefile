TARGETS=stk swm

all: deps

deps:
	if [ ! -d bin ]; then mkdir bin; fi
	for target in $(TARGETS); do ($(MAKE) -C $$target); done

clean:
	for target in $(TARGETS); do ($(MAKE) -C $$target clean); done

default: all
