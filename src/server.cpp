/** Server side code
*/
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<cstring>
#include<cstdlib>
#include<queue>
#include<list>
#include<map>
#include<math.h>
#include<fstream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
using namespace std;
int  PORT;// port on which server is listening
void initialize(fd_set &users, fd_set &free_workers, fd_set &engaged_workers, fd_set &all, struct sockaddr_in & myAddr, struct timeval & timeout)
{  FD_ZERO(&users);
   FD_ZERO(&free_workers);
   FD_ZERO(&engaged_workers);
   FD_ZERO(&all);
   timeout.tv_sec = 2;
   timeout.tv_usec = 500000;


   /* setting the IP address of the server from config file*/
   ifstream ifile;
   ifile.open("../config.txt");
   char MY_IP[16];
   ifile>>MY_IP;
   ifile.close();
   
   /* set up  addresss for binding */
   myAddr.sin_family=AF_INET;
   myAddr.sin_addr.s_addr=inet_addr(MY_IP);
   myAddr.sin_port = htons(PORT);
   memset(&(myAddr.sin_zero),'\0',8);
}

int main(int argc, char** argv)
{  
   if(argc<2|| argc>2)
   {  printf("Usage: ./server PORT-No"); 
      exit(0);}
   PORT=stoi(string(argv[1]),NULL,10);    
   /* set of file descriptors for users, workers(free and working) and all of them together conneted to the server */
   fd_set users, free_workers, engaged_workers, all;
   /* size of the fd_sets */
   int usersCount=0, free_workersCount=0, engaged_workersCount=0;
   /* temporary file descriptor list for select() */
   fd_set read_fds;
   FD_ZERO(&read_fds);
   struct timeval timeout;

   /* the max file descriptor number */
   int fd_max=0;

   /* listener and remote host socket descriptor */
   int listener, new_fd;
   
   /* server address */
   struct sockaddr_in myAddr;
   /* client address */
   struct sockaddr_in remoteAddr;
   socklen_t addrlen;
   
   initialize(users, free_workers, engaged_workers, all, myAddr, timeout);
   
   /* start listennig on the PORT */
   listener=socket(AF_INET, SOCK_STREAM,0);
   if(listener==-1)
   { perror("Error!");
	 printf("Error in creating socket\n");  exit(1); }
   /* for address aldready in use error message */
   int yes=1;
   if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,&yes, sizeof(int))==-1)
   {  perror("Error!");
	  printf("Error in setting up socket!\n"); }
   
   /* binding to the PORT */
   if(bind(listener, (struct sockaddr *) &myAddr, sizeof(myAddr))==-1)
   {  perror("Error!");
	  printf("Error in binding!\n"); exit(1);}
   
   printf("Success in binding to %s!\n",inet_ntoa(myAddr.sin_addr));
  
   printf("Server listening...\n");
  
   /* listen on PORT */
   if(listen(listener,10)==-1)
   {  perror("Error!");
	  printf("Error in listening!\n");  exit(1);}   
   
   
   /* add listener to set of all filedescriptors */
   FD_SET(listener,&all);
   fd_max=listener;
   
   unsigned int i, j, len=0;
   queue<pair<int, char*> > request;
   map<int,list<int> > request_state;
   list<int>::iterator it, it_end, it_begin;
   list<int> *r_s;
   char buffer[256];
   unsigned int flags=0, bytesReceived;         
   while(true)
   {  read_fds=all;  //copy
      
      if(select(fd_max+1,&read_fds, NULL, NULL, timeout)==-1)
      {  printf("Error in select()!\n"); exit(1);}
      
      /* first come first serve principle */
      if(free_workersCount>=engaged_workersCount&& free_workersCount>0&&!request.empty())
      { cout<<"In Queue\n";
        /* implement:            receive all message */
        pair<int,char*> *p=&request.front();
        request.pop();
        i=p->first;
        int x=i,y=0, start=0; 
        len=strlen(p->second);
        /* writing socketfd for 'user' as characters representing the number in host byte order */
        buffer[0]='\0';
        strcat(buffer,p->second);
        for(int k=len+3;k<=len+6;k++){buffer[k]=x%(1<<8)+32; x=x>>8;}
        buffer[len+1]='u';
		buffer[len+2]=' ';
		buffer[len+7]='\0';
		buffer[len]='l';
		cout<<"socket fd"<<i<<endl;
		cout<<"Buffer from user in queue: "<< buffer<<endl;
        
        /* division of work amongst the workers */
        start=0;
		/* setting step size x and total search domain length y */
        x=ceil(((buffer[len-1]-'0')*10.0+(buffer[len-2]-'0')*26.0+(buffer[len-3]-'0')*26.0)/(free_workersCount));
        y=(buffer[len-1]-'0')*10.0+(buffer[len-2]-'0')*26.0+(buffer[len-3]-'0')*26.0;
        r_s=new list<int>;
        for(int j=0;j<=fd_max&&free_workersCount>0;j++)
        {  if(FD_ISSET(j,&free_workers))
           {  /* setting start and end character for the search position */
			  buffer[len]=(start)+32;
			  buffer[len+1]=min(y,start+x)+32;
			  start+=x;
			  cout<<"Message sent to worker: "<<buffer<<endl; 
			  
			  if(send(j,buffer,len+8,flags)==-1)
			  {  perror("Error!");printf("Error in sending message to socket %d!\n",j);}
			  /* allocating worker to user's request */
			  r_s->push_back(j);
			  /* managing worker status */
			  FD_CLR(j,&free_workers);
			  free_workersCount--;
			  FD_SET(j,&engaged_workers);
			  engaged_workersCount++;
           } 
           request_state[i]=*r_s;  
        }
      }

      /* run through all existing connections looking for the file descriptors */	
      for(i=0;i<=fd_max;i++)
      {  if(FD_ISSET(i, &read_fds))
         {  if(i==listener)
            {  /* handle new connection here */
               addrlen = sizeof(remoteAddr);
               new_fd = accept(listener, (sockaddr*)&remoteAddr, &addrlen);  
               if(new_fd==-1)
               {  perror("Error in accepting connection to remote host!"); }
               else
               {  FD_SET(new_fd, &all);
                  fd_max=max(fd_max,new_fd);
                  printf("SERVER: New connection from:%s on socket %d! :D\n", inet_ntoa(remoteAddr.sin_addr), new_fd);
               }	
            }
            else
            {  /*  handle data from workers and users */
               bytesReceived=recv(i,buffer, sizeof(buffer),flags); 
               /* problem in connection */
               if(bytesReceived<=0)
               { if(bytesReceived==0)
                 {  perror("Error!");printf("SERVER: Socket %d hung up! :P\n", i); }
                 else
                 {  perror("Error!");printf("Error in receiving data from socket %d\n",i); }	
                 /* closing connection from the socket and removing the socket from our record */
                 close(i);
                 cout<<"Socket closed: "<<i<<endl;
                 /* assuming that if worker is engaged he does not quit and client waits until receiving the result */
                 /* free the host from the records */
                 FD_CLR(i, &all);
                 if(FD_ISSET(i, &engaged_workers)) 
                 {  FD_CLR(i, &engaged_workers);
                 	engaged_workersCount--;
                 }
                 else if(FD_ISSET(i, &free_workers))	
                 {  FD_CLR(i, &free_workers);
                    free_workersCount--;
                 }
                 else if(FD_ISSET(i, &users))
                 {  FD_CLR(i, &users);
                    usersCount--; 
                 }
                 else
                 {  perror("Terminating unknown connecion!\n"); }
               }
               else
               {  cout<<"SERVER: Message Received: "<<buffer<<endl;
				  /* worker's message received */
               	  if(buffer[0]=='0')
               	  {  if(!FD_ISSET(i,&engaged_workers)&&!FD_ISSET(i,&free_workers))
               	  	 { FD_SET(i,&free_workers);
               	  	   free_workersCount++;
               	  	 }
               	  	 /* implement:            receive all message */
               	  	 else if(buffer[1]=='1')
               	  	 {  /* implement:            receive all message */
            			      /* stop all the other workers! */
                        cout<<"Message received from worker: "<<buffer<<endl;
                        unsigned int x=1;
                        j=0;
                        len=strlen(buffer);
                        for(int k=len-4;k<=len-1;k++) {  j+=(buffer[k]-32)*x; x=x<<8; }
                        it_end=request_state[j].end();
                        for(it=request_state[j].begin(); it!=it_end;it++)
                        {  if(send(*it, "s", 2, flags)==-1)  //sending the stop message "s"
                           {  printf("Error in sending to socket %d!\n",*it); }
                           FD_CLR(*it,&engaged_workers);
                           engaged_workersCount--;
                           FD_SET(*it,&free_workers);
                           free_workersCount++;
                           request_state[j].erase(it);
                           it--;
                        }
                        /* send result to user */
                        buffer[len-4]='\0';
                        cout<<"Message sent to user:"<<buffer+2<<endl;
                        if(send(j,buffer+2, len-4,flags)==-1)
                        { perror("Error sending to user!\n");
						  printf("Sockfd :%d\n",j); }
						close(j);
                        FD_CLR(j,&all);
                        FD_CLR(j,&users);
                        usersCount--;  
               	  	 }
               	  	 else if(buffer[1]=='0')
               	  	 {  /* free the worker */
        				 if(FD_ISSET(i,&engaged_workers))
                         { /* explicitly send a stop signal to end channeled session between user and worker */
						   if(send(i, "s", 2, flags)==-1)  //sending the stop message "s"
                           {  printf("Error in sending to socket %d!\n",i); }
						   FD_CLR(i,&engaged_workers);
        			       engaged_workersCount--;
            			   FD_SET(i,&free_workers);
          			       free_workersCount++;
                 	  	   /* implement:             free the worker from the state info of the user       */
                 	  	   unsigned int x=1;
                           j=0;
                           len=strlen(buffer);
                           for(int k=len-4;k<=len-1;k++)       
                           {  j+=(buffer[k]-32)*x; x=x<<8; }
                           request_state[j].remove(i);   
                           /* password not found! */
                           if(request_state[j].empty())
                           {  if(send(j,"Password Not Found! ;P",23,flags)==-1)
							  {  perror("Error sending to user!\n");
								 printf("Sockfd :%d\n",j); }
						   }                   
                         }
                     }		
               	  }
               	  /* handle the user request */
               	  else
               	  {  if(free_workersCount>=engaged_workersCount&& free_workersCount>0)
                     {  /* implement:            receive all message */
                        len=strlen(buffer);
                        int x=i, y, start=0; 
                        /* writing socketfd for user as characters representing the number */
                        buffer[len]='l';
                        buffer[len+1]='u';
                        buffer[len+2]=' ';
                        buffer[len+7]='\0';
                        cout<<"socket fd"<<i<<endl;
                        for(int k=len+3;k<=len+6;k++){buffer[k]=x%(1<<8)+32 ; x=x>>8;cout<<int(buffer[k])<<endl;}
                        cout<<"socket fd"<<i<<endl;
		                cout<<"Buffer from user in queue: "<< buffer<<endl;
						/* division of work amongst the workers */
                        start=0;
                        /* setting step size x and total search domain length y */
                        x=ceil(((buffer[len-1]-'0')*10.0+(buffer[len-2]-'0')*26.0+(buffer[len-3]-'0')*26.0)/(free_workersCount));
                        y=(buffer[len-1]-'0')*10.0+(buffer[len-2]-'0')*26.0+(buffer[len-3]-'0')*26.0;
                        /* creating new list of workers assigned the user */
                        r_s=new list<int>;
                        for(int j=0;j<=fd_max&&free_workersCount>0;j++)
                        {  if(FD_ISSET(j,&free_workers))
                           {  /* setting start and end character for the search position */
							  buffer[len]=(start)+32;
                              buffer[len+1]=min(y,start+x)+32;
                              start+=x;
                              cout<<"Message sent to worker: "<<buffer<<endl; 
                              
                              if(send(j,buffer,len+8,flags)==-1)
                              {  perror("Error!");printf("Error in sending message to socket %d!\n",j);}
                              /* allocating worker to user's request */
							  r_s->push_back(j);
							  /* managing worker status */FD_CLR(j,&free_workers);
                              free_workersCount--;
                              FD_SET(j,&engaged_workers);
                              engaged_workersCount++;
                           } 
                           request_state[i]=*r_s;  
                        }

                     }
                     else
                     {  /* Copy received buffer into copy and  */
						char bufferCopy[256];
						strcpy(bufferCopy,buffer);
						request.push(pair<int,char*>(i,bufferCopy));}               	  	
               	  }

               }	
            }
            
         } 
      }
   }
}
