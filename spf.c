
/*****************************************************************************************************************
*Name  : Jayesh J 
*
*Graph  Problem
*
************************************************************************************************************/


#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/uio.h>
#include <errno.h>
void *client_chld(void *);
void *server_chld(void *);
void insert_node_db(char[16],char[16] , int );


/* Adjacency LinK  List  */
struct  Adjlist 
{
	char addr[16];
	struct  Adjnode  *  node;
	struct Adjlist * next ;
	struct  Adjlist * prev ;
};

struct Adjnode 
{
	char addr[16];
	int cost;
	struct Adjnode * next ;
	struct Adjnode * prev ;
};


/* Global Adjacency  link list head  */

struct  Adjlist  * li; 


/******************************************
*
*  MAIN API 
********************************************/ 


int main(int argc, char const *argv[])
{

	char buf[20];
	int cost ; 
	int port  ;   
	int port_server = atoi(argv[1]);
	int new_socket,server_fd;

	struct sockaddr_in ser_addr, cli_addr;
	struct  Adjlist  * list = NULL;
	struct  Adjnode  * node  = NULL; 
	char  buf_peer[16];


	node  =(struct Adjnode *) malloc(sizeof(struct Adjnode)+1);

	if (NULL  ==  node )
	{
		printf ("Memory  Allocation  Failure \n");
	}




	strcpy(node->addr, argv[2]);
	node->cost =  0;
	node->next = NULL;
	node->prev = NULL;

	list  =(struct Adjlist *) malloc(sizeof(struct Adjlist)+1);
	if ( NULL  == list )
	{
		printf  (" Memory Allocation Failure \n");
	}

	list->node = node;
	strcpy(list->addr ,node->addr);
	list->next = NULL;
	list->prev = NULL;

	rn  = node ;
	li  = list ;

	strcpy(buf_peer,argv[2]);
	FILE *fp = NULL ; /* File descriptor */
	FILE *fp1 = NULL;
	pthread_t chld_thr;
	pthread_t chld1_thr;
	char  filename[]= "adjacency.txt";   /* Intial Configuration file */

	fp = fopen(filename,"r");
	fp1 = fopen(filename, "r");
	if ( NULL  == fp )
	{
		printf("FILE Read Error : Unable  to access adjacency.txt");
	}

	/* Acting as reciever  **************************/
	printf ("%s %d ",argv[2],port_server);
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = inet_addr(argv[2]);
	ser_addr.sin_port = htons(port_server);

	pthread_create(&chld1_thr, NULL, server_chld, (void *)(&ser_addr));
	while (fscanf(fp1, "%s %d %d", buf, &cost, &port) != EOF)
	{
		insert_node_db(buf_peer,buf,cost);
	}
	sleep(5);
	while (fscanf(fp, "%s %d %d", buf, &cost, &port) != EOF)
	{
		printf("%s %d\n",buf,cost,port);
		insert_node_db(buf_peer,buf,cost);
		memset(&cli_addr, '0', sizeof(cli_addr));

		cli_addr.sin_family = AF_INET;
		cli_addr.sin_port = htons(port);
		if(inet_pton(AF_INET, buf, &cli_addr.sin_addr)<=0)
		{
			printf("\nInvalid address/ Address not supported \n");
			return -1;
		}

		pthread_create(&chld_thr, NULL,client_chld, (void *)(&cli_addr));
	}
	pthread_join(chld_thr,NULL);
	pthread_join(chld1_thr,NULL);
}



/******************************************************************************************************/


/***********************************************************************
*
*   THREAD IMPLEMENTAION 
************************************************************************/


void *server_chld(void *arg)
{
	int server_fd, new_socket, valread;
	struct sockaddr_in * ser_addr =  arg;
	char data[1024];
	char frombuf[16];
	char tobuf[16];
	int cost;
	int nAddrSize = sizeof((*ser_addr));
	int opt = 1;
	printf( "Server");
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		pthread_exit(NULL);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				&opt, sizeof(opt)))
	{
		perror("setsockopt");
		pthread_exit(NULL);
	}
	if (bind(server_fd, (struct sockaddr *)ser_addr,
				sizeof((*ser_addr)))<0)
	{
		perror("bind failed");
		pthread_exit(NULL);
	}
	if (listen(server_fd, 5) < 0)
	{
		perror("listen");
		pthread_exit(NULL);
	}
	for(;;){

		if ((new_socket = accept(server_fd, (struct sockaddr *)ser_addr,
						&nAddrSize))<0)
		{
			perror("accept");
		}

		read(new_socket, data, 1024);

		while (sscanf(data, "%s %s %d", frombuf, tobuf, &cost) == 3)
		{
			insert_node_db(frombuf,tobuf,cost);
		}


		close(new_socket);



	}
}








