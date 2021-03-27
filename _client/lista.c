#include "lista.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void CriaListaVazia(Lista* lista){
    lista->primeiro = (apontador)malloc(sizeof(celula));
    lista->tam = 0;
    lista->primeiro->prox = NULL;
}

int VerificaVazia(Lista lista){
    return(lista.tam == 0);
}

int GetTamanho(Lista lista){
    return lista.tam;
}

int BuscaItem(Lista lista, int item){
    apontador aux;
    aux = lista.primeiro;
    while(aux->prox != NULL){
        aux = aux->prox;
        if(aux->data == item ) return 1;
    }
    return 0;
}

//Caso ja esteja na lista, nao faz nada
void InsereFinal(Lista* lista, int item){
    apontador aux;
    aux = lista->primeiro;
    while(aux->prox != NULL){
        aux = aux->prox;
        if(aux->data == item ) return ;
    }

    apontador novo = (apontador)malloc(sizeof(celula));
    novo->data = item;
    novo->prox = NULL;

    aux->prox = novo;
    lista->tam++;
}

void RemoveItem(Lista* lista, int item){
    apontador aux;
    aux = lista->primeiro;
    while(aux->prox != NULL){
        if(aux->prox->data == item){
            apontador prox = aux->prox->prox;
            DesalocaCelula(aux->prox);
            aux->prox = prox;
            lista->tam--;
        }
        aux = aux->prox;
        if(aux == NULL) break;
    }
}

void DesalocaLista(Lista *lista){
    apontador aux, aux2;
    aux = lista->primeiro;
    while(aux != NULL){
        aux2 = aux->prox;
        DesalocaCelula(aux);
        aux = aux2;
    }   
}

void ImprimeLista(Lista lista){
    apontador aux;
    aux = lista.primeiro->prox;
    printf("Inicio lista\n");
    while(aux != NULL){
        printf("%d\n",aux->data);
        aux = aux->prox;
    }
    printf("Fim lista\n");
}



// Celula
void DesalocaCelula(celula* celula){
    free(celula);
}