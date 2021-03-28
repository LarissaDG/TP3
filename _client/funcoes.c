#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <math.h>
#include "funcoes.h"

void logexit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void imprime_pacote_UDP(udp_file_msg pacote){
    printf("Msg id: %u\n",pacote.msg_id);
    printf("Pacote id: %u\n",pacote.num_sequencia);
    printf("Payload size: %u\n",pacote.payload_size);
    printf("payload: %s\n",pacote.payload);
    printf("\n\n");
}

void imprime_dados_arquivo(file_metadata arquivo){
	printf("Nome do arquivo: %s\n", arquivo.nome_arquivo);
    printf("Tamanho do arquivo: %ld\n", arquivo.tamanho_arquivo);
    printf("Numero de pacotes: %d\n",arquivo.num_pacotes);
    
}

void imprime_mini_msg(mini_msg mail){
    printf("ID: %u\nnum_chunks: %u\n", mail.msg_id,mail.num_chunks);
    printf("Chunks %s\n", mail.chunk);
    //ImprimeLista(mail.chunks);
}

void imprime_response(response msg){
    printf("ID: %u\nchunks_id: %u\n", msg.msg_id,msg.chunks_id);
    printf("Chunks %u\n", msg.chunk_size);
}

void imprime_query(query pergunta){
	printf("ID: %u\n", pergunta.msg_id);
	printf("IP: %s\n",inet_ntoa(pergunta.ip));
	printf("Porto: %i\n",ntohs(pergunta.port));
	printf("Peer_TTL: %d\n",pergunta.peer_TTL);
	printf("num_chunks: %d\n",pergunta.num_chunks);
    printf("Chunks %s\n", pergunta.chunk);
}


int get_tam_arquivo(char nomeArquivo[], int tam){
	FILE *arquivo = fopen(nomeArquivo, "r");
	if (arquivo != NULL) {
		fseek(arquivo, 0, SEEK_END);
		int posicao = ftell(arquivo);
		fclose(arquivo);
		return posicao;
	}
	else{
		logexit("Arquivo inexistente");
		return -1;
	}
}

int get_num_pacotes(long tamanho_arquivo){

	int num_pacotes;
	float aux;

	//Vê o número de pacotes
	aux = (float)tamanho_arquivo/(float)BUFSZ;
	num_pacotes = ceil(aux);
	
	printf("**tamanho_arquivo** %ld\n", tamanho_arquivo);
	printf("**aux %f = %ld/%d ** \n", aux,tamanho_arquivo,BUFSZ);
	printf("**Numero de pacotes %d **\n",num_pacotes);

	return num_pacotes;

}

void segmenta_arquivo(file_metadata dadosArquivo, udp_file_msg *vetorPacotes){
	int i;
	int tam;
	//int count = 0;

	//Abre o arquivo
	FILE *arquivo = fopen(dadosArquivo.nome_arquivo,"rb");
	if(arquivo == NULL ){
		logexit("Erro na abertura do arquivo");
	}

	//Coloco o ponteiro no inicio do arquivo:
	fseek(arquivo,0,SEEK_SET);

	tam = dadosArquivo.tamanho_arquivo;

	//Lê 1000 bytes
	for(i=0;i<dadosArquivo.num_pacotes;i++){
		vetorPacotes[i].msg_id = 6;
		vetorPacotes[i].num_sequencia = i;
		memset(&vetorPacotes[i].payload,0,sizeof(vetorPacotes[i].payload));
		fread(vetorPacotes[i].payload,sizeof(vetorPacotes[i].payload),1,arquivo);
		if(tam > BUFSZ){
			tam = tam - BUFSZ;
			vetorPacotes[i].payload_size = BUFSZ;
			//printf("%d) TAM: %d\n",count,tam);
		}
		else{
			vetorPacotes[i].payload_size = tam;
			//printf("%d) TAM: %d\n",count,tam);
		}
		//printf("TAM: %d\n",tam);
		//count ++;
	}
	//Imprime
	/*for(i=0;i<dadosArquivo.num_pacotes;i++){
		imprime_pacote_UDP(vetorPacotes[i]);	
	}*/

	fclose(arquivo);
}

void renomeia_arquivo(char* nome, char* aux){
    int i,pos=0;

    for(i=0;i<strlen(nome);i++){
        if(nome[i] != '.'){
            aux[i] = nome[i];
        }
        else{
            aux[i] = '2';
            pos = i;//Posição do ponto
            break;
        }
    }
    for(i=pos;i<=strlen(nome);i++){
        aux[pos+1]=nome[i];
        pos++;
    }

    aux[i] = '\0';
    //printf("Novo nome do arquivo %s\n", aux);
}

void monta_arquivo(file_metadata dadosArquivo, udp_file_msg *vetorPacotes){
    FILE * arquivo;
    int i;
    ///char str[] = "Novo pacote\n";
    char novoNome[20];

    renomeia_arquivo(dadosArquivo.nome_arquivo,novoNome);

    if((arquivo = fopen(novoNome,"wb")) == NULL){
        logexit("Falha na criacao do arquivo");
    }

    //printf("num_pacotes %d\n",dadosArquivo.num_pacotes);

    for(i=0;i<dadosArquivo.num_pacotes;i++){
    	//fwrite(str , 1 , strlen(str) ,arquivo);
        fwrite(vetorPacotes[i].payload,vetorPacotes[i].payload_size,1,arquivo);
    }
    fclose(arquivo);
    printf(" Montou arquivo\n");
}

