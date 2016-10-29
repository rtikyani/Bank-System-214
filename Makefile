all: server client
server: src/account.c src/list.c src/tokenizer.c src/server.c
		gcc -pthread -D_GNU_SOURCE -std=c99 -lm -Wall -o server src/account.c src/list.c src/tokenizer.c src/server.c
client: src/client.c
		gcc -pthread -D_GNU_SOURCE -std=c99 -lm -Wall -o client src/client.c
clean:
		rm server client
