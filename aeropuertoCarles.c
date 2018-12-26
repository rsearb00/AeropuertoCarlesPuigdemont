#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Contador de usuarios */
int contadorUsuarios = 0;

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

int main (int argc, char const *argv[]){

    struct sigaction u;
    u.sa_handler = nuevoUsuario;

    sigaction (SIGUSR1,&u,NULL);
    sigaction (SIGUSR2,&u,NULL);
    
    while(1){

        printf ("Esperando... \n");
        pause();
    }

    return 0;
}

/* Funcion que crea un nuevo usuario */
void nuevoUsuario(int sig){
    
    /* Comprobación lista facturación */
    if (contadorUsuarios <= 10){
        //NO ESTÁ LLENO
        printf ("Se puede crear un nuevo usuario.\n");
        // SEÑALES
        switch (sig){
        
            case SIGUSR1:
            printf ("Nuevo usuario: Usuario normal.\n");
            contadorUsuarios++;
            exit(0);
    
            case SIGUSR2:
            printf ("Nuevo usuario: Usuario VIP.\n");
            contadorUsuarios++;
            exit(0);
                        
            default:
            printf ("ERROR AL CREAR UN NUEVO USUARIO.\n");
        }
    }
    if (contadorUsuarios >= 10){
        //ESTÁ LLENO
        printf ("NO se puede crear un nuevo usuario.\n");
    }
return;   
}
