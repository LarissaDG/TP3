#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h> 
#include "lista.h"

#define BUFSZ 1024
#define VAR 30
#define TAM 20
#define NUM 11
#define TTL 3
#define TIMEOUT 5

//1)Hello/3)Chunk info/4)Get/
typedef struct{
    unsigned short int msg_id;
    unsigned short int num_chunks;
    char chunk[TAM];
}mini_msg;


//2) Query
typedef struct{
    unsigned short int msg_id;
    struct in_addr ip;
    unsigned short port;
    unsigned short peer_TTL;
    unsigned short int num_chunks;
    char chunk[TAM];
}query;

//5) Response
typedef struct{
    unsigned short int msg_id;
    unsigned short int chunks_id;
    unsigned short int chunk_size;
    char payload [BUFSZ];
}response;

typedef struct{
    int chunks_id;
    char string[TAM];
}chunk_data;

typedef struct{
    unsigned short port;
    char string[TAM];
}vizinhos;

typedef struct{
    int solicitado;
    int recebido;
}status;

typedef struct{
    struct in_addr ip;
    unsigned short port;
    int jaEnviouGet;
    status Chunk[NUM];
}peer_data;

//UDP
//Vulgo pacote
typedef struct {
    unsigned short int msg_id;
    unsigned num_sequencia;
    unsigned short int payload_size;
    char payload [BUFSZ];
} udp_file_msg;

typedef struct{
    char nome_arquivo [TAM];
    long tamanho_arquivo;
    int num_pacotes;
}file_metadata;