void *client_chld(void *arg)
{
	int server_fd, new_socket, valread,sock;
	struct sockaddr_in  * cli_addr = arg;
	struct sockaddr_in serv_addr;
	char buffer[1024];
	int ret; 
	struct  Adjnode  *  temp;
	struct Adjlist *  t ;
	int ln ;
	t = li ;
	temp = li->node->next;

	struct timeval timeout;      
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	while(t != NULL)
	{
		temp = t->node->next;
		while ( temp != NULL)
		{
			sprintf(buffer + ln,"%s %s %d ",t->addr,temp->addr,temp->cost);
			temp = temp->next;
			ln = strlen(buffer);
		}
		t= t->next;
	}


	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		pthread_exit(NULL);
	}
	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
				sizeof(timeout)) < 0)
		error("setsockopt failed\n");

	if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
				sizeof(timeout)) < 0)
		error("setsockopt failed\n");

	if (connect(sock, (struct sockaddr *)cli_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		printf("Oh, something went wrong with connect()! %s\n", strerror(errno));
		close(sock);
		pthread_exit(NULL);

	}



	write(sock,buffer,1024);


	close(sock);
	pthread_exit(NULL);


}






/************************************************************ API  FOR  DATASTRUCTURE UPDATAIon********************************/



void insert_node_db(char frombuf[16] ,char tobuf[16] , int  cost) 
{
	struct Adjnode * node = NULL; 
	struct Adjlist * temp = li;
	struct Adjnode * temp1 = NULL;
	struct Adjlist *temp2 = NULL;
	struct Adjnode *node3 = NULL;
	struct Adjlist * node1 = NULL;
	node  = (struct Adjnode *)malloc(sizeof(struct Adjnode)+1);
	if (NULL  ==  node )
	{
		printf ("Memory  Allocation  Failure \n");
	}
	strcpy(node->addr,tobuf); 
	node->cost = cost;
	node->next = NULL; 
	node->prev = NULL; 

	while (temp != NULL)
	{
		if ( strcmp(temp->addr,frombuf) == 0)
		{ 
			temp1 = temp->node;
			while( temp1->next != NULL ) 
			{
				if (strcmp(node->addr , temp->node->addr)== 0)
				{
					printf("Cost updated from  %d  to %d  for link  %s - %s",temp->node->cost,cost,frombuf,tobuf);
					temp->node->cost  = cost; 

					free(node);
					break;
				}
				temp1 = temp1->next;
			}
			if(temp1->next == NULL) 
			{
				if (strcmp(node->addr ,temp->node->addr) == 0)
				{
					temp->node->cost  = cost;

					free(node);
					break;
				}

				temp1->next = node;
				node->prev = temp1;
				break ;
			}
		}
		else
		{
			printf("Not going inside ");
		}

		temp2 =temp ;
		temp = temp2->next;
	}

	if ( temp == NULL)
	{

		node1  = (struct Adjlist *)malloc(sizeof(struct Adjlist)+1);
		if (NULL  ==  node1 )
		{
			printf ("Memory  Allocation  Failure \n");
		}
		strcpy(node1->addr, frombuf);
		node3  = (struct Adjnode *)malloc(sizeof(struct Adjnode)+1);
		if (NULL  ==  node3 )
		{
			printf ("Memory  Allocation  Failure \n");
		}
		strcpy(node3->addr, frombuf);
		node3->cost = 0 ;

		node3->next=node;
		node->prev = node3;
	}

	temp1  = li->node ;
	while ( temp1 != NULL ) 
	{
		printf (" %d  %s---- to %s \n", temp1->cost ,li->node->addr, temp1->addr);
		temp1 = temp1->next;
	}

}

  


   
       

      

    

              

