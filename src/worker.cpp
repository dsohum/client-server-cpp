#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <pthread.h>


using namespace std;
int sockfd;

void  generate(char *str,int length, char * hash,char* salt,char * passwd,bool lower, bool upper,bool numeric){
	if(strlen(str)==length){
		if(!strcmp(hash,crypt(str,salt))){
			strcpy(passwd,str);
		}
	}
	else if(strlen(str)<length){
		char c;
		if(numeric) c='0';
		else if(upper) c='A';
		else if(lower) c='a';

		while(1){ 
			int l=strlen(str);
			str[l]=c;
			str[l+1]='\0';
			//cout<<l+1<<" "<<str<<" "<<endl;
			generate(str,length,hash,salt,passwd,lower,upper,numeric);
			str[l]='\0';
			if(lower&&c=='z')break;
			if(upper&&(!lower)&&c=='Z')break;
			if(numeric&&(!lower)&&(!upper)&&c=='9')break;
			if(upper&&c=='9') c='A';
			else if((lower&&c=='Z')||lower&&(!upper)&&c=='9') c='a';
			else c++;
		}
	}
}
void *solve(void* arg){
	char *buf=(char*)(arg)+1;
	cout<<"Message in thread"<<buf<<endl;
	char start=buf[17]-32,end =buf[18]-32;
	bool lower=(buf[14]=='1');
	bool upper=(buf[15]=='1');
	bool numeric=(buf[16]=='1');
	cout<<"start: "<<int(start)<<endl;
	cout<<"end: "<<int(end)<<endl;
    /* determining the start and end character*/
    int type=(buf[14]-'0')*4+(buf[15]-'0')*2+(buf[16]-'0');       
    switch(type)
    {   case 0: start=end='0';break;
		case 1: start=start+'0'; end=end+'0'; break;
		case 2: start=start+'A'; end=end+'A'; break;
		case 3: if(end<10) {start=start+'0'; end=end+'0'; }
 				else if(start>10) {start=start+'A'-10; end=end+'A'-10; }
 				else  {start=start+'0'; end=end+'A'-10; }	
		        break;
		case 4: start=start+'a'; end=end+'a'; break;
		case 5: if(end<10) {start=start+'0'; end=end+'0'; }
				else if(start>10) {start=start+'a'-10; end=end+'a'-10; }
				else {start=start+'0'; end=end+'a'-10; }		
				break;
		case 6: if(end<26) {start=start+'A'; end=end+'A'; }
				else if(start>26) {start=start+'a'-26; end=end+'a'-26; }
				else {start=start+'A'; end=end+'a'-26; }		
		        break;
		case 7: if(end<10) {start=start+'0'; end=end+'0'; }
		        else if(start<10)
		        {  if(end<36) {start=start+'0'; end=end+'A'-10; }	
		           else       {start=start+'0'; end=end+'a'-36; }	 
		        }	
		        else if(start<36)
		        {  if(end<36) {start=start+'A'-10; end=end+'A'-10; }	
		           else       {start=start+'A'-10; end=end+'a'-36; }	 
		        }	
		        else {start=start+'a'-36; end=end+'a'-36; }		
		        break;
    }
    /**/	
	int length=int(buf[13])-48;
	char sock[5];
	memcpy(sock,&buf[20],4);
    cout<<buf+20<<"endl"<<endl;
	sock[4]='\0';
	char hash[14],salt[3];
	memcpy(hash, &buf[0],13);
	hash[13] = '\0';
	memcpy(salt,&buf[0],2);
	salt[2]='\0';
	char passwd[13]="";
	char c=start; 
	int len=0,bytes_sent=0;
	cout<<"Hash: "<<hash<<endl;
	cout<<"Length: "<<length<<endl;
	cout<<"salt: "<<salt<<endl;
	cout<<"start: "<<start<<endl;
	cout<<"end: "<<end<<endl;
	cout<<"socket fd : "<<sock<<endl;
	while(c!=end){
		char str[9];
		str[0]=c;
		str[1]='\0';
		generate(str,length,hash,salt,passwd,lower,upper,numeric);
		if(upper&&c=='9') c='A';
		else if((lower&&c=='Z')||lower&&(!upper)&&c=='9') c='a';
		else c++;		
	}
	if(passwd[0]!='\0'){
		char m[20]="0";
		strcat(m,"1");
		strcat(m,passwd);
		strcat(m,sock);
		len=strlen(m)+1;
		cout<<"worker : sending message : "<<m<<"Length :"<<len<<endl;
		if((bytes_sent = send(sockfd, m, len, 0))==-1){
			perror("Error in Sending");
			exit(1);
		}
	}
	else{
		char m[20]="00";
		strcat(m,sock);
		len=strlen(m)+1;
		cout<<"worker : sending message : "<<m<<endl;
		if((bytes_sent = send(sockfd, m, len, 0))==-1){
			perror("Error in Sending");
			exit(1);
		}
	}


}

int main(int argc, char const *argv[])
{
	if(argc!=3){
		cout<<"Enter corect arguments !!!"<<endl;
		exit(1);
	}
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
	char msg[2]="0";
	int len,bytes_sent;
	len=strlen(msg);
	cout<<"worker : sending message : "<<msg<<"\nLength:"<<len<<endl;
	if((bytes_sent = send(sockfd, msg, len, 0))==-1){
		perror("Error in Sending");
		exit(1);
	}	
while(1){				//loop for multiple passwords
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
		cout<<"worker : recieved message : "<<buf<<endl;

		//call thread to process and send the answer use recieve to recieve stop signal
		/*	
		*/
		int i;
		pthread_t p;
		i=pthread_create(&p,NULL,solve,(void*)buf);
		if(i){
			perror("Error in creating thread");
			exit(1);
		}
		pthread_join(p,NULL);

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
		if(!(strcmp(buf,"s"))){
			pthread_cancel(p);
		}
		cout<<"worker : recieved message : "<<buf<<endl;		
}
	// char salt[3],hash[14];
	// int length;
	// salt[0]=buf[0];
	// salt[1]=buf[1];
	// salt[2]='\0';
	// for(int i=0;i<13;i++){
	// 	hash[i]=buf[i];
	// }
	// hash[13]='\0';
	// length=int(buf[13])-48;
	// //cout<<hash<<" "<<length<<endl;
	// char str[5]="";
	// char passwd[5]="";
	// generate(str,length,hash,salt,passwd);
	// cout<<passwd;
	// char *msg = "Beej was here!";
	// int len, bytes_sent;
	// len = strlen(msg);
	// cout<<"A"<<endl;
	// if((bytes_sent = send(new_fd, msg, len, 0))==-1){
	// 	perror("Error in Sending");
	// 	exit(1);
	// }


	close(sockfd);
	// size_t size=1023;
	// char c[1024];
	// c[1023]='\0';
	// if(!gethostname(c,size)){
	// 	cout<<c<<endl;
	// }
	return 0;
}