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
int usuariosFacturados;

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
void *accionesFacturador(void *tipoFacturador);
void *accionesAgenteSeguridad();
void *accionesUsuario(void *user);

/*funciones*/
void nuevoUsuario(int sig);
void writeLogMessage(char *id, char *msg);
int calculaAleatorios (int min, int max, int id);
void finalizarPrograma(int sig);



int main(int argc, char const *argv[])
{

	/*señales de usuario*/
    signal(SIGUSR1, nuevoUsuario); 
    signal(SIGUSR2, nuevoUsuario);

	/*señal de finalización*/
	signal(SIGINT, finalizarPrograma);

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
	usuarioEnControl = 0;
	usuariosFacturados=0;

	/*cola de facturación (usuarios)*/
    int i;
    for( i = 0; i < 10; i++){

    	usuarios[i].idUsuario = 0;
    	usuarios[i].facturado = 0;
		usuarios[i].enAtencion = 0;
    	usuarios[i].atendido = 0;
    	usuarios[i].tipo = 0;
    }

    /*inicialización de los facturadores y creación de sus hilos*/
	for(i=0;i<2;i++){

		facturadores[i].ocupado=0;
		facturadores[i].usuariosAtendidos=0;
		facturadores[i].tipoFacturador=i+1;
		pthread_create(&facturadores[0].hiloFacturador,NULL,accionesFacturador,(void *)&facturadores[i].tipoFacturador);

	}

	/*creación del hilo del controlador (agente de seguridad)*/
	agenteDeSeguridad.ocupado=0;
    pthread_create(&agenteDeSeguridad.hiloAgenteSeguridad, NULL, accionesAgenteSeguridad, NULL);


    sleep(2);

	/*espera a recibir las señales*/
    while(1){
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
						usuarios[listaUsuarios].tipo = 1;
					break;
					
					case SIGUSR2:
						usuarios[listaUsuarios].tipo = 2;
					break;
					
					default:
						perror("ERROR AL CREAR UN NUEVO USUARIO.\n");
				}

				/*crea el hilo del usuario*/
				pthread_create (&usuarios[listaUsuarios].hiloUsuario, NULL, accionesUsuario, (void *)&contadorUsuarios);
			
				/*incrementa la lista (hay un hueco menos en  la cola)*/
				listaUsuarios++;

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
	while(usuarios[posLista].enAtencion==0 && usuarios[posLista].facturado==0){
		
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

		pthread_mutex_lock(&semaforoSeguridad);

		/*espera a poder acceder al control de seguridad*/
		while(usuarios[posLista].enAtencion==0 && usuarioEnControl!=id){

			sleep(1);
		}

		/*espera a salir del control de seguridad*/
		while(usuarioEnControl==id && usuarioEnControl==id){

			sleep(1);
		}

		pthread_mutex_unlock(&semaforoSeguridad);

		/*escribe en el registro que embarca*/
		pthread_mutex_lock(&semaforoLog);
		writeLogMessage(usuario,"Ha embarcado.");
		pthread_mutex_unlock(&semaforoLog);

		/*termina el hilo*/
		pthread_exit(NULL);

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

void *accionesFacturador(void *tipoFacturador){
	
	/*halla el tipo de facturador que es (transforma el argumento en int)*/
	int tipo=* (int *)tipoFacturador;

	int i, usuarioAtendido, posUsuario, tiempo;
	int seleccionador=999999; //ayuda en la elección del hilo a atender 
	int encontrado=0; //indica si se ha encontrado algún usuario o no
	char facturador[50];

	/*escribe en el registro y almacena de qué facturador se trata*/
	if(tipo==1){

		sprintf(facturador,"Facturador normal");

		pthread_mutex_lock(&semaforoLog);
		writeLogMessage(facturador,"El facturador normal ha comenzado su turno.");
		pthread_mutex_unlock(&semaforoLog);
	}
	else if(tipo==2){

		sprintf(facturador,"Facturador VIP");

		pthread_mutex_lock(&semaforoLog);
		writeLogMessage(facturador,"El facturador VIP ha comenzado su turno.");
		pthread_mutex_unlock(&semaforoLog);
	}
	else{
		perror("Error al indicar el tipo de facturador.");
	}


	/*se repiten de forma indefinida las mismas acciones*/
	while(1){

		/*mientras el facturador no esté ocupado*/
		while(facturadores[tipo-1].ocupado==0){

			encontrado=0;
			seleccionador=999999;

			/*bloquea el semáforo para que no cambien los datos de los usuarios*/
			pthread_mutex_lock(&semaforoUsuario);

			/*comprueba si hay algún usuario a la espera de que le atiendan
			  y selecciona al que más tiempo lleve esperando (el de menor id)*/
			for(i=0;i<10;i++){

				if(usuarios[i].idUsuario!=0 && tipo==usuarios[i].tipo
				  && usuarios[i].enAtencion==0 && usuarios[i].facturado==0){

					if(usuarios[i].idUsuario<seleccionador){

						seleccionador=usuarios[i].idUsuario;
						posUsuario=i;
						encontrado=1;
					} 					
				}
			}

			/*si hay algún usuario que atender almacena su id
			  y cambia su estado a ocupado*/
			if(encontrado==1){

				usuarioAtendido=seleccionador;
				usuarios[posUsuario].enAtencion=1;
				facturadores[tipo-1].ocupado=1;

			}

			/*si no encontró ningún usuario de su mismo tipo, y el facturador
			  es de tipo VIP, comprueba si puede atender a algún usuario normal*/
			else if(facturadores[1].ocupado==0 && tipo==2 && (facturadores[0].ocupado==1 || facturadores[0].usuariosAtendidos!=0)){

				for(i=0;i<10;i++){

					if(usuarios[i].idUsuario!=0 && usuarios[i].enAtencion==0
					   && usuarios[i].tipo==1 && usuarios[i].facturado==0){

						   if(usuarios[i].idUsuario<seleccionador){

							   seleccionador=usuarios[i].idUsuario;
							   posUsuario=i;
							   encontrado=1;
						   }
					}
				}
			}

			if(encontrado==1){

				usuarioAtendido=seleccionador;
				usuarios[posUsuario].enAtencion=1;
				facturadores[tipo-1].ocupado=1;
			}

			pthread_mutex_unlock(&semaforoUsuario);

			/*si sigue sin estar ocupado duerme 1 segundo antes de volver a comprobar la cola*/
			if(facturadores[tipo-1].ocupado==0){

				sleep(1);
			}

		}

		/*calcula qué ocurre con la facturación del usuario*/
		int num=calculaAleatorios(1,100,usuarioAtendido);

		if(num<=80){ //factura correctamente (80%)

			/*calcula cuánto tiempo tarda*/
			tiempo=calculaAleatorios(1,4,usuarioAtendido);
			sleep(tiempo);

			/*indica que ha facturado*/
			pthread_mutex_lock(&semaforoUsuario);
			usuarios[posUsuario].facturado=1;
			usuarios[posUsuario].enAtencion=0;
			pthread_mutex_unlock(&semaforoUsuario);

			usuariosFacturados++;

			/*escribe en el registro*/
			pthread_mutex_lock(&semaforoLog);
			sprintf(mensaje,"El usuario %d ha facturado correctamente en %d segundos.",usuarioAtendido,tiempo);
			writeLogMessage(facturador,mensaje);
			pthread_mutex_unlock(&semaforoLog);

		}
		else if(num>80 && num<=90){ //factura con exceso de peso (10%)

			/*calcula cuánto tiempo tarda*/
			tiempo=calculaAleatorios(2,6,usuarioAtendido);
			sleep(tiempo);

			/*indica que ha facturado*/
			pthread_mutex_lock(&semaforoUsuario);
			usuarios[posUsuario].facturado=1;
			usuarios[posUsuario].enAtencion=0;
			pthread_mutex_unlock(&semaforoUsuario);

			usuariosFacturados++;

			/*escribe en el registro*/
			pthread_mutex_lock(&semaforoLog);
			sprintf(mensaje,"El usuario %d ha facturado con exceso de peso en %d segundos.",usuarioAtendido,tiempo);
			writeLogMessage(facturador,mensaje);
			pthread_mutex_unlock(&semaforoLog);
		}
		else{ //no tiene el visado en regla, no factura

			/*calcula cuánto tiempo tarda*/
			tiempo=calculaAleatorios(6,10,usuarioAtendido);
			sleep(tiempo);

			/*escribe en el registro*/
			pthread_mutex_lock(&semaforoLog);
			sprintf(mensaje,"El usuario %d no ha facturado porque no tenía el visado en regla y ha tardado %d segundos.",usuarioAtendido,tiempo);
			writeLogMessage(facturador,mensaje);
			pthread_mutex_unlock(&semaforoLog);
			
		}

		/*aumenta el número de usuarios a los que ha atendido el facturador*/
		facturadores[tipo-1].usuariosAtendidos=facturadores[tipo-1].usuariosAtendidos+1;

		/*comprueba si le toca descansar (cada 5 usuarios atendidos)*/
		if(facturadores[tipo-1].usuariosAtendidos%5==0  && facturadores[tipo-1].usuariosAtendidos!=0){

			/*escribe en el registro que se va*/
			pthread_mutex_lock(&semaforoLog);
			writeLogMessage(facturador,"Se va a tomar un café.");
			pthread_mutex_unlock(&semaforoLog);

			sleep(10);

			/*escribe en el registro que ha vuelto*/
			pthread_mutex_lock(&semaforoLog);
			writeLogMessage(facturador,"Ha vuelto de tomar un café.");
			pthread_mutex_unlock(&semaforoLog);
		}

		/*vuelve a estar desocupado*/
		facturadores[tipo-1].ocupado=0;
	}

	/*finaliza el hilo*/
	pthread_exit(NULL);
}

void *accionesAgenteSeguridad(){

	pthread_mutex_lock(&semaforoLog);
	writeLogMessage("Agente de seguridad","Ha comenzado su turno.");
	pthread_mutex_unlock(&semaforoLog);

	/*bloquea el semáforo del control*/
	pthread_mutex_lock(&semaforoSeguridad);

	int encontrado, seleccionador, i, posUsuario, tiempo;

	/*se repiten de forma indefinida las mismas acciones*/
	while(1){

		while(agenteDeSeguridad.ocupado==0){

			encontrado=0;
			seleccionador=999999;

			/*bloquea el semáforo de la cola*/
			pthread_mutex_lock(&semaforoUsuario);

			/*comprueba si hay algún usuario esperando*/
			for(i=0;i<10;i++){

				if(usuarios[i].idUsuario!=0 && usuarios[i].facturado==1
				   && usuarios[i].enAtencion==0){

					if(usuarios[i].idUsuario<seleccionador){

						seleccionador=usuarios[i].idUsuario;
						posUsuario=i;
						encontrado=1;
					}
				}
			}

			/*si hay alguno almacena el id en usuarioEnControl*/
			if(encontrado==1){

				usuarioEnControl=seleccionador;
				usuarios[posUsuario].enAtencion=1;
				agenteDeSeguridad.ocupado=1;
			}

			pthread_mutex_unlock(&semaforoUsuario);

			sleep(1);
		}

		/*escribe en el log la llegada del usuario*/
		pthread_mutex_lock(&semaforoLog);
		sprintf(mensaje,"El usuario %d ha llegado al control de seguridad.",usuarioEnControl);
		writeLogMessage("Agente de seguridad",mensaje);
		pthread_mutex_unlock(&semaforoLog);

		/*libera una posición de la cola*/
		pthread_mutex_lock(&semaforoUsuario);
		listaUsuarios--;
		usuarios[posUsuario].idUsuario=0;
		pthread_mutex_unlock(&semaforoUsuario);

		/*calcula la atención que recibe*/
		int atencion=calculaAleatorios(1,100,usuarioEnControl);

		if(atencion<=60){  //supera el control correctamente (60%)

			/*calcula el tiempo que tarda*/
			tiempo=calculaAleatorios(2,3,usuarioEnControl);
			sleep(tiempo);

			/*indica que ha superado el control*/
			pthread_mutex_lock(&semaforoUsuario);
			usuarios[posUsuario].atendido=1;
			pthread_mutex_unlock(&semaforoUsuario);

			/*escribe en el registro*/
			pthread_mutex_lock(&semaforoLog);
			sprintf(mensaje,"El usuario %d ha superado el control de seguridad.",usuarioEnControl);
			writeLogMessage("Agente de seguridad",mensaje);
			pthread_mutex_unlock(&semaforoLog);
			

		}
		else{  //el usuario es apartado para una inspección de seguridad (40%)

			/*calcula el tiempo que tarda*/
			tiempo=calculaAleatorios(10,15,usuarioEnControl);
			sleep(tiempo);

			/*indica que ha superado el control*/
			pthread_mutex_lock(&semaforoUsuario);
			usuarios[posUsuario].atendido=1;
			pthread_mutex_unlock(&semaforoUsuario);

			/*escribe en el registro*/
			pthread_mutex_lock(&semaforoLog);
			sprintf(mensaje,"El usuario %d ha sido apartado para una inspección de seguridad.",usuarioEnControl);
			writeLogMessage("Agente de seguridad",mensaje);
			pthread_mutex_unlock(&semaforoLog);
		}

		agenteDeSeguridad.ocupado=0;
	}

	pthread_mutex_unlock(&semaforoSeguridad);

	pthread_exit(NULL);
}

void finalizarPrograma(int sig){

	sprintf(mensaje,"Han facturado un total de %d usuarios.",usuariosFacturados);

	pthread_mutex_lock(&semaforoLog);
	writeLogMessage("Aeropuerto Carles Puigdemont",mensaje);
    writeLogMessage("Aeropuerto Carles Puigdemont","El aeropuerto cierra por hoy, hasta luego.");
    pthread_mutex_unlock(&semaforoLog);

	exit(-1);
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