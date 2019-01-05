#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>

//Declaración de las variables globales:

/*semáforos*/
pthread_mutex_t semaforoUsuario;
pthread_mutex_t semaforoLog;
pthread_mutex_t semaforoSeguridad;

/*contador de usuarios*/
int contadorUsuarios;

int listaUsuarios;
char mensaje[100];

/*usuarios*/
struct usuario{
    int idUsuario;
    int facturado;
	int enAtencion;
    int atendido;
    int tipo;
    pthread_t hiloUsuario;
};

/*usuarios en la cola de facturación*/
struct usuario usuarios[10];

/*facturadores*/
struct facturador{

	int ocupado;
	int usuariosAtendidos;
	int tipoFacturador;
	pthread_t hiloFacturador;
};

struct facturador facturadores[2];

/*agente de seguridad*/
struct agenteSeguridad{

    int ocupado;
    pthread_t hiloAgenteSeguridad;
};

struct agenteSeguridad agenteDeSeguridad;

/*usuario en el puesto de control*/
int usuarioEnControl;

/*fichero log*/
FILE *logFile;

/*acciones de los hilos*/
void *accionesFacturador(void *facturador);
void *accionesAgenteSeguridad();
void *accionesUsuario(void *user);

/*funciones*/
void nuevoUsuario(int sig);
void writeLogMessage(char *id, char *msg);
int calculaAleatorios (int min, int max, int id);



int main(int argc, char const *argv[])
{

	/*señales de usuario*/
    signal(SIGUSR1, nuevoUsuario); 
    signal(SIGUSR2, nuevoUsuario);


	// Inicialización de los recursos:

    /*semáforos*/
    pthread_mutex_init(&semaforoUsuario, NULL);
    pthread_mutex_init(&semaforoLog, NULL);
    pthread_mutex_init(&semaforoSeguridad, NULL);

	/*fichero de registro de los logs*/
	logFile=fopen("registroTiempos.log", "w");

	//deja registrado el inicio del aeropuerto
	pthread_mutex_lock(&semaforoLog);
    writeLogMessage("Aeropuerto Carles Puigdemont","Bienvenido.");
    pthread_mutex_unlock(&semaforoLog);

	/*variables*/
	contadorUsuarios = 0;
    listaUsuarios = 0;

	/*cola de facturación (usuarios)*/
    int i;
    for( i = 0; i < 10; i++){

    	usuarios[i].idUsuario = 0;
    	usuarios[i].facturado = 0;
		usuarios[i].enAtencion = 0;
    	usuarios[i].atendido = 0;
    	usuarios[i].tipo = 0;
    }
	printf("Usuarios inicializados.\n\n");

    /*inicialización de los facturadores y creación de sus hilos*/
	for(i=0;i<2;i++){

		facturadores[i].ocupado=0;
		facturadores[i].usuariosAtendidos=0;
		facturadores[i].tipoFacturador=i+1;
		pthread_create(&facturadores[0].hiloFacturador,NULL,accionesFacturador,(void *)&facturadores[i].tipoFacturador);

		printf("Facturador %d inicializado.\n\n",i+1);
	}

	/*creación del hilo del controlador (agente de seguridad)*/
    pthread_create(&agenteDeSeguridad.hiloAgenteSeguridad, NULL, accionesAgenteSeguridad, NULL);	
	printf("Controlador inicializado.\n\n");

    sleep(2);

	/*espera a recibir las señales*/
    while(1){
	printf("Introduce `kill -10 PID' si lo que quieres es introducir un usuario normal a la lista.\n");
	printf("Introduce `kill -12 PID' si lo que quieres es introducir un usuario VIP a la lista.\n");
        printf ("Esperando... \n\n");
        pause();
    }

    return 0;
}

