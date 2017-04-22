#makefile for http digest access authentication
TARGET=http_digest_crk
CFLAGS=-Wall -Wextra -g
OBJS=http_digest_crk.o parser.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

http_digest_crk.o: http_digest_crk.c http_digest_crk.h parser.h
	$(CC) $(CFLAGS) -c http_digest_crk.c

clean:
	rm *.o $(TARGET) ~*
