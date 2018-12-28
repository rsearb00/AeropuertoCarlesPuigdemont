#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

/* Semaforos */
pthread_mutex_t semaforoUsuario;

pthread_mutex_t semaforoLog;

/* Contador de usuarios */
int contadorUsuarios;

/* Lista de usuarios */
int listaUsuarios;

/* Lista de usuarios */
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

/* Declaracion de funciones */
void nuevoUsuario (int sig);
int calculaAleatorios (int min, int max);
void *accionesUsuario(void *user);

void writeLogMessage(char *id, char *msg);

int main (int argc, char const *argv[]){

    struct sigaction u;
    u.sa_handler = nuevoUsuario;

    sigaction (SIGUSR1,&u,NULL);
    sigaction (SIGUSR2,&u,NULL);
    
    //INICIALIZACIÓN DE RECURSOS
    pthread_mutex_init(&semaforoUsuario, NULL);
    pthread_mutex_init(&semaforoLog, NULL);
	
    contadorUsuarios = 0;
    listaUsuarios = 0;

    int i;
    for( i = 0; i <= 10; i++){

    	usuarios[i].idUsuario = 0;
    	usuarios[i].facturado = 0;
    	usuarios[i].atendido = 0;
    	usuarios[i].tipo = 0;
    }
	
    while(1){

        printf ("Esperando... \n");
        pause();
    }
	
    return 0;
}

/* Funcion que crea un nuevo usuario */
void nuevoUsuario(int sig){
	
    //BLOQUEAMOS LA LISTA PARA LIMITAR EL ACCESO A UN SOLO RECURSO
    pthread_mutex_lock (&semaforoUsuario);

    if (listaUsuarios < 10){
        //LA LISTA NO ESTÁ LLENA
        printf ("Se puede crear un nuevo usuario.\n");
         
        contadorUsuarios++;

        usuarios[listaUsuarios].idUsuario = contadorUsuarios;
         
        // SEÑALES
        switch (sig){
        
            case SIGUSR1:
            printf ("Nuevo usuario: Usuario normal.\n");
            usuarios[listaUsuarios].tipo = 0;
	    break;
    
            case SIGUSR2:
            printf ("Nuevo usuario: Usuario VIP.\n");
            usuarios[listaUsuarios].tipo = 1;
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

void *accionesUsuario(void *user){
	//char *entrada="User creado";
	//char *tipo=(char *)usuarios[*(int *)usuario].tipo;
	//writeLogMessage((char *)usuarios[*(int *)usuario].tipo, entrada);
	//writeLogMessage(tipo, entrada);
	int posLista= *(int *) user;
	sleep(4);
	if(usuarios[posLista].atendido==0){
		printf("Hola, estoy esperando");
	}
	else{
		printf("Hola, estoy siendo atendido");
	}
	pthread_exit(NULL);
}

int calculaAleatorios (int min, int max){
    srand (time(NULL));
    return rand() % (max-min+1) +min;
}

void writeLogMessage(char *id, char *msg){
	/*bloqueamos la función con el mutex*/
	pthread_mutex_lock(&semaforoLog);
	
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

	/*Desbloqueamos el mutex*/	
	pthread_mutex_unlock(&semaforoLog);
}
