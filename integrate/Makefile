IDIR=inc
SDIR=src
ODIR=out
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
