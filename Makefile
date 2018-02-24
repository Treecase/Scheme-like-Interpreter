

CC=gcc
NAMES=functable interpret primitive
SNAMS=$(addprefix src/, $(NAMES))
HEADRS=src/data.h $(addsuffix .h, $(SNAMS))
SRCS=$(addsuffix .c, $(SNAMS))
OBJS=$(addsuffix .o, $(SNAMS))
OBJC=$(addprefix src/obj/, $(addsuffix .o, $(NAMES)))


interpret : src/main.c $(OBJC)
	$(CC) -g src/main.c $(OBJC) -o interpret

$(OBJC) : $(SRCS) $(HEADRS)
	$(CC) -g -c $(SRCS)
	mv $(addsuffix .o, $(NAMES)) src/obj

clean :
	rm interpret $(OBJC)
