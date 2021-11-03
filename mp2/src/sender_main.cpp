/* 
 * File:   sender_main.c
 * Author: shuhong3
 *
 * Created on 10.29
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include<iostream>
#include <sys/time.h>
#include <queue>
#include<math.h>

using namespace std;

struct sockaddr_in si_other;
int s, slen;

//////////////////
#define MAX_SIZE 2000
#define DATA 0
#define ACK 1
#define SYN 2
#define SYNACK 3
#define FIN 4
#define FINACK 5
#define MAX_SEQ 2000
#define RTT 20000
#define RTO_TH 3*RTT
#define DATA_QUEUE_SIZE 400

FILE *fp;
unsigned long long int bytesToPacket;
unsigned long long int seq_num=0;
int queue_done_flag=0;
int flow=0;


typedef struct{
    int ptype;   //data 0，ack 1，syn 2, synack 3,fin 4, finack 5
    int psize;
    union ID{
        int data_id;
        int ack_id;
    }id;
    char data[MAX_SIZE];
    struct timeval time;//time.tv_sec and time.utv_sec
}packet;

queue <packet> data_queue;
queue <packet> ack_queue;

double CW=5.0;
double SST=256;
int dupACKCount=0;

enum STATE{Slow_Start,Congestion_Avoidance,Fast_Recovery}current_state=Slow_Start;
int timeout = 0;
int new_ack = 0;  //flags
char data_temp[sizeof(packet)]; 
//////////////////
void diep(const char *s) {
    perror(s);
    exit(1);
}

void start_connection(int s);
void finish_connection(int s);
void settimeout(int s);
void queuing_packet(int num);
void sendpacket(int socket);
void state_transition(int s);





void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    //Open the file

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not open file to send.");
        exit(1);
    }

	/* Determine how many bytes to transfer */
    bytesToPacket=bytesToTransfer;

    slen = sizeof (si_other); //s_other as the socket at sender

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    //open a socket using udp, bind the third parameter with 0 works either
        diep("socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);

    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    start_connection(s); //handshake

    queuing_packet(1);

    settimeout(s); 

    sendpacket(s);
    while(!data_queue.empty() || !ack_queue.empty()){
        memset(&data_temp, 0, sizeof(packet));
        if(recvfrom(s, data_temp, sizeof(packet), 0, NULL , NULL)==-1){
            if(errno != EAGAIN || errno != EWOULDBLOCK){
                diep("can not receive ack");
            }
            timeout=1;
            new_ack=0;
            cout<<"timeout"<<endl;
            state_transition(s);
        }
        else{
            packet cur_packet;
            memcpy(&cur_packet, data_temp, sizeof(packet));
            if(cur_packet.ptype==ACK){
                cout<<"receive ack"<<cur_packet.id.ack_id<<endl;
                if(cur_packet.id.ack_id==ack_queue.front().id.data_id){
                    new_ack=1;
                    ack_queue.pop();
                    state_transition(s);
                }
                else if(cur_packet.id.ack_id>ack_queue.front().id.data_id){
                    while(!ack_queue.empty() && ack_queue.front().id.data_id<=cur_packet.id.ack_id){
                        ack_queue.pop();
                        new_ack=1;
                        state_transition(s);
                    }

                    
                    
                }
                else{
                    new_ack=0;
                    timeout=0;
                    state_transition(s);
                }

            }
            
        }



    }


	/* Send data and receive acknowledgements on s*/
    finish_connection(s);

    printf("Closing the socket\n");
    close(s);
    fclose(fp);
    cout<<"done transmit"<<endl;
    return;

}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);



    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);
    

    return (EXIT_SUCCESS);
}

void start_connection(int s){
    packet start_syn;
    char temp[sizeof(packet)];
    start_syn.ptype=SYN;
    start_syn.psize=0;
    memcpy(temp, &start_syn, sizeof(packet));
    if(sendto(s,temp,sizeof(packet),0,(sockaddr*)&si_other, (socklen_t)slen)==-1){
        diep("start connection error when send syn");
    }
    if(recvfrom(s, temp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t*)&slen)==-1){
        diep("error when receive synack");
    }
    packet start_ack;
    start_ack.ptype=ACK;
    start_ack.psize=0;
    memcpy(temp, &start_ack, sizeof(packet));
    if(sendto(s,temp,sizeof(packet),0,(sockaddr*)&si_other, (socklen_t)slen)==-1){
        diep("start connection error when send ack");
    }
    cout<<"successfully connected"<<endl;

}

void finish_connection(int s){
    packet done_fin;
    char temp[sizeof(packet)];
    done_fin.ptype=FIN;
    done_fin.psize=0;
    memcpy(temp, &done_fin, sizeof(packet));
    if(sendto(s,temp,sizeof(packet),0,(sockaddr*)&si_other, (socklen_t)slen)==-1){
        diep("start connection error when send syn");
    }
    // if(recvfrom(s, temp, sizeof(packet), 0,(sockaddr*)&si_other, (socklen_t*)&slen)==-1){
    //     diep("error when receive synack");
    // }
    while (recvfrom(s, temp, sizeof(packet), 0, NULL , NULL)==-1){
        if(errno != EAGAIN || errno != EWOULDBLOCK){
            diep("error when receive finack");
        }
        if(sendto(s,temp,sizeof(packet),0,(sockaddr*)&si_other, (socklen_t)slen)==-1){
            diep("start connection error when send syn");
        }
        
    }
    packet done_ack;
    done_ack.ptype=ACK;
    done_ack.psize=0;
    memcpy(temp, &done_ack, sizeof(packet));
    if(sendto(s,temp,sizeof(packet),0,(sockaddr*)&si_other, (socklen_t)slen)==-1){
        diep("start connection error when send ack");
    }
    cout<<"successfully connected"<<endl;
}
void settimeout(int s){
    struct timeval RTO;
    RTO.tv_sec=0;
    RTO.tv_usec=RTO_TH;
    if(setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,&RTO,sizeof(RTO))==-1){
        diep("settimeout()");
    }

}