void nuevoUsuario(int sig){  //crea un nuevo usuario (añadiéndolo a la cola de facturación)
    
	/*bloquea el semáforo para limitar el acceso a los recursos*/
	pthread_mutex_lock (&semaforoUsuario);

	/*comprueba si hay espacio en la cola de facturación*/
    if (listaUsuarios < 10){
        
		//hay espacio, se puede crear un usuario

        printf ("Se puede crear un nuevo usuario.\n\n");
        
		/*busca la posición en la que no hay ningún usuario (no afecta al orden de atención)*/
		int i;
		for(i=0;i<10;i++){

			if(usuarios[i].idUsuario==0){

				/*le da un identificador al usuario*/
				contadorUsuarios++;
       			usuarios[listaUsuarios].idUsuario = contadorUsuarios;

				switch (sig) //comprueba qué tipo de usuario es
				{
					case SIGUSR1:
						printf("Nuevo usuario normal. (posición %d de la cola)\n\n",i+1);
						usuarios[listaUsuarios].tipo = 1;
					break;
					
					case SIGUSR2:
						printf("Nuevo usuario vip. (posición %d de la cola)\n\n",i+1);
						usuarios[listaUsuarios].tipo = 2;
					break;
					
					default:
						perror("ERROR AL CREAR UN NUEVO USUARIO.\n");
				}

				/*crea el hilo del usuario*/
				pthread_create (&usuarios[listaUsuarios].hiloUsuario, NULL, accionesUsuario, (void *)&contadorUsuarios);
			
				/*incrementa la lista (hay un hueco menos en  la cola)*/
				listaUsuarios++;

				printf("Hay %d usuarios en la cola.\n\n",listaUsuarios);
			
				/*sale del bucle para no crear más*/
				break;
			}
		}
    }
    else{	//no hay espacio en la cola, el usuario se va
        
		/*escribe en el registro*/
		pthread_mutex_lock(&semaforoLog);
		writeLogMessage("Cola de facturación","Llega un usuario pero la cola está llena y se va.");
		pthread_mutex_unlock(&semaforoLog);

        printf ("No se puede crear un nuevo usuario. La lista de facturación está llena.\n\n");
    }

    /*desbloquea el semáforo*/
    pthread_mutex_unlock (&semaforoUsuario);
}

