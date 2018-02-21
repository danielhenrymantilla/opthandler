EXE=example

OPTHANDLER=opthandler

CFLAGS=-Wall
LDFLAGS=

.PHONY: all test_example clean

all: test_example

test_example: $(EXE)
	./$< --help

$(EXE): $(EXE).c $(OPTHANDLER).o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	@rm -f $(EXE) *.o
