OBJS=swm.o
BIN=swm

CFLAGS+=-Wall -g -O3 -Iinclude
LDFLAGS+=-L/opt/vc/lib/ -lbcm_host -lm -Lstk libstk.a
INCLUDES+=-I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux

TARGETS=stk

all: deps $(BIN)
	
deps:
	for target in $(TARGETS); do ($(MAKE) -C $$target); done

%.o: %.c
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

$(BIN): $(OBJS)
	$(CC) -o $@ -Wl,--whole-archive $(OBJS) $(LDFLAGS) -Wl,--no-whole-archive -rdynamic

clean:
	for target in $(TARGETS); do ($(MAKE) -C $$target clean); done

	@rm -f $(OBJS)
	@rm -f $(BIN)

default: all
