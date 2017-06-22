TARGET=turing
OBJS=main.o fileio.o

$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

main.o: main.c fileio.h
fileio.o:fileio.c fileio.h

.c.o:
	$(CC) -c $<

clean:
	rm $(OBJS)
