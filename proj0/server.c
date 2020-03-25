#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>

//1byte = 8 bit.
//char 8bit
//int 32bit
//long long 64bit
#define MAX_LENGTH 10000000 //unit is byte. 10MB
#define MAX_PACKET_SIZE 9999992
#define MALLOC_SIZE 1000*1000*10 //10MB malloc

struct packet {
	unsigned char op; //8bit
	unsigned char shift; //8bit;
	short checksum; //16bit;
	unsigned int length; //32bit;
	char* string; //data;
};

unsigned short checksum2(const char *buf, unsigned size);

void main (int argc, char **argv) {

	int c; //argument

	char *serverIPAddress = "127.0.0.1"; 
	// char *clientIPAddress;
  char *portNumber; //p
  int encrypt; //o
  int shift; //s

  char* buffer;
  char* written_buffer;
  int numbytes;
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;
	struct addrinfo *clientinfo;
  int index;

  while ((c = getopt (argc, argv, "p:")) != -1) {
    switch (c)
      {
      case 'p':
        portNumber = optarg;
        break;
      default:
      	// printf("other language %s \n", c);
      	perror('wrong inputs');
        abort ();
      }
  }

  for (index = optind; index < argc; index++)
    fprintf(stderr, "Non-option argument %s\n", argv[index]);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(serverIPAddress, portNumber, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  int socket_fd;	  
  int client_fd;
  int client_len = sizeof(clientinfo);

  socket_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  
  if (bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
  	perror("already used portNum \n");
  	exit(1);
  }

  if (listen(socket_fd, 5) == -1 ) {
  	perror("socket listen error \n");
  	exit(-1);
  };

  while(1) {
	  // printf("ready for listen \n");

    int pid;

    client_fd = accept(socket_fd, (struct addrinfo*)&clientinfo, &client_len);
    if (client_fd <= -1) {
    	perror("accept error");
    	exit(-1);
    }

    if ((pid = fork()) == 0) {
    	//parent
	    unsigned int length;
	    unsigned int recv_len = 0;
	    int received_checksum = 0;
    	int recv_cnt = 0;
    	int check;
    	// printf("before get data \n");

    	buffer = malloc(MALLOC_SIZE);
    	
      while(recv_len = read(client_fd, buffer, MALLOC_SIZE ) >0) {
		    // recv_len = recv(client_fd, &buffer[0], MALLOC_SIZE,0);
		    recv_len = strlen(buffer + 8) + 8;		    
		    memcpy(&encrypt, buffer, 1); //op
		    memcpy(&received_checksum, buffer+2, 2);
		    memcpy(&shift, buffer + 1, 1); //shift msg+1byte부터 시작함.
		    written_buffer = malloc(recv_len);
		    memset(written_buffer, 0, recv_len );
		    printf("received_checksum : %d \n", received_checksum);

		    caesar_cipher(written_buffer + 8, buffer+8, encrypt, shift, recv_len - 8);
		    
		    printf("length : %d \n, encrypt : %d \n shift: %d \n, msg: %s \n",recv_len ,encrypt, shift, written_buffer + 8);
		    
		    unsigned short checksum = checksum2(written_buffer + 8, recv_len);
		    memcpy(written_buffer+2, &checksum, 2);
		    printf("check sum : %d \n", checksum);

		    check = send(client_fd, written_buffer, htonl(recv_len), 0);

		    memset(buffer, 0, MALLOC_SIZE);
		    free(written_buffer);
    	}
    	free(buffer);
	    close(client_fd);
    } else if (pid == -1) {
    	perror('pid error');
    } else {
    	// printf("child go next listening \n");
    }
  }

  // printf("\n end? \n\n");
  // fputs(buffer+8 , stdout);
  // fflush(stdout);
  close(socket_fd);
  printf("buffer? %s \n", buffer);
	freeaddrinfo(servinfo);
  return 0;
}


void caesar_cipher (char * written_buffer, char * buffer, unsigned int encrypt, int shift, int recv_len) {
	
	char text;
	shift = shift%26;

	if (encrypt == 0) {
		// printf("encrypt 0!! \n");
		for (int i = 0; i < recv_len; i++) {
			text = tolower(buffer[i]);
			if (text < 123 && text > 96) {
				text = text + shift;
				if (text >= 123) {
					text = text - 26 ;
				} else if(text <= 96) {
					text = text + 26 ;
				}
				written_buffer[i] =  text;
				
			} else {
				written_buffer[i] = text;
			}
		}
	} else if (encrypt == 1) {
		// printf("encrypt 1!! \n");
		for (int i = 0; i< recv_len; i++) {
			text = tolower(buffer[i]);
			if (text < 123 && text > 96) {
				text = text - shift;
				if (text >= 123) {
					text = text - 26 ;
				} else if (text <= 96) {
					text = text + 26 ;
				}
				written_buffer[i] =  text;
			} else {
				written_buffer[i] =  text;
			}
		}
	}
}



unsigned short checksum2(const char *buf, unsigned size)
{
	unsigned long long sum = 0;
	const unsigned long long *b = (unsigned long long *) buf;

	unsigned t1, t2;
	unsigned short t3, t4;

	/* Main loop - 8 bytes at a time */
	while (size >= sizeof(unsigned long long))
	{
		unsigned long long s = *b++;
		sum += s;
		if (sum < s) sum++;
		size -= 8;
	}

	/* Handle tail less than 8-bytes long */
	buf = (const char *) b;
	if (size & 4)
	{
		unsigned s = *(unsigned *)buf;
		sum += s;
		if (sum < s) sum++;
		buf += 4;
	}

	if (size & 2)
	{
		unsigned short s = *(unsigned short *) buf;
		sum += s;
		if (sum < s) sum++;
		buf += 2;
	}

	if (size)
	{
		unsigned char s = *(unsigned char *) buf;
		sum += s;
		if (sum < s) sum++;
	}

	/* Fold down to 16 bits */
	t1 = sum;
	t2 = sum >> 32;
	t1 += t2;
	if (t1 < t2) t1++;
	t3 = t1;
	t4 = t1 >> 16;
	t3 += t4;
	if (t3 < t4) t3++;

	return ~t3;
}