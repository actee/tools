# make file for http basic access authentication
TARGET=http_basic_crk
CFLAGS=-Wall -Wextra -g
OBJS= http_basic_auth.o parser.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c

http_basic_auth.o: http_basic_auth.c base64_table.h parser.h
	$(CC) $(CFLAGS) -c http_basic_auth.c

clean:
	rm *.o $(TARGET) ~*