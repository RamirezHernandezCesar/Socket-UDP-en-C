/* 

Autor: Cesar Ramirez Hernandez
----------Cliente UDP------------

Forma de compilar:
gcc cliente.c -o cliente

Forma de correr:
./cliente Ip_Servidor Puerto_Servidor
./cliente 127.0.0.1 4950

NOTA: Al momento de ejecutar el cliente es necesario especificar la ip y el puerto donde se encuentra el servidor.
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MYPORT 4950        /* El puerto donde se enviaran los datos al servidor*/ 
#define MAXBUFLEN 50000    /* Max. cantidad de bytes que podra recibir en una llamada a recvfrom() */ 


typedef struct segment SEGMENT;


/* Estructura que funcionara como el paquete a enviar */
struct segment{
  int segNum;
  int control;
  int lineLen;
  char line[MAXBUFLEN];
};



int main(int argc, char *argv[]){ 

  
  int sockfd; 
  int port=MYPORT;
  char msg[MAXBUFLEN];
  char * ip;
  
  struct sockaddr_in server; /* Almacenara la direccion IP y numero de puerto del servidor */ 
  
  
  int addr_len;
  long int numbytes; 
  char buf[MAXBUFLEN]; 


  /* Tratamiento de la linea de comandos. */
  if (argc >= 2)
  {
    ip = argv[1];
    
    if (argc == 3)
    {
       port = atoi(argv[2]);
    }
  }
  else
  {
    fprintf(stderr,"Uso: cteUDP <ip> [puerto]\n");
    exit(1);
  }


  /* Creamos el socket */ 
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("Error al crear socket"); 
    exit(1);
  } 


  /* Se establece la estructura server con la direccion ip y puerto del servidor a la cual el cliente se conectara*/ 
  server.sin_family = AF_INET;                /* Familia del socket */ 
  server.sin_port = htons(MYPORT);            /* Puerto convertido a network byte order - (BigEndian) */ 
  server.sin_addr.s_addr = inet_addr(ip);     /* convierte la IP de notación de números y puntos a la de datos binarios */
  bzero(&(server.sin_zero), 8);               /* Se rellena con ceros el resto de la estructura */
   
  
  addr_len = sizeof(struct sockaddr);

  do{
  	/* Solicitamos mensaje */ 
  	printf("\nMensaje a enviar: ");
  	gets(msg);   
  
  	/* enviamos el mensaje, esta linea contiene una barra invertida al final, indicando que sigue abajo*/ 
  	if (( numbytes = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&server, sizeof(struct sockaddr))) == -1 ) 
  	{
    	        perror("Error al enviar mensaje con: sendto"); 
    	        exit(1);
  	} 

  	printf("\tenviados %ld bytes hacia %s\n",numbytes,inet_ntoa(server.sin_addr));
	
	
	// Para manejo de comandos
	char comando[25];
	char param1[25];	
  
	//Que contiene buf??
	sscanf(msg,"%s %s",comando, param1);	// Tiene forma de comando para preparar el archivo donde se guardara lo recibido
	if(strcmp(comando,"cp")==0)
	{
	    printf("\tComando:%s Parámetro:%s",comando, param1);	// Imprime comando y parámetros
	    
	    //Hay que crear el archivo	    
	    FILE *archivo;
	    
	    SEGMENT seg;

	    
	    strcat(param1,".copia");
 
	    if ((archivo = fopen(param1,"wb"))== NULL){		// No se pudo crear el archivo.
                 printf("\n\tError: no se pudo crear el archivo: %s. \n",param1);
		 
	    }	 
	    else{	// Archivo creado OK
 		printf("\n\tArchivo %s creado y listo para recibir...\n",param1);
		int i=0;
		char * data=0;;
		int datalen=0;
		
		do{
		  
		  /* Sirve para limpiar el buf */
		  memset (buf, '\0', MAXBUFLEN);
		  
		  /* Se reciben los datos enviados del servidor */
		  if (( numbytes = recvfrom(sockfd, buf, sizeof(SEGMENT), 0, (struct sockaddr *)&server, &addr_len)) == -1 ) 
		  {
		          perror("\n\tError: No se pudo realizar la recepción de datos con: recvfrom\n"); 
		          exit(1);
		  } 

                  /* El cliente espera 1 segundo para continuar con el proceso de recibir paquetes */
		  sleep(1);
		  		 
		  printf("\nlongitud del paquete en bytes : %ld",numbytes);
		  
		  //buf[numbytes] = '\0'; //      No se ocupa	
		  	  		  
		   /* Se convierten los datos a host recibidos por la red */
		  seg.segNum = ntohs(((SEGMENT *)buf)->segNum);
		  seg.control = ntohs(((SEGMENT *)buf)->control);
		  seg.lineLen = ntohs(((SEGMENT *)buf)->lineLen);   
		  
		  
		  
printf("\n\t[%ld] Segmento recibido del servidor:[%d], Control:[%d]\t\n",seg.lineLen,seg.segNum,seg.control);
		  
		  if(seg.control==0){
		    //fputs(seg.line, archivo);
		    fwrite( ((SEGMENT *)buf)->line,1,seg.lineLen,archivo);
		  }
		  else if(seg.control==2)
		  {
		    printf("\n\tArchivo no encontrado en el servidor...\n");
		    break;
		  }
		  else if(seg.control==1)
		  {
		    printf("\n\tFin de archivo recibido en el segmento:[%d], [%ld]\n",seg.segNum,seg.lineLen);
		    break;
		  }		  
		}while (seg.control==0);
		
	    }
	    fclose(archivo);
	
	}
	
	else{// No es un comando válido, simplemente imprime lo que recibe del servidor de echo.
	
	  if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr *)&server, &addr_len)) == -1) 
	  {
		perror("\n\terror en recvfrom"); 
    		exit(1);
	  } 

	  /* Se visualiza lo recibido */ 
	  printf("\n\tpaquete proveniente de : %s\n",inet_ntoa(server.sin_addr));
	  printf("\n\tlongitud del paquete en bytes : %ld\n",numbytes);
	  buf[numbytes] = '\0'; 
	  printf("\n\tel paquete contiene : %s\n",buf);
  
	}
	
}while(strcmp("salir",msg));
  
  close(sockfd);
  exit(1);

} 