void split_string(char* string,char* palavra, char* palavra2){
    // Extract the first token
    char* token1 = strtok(string, ":");
    char* token2 = strtok (NULL, " ");
    strcpy(palavra,token1);
    strcpy(palavra2,token2);
}

int processa_peer_chunk(file_metadata dadosArquivo, chunk_data *listChunks){
	char lixo;
	int i=0,count=0;

	FILE *arquivo = fopen(dadosArquivo.nome_arquivo,"rb");
	if(arquivo == NULL ){
		logexit("Erro na abertura do arquivo");
	}

	//Coloco o ponteiro no inicio do arquivo:
	fseek(arquivo,0,SEEK_SET);

	//Enquanto não for fim de arquivo eu vou lendo os chunks e coloco na respectiva struct

	while(!feof(arquivo)){
		//printf("EStou aki, count= %d, i = %d\n", count, i);
		//fread(listChunks[i].string,sizeof(listChunks[i].string),1,arquivo);
		fscanf(arquivo,"%d",&listChunks[i].chunks_id);
		fscanf(arquivo,"%c",&lixo);
		fscanf(arquivo,"%c",&lixo);
		fscanf(arquivo,"%s",listChunks[i].string);
		//listChunks[i].chunks_id = atoi(listChunks[i].string);
		count++;
		i++;
	}

	//printf("EStou aki, count= %d, i = %d\n", count, i);
	/*for(i=0;i<count;i++){
		printf("%d: %s\n",listChunks[i].chunks_id,listChunks[i].string);
	}*/

	fclose(arquivo);
	return count;//O numero de chunks lidos
}

void tostring(chunk_data *listChunks, int tam, char *str_chunks,int param, status *vetor){
	char num_str[TAM];
	memset(&num_str,0,sizeof(num_str));
	char *aux;
	int i,pos,max;

	if(param == 2){
		max = listChunks[0].chunks_id;
		for(i=0;i<tam;i++){
			if(listChunks[i].chunks_id > max){
				max = listChunks[i].chunks_id;
			}
			if(listChunks[i].chunks_id >= 2){
				if(vetor[i].solicitado == 1){
					sprintf(num_str, "%d", i);
					strcat(str_chunks,num_str);
					strcat(str_chunks,",");
					vetor[i].solicitado++;
				}
			}
		}	
		if(max >= 2){	
			aux = strrchr(str_chunks,',');
			pos = aux-str_chunks;
			str_chunks[pos] = '\0';
		}
		else{
			return;
		}
		
	}

	else{
		for(i=0;i<tam;i++){
			sprintf(num_str, "%d", listChunks[i].chunks_id);
			strcat(str_chunks,num_str);
			if(i != tam-1){
				strcat(str_chunks,",");
			}
		}
	}
	//printf("OLHA aki %s\n",str_chunks);
}

void processa_chunk_info(char *chunk, char * str,char *resposta, status* vetor){
	int i=0;
	chunk_data aux[11];
	int num;//11 pq os chunks são numerados de 1 a 10 -não uso a posicao 0
	memset(&aux,0,sizeof(aux));

	//Verifica quais caracteres do cliente aparecem no servidor(ignorando as virgulas)
	while(str[i] != '\0'){
		if(isdigit(str[i])){
			num = str[i]-'0';
			aux[num].chunks_id ++; //Estou usando esse campo para contabilizar o numero de ocorrencias de um numero;
		}
		i++;
	}
	i=0;
	while(chunk[i] != '\0'){
		if(isdigit(chunk[i])){
			num = chunk[i]-'0';
			aux[num].chunks_id ++; //Estou usando esse campo para contabilizar o numero de ocorrencias de um numero;
		}
		i++;
	}
	tostring(aux,11,resposta,2,vetor);
	
	
	/*for(i=0;i<11;i++){
		printf("%d-%d\n",i,aux[i].chunks_id);
	}
	printf("resposta %s\n", resposta);
	if(strlen(resposta)==0){
		printf("resposta NULL\n");	
	}*/
}

void processa_get(){
	//pega a lista de chunks tira a virgula
	//Cria um vetor de status
	//enquanto não for o fim do vetor de status manda o trem
}

int processa_cliente(char* string,status *vetor){
	int i=0,val=0;

	while(string[i] != '\0'){
		if(isdigit(string[i])){
			vetor[string[i]-'0'].solicitado = 1;
			val++;
		}
		i++;
	}
	return val;
}

int busca_peer(peer_data *peer,int tam,struct in_addr ip,unsigned short port){
	int i=0;
	char pa[NUM];
	char pb[NUM];
	strcpy(pb,inet_ntoa(ip));

	for(i=0;i<tam;i++){
		strcpy(pa,inet_ntoa(peer[i].ip));
		if(strcmp(pa,pb)){
			if(peer[i].port == port){
				return 1;
			}
		}
	}
	return 0;
}

