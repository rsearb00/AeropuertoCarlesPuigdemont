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



int main (int argc, char *argv[]){

	if (signal(SIGUSR1, nuevoUsuario) == SIG_ERR) {

		perror("Llamada a signal.");
		exit(-1);
	}

	if (signal(SIGUSR2, nuevoUsuario) == SIG_ERR) {

		perror("Llamada a signal.");
		exit(-1);
	}

}

void nuevoUsuario(int s) {

	if (s == SIGUSR1) {
		tipo = 0;
		printf("Soy un usuario normal.");
	}

	if (s == SIGUSR2) {
		tipo = 1;
		printf("Soy un usuario vip.");
	}

}



