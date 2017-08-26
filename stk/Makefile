OBJS=stk.o
LIB=../libstk.a

CFLAGS+=-c -Wall -g -O3 -I../include
LDFLAGS+=-L/opt/vc/lib/ -lbcm_host -lm
INCLUDES+=-I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux

all: $(LIB)

%.o: %.c
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@ -Wno-deprecated-declarations

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

clean:
	@rm -f $(OBJS)
	@rm -f $(LIB)
