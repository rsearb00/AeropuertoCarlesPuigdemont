#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

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


/*declaracion de funciones*/
void nuevoUsuario(int sig);

int main(int argc, char const *argv[])
{
    struct sigaction u;
    u.sa_handler=nuevoUsuario;

    sigaction(SIGUSR1,&u,NULL);
    sigaction(SIGUSR2,&u,NULL);
    
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
