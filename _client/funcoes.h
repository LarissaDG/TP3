#include <stdio.h>
#include <stdlib.h>
#include "mensagens.h"

void logexit(const char *msg);

void imprime_pacote_UDP(udp_file_msg pacote);

void imprime_dados_arquivo(file_metadata arquivo);

void imprime_mini_msg(mini_msg mail);

void imprime_response(response msg);

void imprime_query(query pergunta);

int get_tam_arquivo(char nomeArquivo[], int tam);

int get_num_pacotes(long tamanho_arquivo);

void segmenta_arquivo(file_metadata dadosArquivo, udp_file_msg *vetorPacotes);

void renomeia_arquivo(char* nome, char* aux);

void monta_arquivo(file_metadata dadosArquivo, udp_file_msg *vetorPacotes);

void split_string(char* string,char* palavra, char* palavra2);

int processa_peer_chunk(file_metadata dadosArquivo, chunk_data *listChunks);

void tostring(chunk_data *listChunks, int tam, char *str_chunks,int param,status *vetor);

void processa_chunk_info(char *chunk, char * str,char *resposta,status *vetor);

int processa_cliente(char* string,status *vetor);

int busca_peer(peer_data *peer,int tam,struct in_addr ip,unsigned short port);

void processa_response(struct in_addr ip,unsigned short port, response msg, status *vetor);

int jaRecebiTudo(status *vetor,int numChunksSolicitados);

int VerificaIP(struct in_addr ip,unsigned short port,vizinhos viz);

void addLogFaltante(struct in_addr ip,unsigned short port,status *vetor);