OBJS=calc.o
BIN=../bin/calc

CFLAGS+=
LDFLAGS+=-lm ../bin/libstk.a
INCLUDES+=-I../include

all: $(BIN)
	
%.o: %.c
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BIN): $(OBJS)
	$(CC) -o $@ -Wl,--whole-archive $(OBJS) $(LDFLAGS) -Wl,--no-whole-archive -rdynamic

clean:
	@rm -f $(OBJS)
	@rm -f $(BIN)

default: all