void queuing_packet(int num){     
    cout<<"now in queue function"<<endl;
    cout<<queue_done_flag<<num<<bytesToPacket<<endl;
    if (queue_done_flag==1){
        return;
    }
    if (num<1 ){
        cout<<"due to CW, no new queuing"<<endl;
        return;
    } 
    if (bytesToPacket==0){
        cout<<"queeing all done"<<endl;
        queue_done_flag=1;
        return ;
    }                                            

    while(bytesToPacket>0 && num>0){
        packet my_packet;
        char buf[MAX_SIZE];
        int byte_read;
        int read_size;
        if(bytesToPacket<MAX_SIZE){
            byte_read=bytesToPacket;
        }
        else{
            byte_read=MAX_SIZE;
        }
        // cout<<"queuing "<<num<<" packets begin, still"<<bytesToPacket<<"Byte not queuing"<<endl;
        read_size=fread(buf, sizeof(char), byte_read, fp);
        if(read_size>0){
            my_packet.ptype = DATA;
            my_packet.psize = read_size;
            my_packet.id.data_id = seq_num;
            seq_num = seq_num+1;
            memcpy(my_packet.data,&buf,sizeof(char)*byte_read);
            data_queue.push(my_packet);
            cout<<"flow "<<flow<<" queueing packet "<<my_packet.id.data_id<<endl;
            
            bytesToPacket=bytesToPacket-read_size;
            num-=1;
        }
    }
    cout<<"flow "<<flow<<" queueing finish"<<endl;
    flow+=1;
    
}

void sendpacket(int socket){
    int num_p_send=CW-ack_queue.size();
    queuing_packet(num_p_send);
    cout<<"flow "<<flow<<"CW is "<<CW<<","<<ack_queue.size()<<" not receive ack, "<<"send "<<num_p_send<<" packets"<<endl;
    
    char temp[sizeof(packet)]; 
    if (num_p_send<1.0){
        if (timeout==1){
            cout<<"resend packet "<<ack_queue.front().id.data_id<<" due to time out"<<endl;
        }
        if (dupACKCount==3){
            cout<<"resend packet "<<ack_queue.front().id.data_id<<" due to duplicate"<<endl;
        }
        memcpy(temp, &ack_queue.front(), sizeof(packet));
        if (sendto(socket, temp, sizeof(packet), 0,(sockaddr*)&si_other, (socklen_t)slen)==-1){
            diep("sendto()");
        }
        cout<<"resend packet "<<ack_queue.front().id.data_id<<" done"<<endl;
                
        
    }
    else{
        if(data_queue.empty()){
            cout<<"data packets all send"<<endl;
        }
        else{
            if (num_p_send>data_queue.size()){
                num_p_send=data_queue.size();
            }
            for(int i=0;i<num_p_send;++i){
                memcpy(temp,&data_queue.front(),sizeof(packet));
                if (sendto(socket, temp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t)slen)==-1){
                    diep("sendto()");
                }    
                cout<<"flow "<<flow<<", send packet "<<data_queue.front().id.data_id<<"done"<<endl;
                ack_queue.push(data_queue.front());
                data_queue.pop();           
            }
        }
    }
    flow+=1;
    

}





void state_transition(int s){
    cout<<"flow "<<flow<<" now we are in state "<<current_state<<endl;
    switch(current_state){
        case Slow_Start:
            if(new_ack){
                CW++;
                dupACKCount=0;
                sendpacket(s);
                new_ack=0;
            }
            else{
                dupACKCount++;
            }
            
            if(timeout){
                SST=CW/2.0;
		if(SST<1){
		    SST=1;
		}
                CW=1;
                dupACKCount=0;
                sendpacket(s);
                timeout=0;
            }
            if(CW>=SST){
                current_state=Congestion_Avoidance;
                return;
            }
            if(dupACKCount==3){
                SST=CW/2.0;
                CW=SST+3.0;
		if(SST<1){
		    SST=1;
		}
                sendpacket(s);
                current_state=Fast_Recovery;
                return;
            }
            break;
        case Congestion_Avoidance:
            if(new_ack){
                CW=CW+1.0/floor(CW);
                dupACKCount=0;
                sendpacket(s);
                new_ack=0;
            }
            else{
                dupACKCount++;
            }
            if(timeout){
                SST=CW/2.0;
		if(SST<1){
		    SST=1;
		}
                CW=1.0;
                dupACKCount=0;
                sendpacket(s);
                current_state=Slow_Start;
                timeout=0;
                return;
            }
            
            if(dupACKCount==3){
                SST=CW/2.0;
		if(SST<1){
		    SST=1;
		}
                CW=SST+3.0;
                sendpacket(s);
                current_state=Fast_Recovery;
                return;
            }
            break;
        case Fast_Recovery:
            
            if(new_ack){
                CW=SST;
                dupACKCount=0;
                sendpacket(s);
                current_state=Congestion_Avoidance;
                new_ack=0;
                return;
            }
            else{
                dupACKCount++;
                CW++;
                sendpacket(s);
            }
            if(timeout){
                SST=CW/2.0;
		if(SST<1){
		    SST=1;
		}
                CW=1.0;
                dupACKCount=0;
                sendpacket(s);
                current_state=Slow_Start;
                timeout=0;
                return;
            }
            break;
        default:
            break;

    }
}
