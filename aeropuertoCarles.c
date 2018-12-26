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

/* Contador de usuarios */
int contadorUsuarios;

/* Lista de usuarios */
struct usuario{
    int idUsuario;
    int facturado;
    int atendido;
    int tipo;
};
struct usuario usuarios[10];


/* Declaracion de funciones */
void nuevoUsuario (int sig);
int calculaAleatorios (int min, int max);

int main (int argc, char const *argv[]){

    struct sigaction u;
    u.sa_handler = nuevoUsuario;

    sigaction (SIGUSR1,&u,NULL);
    sigaction (SIGUSR2,&u,NULL);
    
    while(1){

        printf ("Esperando... \n");
        pause();
    }
    
    //INICIALIZACIÓN DE RECURSOS
    contadorUsuarios = 0;

    int i;
    for( i = 0; i <= 10; i++){

    	usuarios[i].idUsuario = 0;
    	usuarios[i].facturado = 0;
    	usuarios[i].atendido = 0;
    	usuarios[i].tipo = 0;
    }

    return 0;
}

/* Funcion que crea un nuevo usuario */
void nuevoUsuario(int sig){
    
    int i;
	int usuario = 0;
    
   for (i = 0; i <= 10; i++){
       
     if (contadorUsuarios <= 10){
        //LA LISTA NO ESTÁ LLENA
        printf ("Se puede crear un nuevo usuario.\n");
       
        usuario++;
         
        contadorUsuarios++;

        nuevoUsuario.id = contadorUsuarios;

        nuevoUsuario.atendido = 0;
         
        // SEÑALES
        switch (sig){
        
            case SIGUSR1:
            printf ("Nuevo usuario: Usuario normal.\n");
            usuarios[i].tipo = 0;
            break;
    
            case SIGUSR2:
            printf ("Nuevo usuario: Usuario VIP.\n");
            usuarios[i].tipo = 1;
            break;
                        
            default:
            printf ("ERROR AL CREAR UN NUEVO USUARIO.\n");
        }
     }
     else{
        // LA LISTA ESTÁ LLENA
        printf ("NO se puede crear un nuevo usuario. La lista de facturación está llena.\n");
    }
   }
    
}

int calculaAleatorios (int min, int max){
    srand (time(NULL));
    return rand() % (max-min+1) +min;
}

