#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <crypt.h>

using namespace std;

int main(int argc, char const *argv[])
{
	if(argc!=6){
		cout<<"user : enter the correct arguments"<<endl;
		exit(0);
	}
	int sockfd;
	sockaddr_in dest_addr; // will hold the destination addr
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // do some error checking!
	if(sockfd==-1){
		perror("Error in Socket");
		exit(1);
	}
	dest_addr.sin_family = AF_INET;
	// host byte order
	dest_addr.sin_port = htons(atoi(argv[2]));
	// short, network byte order
	dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
	memset(&(dest_addr.sin_zero), '\0', 8); // zero the rest of the struct
	// don’t forget to error check the connect()!

	if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in))==-1){
		perror("Error in Connect");
		exit(1);
	}	
	printf("user : connected to the server : %s\n",inet_ntoa(dest_addr.sin_addr));
	
	char msg[18];
	strcpy(msg,argv[3]);
	msg[13]=argv[4][0];
	strcat(msg,argv[5]);
	char m[20]="1";
	strcat(m,msg);
	cout<<"user : sending message : "<<m<<endl;

	int len, bytes_sent;
	len = strlen(m)+1;
	//cout<<"A"<<endl;
	if((bytes_sent = send(sockfd, m, len, 0))==-1){
		perror("Error in Sending");
		exit(1);
	}	
	char buf[1024];
	len=1024;
	int bytes_received;
	unsigned int flags =0;
	bytes_received =  recv(sockfd,buf,len-1	,flags);
	if(bytes_received==-1){
		perror("Error in Receiving");
		exit(1);	
	}
	if(bytes_received==0){
		cout<<"Connection Closed";
		exit(1);
	}
	buf[bytes_received]='\0';
	cout<<"user : recieved message : "<<buf<<endl;
	close(sockfd);
	return 0;
}