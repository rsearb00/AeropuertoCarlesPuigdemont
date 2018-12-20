#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
/*contador de usuarios*/
int contadorUsuarios;

/*lista de usuarios*/

struct usuario{
    int idUsuario;
    int facturado;
    int atendido;
    int tipo;
};

struct usuario usuarios[10];

/*fichero log*/
FILE *logFile;

/*declaracion de funciones*/

/*Manejadora para crear hilos de usuarios*/
void nuevoUsuario(int sig);

/*Función para escribir en el log*/
void writeLogMessage(char *id, char *msg);


/*Función de los hilos facturadores*/
void *HiloFacturador(void *arg){
}
void *HiloAgenteSeguridad(void *arg){
}


int main(int argc, char const *argv[])
{
    struct sigaction u;
    u.sa_handler=nuevoUsuario;

    sigaction(SIGUSR1,&u,NULL);
    sigaction(SIGUSR2,&u,NULL);
    sigaction(SIGINT, &u, NULL);

    /*Creación de los hilos facturadores*/		
    pthread_t facturador1, facturador2, agenteSeguridad;
    pthread_create(&facturador1, NULL, HiloFacturador, "Creado el facturador 1");
    pthread_create(&facturador2, NULL, HiloFacturador, "Creado el facturador 2");
    pthread_create(&agenteSeguridad, NULL, HiloAgenteSeguridad, "Creado el agente de seguridad");
	
    /*Bucle para esperar a recibir las señales*/
    while(1){

        printf("esperando\n");
        pause();
    }

    return 0;
}

void nuevoUsuario(int sig){

    
    switch (sig)
    {
        case SIGUSR1:
            printf("usuario normal\n");
            exit(0);
    
        case SIGUSR2:
            printf("usuario vip\n");
            exit(0);
    }
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

