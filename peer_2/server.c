#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "funcoes.h"

void usage(int argc, char **argv) {
    printf("usage: %s <ip-local>:<local-port> <key-values-files_peer[id]> <ip1:port1> ... <ipN:portN>\n", argv[0]);
    printf("example: %s 10.0.0.a:5001 key-values-files_peer1 ip_peer3:5003 ip_peer2:5002\n", argv[0]);
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv){
    //Declaração de variáveis

    struct timeval timeout;      

    int i=0,j=0,tam=0,flag=1;
    vizinhos *nei_peers;

    mini_msg mail;
    response msg;
    query pergunta;
    status controle[NUM];

    chunk_data listChunks[VAR];//Armazena os chunk ids do peer
    char token1[VAR],token2[VAR];
    char palavra1[NUM],palavra2[NUM];
    char str_chunks[TAM];
    int count=0;

    file_metadata arquivo;
    int resposta;

    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_struct_length = sizeof(client_addr);


    //--------PROCESSAMENTO DE ENTRADA E INICIALIZAÇÕES --------------------
    //Inicializa msg
    memset(&str_chunks,0,sizeof(str_chunks));
    memset(&arquivo,0,sizeof(arquivo));
    memset(&pergunta,0,sizeof(pergunta));
    memset(&resposta,0,sizeof(resposta));
    memset(&mail,0,sizeof(mail));
    memset(&msg,0,sizeof(msg));

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    //Tratando os parametros recebidos
    split_string(argv[1],palavra1,palavra2); //Processa argv1s
    strcpy(arquivo.nome_arquivo,argv[2]);
    count = processa_peer_chunk(arquivo,listChunks);//Processa arquivo argv2
    tostring(listChunks,count,str_chunks,0,controle);
    //printf("OLHA aki %s\n",str_chunks);
    //printf("Count %d\n",count);

    tam = argc-3;

    nei_peers = (vizinhos*) calloc(tam,sizeof(vizinhos));

    for(i=3;i<argc;i++){
        split_string(argv[i],token1,token2);

        strcpy(nei_peers[j].string, token1);
        nei_peers[j].port = atoi(token2);

        printf("Porta: %u\n",nei_peers[j].port);
        printf("Vizinho: %s\n",nei_peers[j].string);
        j++;
    }
    //------------------------------------------------------------------------
    
    // Create UDP socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        logexit("socket");
        return -1;
    }

    //Seta opções do socket/ timeout para o recv e o send
    if (setsockopt (socket_desc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
        sizeof(timeout)) < 0){
        logexit("setsockopt failed\n");
    }

    if (setsockopt (socket_desc, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
        sizeof(timeout)) < 0){
        logexit("setsockopt failed\n");
    }

    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(palavra2));
    server_addr.sin_addr.s_addr = inet_addr(palavra1);

    int enable = 1;
    if (0 != setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        logexit("bind"); 
    }
    
    printf("Listening for incoming messages...\n\n");

    //--------TROCA DE MENSAGENS ----------------------------------------------
    while(flag){
        //Recebo uma mensagem
        //verifico o seu msg_id
        //a partir disso processo a mensagem de acordo
        if (recvfrom(socket_desc, &mail, sizeof(mail), MSG_PEEK,
             (struct sockaddr*)&client_addr, &client_struct_length) < 0){
            logexit("receive");
        }

        //Printo os dados coletados da mensagem
        printf("Received message from IP: %s and port: %i\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("Mini msg %u \n",mail.msg_id);

        //A partir do que eu recebi faço um processamento diferente
        switch(mail.msg_id){
            case 1:
            //Recebe o HEllo
            if (recvfrom(socket_desc, &mail, sizeof(mail), 0,
                 (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                logexit("receive");
            }
            
            printf("Recebo HELLO: \n");
            imprime_mini_msg(mail);
            printf("\n\n");

            //Manda o chunk_info
            //Não faz processamentos. Só fala quais chunks o peer tem
            mail.msg_id = 3;
            mail.num_chunks = strlen(str_chunks);
            strcpy(mail.chunk,str_chunks);

            printf("Envio CHUNK_INFO: \n");
            imprime_mini_msg(mail);
            printf("\n\n");

            if (sendto(socket_desc, &mail, sizeof(mail), 0,
                (struct sockaddr*)&client_addr, client_struct_length) < 0){
                logexit("send");
            }

            //1)caso: Ele é o primeiro a mandar a mensagem de Query:
            //Monta a mensagem
            pergunta.msg_id = 2;
            pergunta.ip = client_addr.sin_addr;
            pergunta.port = client_addr.sin_port;
            pergunta.peer_TTL = TTL;
            pergunta.num_chunks = strlen(str_chunks);
            strcpy(pergunta.chunk,str_chunks);

            //Imprime a mensagem
            printf("Mando a query:\n");
            imprime_query(pergunta);
            printf("\n\n");

            //Manda para os vizinhos
            for(i=0;i<tam;i++){
                client_addr.sin_addr.s_addr = inet_addr(nei_peers[i].string);
                client_addr.sin_port = htons(nei_peers[i].port);
                if (sendto(socket_desc, &pergunta, sizeof(pergunta), 0,
                    (struct sockaddr*)&client_addr, client_struct_length) < 0){
                    logexit("send");
                }
            }
            break;
            
            case 2:
            //Recebe a query
            if (recvfrom(socket_desc, &pergunta, sizeof(pergunta), 0,
                 (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                logexit("receive");
            }
            //Decrementa o TTL
            pergunta.peer_TTL --;
            if(pergunta.peer_TTL > 0){
                //Manda para todos os peers menos o que recebeu
                for(i=0;i<tam;i++){
                    client_addr.sin_addr.s_addr = inet_addr(nei_peers[i].string);
                    client_addr.sin_port = htons(nei_peers[i].port);
                    if(!VerificaIP(client_addr.sin_addr,client_addr.sin_port,nei_peers[i])){
                        if (sendto(socket_desc, &pergunta, sizeof(pergunta), 0,
                            (struct sockaddr*)&client_addr, client_struct_length) < 0){
                            logexit("send");
                        }
                    }
                }
            }

            break;

            case 4:
            if (recvfrom(socket_desc, &mail, sizeof(mail), 0,
                 (struct sockaddr*)&client_addr, &client_struct_length) < 0){
                logexit("receive");
            }

            //Recebe o get 
            printf("Recebo Get: \n");
            imprime_mini_msg(mail);
            printf("\n\n");

            //Procesa o get
        
            processa_cliente(mail.chunk,controle);
            for(i=0;i<NUM;i++){
                if(controle[i].solicitado == 1){
                    //Monta a response
                    msg.msg_id = 5;
                    msg.chunks_id = i;
                    strcpy(token1,"BigBuckBunny_");
                    sprintf(token2, "%d", i);
                    strcat(token1,token2);
                    strcat(token1,".m4s");
                    printf("NOME ARQUIVO %s\n",token1);
                    msg.chunk_size = get_tam_arquivo(token1,strlen(token1));
                    FILE *arquivo = fopen(token1,"rb");
                    if(arquivo == NULL ){
                        logexit("Erro na abertura do arquivo");
                    }

                    fseek(arquivo,0,SEEK_SET);

                    fread(msg.payload,sizeof(msg.payload),1,arquivo);
                    fclose(arquivo);
                    imprime_response(msg);
                    printf("\n\n");

                    //Manda a response
                    if (sendto(socket_desc, &msg, sizeof(msg), 0,
                        (struct sockaddr*)&client_addr, client_struct_length) < 0){
                        logexit("send");
                    }
                }
            }
            //flag=0;
            break;
            
            default:
            logexit("msg_id invalido");
        }
        
    }

    //--------FINALIZAÇÕES ---------------------------------------
    printf("Saio do while\n");

    // Close the socket:
    close(socket_desc);
    free(nei_peers);
    
    return 0;
}