#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>

/* Semaforos */
pthread_mutex_t semaforoUsuario;
pthread_mutex_t semaforoLog;

/*contador de usuarios*/
int contadorUsuarios;

/* Lista de usuarios */
int listaUsuarios;

struct usuario{
    int idUsuario;
    int facturado;
    int atendido;
    int tipo;
    pthread_t hiloUsuario;
};

struct usuario usuarios[10];

/*fichero log*/
FILE *logFile;

/*DECLARACIÓN DE FUNCIONES*/
/*Manejadora para crear hilos de usuarios*/
void nuevoUsuario(int sig);

/*Función para escribir en el log*/
void writeLogMessage(char *id, char *msg);

/*Función de los números aleatorios*/
int calculaAleatorios (int min, int max);

/*Función de los hilos facturadores*/
void *HiloFacturador(void *arg){
	printf("%s\n", (char *) arg);
	pthread_exit(NULL);
}
void *HiloAgenteSeguridad(void *arg){
	printf("%s\n", (char *) arg);
	pthread_exit(NULL);
}
void *accionesUsuario(void *user){
	/*Al incrementarlo en la funcion que crea users, aquí la posicion de la lista llega como +1
	Hay que guardarlo como -1 para acceder al usuario real que hemos creado*/
	int posLista= *(int *) user-1;
	printf("Creado usuario num %d\n", usuarios[posLista].idUsuario);
	pthread_mutex_lock(&semaforoLog);
	char *entrada="User creado";
	//char *tipo=(char *)usuarios[*(int *)usuario].tipo;
	//writeLogMessage((char *)usuarios[*(int *)usuario].tipo, entrada);
	//writeLogMessage(tipo, entrada);
	pthread_mutex_unlock(&semaforoLog);
	
	sleep(4);
	pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    signal(SIGUSR1, nuevoUsuario); 
    signal(SIGUSR2, nuevoUsuario);
    contadorUsuarios = 0;
    listaUsuarios = 0;
    pthread_mutex_init(&semaforoUsuario, NULL);
    int i;
    for( i = 0; i < 10; i++){

    	usuarios[i].idUsuario = 0;
    	usuarios[i].facturado = 0;
    	usuarios[i].atendido = 0;
    	usuarios[i].tipo = 0;
    }
    /*Creación de los hilos facturadores*/		
    pthread_t facturador1, facturador2, agenteSeguridad;
    pthread_create(&facturador1, NULL, HiloFacturador, "Creado el facturador 1");
    pthread_create(&facturador2, NULL, HiloFacturador, "Creado el facturador 2");
    pthread_create(&agenteSeguridad, NULL, HiloAgenteSeguridad, "Creado el agente de seguridad");
	
    /*Bucle para esperar a recibir las señales*/
    sleep(10);
    while(1){
	printf("Introduce `kill -10 PID' si lo que quieres es introducir un usuario normal a la lista.\n");
	printf("Introduce `kill -12 PID' si lo que quieres es introducir un usuario VIP a la lista.\n");
        printf ("Esperando... \n");
        pause();
    }

    return 0;
}

void nuevoUsuario(int sig){
    
    
    //BLOQUEAMOS LA LISTA PARA LIMITAR EL ACCESO A UN SOLO RECURSO
    pthread_mutex_lock (&semaforoUsuario);
    if (listaUsuarios < 10){
        //LA LISTA NO ESTÁ LLENA
        printf ("Se puede crear un nuevo usuario.\n");
         
        contadorUsuarios++;

        usuarios[listaUsuarios].idUsuario = contadorUsuarios;

	    switch (sig)
	    {
		case SIGUSR1:
		    printf("usuario normal\n");
		    usuarios[listaUsuarios].tipo = 1;
		    break;
	    
		case SIGUSR2:
		    printf("usuario vip\n");
		    usuarios[listaUsuarios].tipo = 2;
		    break;
		
		default:
                    printf ("ERROR AL CREAR UN NUEVO USUARIO.\n");
	    }
	//HILO DE CADA USUARIO
        pthread_create (&usuarios[listaUsuarios].hiloUsuario, NULL, accionesUsuario, (void *) &listaUsuarios);  
	//INCREMENTAMOS LA POSICIÓN DE LA LISTA
        listaUsuarios++;
     }
     else{
        // LA LISTA ESTÁ LLENA
        printf ("NO se puede crear un nuevo usuario. La lista de facturación está llena.\n");
     }
     //DESBLOQUEAMOS LA LISTA
     pthread_mutex_unlock (&semaforoUsuario);
}

/*Función para escribir en el log*/
void writeLogMessage(char *id, char *msg){
	/*Calculamos la hora*/
	time_t now = time(0);
	struct tm *tlocal = localtime(&now);
	char stnow[26];
	strftime(stnow, 26, "%d/%m/%y %H:%M:%S", tlocal);
	
	/*Escribimos en el log
	La "a" significa: que si el fichero no existe lo crea para escribir en él 
	y lo deja abierto, por defecto, en modo texto, y si ya existe permite añadir 
	mas datos al final de este respetando sus datos anteriores.
	*/
	logFile=fopen("logFile.txt", "a");
	fprintf(logFile, "[%s] %s: %s:\n", stnow, id, msg);
	fclose(logFile);
}

int calculaAleatorios (int min, int max){
    srand (time(NULL));
    return rand() % (max-min+1) +min;
}
