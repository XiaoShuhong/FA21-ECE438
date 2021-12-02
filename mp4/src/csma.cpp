#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<map>
#include<set>
#include<fstream>
#include<iostream>
#include<queue>
#include <typeinfo> 
#include<string>
#include<sstream>
#include <iomanip>
using namespace std;
typedef struct{
    int node_idx;
    int R_idx;
    int backoff;
    int collision_count;
}node;
map<int,node> nodes;
vector<int> random_time;
vector<int> collision_nodes;
vector<int> reset_nodes;

int N,L,M,R,T;

int channelOccupied;
int cur_tick;
int packet_trans;
int trans_time;
void initialize(char* inputfile);
void simulation();
int randtime(int id, int cur_tick, int r);
void check_backoff();
void decrease_all_backoff();
void collision_trigger();

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 2) {
        printf("Usage: ./csma input.txt\n");
        return -1;
    }
    FILE *fpOut;
    fpOut = fopen("output.txt", "w");
    
    initialize(argv[1]);
    channelOccupied=0;
    packet_trans=0;
    simulation();
    float util_time=1.0*packet_trans*L/T;
    // cout<<packet_trans<<endl;
    // cout<<setprecision(2)<<util_time<<endl;
    fprintf(fpOut,"%.2f\n",util_time);
    // fpOut<<setprecision(3)<<util_time<<endl;
    
    fclose(fpOut);
    return 0;
}



void initialize(char* inputfile){
    
    ifstream inf;
    inf.open(inputfile);
    char n,l,m,r,t;
    inf>>n>>N>>l>>L>>m>>M>>r;
    // cout<<N<<L<<M<<endl;
    for(int i=0;i<6;i++){
        inf>>R;
        random_time.push_back(R);
        // cout<<R<<endl;
    }
    inf>>t>>T;
    // cout<<T<<endl;
    inf.close();
    cout<<"initialize success!"<<endl;

}

void simulation(){
    for(int i=0;i<N;i++){
        nodes[i].node_idx=i;
        nodes[i].R_idx=0;
        nodes[i].backoff=randtime(i,0,random_time[0]);  
        nodes[i].collision_count=0;
        // cout<<nodes[i].backoff<<endl;
    }
    for(int i=0;i<T;i++){
        cur_tick=i;
        cout<<"now at time "<<cur_tick<<endl;
        check_backoff();
//   ///////////////////   
        // cout<<"current tick is: "<<cur_tick<<endl;
        // if(channelOccupied==1){
        //     cout<<"current trans node is "<<reset_nodes[0]<<endl;
        //     cout<<"trans time left is "<<trans_time<<endl;
        // }
        // else{
        //     if(collision_nodes.size()==0){
        //         cout<<"nothing special, all backoff --"<<endl;
        //     }
        //     else if(collision_nodes.size()==1){
        //         cout<<"node "<<collision_nodes[0]<<" begin to trans"<<endl;
        //     }
        //     else{
        //         cout<<collision_nodes.size()<<" nodes have collusion, they are ";
        //         for(int j=0;j<collision_nodes.size();j++){
        //             cout<<collision_nodes[j]<<" ";
        //         }
        //         cout<<endl;
        //     }
        // }
    ///////////////////////////
       
        if(channelOccupied==1){
            trans_time-=1;
            



        }
        else {
            if(collision_nodes.size()==0){
                decrease_all_backoff();
                continue;

            }   
            else if(collision_nodes.size()==1){
		packet_trans+=1;
                channelOccupied=1;
                trans_time=L-1;
                nodes[collision_nodes[0]].collision_count=0;
                reset_nodes.push_back(collision_nodes[0]);
                cout<<"node "<<collision_nodes[0]<<" begin to trans"<<endl;
                continue;

            }
            else if(collision_nodes.size()>1){
                collision_trigger();
                continue;
            }

        }
        if(trans_time==0){
            //packet_trans+=1;
            cout<<"node "<<reset_nodes[0]<<" finish trans"<<endl;
            nodes[reset_nodes[0]].R_idx=0;
            nodes[reset_nodes[0]].backoff=randtime( nodes[reset_nodes[0]].node_idx,cur_tick+1,random_time[ nodes[reset_nodes[0]].R_idx]);
            nodes[reset_nodes[0]].collision_count=0;
            cout<<"reset backoff for node "<<reset_nodes[0]<<" value is "<<nodes[reset_nodes[0]].backoff<<" at time "<<cur_tick<<endl;
            reset_nodes.clear();
            channelOccupied=0;

        }
        
        
    }

}

int randtime(int id, int cur_tick, int r){
    return (id+cur_tick)%r;
}

void check_backoff(){
    collision_nodes.clear();
    for(int i=0;i<N;i++){
        if(nodes[i].backoff==0){
            collision_nodes.push_back(nodes[i].node_idx);
            cout<<"node "<<i<<" have backoff 0"<<endl;
        }
    }
}

void decrease_all_backoff(){
    cout<<"at time "<<cur_tick<<"apply backoff decrease"<<endl;
    for(int i=0;i<N;i++){
        cout<<"node "<<i<<" backoff become "<<nodes[i].backoff<<endl;
        nodes[i].backoff-=1;

    }
}

void collision_trigger(){
    for(int i=0; i<collision_nodes.size();i++){
        nodes[collision_nodes[i]].collision_count+=1;
        if(nodes[collision_nodes[i]].collision_count>=6){
            nodes[collision_nodes[i]].collision_count=0;
            nodes[collision_nodes[i]].R_idx=0;
            nodes[collision_nodes[i]].backoff=randtime(nodes[collision_nodes[i]].node_idx,cur_tick+1,random_time[nodes[collision_nodes[i]].R_idx]);
        }
        else{
            nodes[collision_nodes[i]].R_idx+=1;
            nodes[collision_nodes[i]].backoff=randtime(nodes[collision_nodes[i]].node_idx,cur_tick+1,random_time[nodes[collision_nodes[i]].R_idx]);
        }
        cout<<"set node "<<collision_nodes[i]<<" collision_count become "<< nodes[collision_nodes[i]].collision_count<<"; new backoff become "<<nodes[collision_nodes[i]].backoff<<endl;
    }
}
