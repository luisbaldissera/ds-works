IDIR=inc
SDIR=src
ODIR=out
BINP=$(HOME)/.local/bin
vpath %.h $(IDIR)
vpath %.c $(SDIR)
vpath %.o $(ODIR)
APP=integ
INC=$(wildcard $(IDIR)/*.h)
SRC=$(wildcard $(SDIR)/*.c)
OBJ=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
CC=gcc
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

compile: $(APP)

$(APP): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR)/main.o: $(SDIR)/main.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(ODIR)/%.o: %.c %.h
	mkdir -p $(ODIR)
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -fr $(ODIR)/*.o $(APP)

uninstall:
	@echo "Removing program from $(BINP) ..."
	rm $(BINP)/$(APP)
