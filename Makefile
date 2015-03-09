#############################################################
#	Common Makefile for simple projects
#############################################################

#CROSS=/usr/local/arm/4.2.2-eabi/usr/bin-ccache/arm-linux-
CC=$(CROSS)gcc

CFLAGS=-g -I.

LFLAGS= -L.
LFLAGS+=-static

CSRCS = sudoku.c

OBJS = $(CSRCS:.c=.o)

.SUFFIXES:.c.o

TARGET=sudoku-buster

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "LD " $@
	@$(CC) $(CFLAGS) -g -o $@ $(OBJS) $(LFLAGS)

clean:
	@echo "RM " $(TARGET) $(OBJS)
	@rm -f $(TARGET) $(OBJS)

# Rules
.c.o:
	@echo "CC " $<
	@$(CC) $(CFLAGS) -c -o $@ $<

