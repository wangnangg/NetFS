# You can compile with either gcc or g++
CC = g++
CFLAGS = -o 
OPTS = -O2
LINKED = -lPocoNet -lPocoUtil -lPocoFoundation
OBJ = StorageServerConnection.o StorageServerConnectionFactory.o StorageServer.o StorageServerParams.o StorageInterface.o 

all:
	$(CC) -c StorageServer.cpp -std=c++11 $(OPTS)
	$(CC) -c StorageServerConnection.cpp -std=c++11 $(OPTS)
	$(CC) -c StorageServerConnectionFactory.cpp -std=c++11 $(OPTS)
	$(CC) -c StorageServerParams.cpp -std=c++11 $(OPTS)
	$(CC) -c StorageInterface.cpp -std=c++11 $(OPTS)
	$(CC) $(CFLAGS) storageServer $(OBJ) $(LINKED) -std=c++11 $(OPTS)

clean:
	rm storageServer *~ $(OBJ)
