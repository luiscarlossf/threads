/*
*UNIVERVISADE FEDERAL DO PIAUÍ
*Curso Bacharelado em Ciência da Computação
*Disciplina: Sistemas Operacionais
*Luis Carlos da Silva Filho 
*Objetivo: Implementação do problema do jantar dos filósofos usando threads e semáforos.
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define N        5             /*numero de filosofos*/
#define LEFT     (i+N-1)%N     /*numero do vizinho a esquerda de i*/
#define RIGHT    (i+1)%N       /*numero do vizinho a direita de i*/
#define THINKING 0             /*o filosofo esta pensando*/
#define HUNGRY   1             /*o filosofo esta tentando pegar garfos*/
#define EATING   2             /*o filosofo esta comendo*/
#define TRUE 1
#define TEMPO_PENSANDO 6       /*tempo do filosofo pensando em segundos*/
#define TEMPO_COMENDO 15       /*tempo do filoso comendo em segundos */


typedef pthread_mutex_t semaphore;        /*semaforos sao um tipo mutex da biblioteca pthread*/
int state[N];                             /*arranjo para controlar o estado de cada um*/
pthread_mutex_t mutex;                    /*exclusao mutua para as regioes criticas*/
pthread_mutex_t s[N];                     /*um semaforo por filosofo*/


/*Estrutura auxilia na demonstração de concorrência
 no momento em que tem dois filósofos comendo*/
struct{
	int id_gulosos[2];       /*arranjo guarda o id dos dois filósofos comendo simultaneamente*/
	int cont;                /*contador de filósofos comendo*/
}typedef chekin_t;  

chekin_t c;

int chungry;                /*contador de filósofos famintos*/


clock_t clk;                /*recebe o tempo de CPU decorrido desde o início da execução do seu programa.
                              implementado meramente para ajudar no acompanhamento da concorrência das threads.
							  "Cuidado: Depois de aproximadamente 35 minutos, ocorre overflow."
							*/      


//========== Funções ===========
void think();
void eat();
void * philosopher(void * args);
void take_forks(int i);
void put_forks(int i);
void test(int i);


void think(){
	sleep(TEMPO_PENSANDO); 
}

void eat()
{
	sleep(TEMPO_COMENDO);
}

void * philosopher(void * args)        /*i: o numero do filosofo, de 0 a N-1 */
{
	int i = (long) args;
	while(TRUE){               /*repete para sempre*/
		think();               /*o filosofo esta pensando */
		take_forks(i);         /*pega dois garfos ou bloqueia */
	 	eat();                 /*hummm, espaguete!*/
		put_forks(i);          /*devolve os dois garfos a mesa*/
	}
}

void take_forks(int i)         /*i: o numero do filosofo, de 0 a N-1*/
{
	pthread_mutex_lock(&mutex);              /*entra na regiao critica*/
	state[i] = HUNGRY;                       /*registra que o filosofo esta faminto*/
	test(i);                                 /*tenta pegar dois garfos*/
	pthread_mutex_unlock(&mutex);            /*sai da regiao critica*/
	pthread_mutex_lock(&s[i]);        /*bloqueia se os garfos nao foram pegos*/
}

void put_forks(int i)                        /*i: o numero do filosofo, de 0 a N-1*/
{
	pthread_mutex_lock(&mutex);              /*entra na regiao critica*/

	//++++++++++++++++++++++++++++
	clk = clock();                           /*artibui o tempo de CPU decorrido até o momento*/
	if (state[i]==EATING){
		printf("%ld - O filósofo %d largou os garfos\n", clk, i);
		c.cont--;
		}
	//++++++++++++++++++++++++++++

	state[i] = THINKING;       /*o filosofo acabou de comer*/
	test(LEFT);                /*ve se o vizinho da esquerda pode comer agora*/
	test(RIGHT);               /*ve se o vizinho da direita pode comer agora*/
	pthread_mutex_unlock(&mutex);                /*sai da regiao critica*/
}


void test(int i)                   /*i: o numero do filosofo, de 0 a N-1*/            
{
	if(state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING){
		state[i] = EATING;

		//+++++++++++++++++++++++++++++++++++
		clk = clock();             /*atribui o tempo de CPU decorrido até o momento*/
		printf("%ld - O filósofo %d pegou os garfos\n", clk, i);

		/*Exibe os filósofos que estão comendo simultaneamente*/
		c.cont++;                  /*Mais um filósofo comendo*/
		switch(c.cont){
			case 1: c.id_gulosos[c.cont-1] = i; break;   /* o primeiro filósofo é colocado na estrutura*/
			case 2: c.id_gulosos [c.cont-1] = i;         /*o segundo filósofo é colocado na estrutura*/
			        clk = clock();                       /*atribui o tempo de CPU decorrido até o momento*/
			        printf("%ld - Os filósofos %d e %d estão comendo simultaneamente.\n\n+++++++++++++++++++++++++++++++++++++++++\n\n", clk, c.id_gulosos[0], c.id_gulosos[1]); 
					break;     
			default:
			    break;
		}
		// ++++++++++++++++++++++++++++++++++


		pthread_mutex_unlock(&s[i]);

	//+++++++++++++++++++++++++++++++++++++++
	}else if(state[i]==HUNGRY){        /*se o filosofo não conseguiu comer e está faminto*/
		chungry++;                     /*mais um faminto*/
		clk = clock();                 /*atribui o tempo de CPU decorrido até o momento*/
		printf("%ld - O filosofo %d está faminto esperando por garfos.\n", clk, i);
		if(chungry==3){                /*quando a concorrência é máxima*/
			clk = clock();             /*atribui o tempo de CPU decorrido até o momento*/
			printf("%ld - %d filosofos na fila esperando por garfos.\n", clk, chungry);
			chungry = 0;
		}
	}
	//+++++++++++++++++++++++++++++++++++++++
}



int main(int arg, char * args[], char * envp[]){
	pthread_t philosophers[N];        /*uma thread para cada filósofo*/
	int rc;                           /*recebe código de error da criação de uma thread*/
	c.cont = 0;                       /*inicializa contador de filósofos comendo*/
	chungry = 0;                      /*inicializa contador de filósofos famintos*/
	pthread_mutex_init(&mutex, 0);    /*inicializa mutex*/

	printf("\nDigite 0 para se quiser encerrar o programa...\n\n\n");
	for(long i = 0; i < N; i++){
		pthread_mutex_init(&s[i],0);  /*inicializa os semáforos de cada filósofo*/
		rc = pthread_create(&philosophers[i], NULL, philosopher, (void *) i); /*cria thread de cada filosofo*/
		if (rc){ 
          printf("ERROR; return code from pthread_create() is %d\n", rc);
          return -1;
       }
	}
	char flag[100];
	while(TRUE){ /*repete até que seja digitado 0*/
		fgets(flag,100, stdin);
		if ( strcmp(flag, "0\n")== 0)
		    break;
	}; 
  
    /*destrói todos os semáforos*/
	for (int i = 0; i < N; i++)
    	pthread_mutex_destroy(&s[i]); 
	pthread_mutex_destroy(&mutex);
	return 0;
}
