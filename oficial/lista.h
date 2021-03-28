#define TAGSZ 500

typedef struct celula *apontador;

typedef struct celula{
  int data;
  apontador prox;
}celula;

typedef struct Lista{
  apontador primeiro;
  int tam;
}Lista;


void CriaListaVazia(Lista* lista);
int VerificaVazia(Lista lista);
int GetTamanho(Lista lista);
void InsereFinal(Lista* lista, int item);
void ImprimeLista(Lista lista);
void RemoveItem(Lista* lista, int item);
void DesalocaLista(Lista* lista);
int BuscaItem(Lista lista, int item);

void DesalocaCelula(celula*);

