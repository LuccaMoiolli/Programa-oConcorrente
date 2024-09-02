#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<string.h>

#define CAMINHOES 10
#define CLIENTES 100
#define ESTOQUE_MAX 100
#define CIDADES 5
#define ESPACO_CAMINHAO 50



pthread_mutex_t lock_dia = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_pedidos = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cliente = PTHREAD_COND_INITIALIZER;

int ENVIADO = -1, CANCELADO = -2;
int hoje = 0, num_pedidos = 0; //dia atual e tamanho do vetor de pedidos
int** pedidos; //vetor que guarda os pedidos ativos


//desaloca um pedido, deve ser chamada em lock_pedidos
void desalocar_pedido(int pedido){
    num_pedidos--;
    for(int i = pedido; i < num_pedidos; i++)
        pedidos[i] = pedidos[i+1];
    pedidos = (int**)realloc(pedidos, num_pedidos * sizeof(int*));
}

//funcao que simula o comportamento da loja
void* loja(){
    int enviar[CIDADES][2]; //vetor de prioridade das cidades que devem receber envios

    while(1){
        //olhar pedidos e mandar
        memset(enviar, 0, CIDADES*sizeof(int)*2);
        pthread_mutex_lock(&lock_dia);
            int local_hoje = hoje;
        pthread_mutex_unlock(&lock_dia);
        pthread_mutex_lock(&lock_pedidos);
            for(int i = 0; i < num_pedidos; i++){
                enviar[pedidos[i][1]][0]+= pedidos[i][2];
                enviar[pedidos[i][1]][1] = min(pedidos[i][0], enviar[pedidos[i][1]][1]);
            }   
            
        pthread_mutex_lock(&lock_pedidos);



        //repor estoque

        //acordar threads e atualizar o dia

    }
}

//funcao que simula o comportamento de um caminhao
void* caminhao(){


}

//funcao que simula o comportamento de um cliente
void* cliente(void* cidade_p){
    int cidade = *(int*)cidade_p;
    int prox_pedido;// data do proximo pedido do cliente

    //criando epaço para fazer pedido

    while(1){    
        //verificando se deve fazer um pedido hoje
        pthread_mutex_lock(&lock_dia);
            prox_pedido = hoje + rand()%10;
            while(hoje < prox_pedido)
                pthread_cond_wait(&cond_cliente, &lock_dia);
        pthread_mutex_unlock(&lock_dia);

        //escolhendo o que pedir e efetuando o pedido
        pthread_mutex_lock(&lock_pedidos);
            int meu_pedido = num_pedidos;
            pedidos = (int**)realloc(pedidos, ++num_pedidos * sizeof(int*));
            int *itens = (int*)malloc(3*sizeof(int)); // dentro do lock por garantia
            itens[0] = prox_pedido;
            itens[1] = cidade;
            itens[2]+= rand()%ESTOQUE_MAX/10;
            
            pedidos[meu_pedido] = itens;

            //esperando o pedido
            while(*itens != ENVIADO){

                pthread_cond_wait(&cond_cliente, &lock_pedidos);

                int tempo;
                pthread_mutex_lock(&lock_dia);
                    tempo = hoje - prox_pedido;
                pthread_mutex_unlock(&lock_dia);

                if(rand()%100 < tempo){
                    itens = (int*)realloc(itens, sizeof(int));
                    desalocar_pedido(meu_pedido);
                    *itens = CANCELADO;
                }
            }
        pthread_mutex_unlock(&lock_pedidos);
    
    }
}


int main(){


    pthread_t caminhoes[CAMINHOES];
    pthread_t clientes[CLIENTES];
    pthread_t Loja;

    pthread_create(&Loja, NULL, loja, NULL);

    for(int i = 0; i < CAMINHOES; i++){
        pthread_create(&caminhoes[i], NULL, caminhao, NULL);
    }

    for(int i = 0; i < CLIENTES; i++){
        int cidade = i%CIDADES;
        pthread_create(&clientes[i], NULL, cliente, (void*)&cidade);
    }

    for(int i = 0; i < CAMINHOES; i++){
        if(pthread_join(caminhoes[i], NULL)){
            printf("\nerro ao juntar threads!\n");
            exit(1);
        }
    }

    for(int i = 0; i < CLIENTES; i++){
        if(pthread_join(clientes[i], NULL)){
            printf("\nerro ao juntar threads!\n");
            exit(1);
        }
    }
    
    if(pthread_join(Loja, NULL)){
        printf("\nerro ao juntar threads!\n");
        exit(1);
    }

    return 0;
}
