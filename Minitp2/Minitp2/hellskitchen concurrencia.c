#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> //usleep
#include <stdbool.h>
#include <string.h>

#define ESCRITURA 20000
//Variables globales
int jueces;
int equipoGanador;
char accion[50];
int grupo;

struct burgers{
	bool ajo_cortado, cebolla_cortada, perejil_cortado;
	bool medallones_armados, panes_horneados;
	bool hamburguesa_plancha, tomates, lechuga;
};

struct equipo{
	struct burgers b;
	int nro;
	int puesto;
};

//Semaforos
sem_t sem_plancha;
sem_t sem_horno;
sem_t sem_salero;
sem_t sem_grupo;	//Le da tiempo al thread que escribe a guardar los datos sin que se sobreescriban

//Flags
int salero;
int plancha;
int horno;

void * writter (void * data){
	FILE * log;
	log = fopen("log.txt","w");
	fprintf(log, "\nConcurso de Hamburguesas de Hell's Kitchen!\n \n");
	while (jueces != 3){
		if ((grupo != 0)&&(accion != "")){
			fprintf(log,"Equipo %d ",grupo);
			fprintf(log,"%s",accion);
			strcpy(accion,"");
			grupo = 0;
		}
	}
	//Escribe la última acción
	fprintf(log,"Equipo %d ",grupo);
	fprintf(log,"%s",accion);
	strcpy(accion,"");
	
	//Escribe el ganador y cierra el txt
	fprintf(log, "\nEquipo %d ganó! \nFin del Concurso!\n", equipoGanador);
	fclose(log);
	
	pthread_exit(NULL);
}

void cortarVerdura(struct equipo * e){
	sem_wait(&sem_grupo);	
	grupo = e->nro;
	strcpy(accion, "está cortando el ajo\n");	
	usleep( ESCRITURA );
	sem_post(&sem_grupo);

	e->b.ajo_cortado = true;
	
	sem_wait(&sem_grupo);	
	grupo = e->nro;
	strcpy(accion, "está cortando la cebolla\n");
	usleep( ESCRITURA );
	sem_post(&sem_grupo);	

	e->b.cebolla_cortada = true;

	sem_wait(&sem_grupo);
	grupo = e->nro;
	strcpy(accion, "está picando perejil\n");	
	usleep( ESCRITURA );
	sem_post(&sem_grupo);
	
	e->b.perejil_cortado = true;
	
	sem_wait(&sem_grupo);	
	grupo = e->nro;
	strcpy(accion, "terminó de picar verdura\n \n");
	usleep( ESCRITURA );
	sem_post(&sem_grupo);
	
	printf("Equipo %d terminó de picar verdura\n",e->nro);
}

void salarCarne (struct equipo * e){
	sem_wait(&sem_grupo);
	grupo = e->nro;
	strcpy(accion, "mezcló las verduras en la carne\n");
	usleep(50000);
	sem_post(&sem_grupo);
	
	sem_wait(&sem_grupo);
	grupo = e->nro;
	strcpy(accion, "saló la carne y le dió forma de medallón\n \n");
	usleep(50000);
	sem_post(&sem_grupo);
	
	e->b.medallones_armados = true;
	printf("Equipo %d terminó de salar la carne\n",e->nro);
}

void cocinar (struct equipo * e){
	sem_wait(&sem_grupo);	
	grupo = e->nro;
	strcpy(accion, "está usando la plancha!\n");
	usleep(ESCRITURA);
	sem_post(&sem_grupo);	
	
	usleep(5000000);
	
	sem_wait(&sem_grupo);	
	grupo = e->nro;
	strcpy(accion, "hizo la primer hamburguesa!\n");
	usleep(ESCRITURA);
	
	grupo = e->nro;
	strcpy(accion, "está calentando la segunda hamburguesa!\n");
	usleep(ESCRITURA);
	sem_post(&sem_grupo);
	
	usleep(5000000);

	
	sem_wait(&sem_grupo);	
	grupo = e->nro;
	strcpy(accion, "terminó la segunda hamburguesa!\n \n");
	usleep(ESCRITURA);
	sem_post(&sem_grupo);
	
	e->b.hamburguesa_plancha = true;
	printf("Equipo %d terminó de usar la plancha\n",e->nro);
}

void hornear (struct equipo * e){
	sem_wait(&sem_grupo);	
	grupo = e->nro;
	strcpy(accion, "hizo la masa para el pan!\n");
	usleep(ESCRITURA);
		
	grupo = e->nro;
	strcpy(accion, "está usando el horno para pan!\n");
	usleep(ESCRITURA);
	sem_post(&sem_grupo);
	
	usleep(10000000);

	sem_wait(&sem_grupo);		
	grupo = e->nro;
	strcpy(accion, "panes listos!\n \n");
	usleep(ESCRITURA);
	sem_post(&sem_grupo);
	
	e->b.panes_horneados = true;
	printf("Equipo %d tiene 2 panes recién horneados!\n",e->nro);
}

