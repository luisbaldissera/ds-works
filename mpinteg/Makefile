IDIR=inc
SDIR=src
ODIR=out
BINP=$(HOME)/.local/bin
vpath %.h $(IDIR)
vpath %.c $(SDIR)
vpath %.o $(ODIR)
APP=mpinteg
INC=$(wildcard $(IDIR)/*.h)
SRC=$(wildcard $(SDIR)/*.c)
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
CC=mpicc
CFLAGS=\
	   -lm			\
	   -I./$(IDIR)

.PHONY: clean compile

all: compile

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

test:
	. make_experiments.sh

clean:
	rm -fr $(ODIR)/*.o $(APP)

