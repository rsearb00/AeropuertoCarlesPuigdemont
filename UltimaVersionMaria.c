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
pthread_mutex_t semaforoSeguridad;

/*contador de usuarios*/
int contadorUsuarios;

/* Lista de usuarios */
int listaUsuarios;

/*Guarda el usuario que está pasando el control de seguridad*/
int usuarioEnControl;

struct usuario{
    int idUsuario;
    int facturado;
    int atendido;
    int tipo;
    pthread_t hiloUsuario;
};

struct usuario usuarios[10];

/* Lista facturadores*/

struct facturador{
	int idFacturador;
	int ocupado;
	int usuariosAtendidos;
	int tipoFacturador;
	pthread_t hiloFacturador;
};

struct facturador facturadores[2];

/*fichero log*/
FILE *logFile;

/*DECLARACIÓN DE FUNCIONES*/
/*Manejadora para crear hilos de usuarios*/
void nuevoUsuario(int sig);

/*Función para escribir en el log*/
void writeLogMessage(char *id, char *msg);

/*Función de los números aleatorios*/
int calculaAleatorios (int min, int max, int id);

/*Función de los hilos facturadores*/
void *HiloFacturador(void *tipoFacturador){
	//Esto solo lo puse para ver en la terminal que se crea el facturador 
	//printf("%s\n", (char *) arg);

	//int facturador = (intptr_t)idFacturador;

	int facturador=* (int *)tipoFacturador;

	char identificadorFacturador[30];
	char mensaje[50];
	sprintf(identificadorFacturador, "Facturador %d",facturador);
	sprintf(mensaje, "comienza a atender usuarios");
	writeLogMessage(identificadorFacturador, mensaje);

	facturadores[facturador].ocupado =0;

	int usuarioAtendido;
	
	while(1){
	int posicion;
	/*Mientras el facturador esta ocupado*/
		while(facturadores[facturador].ocupado==0){

		//BLOQUEAMOS LA LISTA PARA LIMITAR EL ACCESO A UN SOLO RECURSO
    			pthread_mutex_lock (&semaforoUsuario);
		int i;
		//El seleccionador es el que va a comprobar cual es el hilo que mas ha esperado en 			la cola
		int seleccionador=999999;	
		//Recorremos la lista de vehículos
			for( i = 0; i < 10; i++){

				if(usuarios[i].idUsuario != 0 && facturadores[facturador].tipoFacturador == usuarios[i].tipo 
			&& usuarios[i].atendido == 0 && usuarios[i].facturado==0){

				     if(usuarios[i].idUsuario<seleccionador){
					
					usuarioAtendido=usuarios[i].idUsuario;
					 posicion=i;

					//El usuario esta siendo atendido por el facturador
					usuarios[posicion].atendido = 1;

					//El facturador esta atendiendo a un usuario
					facturadores[facturador].ocupado = 1;
						seleccionador = usuarios[i].idUsuario;
				//DESBLOQUEAMOS LA LISTA
    			pthread_mutex_unlock (&semaforoUsuario);
			
				     }

				}
		
			} 
			//No se encontro usuario del tipo al que atender

			if(facturadores[facturador].ocupado = 0){
			     for(i = 0; i < 10; i++){

				if(usuarios[i].idUsuario != 0 && usuarios[i].atendido == 0 && usuarios[i].facturado==0){

				    if(usuarios[i].idUsuario < seleccionador){
	
					usuarioAtendido=usuarios[i].idUsuario;
					posicion=i;

					//El usuario esta siendo atendido por el facturador
					usuarios[posicion].atendido = 1;

					//El facturador esta atendiendo a un usuario
					facturadores[facturador].ocupado = 1;
					
					//DESBLOQUEAMOS LA LISTA
    			pthread_mutex_unlock (&semaforoUsuario);	
				    }
				}
			     }
			}
			//DESBLOQUEAMOS LA LISTA
    			pthread_mutex_unlock (&semaforoUsuario);
			
			if(facturadores[facturador].ocupado = 0){
			
				sleep(1);

			}
		}


		//Inicio de la facturacion en el log

		char comienzoFacturacion[30];
		char mensaje1[50];
		sprintf(comienzoFacturacion, "Facturacion usuario %d, ",usuarioAtendido);
		sprintf(mensaje1, "inicio facturacion.");
		writeLogMessage(comienzoFacturacion, mensaje1);

		
		//Contamos los usuarios		
		int i=0;
		int posLista;
		
		do{
		      if(usuarios[i].idUsuario==0){
			posLista=i;
		      }
	        i++;
		}while(i<10);

		//Tipos de facturaciones
		int num=calculaAleatorios(1,10,posLista);
		//Si sale un 8, pertenece al 80% de facturacion correcta, y espera para despues ir al control de seguridad
		if(num<=8){
		
		srand(time(NULL));
		int correcto= rand() % (4-1+1) +1;
		
		//Fin de la facturacion correcta en el log
		char facturacionCorrecta[30];
		char mensaje2[50];
		sprintf(facturacionCorrecta, "Fin facturacion usuario %d, ",usuarioAtendido);
		sprintf(mensaje2, "ha sido correcta. Ha tardado %d segundos.", correcto);
		writeLogMessage(facturacionCorrecta, mensaje2);

		//Espera el tiempo necesario para facturar
		sleep(correcto);
		
		//Va al control de seguridad

		}
		//Si sale un 9, pertenece a un 10% que posee exceso de peso, espera mas tiempo y va al control de seguridad
		else if(num=9){

		srand(time(NULL));
		int peso= rand() % (6-2+1) +2;

		//Fin de la facturacion con exceso de peso en el log
		char facturacionPeso[30];
		char mensaje3[50];
		sprintf(facturacionPeso, "Fin facturacion usuario %d, ",usuarioAtendido);
		sprintf(mensaje3, "con exceso de peso. Ha tardado %d segundos.", peso);
		writeLogMessage(facturacionPeso, mensaje3);

		//Espera el tiempo necesario para facturar
		sleep(peso);
		//Va al control de seguridad
		}
		//Si sale un 10, no tiene su visado en regla, espera mucho mas y no pasa el control de seguridad, deja hueco en la cola
		else if(num=10){
		srand(time(NULL));
		int visado= rand() % (10-6+1) +6;

		//Espera el tiempo necesario para facturar
		sleep(visado);
		//Bloqueamos para disminuir la variable global de la lista de usuarios
			pthread_mutex_lock(&semaforoUsuario);
			listaUsuarios--;
			pthread_mutex_unlock(&semaforoUsuario);

		//Fin de la facturacion con exceso de peso en el log
		char facturacionIncorrecta[30];
		char mensaje4[50];
		sprintf(facturacionIncorrecta, "Fin facturacion usuario %d, ",usuarioAtendido);
		sprintf(mensaje4, "con visado incorrecto. Ha tardado %d segundos en dejar un hueco en la cola.", visado);
		writeLogMessage(facturacionIncorrecta, mensaje4);

		
		}

		pthread_mutex_lock(&semaforoUsuario);
		usuarios[posicion].facturado=1;
		pthread_mutex_unlock(&semaforoUsuario);

		facturadores[facturador].usuariosAtendidos+=1;

		//Comprobar si descansa el facturador
		if(facturadores[facturador].usuariosAtendidos % 5 == 0){

			//Inicio del descanso en el log
			char inicioDescanso[30];
		char mensaje5[50];
		sprintf(inicioDescanso, "Facturador %d, ",facturadores[facturador].tipoFacturador);
		sprintf(mensaje5, "se toma un descanso.");
		writeLogMessage(inicioDescanso, mensaje5);

			facturadores[facturador].ocupado = 1;
			sleep(10);

			//Fin del descanso en el log
			char finDescanso[30];
		char mensaje6[50];
		sprintf(finDescanso, "Facturador %d, ",facturadores[facturador].tipoFacturador);
		sprintf(mensaje6, "finaliza su descanso.");
		writeLogMessage(finDescanso, mensaje6);
			
		}

		facturadores[facturador].ocupado = 0;
	
	}
	
	pthread_exit(NULL);
}
void *HiloAgenteSeguridad(void *arg){
	printf("%s\n", (char *) arg);
	pthread_exit(NULL);
}
void *accionesUsuario(void *user){
	int id=* (int *)user; //Sacar la posición de la lista
	char usuario[30];
	char mensaje[50];
	sprintf(usuario, "Usuario %d", id);
	
	int i=0;
	int posLista;

	pthread_mutex_lock(&semaforoUsuario);
	
	do{
		if(usuarios[i].idUsuario==id){
			posLista=i;
		}
	        i++;
	}while(i<10);
	
	if(usuarios[posLista].tipo==1){
		sprintf(mensaje, "Es un pasajero normal y acaba de llegar a la cola de facturación");
	}
	else if(usuarios[posLista].tipo==2){
		sprintf(mensaje, "Es un pasajero VIP y acaba de llegar a la cola de facturación");
	}
	
	pthread_mutex_unlock(&semaforoUsuario);
	
	pthread_mutex_lock(&semaforoLog);
	writeLogMessage(usuario, mensaje);
	pthread_mutex_unlock(&semaforoLog);
	
 	sleep(4);
	
	while(usuarios[posLista].atendido!=1){
		//pthread_mutex_lock(&semaforoUsuario);
		int num=calculaAleatorios(1,10,posLista);
		int banyo=calculaAleatorios(1,10,posLista);
		//pthread_mutex_unlock(&semaforoUsuario);
	
		//Posible Problema: si viene otro hilo y cambia el valor cuando va a entrar en el if, ya no sirve y en el mismo segundo 		sacan el mismo número*/
		if(num<=2){
			//escribir en el log que se va por cansancio
			char cansancio[30];
			sprintf(cansancio, "Está cansado y se va, num %d", num);
			pthread_mutex_lock(&semaforoLog);
			writeLogMessage(usuario, cansancio);
			pthread_mutex_unlock(&semaforoLog);
	
			//Bloqueamos para disminuir la variable global de la lista de usuarios
			pthread_mutex_lock(&semaforoUsuario);
			listaUsuarios--;
			pthread_mutex_unlock(&semaforoUsuario);
			
			//El hilo se cierra
			pthread_exit(NULL);
		}
		else if(banyo==3){
			//escribir en el log que se va al baño
			char toilet[30];
			sprintf(toilet, "Va al baño, num %d", banyo);
			pthread_mutex_lock(&semaforoLog);
			writeLogMessage(usuario, toilet);
			pthread_mutex_unlock(&semaforoLog);
	
			//Bloqueamos para disminuir la variable global de la lista de usuarios
			pthread_mutex_lock(&semaforoUsuario);
			listaUsuarios--;
			pthread_mutex_unlock(&semaforoUsuario);
			
			//El hilo se cierra
			pthread_exit(NULL);		
		}
		else{
			sleep(3);
		}
	}
	int atendido=1;
	while(usuarios[posLista].atendido==1){
		sleep(1);
	}
	if(usuarios[posLista].facturado==1){
		//Ha facturado, tiene que esperar al control
		//Libera la cola de facturación
		pthread_mutex_lock(&semaforoUsuario);
		int idSeg=usuarios[posLista].idUsuario;
		listaUsuarios--;

		//Entra al control
		usuarioEnControl=idSeg;
		while(usuarioEnControl==idSeg){
			sleep(1);
		}
		//¿El de seguridad pone usuarioEnControl a 0?
		char embarca[30];
		char dejaControl[50]; 
		sprintf(dejaControl, "Ha pasado el control de seguridad");
		sprintf(embarca, "Ha embarcado");
		
		//Puede que sobren estos, al estar ya dentro del de usuario
		pthread_mutex_lock(&semaforoLog);
		writeLogMessage(usuario, dejaControl);
		writeLogMessage(usuario, embarca);
		pthread_mutex_unlock(&semaforoLog);

		pthread_mutex_unlock(&semaforoUsuario);	
	}
	else if(usuarios[posLista].facturado==0){
		//No ha facturado
		//Libera la cola de facturación
		pthread_mutex_lock(&semaforoUsuario);
		listaUsuarios--;
		pthread_mutex_unlock(&semaforoUsuario);
		
		//escribir en el log que se va 
		char noFacturado[30]; 
		sprintf(noFacturado, "Se va sin facturar");
		pthread_mutex_lock(&semaforoLog);
		writeLogMessage(usuario, noFacturado);
		pthread_mutex_unlock(&semaforoLog);
		
		//El hilo se cierra
		pthread_exit(NULL);
	}
}