void lechuMate (struct equipo * e){

	sem_wait(&sem_grupo);
	grupo = e->nro;
	strcpy(accion, "está cortando lechuga y tomate!\n");
	
	usleep(ESCRITURA);
	
	
	grupo = e->nro;
	strcpy(accion, "Todo listo! Hamburguesas listas!\n \n");
	usleep(ESCRITURA);
	sem_post(&sem_grupo);
	
	printf("Equipo %d tiene las hamburguesas listas!\n",e->nro);
}

void * cooking(void * data){
	struct equipo * e = (struct equipo *) data;
	
	printf("Hola! Somos el equipo numero %d\n", e->nro);
	
	cortarVerdura(e);
	salero = sem_wait(&sem_salero);
	if(salero == 0){
		//Sección Crítica
		//Usa el salero
		salarCarne(e);
		//Da señal
		sem_post(&sem_salero);
		
		plancha = sem_trywait(&sem_plancha);
		if(plancha == 0){
			//Sección Crítica
			//Usa la plancha para las dos hamburguesas
			cocinar(e);
			//Da señal
			sem_post(&sem_plancha);
		}
		//Sección Crítica
		sem_wait(&sem_horno);
		//Usa el horno para pan
		hornear(e);
		//Da señal
		sem_post(&sem_horno);
		
		if(!e->b.hamburguesa_plancha){
			//Seccion Crítica
			sem_wait(&sem_plancha);
			cocinar(e);
			sem_post(&sem_plancha);
		}
			
	}
	else{
		//Sección Crítica
		sem_wait(&sem_horno);
		//Usa el horno para pan
		hornear(e);
		//Da señal
		sem_post(&sem_horno);
		
		//Espera por el salero
		sem_wait(&sem_salero);
		salarCarne(e);
		sem_post(&sem_salero);

		//Hace las hamburguesas
		sem_wait(&sem_plancha);
		cocinar(e);
		sem_post(&sem_plancha);
	}
	//Último paso
	lechuMate(e);
	
	if(e->b.hamburguesa_plancha && e->b.panes_horneados){
		jueces++;
		e->puesto = jueces;
	
		sem_wait(&sem_grupo);
		grupo = e->nro;
		strcpy(accion, "entregó las hamburguesas!\n");
		usleep(50000);
		sem_post(&sem_grupo);
	
		if(e->puesto == 1){
			equipoGanador = e->nro;
		
			//Espera que los otros equipos terminen de entregar las hamburguesas
			usleep(15000000);
		
			printf("Equipo %d ganó!\n",equipoGanador);
		}
		pthread_exit(NULL);
	}
}

void teamSetter(struct equipo e){
	e.b.ajo_cortado = false;
	e.b.cebolla_cortada = false;
	e.b.perejil_cortado = false;
	e.b.medallones_armados = false;
	e.b.panes_horneados = false;
	e.b.hamburguesa_plancha = false;
	e.b.tomates = false;
	e.b.lechuga = false;
}

void main() {
	struct equipo e1, e2, e3;
	e1.nro = 1;
	e2.nro = 2;
	e3.nro = 3;
	
	//Seteo los struct
	teamSetter(e1);
	teamSetter(e2);
	teamSetter(e3);
	jueces = 0;
	equipoGanador = 0;
	
	//Inicializo los Semáforos
	sem_init(&sem_salero,0,1);
	sem_init(&sem_plancha,0,1);
	sem_init(&sem_horno,0,1);
	sem_init(&sem_grupo,0,1);
	
	//Creo los threads
	pthread_t equipo[4];
	
	pthread_create(&equipo[0],NULL,writter,NULL);
	//Le da tiempo al thread "escritor" a ir a la función y escribir cuando los otros equipos empiecen
	usleep( 500000 );
	pthread_create(&equipo[1],NULL,inicio,(void *) &e1);
	pthread_create(&equipo[2],NULL,cooking,(void *) &e2);
	pthread_create(&equipo[3],NULL,cooking,(void *) &e3);
	
	//Elimino los semáforos
	sem_destroy(&sem_salero);
	sem_destroy(&sem_plancha);
	sem_destroy(&sem_horno);
	
	//Espera a los threads
	for(int i = 0; i<4; i++){
		pthread_join(equipo[i],NULL);
	}
}
