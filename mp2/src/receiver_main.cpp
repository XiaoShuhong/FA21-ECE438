/* 
 * File:   receiver_main.c
 * Author: zicheng5
 *
 * Created on 2021.10.30
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
#include <iostream>
#include <sys/time.h>
#include <queue>
#include <map>
#include <algorithm>
using namespace std;

#define BUFFERSIZE 3000 // max number of packets that can be buffered at the receiver side
#define MAX_SIZE 2200
#define DATA 0
#define ACK 1
#define SYN 2
#define SYNACK 3
#define FIN 4
#define FINACK 5
#define MAX_SEQ 2000

struct sockaddr_in si_me, si_other;
int s, slen;
FILE * fPtr;

void diep(const char *s) {
    perror(s);
    exit(1);
}

typedef struct{
    int ptype;   //data 0，ack 1，syn 2, synack 3,fin 4, finack 5
    int psize;
    union ID{
        int data_id;
        int ack_id;
    }id;
    char data[MAX_SIZE];
    // struct timeval time；//time.tv_sec and time.utv_sec
}packet;


void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {

    fPtr =  fopen(destinationFile, "w");
    if(fPtr == NULL){
        diep("Could not open file to write.");
    }
                    // fputs(((packet*) &buf[ACKseq % MAX_SEQ])->data, fPtr);
    
    slen = sizeof (si_other);
    int numbytes;
    int ACKseq = 0;
    int buf_packet_num = 0;
    // vector<packet> buf(BUFFERSIZE);     //buffering received packets
    // char buf[BUFFERSIZE];
    map<int,packet> buf;
    map<int,int> flag;
    // fill(buf,buf + BUFFERSIZE,0);
    char recvtemp[sizeof(packet)];      //recv() function buffer
    char sendtemp[sizeof(packet)];      //sendto() function buffer

    /* Initialize the flag map */
    for(int i = 0; i<MAX_SEQ; i++){
        flag.insert(pair<int,int>(i,0));
    }


    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");


	/* Now receive data and send acknowledgements */ 

    // if ((numbytes = recvfrom(s, recvtemp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t*)&slen)) == -1) {
	//     diep("recv");
	// }

    // packet recv_syn_packet;
    // memcpy((void*)&recv_syn_packet,recvtemp,sizeof(packet));

    // /* receiving a SYN */
    // if (recv_syn_packet.ptype == SYN) {  
    //     packet synack_packet;
    //     synack_packet.ptype = SYNACK;
    //     synack_packet.psize = 0;
    //     memcpy(sendtemp,&synack_packet,sizeof(packet)); 

    //     if (sendto(s, sendtemp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t)slen)==-1){
    //         diep("SYNACK's sendto()");
    //     }  
    // }
    cout<<"Now entering the loop "<< endl;
    while(1){
        cout<<"Now successfully entering the loop "<< endl;
        if ((numbytes = recvfrom(s, recvtemp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t*)&slen)) == -1) {
            diep("recv");
        }
        packet recv_packet;
        memcpy((void*)&recv_packet,recvtemp,sizeof(packet));

        /* receiving a SYN */
        if (recv_packet.ptype == SYN) {  
            cout<<"receive SYN "<< endl;
            packet synack_packet;
            synack_packet.ptype = SYNACK;
            synack_packet.psize = 0;
            memcpy(sendtemp,&synack_packet,sizeof(packet)); 

            if (sendto(s, sendtemp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t)slen)==-1){
                diep("SYNACK's sendto()");
            }  
        }

        /* receiving a FIN */
        if (recv_packet.ptype == FIN) { 
            cout<<"receive FIN "<< endl; 
            packet finack_packet;
            finack_packet.ptype = FINACK;
            finack_packet.psize = 0;
            memcpy(sendtemp,&finack_packet,sizeof(packet));

            if (sendto(s, sendtemp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t)slen)==-1){
                diep("FINACK's sendto()");
            } 
            break; 
        }

        /* receiving a normal data packet */
        if (recv_packet.ptype == DATA) {
            /* 1.receiving an out of date packets */
            if (recv_packet.id.data_id < ACKseq){
                cout<<"received data_id "<< recv_packet.id.data_id <<endl;
                cout<<"Now the ACKseq is "<< ACKseq <<endl;
                cout<<"receive out of date packet "<< recv_packet.id.data_id <<endl;
                /* send normal ACK back to sender */
                packet pastack_packet;
                pastack_packet.ptype = ACK;
                pastack_packet.psize = 0;
                pastack_packet.id.ack_id = ACKseq - 1;
                memcpy(sendtemp,&pastack_packet,sizeof(packet));
                cout<<"prepare to send ACK "<< ACKseq-1 <<endl;

                if (sendto(s, sendtemp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t)slen)==-1){
                    diep("past ACK's sendto()");
                }

                cout<<"send ACK "<< ACKseq-1 <<" successfully!"<<endl;
            }
            /* 2.receiving exactly the right packet */
            else if (recv_packet.id.data_id == ACKseq ) {
                cout<<"received data_id "<< recv_packet.id.data_id <<endl;
                cout<<"Now the ACKseq is "<< ACKseq <<endl;
                cout<<"receive right packet "<< recv_packet.id.data_id <<endl;

                int cnt = ACKseq;
                buf[ACKseq % MAX_SEQ] = recv_packet;
                flag[ACKseq % MAX_SEQ] = 1;
                // memcpy(buf+ACKseq,&recv_packet,sizeof(packet));
                while(flag[cnt % MAX_SEQ]!=0){
                    cnt++;
                }
                /* send normal ACK back to sender */
                packet goodack_packet;
                goodack_packet.ptype = ACK;
                goodack_packet.psize = 0;
                goodack_packet.id.ack_id = cnt-1;
                memcpy(sendtemp,&goodack_packet,sizeof(packet));
                cout<<"prepare to send ACK "<< cnt-1 <<endl;

                if (sendto(s, sendtemp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t)slen)==-1){
                    diep("good ACK's sendto()");
                }

                cout<<"send ACK "<< cnt-1 <<" successfully!"<<endl;

                /* Write the packet data into destinationFile, if there's a packet in the next position,
                write it too. Do it till no packet buffered in the next position, 
                then ACKseq++ and buf_packet_num-- */
                 
                while(flag[ACKseq % MAX_SEQ] != 0){
                
                    packet temp_packet;
                    memcpy(&temp_packet, (packet*) &buf[ACKseq % MAX_SEQ], sizeof(packet));
                    // data_size=((packet*) &buf[ACKseq % MAX_SEQ]))->psize;
    
                    fwrite(temp_packet.data,sizeof(char),temp_packet.psize,fPtr);
                    
                    cout<<"File written successfully for ACKseq "<< ACKseq <<endl;
                    cout<<endl;

                    
                    flag[ACKseq % MAX_SEQ] = 0;
                    // buf[ACKseq] = 0;
                    ACKseq++;
                    buf_packet_num--;
                }
                // *(buf + 0) = recv_packet;
                // while(buf[0] != 0){
                //     fPtr =  fopen(destinationFile, "w");
                //     if(fPtr == NULL){
                //         diep("Could not open file to write.");
                //     }
                //     fputs(buf[0], fPtr);
                //     fclose(fPtr);

                //     buf[0] = 0;
                //     ACKseq = ((ACKseq+1) % MAX_SEQ);
                //     buf_packet_num--;
                //     rotate(buf.begin(),buf.begin()+1,buf.end())
                //  }
            }
            /* 3.receiving future packets */
            else if (recv_packet.id.data_id > ACKseq) {
                cout<<"received data_id "<< recv_packet.id.data_id <<endl;
                cout<<"Now the ACKseq is "<< ACKseq <<endl;
                cout<<"receive future packet "<< recv_packet.id.data_id <<endl;
                buf_packet_num++;
                if (buf_packet_num == BUFFERSIZE){
                    diep("Full receiver buffer");
                }

                /* send dup ACK back to sender */
                packet dupack_packet;
                dupack_packet.ptype = ACK;
                dupack_packet.psize = 0;
                dupack_packet.id.ack_id = ACKseq-1;
                memcpy(sendtemp,&dupack_packet,sizeof(packet));

                if (sendto(s, sendtemp, sizeof(packet), 0, (sockaddr*)&si_other, (socklen_t)slen)==-1){
                    diep("dup ACK's sendto()");
                }

                /* buffer the received packets */
                if (flag[recv_packet.id.data_id % MAX_SEQ] == 0){
                    // memcpy(buf + (recv_packet.id.data_id % MAX_SEQ),&recv_packet,sizeof(packet));
                    buf[recv_packet.id.data_id % MAX_SEQ] = recv_packet;
                    flag[recv_packet.id.data_id % MAX_SEQ] = 1;
                }
                // if (buf[recv_packet.id.data_id - (ACKseq % MAX_SEQ)] == 0){
                //     *(buf + recv_packet.id.data_id - (ACKseq % MAX_SEQ)) = recv_packet;
                // }
            }
        }
    }      
    fclose(fPtr);
    close(s);
	printf("%s received.", destinationFile);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}

