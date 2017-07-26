#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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


int main(int argc, char const *argv[])
{

	char buf[20];
        int new_socket,server_fd;

        char  buf_peer[16];

	int status;
	struct addrinfo addrs_info,addrc_info;
	struct addrinfo *servinfo, *clientinfo;


	FILE *fp = NULL ; /* File descriptor */
        FILE *fp1 = NULL;
        char  filename[]= "adjacency.txt";   /* Intial Configuration file */

	/* Asjancey  Link-List */
        struct  Adjlist  * list = NULL;
        struct  Adjnode  * node  = NULL;


	pthread_t client_thr[10];
        pthread_t server_thr;

	int i = 0;

        char buf_addr[20];
        char port_addr[10] ;
        int port,cost;
	if (argc  != 2)
        {
                printf("Please  provide  valid  inputs \n");
                exit(1);
        }




 	/*Creation  of  intial node /root node for adjacency  linklist  */

      	node  =(struct Adjnode *) malloc(sizeof(struct Adjnode)+1);

        if (NULL  ==  node )
        {
                printf ("Memory  Allocation  Failure \n");
        }

        strcpy(node->addr, argv[1]);
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

        li  = list ;
        
	strcpy(buf_peer,argv[1]);
	/* Creation of Server  Thread  */
	memset(&addrc_info,0,sizeof(addrs_info));

	addrs_info.ai_family = AF_INET;
	addrs_info.ai_socktype = SOCK_STREAM;
	
	if((status  =  getaddrinfo(argv[1],argv[2],&addrs_info,&servinfo)) !=0){
		fprintf(stderr, "getaddrinfo error : %s \n", gai_strerror(status));
		exit(1);
	}

	pthread_create(&server_thr, NULL, server_chld, (void *)(servinfo));

        /* Intilising file pointer */
	fp = fopen(filename,"r");
        fp1 = fopen(filename, "r");

        if (( NULL  == fp ) || (NULL == fp1))
        {
                printf("FILE Read Error : Unable  to access adjacency.txt");
        }

        /* Inserting  node  into  adjanceny link list */

        while (fscanf(fp1, "%s %d %d", buf, &cost, &port) != EOF)
        {
                insert_node_db(buf_peer,buf,cost);
        }

        sleep(5);
	while (fscanf(fp, "%s %d %s", buf, &cost, port_addr) != EOF)
	{
		printf("%s %d %s\n",buf,cost,port_addr);
		/* Creation of Server  Thread  */
		memset(&addrc_info,0,sizeof(addrc_info));
		

	        addrc_info.ai_family = AF_INET;
        	addrc_info.ai_socktype = SOCK_STREAM;

		if((status  =  getaddrinfo(buf,port_addr,&addrc_info,&clientinfo)) !=0){
			fprintf(stderr, "getaddrinfo error : %s \n", gai_strerror(status));
			exit(1);
		}



		pthread_create(&client_thr[i], NULL,client_chld, (void *)(clientinfo));
		i++;

		sleep(3);
		freeaddrinfo(clientinfo);
	}

//	pthread_join(client_thr,NULL);
//        pthread_join(server_thr,NULL);




}	


/*
******************************************************************************************
 * *
 * *                           SERVER THREAD
 * *
*******************************************************************************************
*/

void *server_chld(void *arg)
{
        int server_fd, new_socket, valread;
	struct sockaddr_storage client_addr;
	socklen_t addr_size = sizeof(client_addr);
	struct addrinfo *servinfo = arg;
        char data[1024];
        char frombuf[16];
        char tobuf[16];
        int cost;
        int opt = 1;
        printf( "Server");
        if ((server_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == 0)
        {
                perror("socket failed");
                pthread_exit(NULL);
        }

        if (bind(server_fd, servinfo->ai_addr,
                                servinfo->ai_addrlen)<0)
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

                if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr,
                                                &addr_size))<0)
                {
                        perror("accept");
                }

                if ((recv(new_socket, data, 1023,0)) == -1)
		{
			perror("recv");
		}

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
	struct addrinfo *cliinfo = arg;
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


        if ((sock = socket(cliinfo->ai_family, cliinfo->ai_socktype, cliinfo->ai_protocol)) < 0)
        {
                printf("\n Socket creation error \n");
                pthread_exit(NULL);
        }



        if (connect(sock, cliinfo->ai_addr , cliinfo->ai_addrlen) < 0)
        {
                printf("\nConnection Failed \n");
                printf("Oh, something went wrong with connect()! %s\n", strerror(errno));
                close(sock);
                pthread_exit(NULL);

        }



        send(sock,buffer,1024,0);


        close(sock);
        pthread_exit(NULL);


}



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


