CC = g++
CFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -fsanitize=address,undefined -g -lm
MKDIR = mkdir -p

all: server client

server:
	${MKDIR} ./bin/
	${CC} ${CFLAGS} ./server.cpp -o ./bin/server
	
client:
	${MKDIR} ./bin/
	${CC} ${CFLAGS} ./client.cpp -o ./bin/client

clean:
	rm -rf bin