void *accionesUsuario(void *user){

	//acciones realizadas por los hilos de los usuarios

	/*halla el id del usuario (transforma el argumento en int)*/
	int id=* (int *)user;

	char usuario[30]; //almacena el usuario para escribirlo en el registro
	sprintf(usuario, "Usuario %d", id);

	/*busca la posición del usuario en la cola de facturación*/
	int i=0;
	int posLista;
	
	do{
		if(usuarios[i].idUsuario==id){
			posLista=i;
		}
	        i++;
	}while(i<10);
	//no es necesario bloquear el semáforo porque la posición no varía

	/*comprueba de qué tipo es y lo almacena en un char[]*/
	if(usuarios[posLista].tipo==1){
		sprintf(mensaje, "Acaba de llegar a la cola de facturación y es un usuario normal.");
	}
	else if(usuarios[posLista].tipo==2){
		sprintf(mensaje, "Acaba de llegar a la cola de facturación y es un usuario VIP.");
	}
	
	/*escribe en el registro*/
	pthread_mutex_lock(&semaforoLog);
	writeLogMessage(usuario, mensaje);
	pthread_mutex_unlock(&semaforoLog);
	
	/*duerme 4 segundos*/
 	sleep(4);

	/*calcula si está entre el 20% de usuarios que se cansa de esperar*/
	int cansancio=calculaAleatorios(1,100,posLista);

	if(usuarios[posLista].enAtencion==0 && cansancio<=20){  //comprueba el resultado
		
		//se va por cansancio
		
		/*escribe en el registro*/
		pthread_mutex_lock(&semaforoLog);
		writeLogMessage(usuario,"Se va de la cola porque está cansado de esperar.");
		pthread_mutex_unlock(&semaforoLog);

		/*libera un hueco en la cola de facturación bloqueando el semáforo correspondiente*/
		pthread_mutex_lock(&semaforoUsuario);
		listaUsuarios--;
		usuarios[posLista].idUsuario=0;
		pthread_mutex_unlock(&semaforoUsuario);
		
		/*termina el hilo*/
		pthread_exit(NULL);
	}
	
	/*calcula cada 3 segundos (mientras no está siendo atendido) 
	  si tiene ganas de ir al baño (10% de los usuarios)*/
	while(usuarios[posLista].enAtencion==0){
		
		int banyo=calculaAleatorios(1,100,posLista);
	
		if(banyo>=90){ //se va al baño
			
			/*escribe en el registro*/
			pthread_mutex_lock(&semaforoLog);
			writeLogMessage(usuario,"Se va al baño.");
			pthread_mutex_unlock(&semaforoLog);
	
			/*libera un hueco en la cola de facturación*/
			pthread_mutex_lock(&semaforoUsuario);
			listaUsuarios--;
			usuarios[posLista].idUsuario=0;
			pthread_mutex_unlock(&semaforoUsuario);
			
			/*termina el hilo*/
			pthread_exit(NULL);		
		}
		else{
			/*duerme 3 segundos antes de comprobar de nuevo si está siendo atendido*/
			sleep(3);
		}
	}

	/*espera a terminar de ser atendido por el facturador*/
	while(usuarios[posLista].enAtencion==1){

		sleep(1);
	}

	/*si ha facturado correctamente:*/
	if(usuarios[posLista].facturado==1){

		/*espera a poder acceder al control de seguridad*/
		while(usuarios[posLista].enAtencion==0 && usuarioEnControl!=id){

			sleep(1);
		}

		/*cuando se encuentra en el control:*/
		if(usuarios[posLista].enAtencion==1 && usuarioEnControl==id){

			/*sale de la cola de facturación*/
			pthread_mutex_lock(&semaforoUsuario);
			listaUsuarios--;
			usuarios[posLista].idUsuario=0;
			pthread_mutex_unlock(&semaforoUsuario);

			/*escribe en el registro*/
			pthread_mutex_lock(&semaforoLog);
			writeLogMessage(usuario,"Ha llegado al control de seguridad.");
			pthread_mutex_unlock(&semaforoLog);
		}

		/*espera a salir del control de seguridad*/
		while(usuarioEnControl==id && usuarioEnControl==id){

			sleep(1);
		}



	}
	/*si no ha facturado por no tener el visado en regla*/
	else if(usuarios[posLista].facturado==0){
		
		/*sale de la cola de facturación*/
		pthread_mutex_lock(&semaforoUsuario);
		listaUsuarios--;
		usuarios[posLista].idUsuario=0;
		pthread_mutex_unlock(&semaforoUsuario);

		/*escribe en el registro*/
		pthread_mutex_lock(&semaforoLog);
		writeLogMessage(usuario,"No tiene el visado en regla y se va.");
		pthread_mutex_unlock(&semaforoLog);	

		/*termina el hilo*/
		pthread_exit(NULL);
	}
}

void *accionesFacturador(void *facturador){
	
	/*halla el tipo de facturador que es (transforma el argumento en int)*/
	int tipo=* (int *)facturador;

	switch(tipo){

		case 1:
			pthread_mutex_lock(&semaforoLog);
			writeLogMessage("Facturador normal","El facturador normal ha comenzado su turno.");
			pthread_mutex_unlock(&semaforoLog);
		break;

		case 2:
			pthread_mutex_lock(&semaforoLog);
			writeLogMessage("Facturador VIP","El facturador VIP ha comenzado su turno.");
			pthread_mutex_unlock(&semaforoLog);
		break;

		default:
			perror("Error al indicar el tipo de facturador.");
	}

	pthread_exit(NULL);
}

void *accionesAgenteSeguridad(){

	pthread_exit(NULL);
}

void writeLogMessage(char *id, char *msg){ //escribe un mensaje en el registro
	
	/*calcula la hora*/
	time_t now = time(0);
	struct tm *tlocal = localtime(&now);
	char stnow[26];
	strftime(stnow, 26, "%d/%m/%y %H:%M:%S", tlocal);
	
	
	logFile=fopen("registroTiempos.log", "a");

	/*escribe en el registro*/
	fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);

	fclose(logFile);
}

int calculaAleatorios (int min, int max, int id){
   srand (time(NULL)*id);
   return rand() % (max-min+1) +min;
}