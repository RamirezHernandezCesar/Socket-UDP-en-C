/*
Autor: Cesar Ramirez Hernandez 
----------Servidor UDP------------

Forma de compilar:
gcc servidor.c -o servidor

Forma de correr:
./servidor

NOTA: El servidor correra en la ip de la maquina donde se este ejecutando y el puerto donde escuchara sera el 4950.
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>


#define MYPORT 4950       /* Puerto donde el cliente envia los paquetes */ 
#define MAXBUFLEN 50000   /* Max. cantidad de bytes que podra recibir en una llamada a recvfrom() */ 


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
  int port;

  struct sockaddr_in server;	    /* Estructura donde se almacenara la direccion IP y numero de puerto del servidor */ 

  struct sockaddr_in cliente;      /* Estructura donde se almacenara la direccion IP y numero de puerto del cliente */ 


  /* addr_len contendra el tamanio de la estructura sockadd_in y numbytes el numero de bytes recibidos */ 
  int addr_len;
  long int numbytes; 
  char buf[MAXBUFLEN];	/* Buffer de recepcion */ 
  char msg[MAXBUFLEN]; /* Buffer de mensaje   */

  // Para manejo de comandos
  char comando[25];
  char param1[25];
  
  /* Tratamiento de la linea de comandos. */
  if (argc < 2)
  {
    /* No especificó el puerto a escuchar. */
    port=MYPORT;
  }
  else
  {
    /* Asigna a Port el Puerto especificado. */
    port=atoi(argv[1]);
  }
  
  /* se crea el socket */ 
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  { 
    perror("error al crear el socket"); 
    exit(1);
  } 

  /* Se establece la estructura server con la direccion ip y puerto del servidor */ 
  server.sin_family = AF_INET;           /* Familia del socket */ 
  server.sin_port = htons(port);         /* Puerto convertido a network byte order (BigEndian) */ 
  server.sin_addr.s_addr = INADDR_ANY;   /* Se asigna automaticamente la direccion IP local (servidor) */ 
  bzero(&(server.sin_zero), 8);          /* Se rellena con ceros el resto de la estructura */

  /* Se le da un nombre (IP y Puerto) al socket*/ 
  if (bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) 
  {
    perror("error en el bind"); 
    exit(1);
  } 
  
  
  printf("Servidor a la escucha por el puerto: %d\n",ntohs(server.sin_port));
  
  /* Se reciben los datos */ 
  addr_len = sizeof(struct sockaddr);   
  char buffer[MAXBUFLEN];
  
  

  do{
  
        /* El servidor esta esperando la conexion de un cliente */        
  	if (( numbytes = recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr *)&cliente, &addr_len)) == -1 ) 
  	{
    		perror("error en recvfrom"); 
    		exit(1);
  	} 

  	/* Se visualiza lo recibido */ 
  	buf[numbytes] = '\0'; 
  	printf("\nMensaje Recibido: [%s]",buf);
	printf("\n\tMensaje proveniente de : %s:%d",inet_ntoa(cliente.sin_addr), ntohs(cliente.sin_port));
   	printf("\n\tLongitud del paquete en bytes : %ld\n",numbytes);
  	
	
	//Que contiene buf??
	sscanf(buf,"%s %s",comando, param1);	// Tiene forma de comando para iniciar la descarga 
	if(strcmp(comando,"cp")==0)
	{
	    int i=0;
	    char * data=0;
	    int datalen=0;
		
	    printf("\n\tComando Detectado:%s Parámetro:%s",comando, param1);	// Imprime el comando y el nombre del archivo a descargar
	    
	    //Hay que abrir y envíar línea por línea el contenido del archivo de nombre param1
	    
	    FILE *archivo;
	    SEGMENT seg;
	    	    
 
	    if ((archivo = fopen(param1,"rb")) == NULL){  // Si no se pudo abrir el archivo --> Se avisa al cliente (control=2)
                 
                 printf("\n\tError: no se pudo abrir el archivo: %s. \n",param1);
                 		 
		 seg.line[0]=0;		 
		 seg.segNum=0;
		 seg.control=htons(2);
		 seg.lineLen=0;
		 printf("\n\tEnviando Indicador de archivo no encontrado (control):[%d]\n",ntohs(seg.control));
		  
		 data=(char*)&seg;
		 datalen=sizeof(SEGMENT)-sizeof(seg.line)+strlen(seg.line);
		
		 if (( numbytes = sendto(sockfd, data, datalen, 0, (struct sockaddr *)&cliente, sizeof(struct sockaddr))) == -1 ) 
		 {
		    perror("\n\tError al enviar indicador de archivo no encontrado: sendto\n"); 
		    exit(1);
		 }
		 
	    }
	    else{	// Se pudo abrir el archivo
 		printf("\n\tArchivo %s abierto y listo para enviar...\n",param1);
							       
				
		while ( !feof(archivo) )
		{		
		  seg.lineLen = fread(seg.line,1,MAXBUFLEN,archivo);	/* Se guarda el numero de bytes que leyo la funcion */	  
		  seg.segNum=i;
		  seg.control=0;
		  //seg.lineLen=strlen(seg.line);
	printf("\n\tEnviando segmento al cliente:[%d], Control [%d], \n\t[%ld]",seg.segNum,seg.control,seg.lineLen);
		  
		  
		  /* Se convierten los datos a network byte order para ser transmitidos por la red */
		  seg.segNum=htons(seg.segNum);
		  seg.control=htons(seg.control);
		  seg.lineLen=htons(seg.lineLen);		  
		  data=(char*)&seg;
		 
		  /* Se calcula el tamaño del paquete a enviar */
		  datalen=sizeof(SEGMENT);
		  
		  printf("\n[%d] datalen=%d\n",i,datalen);
		  
		  /* Se envia el paquete al cliente destino */		  
		  if (( numbytes = sendto(sockfd, data, datalen, 0, (struct sockaddr *)&cliente, sizeof(struct sockaddr))) == -1 ) 
		  {
		    perror("\n\tError al enviar archivo: sendto\n"); 
		    exit(1);
		  }
		  
		  /* El servidor espera 1 segundo para continuar con el proceso de envio de paquetes */
		   sleep(1);
		  
		  i++;		  
		}
		
		
		seg.line[0]=0;
		seg.control=htons(1); /* La variable se asigna en 1 para indicar al cliente que termino de enviarse el archivo */
		seg.segNum=0;
		seg.lineLen=0;
		printf("\n\tEnviando Indicador de fin del archivo (control):[%d]\n",ntohs(seg.control));
		  
		data=(char*)&seg;
		datalen=sizeof(SEGMENT)-sizeof(seg.line)+strlen(seg.line);
		
		/* Se envia el ultimo paquete para indicar que se termino de enviar el archivo */
		if (( numbytes = sendto(sockfd, data, datalen, 0, (struct sockaddr *)&cliente, sizeof(struct sockaddr))) == -1 ) 
		{
		    perror("\n\tError al enviar EOF: sendto\n"); 
		    exit(1);
		}
		fclose(archivo);
	    }
	    	
	}	
	else{// No es un comando válido, actua como servidor de echo.
	
	
	  sprintf(buffer,"ip:%s puerto:%d msg:%s",inet_ntoa(cliente.sin_addr),ntohs(cliente.sin_port),buf);
	  	
	  if (( numbytes = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&cliente, sizeof(struct sockaddr))) == -1 ) 
	  {
	    perror("Error al enviar mensaje con: sendto"); 
	    exit(1);
	  }

	  printf("Enviados: %ld bytes:--%s--\n",numbytes, buffer);
	  
	}
	
	
  }while(strcmp(buf,"salir"));
  /* devolvemos recursos al sistema */ 
  
  close(sockfd); 

} 
