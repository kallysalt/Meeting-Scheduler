CC = g++
CFLAGS = -g -Wall

all: serverM serverA serverB client

serverM: serverM.cpp project.h
	$(CC) $(CFLAGS) serverM.cpp -o serverM
	
serverA: serverA.cpp project.h
	$(CC) $(CFLAGS) serverA.cpp -o serverA

serverB: serverB.cpp project.h
	$(CC) $(CFLAGS) serverB.cpp -o serverB

client: client.cpp  project.h
	$(CC) $(CFLAGS) client.cpp -o client

clean:
	rm -f serverM serverA serverB client