void processa_response(struct in_addr ip,unsigned short port, response msg, status *vetor){
	char aux[VAR];
	char aux2[VAR];

	strcpy(aux,"output-");
	strcpy(aux2,inet_ntoa(ip));
	strcat(aux,aux2);
	strcat(aux,".log");

	printf("%s\n",aux);
	
	//Abrir o arquivo
	FILE *arquivo = fopen(aux,"a");
	if(arquivo == NULL ){
		logexit("Erro na abertura do arquivo");
	}

	//Escreve no arquivo
	fprintf(arquivo,"%s:%d - %d\n",inet_ntoa(ip),ntohs(port),msg.chunks_id);
	vetor[msg.chunks_id].recebido++;

	fclose(arquivo);

	//salvando o arquivo
	char token1[VAR],token2[VAR];
	strcpy(token1,"BigBuckBunny_");
    sprintf(token2, "%d",msg.chunks_id);
    strcat(token1,token2);
    strcat(token1,".m4s");

	FILE *arq = fopen(token1,"w");
	if(arquivo == NULL ){
		logexit("Erro na abertura do arquivo");
	}

    fwrite(msg.payload,msg.chunk_size,1,arq);
    fclose(arq);
}

void addLogFaltante(struct in_addr ip,unsigned short port,status *vetor){
	printf("DEntro fun 3\n");
	int i;
	char aux[VAR];
	char aux2[VAR];

	strcpy(aux,"output-");
	strcpy(aux2,inet_ntoa(ip));
	strcat(aux,aux2);
	strcat(aux,".log");

	printf("%s\n",aux);
	
	//Abrir o arquivo
	FILE *arquivo = fopen(aux,"a");
	if(arquivo == NULL ){
		logexit("Erro na abertura do arquivo");
	}

	//Escreve no arquivo
	for(i=0;i<VAR;i++){
		if(vetor[i].solicitado >= 1 && vetor[i].recebido == 0 ){
			fprintf(arquivo,"0:0 - %d\n",i);
			vetor[i].recebido++;
		}
	}

	fclose(arquivo);
}

int jaRecebiTudo(status *vetor,int numChunksSolicitados){
	printf("DEntro fun 2\n");
	int i,count=0;

	for(i=0;i<NUM;i++){
		if((vetor[i].solicitado == 2) && vetor[i].recebido == 1){
			count ++;
		}
	}
	//printf("count  = %d\n",count);
	//printf("numChunksSolicitados  = %d\n",numChunksSolicitados);
	if(numChunksSolicitados == count){
		return 1;
	}
	return 0;
}

int VerificaIP(struct in_addr ip,unsigned short port,vizinhos viz){
	char aux[VAR];
	strcpy(aux,inet_ntoa(ip));
	if(!strcmp(aux,viz.string)){
		if(ntohs(port) == viz.port){
			return 1; //Se ip e portas iguais retorna 1
		}
	}
	return 0;
}

int verificaSolicitadosIP(peer_data *peer){
	int numChunksSolicitados = 0,i;
	for(i=0;i<VAR;i++){
        if(peer[i].Chunk->solicitado >= 1){
            numChunksSolicitados ++;
        }
    }
    return numChunksSolicitados;
}

int countSolicitadosEntrada(status *controle){
	int numChunksSolicitados = 0,i;
	for(i=0;i<NUM;i++){
        if(controle[i].solicitado >= 1){
        	numChunksSolicitados++;
        }
    }
    return numChunksSolicitados;
}

void finaliza(peer_data *peer,status *controle){
	printf("DEntro fun 1\n");
	int i,num;

    num = verificaSolicitadosIP(peer); // Se perdeu
    if(num == 0){
    	//Eh porque não tinha
    	num = countSolicitadosEntrada(controle);
    	addLogFaltante(peer[0].ip,peer[0].port,controle);
    }
	//printf("numChunksSolicitados %d\n",num);
    else{


	    for(i=0;i<VAR;i++){
	        if(!jaRecebiTudo(peer[i].Chunk,num)){
	            //Adicionar no arquivo outputlogIP os dados como 0;
	            addLogFaltante(peer[i].ip,peer[i].port,peer[i].Chunk);
	        }
	    }
	}
}

void logexitEspecial(const char *msg,peer_data *peer,status *controle) {
	if(errno == EAGAIN || errno == EWOULDBLOCK){
		printf("Finalizando ... \n");
		finaliza(peer,controle);
    }
    perror(msg);
    exit(EXIT_FAILURE);
}

/*if (recvfrom(socket_desc, &mail, sizeof(mail), 0,
         (struct sockaddr*)&client_addr, &client_struct_length) < 0){
        logexit("receive");
    }
*/ 

/*void processa_mensagem(){
	if(recvfrom((socket_desc, &mail, sizeof(mail), 
		MSG_PEEK, (struct sockaddr*)&client_addr, &client_struct_length) < 0)){
		logexit("receive");
	}

}*/