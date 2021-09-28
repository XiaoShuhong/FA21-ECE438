/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <iostream>


using namespace std;
// #define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 1000 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    
    
	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    //seperate the argv[1]
    //data is seperate into http host port path
    int position1,position2,position3;
    string temp=argv[1];
    position1=temp.find(':');
    // string http=temp.substr(0,position1);
    
    position3=temp.find('/',position1+3);
    string mix=temp.substr(position1+3,position3-position1-3);
    // cout<<mix<<endl;
    string path=temp.substr(position3+1);
    string port="80";
    string host;
    position2=mix.find(':');
    if (position2!=mix.npos){
        host=mix.substr(0,position2);
        port=mix.substr(position2+1);
    
    }
    else{
        host=mix;
    }
    
    cout<<path<<endl;
    cout<<port<<endl;
    cout<<host<<endl;

    

	if ((rv = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;

	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    //write my EGT
    string GET="GET "+path+" HTTP/1.1\r\n" + "User-Agent:  Wget/1.12 (linux-bnu)\r\n"+ "Host:  "+host+':'+port+"\r\n"+"Connection:  Keep-Alive\r\n\r\n";
    send(sockfd,GET.c_str(),GET.size(),0);
    FILE *f=fopen("output","wb");
    int flag=0;
    while(1){
        memset(buf,'\0',MAXDATASIZE);
        int read_num;
        if ((read_num=recv(sockfd,buf,MAXDATASIZE,0))>0){
            cout<<read_num<<endl;
            string s_buf=string(buf);
            int pos= s_buf.find("\r\n\r\n");
            if (flag==0 && pos!=s_buf.npos){
                flag=1;
                string s=string(buf);
                s=s.substr(pos+4);
                fwrite(s.c_str(),sizeof(char),s.size(),f);
            }
            else if(flag==1){
                fwrite(buf,sizeof(char),read_num,f);
            }

        }
        else{
            break;
        }
    }


	
    fclose(f);
	close(sockfd);

	return 0;
}

