OBJS=stk.o
LIB=../bin/libstk.a

CFLAGS+=
LDFLAGS+=-lm
INCLUDES+=-I../include

all: $(LIB)

%.o: %.c
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

clean:
	@rm -f $(OBJS)
	@rm -f $(LIB)
