#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h> 
#include <signal.h>
#include <sys/wait.h>


/*semáforos*/

/*variables de condición*/

/*contador de usuarios*/
int contadorUsuarios;

/*lista usuarios*/

struct usuario {

	int idUsuario;
	int facturado;
	int atendido;
	int tipo;
};

struct usuario usuarios[10];

/*usuarios en control*/

/*fichero log*/

/*declaración de funciones*/

void nuevoUsuario(int sig);


int main (int argc, char *argv[]){

	struct sigaction u;
	u.sa_handler = nuevoUsuario;
	
	sigaction(SIGUSR1,&u,NULL);   // nuevo usuario normal
	sigaction(SIGUSR2, &u, NULL); // nuevo usuario vip

	while (1)
	{
		pause();
	}
}

void nuevoUsuario(int sig) {

	switch (sig) {

		case SIGUSR1:
			printf("usuario normal");
			break;

		case SIGUSR2:
			printf("usuario vip");
			break;
	}

}