int main(int argc, char const *argv[])
{
    signal(SIGUSR1, nuevoUsuario); 
    signal(SIGUSR2, nuevoUsuario);
    contadorUsuarios = 0;
    listaUsuarios = 0;
    pthread_mutex_init(&semaforoUsuario, NULL);
    pthread_mutex_init(&semaforoLog, NULL);
    pthread_mutex_init(&semaforoSeguridad, NULL);
    int i,j;
    for( i = 0; i < 10; i++){

    	usuarios[i].idUsuario = 0;
    	usuarios[i].facturado = 0;
    	usuarios[i].atendido = 0;
    	usuarios[i].tipo = 0;
    }


	/*Inicializa los facturadores*/
	for( j = 0; j < 2;j++){
		facturadores[j].idFacturador = 0;
		/*El 0 significa que el facturador no está atendiendo a ningún usuario*/
		facturadores[j].ocupado = 0;
		facturadores[j].usuariosAtendidos =0;
	}
		/*El facturador que atienda a los usuarios normales tendra el numero 1*/
		facturadores[0].tipoFacturador = 1;
		
		/*El facturador que atienda a los usuarios VIP tendra el numero 2*/
		facturadores[1].tipoFacturador = 2;

    /*Creación de los hilos facturadores*/		
    pthread_t agenteSeguridad;

    //Podríamos hacer esto, dos ints, cada uno que indique el tipo de usuarios que atienden
    int normal=1;
    int VIP=2;

    pthread_create(&facturadores[0].hiloFacturador, NULL, HiloFacturador, (void *)&facturadores[0].tipoFacturador);
    pthread_create(&facturadores[1].hiloFacturador, NULL, HiloFacturador, (void *)&facturadores[1].tipoFacturador);	

/*  pthread_create(&facturador1, NULL, HiloFacturador, "Creado el facturador 1");
    pthread_create(&facturador2, NULL, HiloFacturador, "Creado el facturador 2");*/
    pthread_create(&agenteSeguridad, NULL, HiloAgenteSeguridad, "Creado el agente de seguridad");
	
    /*Bucle para esperar a recibir las señales*/
    sleep(2);
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
        pthread_create (&usuarios[listaUsuarios].hiloUsuario, NULL, accionesUsuario, (void *)&contadorUsuarios);  
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
	fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
	fclose(logFile);
}

int calculaAleatorios (int min, int max, int id){
   srand (time(NULL)*id);
   return rand() % (max-min+1) +min;
}
