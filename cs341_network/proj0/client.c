#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
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


char *read_user_input (void);
unsigned short checksum2(const char *buf, unsigned size);

void main (int argc, char **argv) {

	int c; //argument

	char *ipAddress; //h
  char *portNumber; //p
  uint8_t encrypt; //o
  int shift; //s

  char* input_buffer;
  char* output_buffer;

  int numbytes;

  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;

  int index;

  opterr = 0;

  while ((c = getopt (argc, argv, "h:p:o:s:")) != -1) {
    switch (c)
      {
      case 'h':
        ipAddress = optarg;
        break;
      case 'p':
        portNumber = optarg;
        break;
      case 'o':
      	encrypt = (uint8_t) atoi(optarg);
        break;
      case 's':
      	shift = atoi(optarg);
        break;
      default:
      	perror("other language \n");
        abort ();
      }
  }



  for (index = optind; index < argc; index++)
    fprintf(stderr, "Non-option argument %s\n", argv[index]);
    // printf ("Non-option argument %s\n", argv[index]);
  
  //connect to server.

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(ipAddress, portNumber, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  int socket_fd;
  int check;
  socket_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen);
  check = connect(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen);
  // printf("connect : %d \n", check);

  if (check <= -1) {
    perror("wrong connection \n");
    exit(-1);
  }

  while(1) {
    unsigned long long size = MALLOC_SIZE;
    input_buffer = malloc(size);
    output_buffer = malloc(size);
    if (fgets(input_buffer, MALLOC_SIZE - 8, stdin) == NULL) {
      // fprintf(stderr, "buffer input error\n", gai_strerror(status));
      // exit(-1);
      break;
    }

    //create packet. 
    //shift, encrypt, buffer를 쓰자.
    unsigned char *msg;
    unsigned int length;

    msg = (unsigned char *) malloc(8 + strlen(input_buffer));
    memcpy(msg, &encrypt, 1); //op
    memcpy(msg + 1, &shift, 1); //shift msg+1byte부터 시작함.
    length = htonl(strlen(input_buffer) + 8);
    memcpy(msg + 4, &length, 4);
    memcpy(msg + 8, input_buffer, strlen(input_buffer));
    unsigned short checksum = checksum2(msg, htonl(length));
    printf("%d \n", checksum);
    memcpy(msg+2 , &checksum, 2);

    // printf("message 12byte : %s\n", msg);
    // printf("message 12byte : %s\n", msg+8);

    check = send(socket_fd, msg, htonl(length), 0);

    //윤성우 열혈 TCP/IP 소켓 프로그래밍 Chapter 05-1 P.126

    unsigned int recv_len = 0;
    int recv_cnt = 0;

    while(recv_len < htonl(length)) {
      if((recv_cnt = read(socket_fd, &output_buffer[recv_len], htonl(length))) <= 0) {
        // perror("recv");
        exit(-1);
      }
      recv_len += recv_cnt;
    }
    // printf("server received\n");

    int received_checksum = 0;
    received_checksum = checksum2(output_buffer, htonl(strlen(output_buffer)));
    printf("\n received checksum :%d \n ",received_checksum);
    if (received_checksum != 0 ) {
      fprintf(stderr, "\nchecksum error\n");
      printf("\n output_buffer! : %s and %s \n", msg, output_buffer);
      exit(-1);
      break;
    } 


    output_buffer[recv_len] = '\0';

    printf("input_buffer? \n %s \n", input_buffer);
    printf("output_buffer? \n%s\n", output_buffer + 8);

    free(msg);
    free(output_buffer);
    free(input_buffer);
  }

  // fputs(buffer+8 , stdout);
  // fflush(stdout);
  close(socket_fd);
	freeaddrinfo(servinfo);
  return 0;
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