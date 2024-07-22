CC = gcc
CFLAGS = -Wall -pthread

SRC_Q3 = q3.c
OBJ_Q3 = q3.o

SRC_REACTOR = reactor.c
OBJ_REACTOR = reactor.o

SRC_Q1 = Q1.c
OBJ_Q1 = Q1.o

SRC_Q2 = q2.c
OBJ_Q2 = q2.o

SRC_Q4SERVER = q4server.c
OBJ_Q4SERVER = q4server.o

SRC_Q6SERVER = q6server.c
OBJ_Q6SERVER = q6server.o

SRC_Q7SERVER = q7server.c
OBJ_Q7SERVER = q7server.o

SRC_Q9SERVER = q9server.c
OBJ_Q9SERVER = q9server.o

SRC_CLIENT = client.c
OBJ_CLIENT = client.o

all: Q1 q2 q4server q7server q6server q9server client

$(OBJ_Q3): $(SRC_Q3) q3.h
	$(CC) $(CFLAGS) -c $(SRC_Q3)

$(OBJ_REACTOR): $(SRC_REACTOR) reactor.h
	$(CC) $(CFLAGS) -c $(SRC_REACTOR)

Q1: $(OBJ_Q1) 
	$(CC) $(CFLAGS) -o Q1 $(OBJ_Q1)

q2: $(OBJ_Q2) 
	$(CC) $(CFLAGS) -o q2 $(OBJ_Q2)

q4server: $(OBJ_Q4SERVER) $(OBJ_Q3)
	$(CC) $(CFLAGS) -o q4server $(OBJ_Q4SERVER) $(OBJ_Q3)

q6server: $(OBJ_Q6SERVER) $(OBJ_Q3) $(OBJ_REACTOR)
	$(CC) $(CFLAGS) -o q6server $(OBJ_Q6SERVER) $(OBJ_Q3) $(OBJ_REACTOR)

q7server: $(OBJ_Q7SERVER) $(OBJ_Q3)
	$(CC) $(CFLAGS) -o q7server $(OBJ_Q7SERVER) $(OBJ_Q3)

q9server: $(OBJ_Q9SERVER) $(OBJ_Q3) $(OBJ_REACTOR)
	$(CC) $(CFLAGS) -o q9server $(OBJ_Q9SERVER) $(OBJ_Q3) $(OBJ_REACTOR)

client: $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o client $(OBJ_CLIENT)

clean:
	rm -f *.o Q1 q2 q4server q7server q6server q9server client
