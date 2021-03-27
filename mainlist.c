#include "lista.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char **argv){
	Lista teste;
	int i=0;

	CriaListaVazia(&teste);
	for(i=0;i<5;i++){
		InsereFinal(&teste,i);
	}

	ImprimeLista(teste);

	RemoveItem(&teste, 3);

	ImprimeLista(teste);

	RemoveItem(&teste, 7);

	ImprimeLista(teste);

	DesalocaLista(&teste);


	return 0;
}