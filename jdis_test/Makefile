hashtable_dir = ../hashtable/
opt_dir = ../opt/
holdall_dir = ../holdall/

CC = gcc

CFLAGS = -std=c2x \
    -Wall -Wconversion -Werror -Wextra -Wpedantic -Wwrite-strings \
    -O2 \
    -D_FORTIFY_SOURCE=2 \
    -fstack-protector-strong \
    -fPIE \
    -DxWANT_HASHTABLE_EXT \
    -DWANT_HOLDALL_EXT \
    -I$(hashtable_dir) -I$(opt_dir) -I$(holdall_dir)

LDFLAGS = -pie \
    -Wl,-z,relro,-z,now \
    -Wl,-z,noexecstack

vpath %.c $(hashtable_dir) $(opt_dir) $(holdall_dir)
vpath %.h $(hashtable_dir) $(opt_dir) $(holdall_dir)

objects = main.o hashtable.o opt.o holdall.o
executable = jdis

.PHONY: all clean

all: $(executable)

$(executable): $(objects)
	$(CC) $(objects) -o $(executable) $(LDFLAGS)

main.o: main.c hashtable.h hashtable_ip.h holdall.h holdall_ip.h opt.h opt_ip.h 
hashtable.o: hashtable.c hashtable.h hashtable_ip.h 
opt.o: opt.c opt.h opt_ip.h 
holdall.o: holdall.c holdall.h holdall_ip.h

clean:
	$(RM) $(objects) $(executable)
