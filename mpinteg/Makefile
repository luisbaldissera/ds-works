IDIR=inc
SDIR=src
ODIR=out
BINP=$(HOME)/.local/bin
vpath %.h $(IDIR)
vpath %.c $(SDIR)
vpath %.o $(ODIR)
APP=mpinteg
LAUNCH=launch
INC=$(wildcard $(IDIR)/*.h)
SRC=$(wildcard $(SDIR)/*.c)
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
CC=mpicc
CFLAGS=\
	   -lm			\
	   -I./$(IDIR)

.PHONY: clean compile

all: compile

install: compile $(BINP)
	@echo "Adding program in $(BINP) ..."
	cp $(APP) $(BINP)/$(APP)

$(BINP):
	mkdir -p $(HOME)/.local/bin

compile: $(APP) $(LAUNCH)

$(APP): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR)/main.o: $(SDIR)/main.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(ODIR)/%.o: %.c %.h
	mkdir -p $(ODIR)
	$(CC) -o $@ -c $< $(CFLAGS)


$(LAUNCH):
	@echo "Creating launcher..."
	@echo "#!/bin/bash \nif [ $$""# -lt 2 ]; then \n\techo 'Usage: launch <number> <dx>'; \n\texit; \nfi;\nN=$$""1; \nDX=$$""2; \nmpirun -np $$""N ./$(APP) $$""DX" >> $@
	chmod 0744 $@


clean:
	rm -fr $(ODIR)/*.o $(APP) $(LAUNCH)

test: compile
	./experiments

uninstall:
	@echo "Removing program from $(BINP) ..."
	rm $(BINP)/$(APP)
