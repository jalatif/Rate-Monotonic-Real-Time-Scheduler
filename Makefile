EXTRA_CFLAGS+=
APP_EXTRA_FLAGS:= -O2 -ansi -pedantic

KERNEL_SRC:= /lib/modules/$(shell uname -r)/build
SUBDIR=$(PWD)
GCC:=gcc
RM:=rm

.PHONY : clean

all: clean modules app

obj-m := mp2.o

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(SUBDIR) modules

app: userapp.c
	$(GCC) -std=c99 userapp.c -o userapp -lm

clean:
	$(RM) -f userapp mp2 *~ *.ko *.o *.mod.c Module.symvers modules.order

