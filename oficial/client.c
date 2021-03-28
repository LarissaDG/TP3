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
    printf("usage: %s <file>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511 arquivo.doc\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
    //Declaração de variáveis

    struct timeval timeout;    

    int flag=1,peer_ctrl=0,numChunksSolicitados;

    mini_msg mail;
    response msg;
    peer_data peer[VAR];
    status controle [NUM];
    char token1[NUM],token2[NUM];
    char resposta[TAM];
    
    int socket_desc;
    struct sockaddr_in server_addr;
    socklen_t server_struct_length = sizeof(server_addr);

    //Processa a entrada
    if (argc < 3) {
        usage(argc, argv);
    }

    //--------PROCESSAMENTO DE ENTRADA E INICIALIZAÇÕES --------------------
    memset(&mail,0,sizeof(mail));
    memset(&msg,0,sizeof(msg));
    memset(&peer,0,sizeof(peer));
    memset(&controle,0,sizeof(controle));
    memset(&resposta,0,sizeof(resposta));

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    split_string(argv[1],token1,token2);
    numChunksSolicitados = processa_cliente(argv[2],controle);
    //------------------------------------------------------------------------

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        logexit("socket");
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
    server_addr.sin_port = htons(atoi(token2));
    server_addr.sin_addr.s_addr = inet_addr(token1);


    //--------TROCA DE MENSAGENS ----------------------------------------------
    //Mando o Hello
    mail.msg_id = 1;
    mail.num_chunks = strlen(argv[2]);
    strcpy(mail.chunk,argv[2]);

    //Printo os dados antes de enviar
    printf("Enviando HELLO: \n");
    imprime_mini_msg(mail);
    printf("\n\n");

    if(sendto(socket_desc, &mail, sizeof(mail), 0,
         (struct sockaddr*)&server_addr, server_struct_length) < 0){

        logexitEspecial("send",peer,controle);
    }

    //Recebo uma mensagem
    //verifico o seu msg_id
    //a partir disso processo a mensagem de acordo
    while(flag){
        if(recvfrom(socket_desc, &mail, sizeof(mail),MSG_PEEK,
             (struct sockaddr*)&server_addr, &server_struct_length) < 0){
            logexitEspecial("recv",peer,controle);
        }
        //Printo os dados coletados da mensagem
        printf("Received message from IP: %s and port: %i\n",
           inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
        printf("Mini msg %u \n\n",mail.msg_id);

        switch(mail.msg_id){
            case 3:
            //recebe o chunk info
            if(recvfrom(socket_desc, &mail, sizeof(mail),0,
                 (struct sockaddr*)&server_addr, &server_struct_length) < 0){
                logexitEspecial("recv",peer,controle);
            }
            printf("Recebo CHUNK_INFO: \n");
            imprime_mini_msg(mail);
            printf("\n\n");
            //Processa o chunk info
            printf("Processando..... CHUNK_INFO: \n");

            //Vê os matchs
            processa_chunk_info(mail.chunk, argv[2],resposta,controle);
            
            //Monta mensagem get
            memset(&mail,0,sizeof(mail));
            mail.msg_id = 4;
            mail.num_chunks = strlen(resposta);
            strcpy(mail.chunk,resposta);

            //Procura se já mandei Get por peer
            //Se sim não manda de novo
            if(!busca_peer(peer,peer_ctrl,server_addr.sin_addr,ntohs(server_addr.sin_port))){
                //Se não mandei
                //Registra o peer que eu to mandando
                peer[peer_ctrl].ip = server_addr.sin_addr;
                peer[peer_ctrl].port = ntohs(server_addr.sin_port);
                peer[peer_ctrl].jaEnviouGet = 1;
                //Registro os chunks que eu to pedindo
                processa_cliente(resposta,peer[peer_ctrl].Chunk);
                //processa_cliente(resposta,controle);
                peer_ctrl ++;

                printf("Mando GET: \n");
                imprime_mini_msg(mail);
                printf("\n\n");

                if(sendto(socket_desc, &mail, sizeof(mail), 0,
                     (struct sockaddr*)&server_addr, server_struct_length) < 0){
                    logexitEspecial("send",peer,controle);
                }
            }
            break;
            
            case 5:
            //recebe a response
            if(recvfrom(socket_desc, &msg, sizeof(msg),0,
                (struct sockaddr*)&server_addr, &server_struct_length) < 0){
                logexitEspecial("recv",peer,controle);
            }

            imprime_response(msg);
            printf("\n\n");

            //Trata o response
            //Atualiza o arquivo log
            //Faz o controle das respostas recebidas na estrutura peer
            processa_response(server_addr.sin_addr,server_addr.sin_port,msg,controle);

            if(jaRecebiTudo(controle,numChunksSolicitados)){
                flag=0;
            }            
            break;
            
            default:
            logexit("msg_id invalido");
        }

    }

    //--------FINALIZAÇÕES ---------------------------------------
    printf("Saio do while\n");

    /*for(i=0;i<NUM;i++){
         printf("%d) Solicitado: %d Recebido %d\n",i,controle[i].solicitado,controle[i].recebido);
    }*/

    // Close the socket:
    close(socket_desc);
    
    return 0;
}
