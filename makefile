
all: parque  gerador

parque: parque.c common.h
	gcc parque.c -Wall -lpthread -o bin/parque

gerador: gerador.c common.h
	gcc gerador.c -Wall -lpthread -o bin/gerador
