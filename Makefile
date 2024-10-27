CC=g++ 
CFLAGS=-I. -I/json -g -O0

path = /home/luca/homework4-public/json
client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) -o client client.cpp requests.cpp helpers.cpp buffer.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
