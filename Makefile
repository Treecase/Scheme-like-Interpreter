
CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=-lreadline

SRC=$(wildcard src/*.c)
OBJ=$(subst .c,.o,$(addprefix src/obj/, $(notdir $(SRC))))
DEP=$(subst obj,dep,$(subst .o,.d,$(OBJ)))

interpret : $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

-include $(DEP)

src/obj/%.o : src/%.c
	$(CC) $< -c -o $@ $(CFLAGS)

src/dep/%.d : src/%.c
	$(CC) $< -MM -MT $(subst dep,obj,$(subst .d,.o,$@)) >$@


.PHONY: clean
clean :
	rm -f interpret src/obj/* src/dep/*

