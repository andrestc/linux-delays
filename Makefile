CC=gcc
CFLAGS=-Wall $(shell pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)

ODIR=obj

_OBJ = getdelays.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

getdelays: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
