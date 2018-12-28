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
	//Escribe en el log
	pthread_mutex_lock(&semaforoLog);
	char *entrada="User creado";
	//char *tipo=(char *)usuarios[*(int *)usuario].tipo;
	//writeLogMessage((char *)usuarios[*(int *)usuario].tipo, entrada);
	//writeLogMessage(tipo, entrada);
	pthread_mutex_unlock(&semaforoLog);
	
	int posLista= *(int *) user;
	sleep(4);
	while(usuarios[posLista].atendido!=1){
		int num=calculaAleatorios(1,10);
		if  (num<=2){
			//escribir en el log que se va por cansancio
			pthread_mutex_lock(&semaforoLog);
			char *cansancio="User cansado";
			//char *tipo=(char *)usuarios[*(int *)usuario].tipo;
			pthread_mutex_unlock(&semaforoLog);
	
			//Bloqueamos para disminuir la variable global de la lista de usuarios
			pthread_mutex_lock(&semaforoUsuario);
			listaUsuarios--;
			pthread_mutex_unlock(&semaforoUsuario);
			
			//El hilo se cierra
			pthread_exit(NULL);
		}
		int banyo=calculaAleatorios(1,10);
		if(banyo==1){
			//escribir en el log que se va al baño
			pthread_mutex_lock(&semaforoLog);
			char *cansancio="User va al baño";
			//char *tipo=(char *)usuarios[*(int *)usuario].tipo;
			pthread_mutex_unlock(&semaforoLog);
	
			//Bloqueamos para disminuir la variable global de la lista de usuarios
			pthread_mutex_lock(&semaforoUsuario);
			listaUsuarios--;
			pthread_mutex_unlock(&semaforoUsuario);
			
			//El hilo se cierra
			pthread_exit(NULL);		
		}
		/*else{//Baño
			int banyo=calculaAleatorios(1,10);
			while(banyo!=1){
				sleep(3);
			}
		}	*/
		else{
			sleep(3);
		}
	}

	//Espera mientras le atienden en facturación
	while(usuarios[posLista].atendido==1){
		//Espera a qu
	}
	//Espera mientras pasa el control
	if(usuarios[posLista].facturado==1){
		//Ha facturado, tiene que esperar al control
		//Libera la cola de facturación
		pthread_mutex_lock(&semaforoUsuario);
		listaUsuarios--;
		pthread_mutex_unlock(&semaforoUsuario);
		
	}	
	else if(usuarios[posLista].facturado==0){
		//No ha facturado
		//Libera la cola de facturación
		pthread_mutex_lock(&semaforoUsuario);
		listaUsuarios--;
		pthread_mutex_unlock(&semaforoUsuario);
		
		pthread_mutex_lock(&semaforoLog);
		char *cansancio="User no pasa";
		//char *tipo=(char *)usuarios[*(int *)usuario].tipo;
		//writeLogMessage((char *)usuarios[*(int *)usuario].tipo, entrada);
		//writeLogMessage(tipo, entrada);
		pthread_mutex_unlock(&semaforoLog);
		pthread_exit(NULL);
	}
	//pthread_exit(NULL);
}

int calculaAleatorios (int min, int max){
    srand (time(NULL));
    return rand() % (max-min+1) +min;
}

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
