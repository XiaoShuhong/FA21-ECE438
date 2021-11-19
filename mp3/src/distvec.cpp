#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<map>
#include<set>
#include<fstream>
#include<iostream>
#include<queue>
#include <typeinfo> 

#define INF -999

using namespace std;
ofstream fpOut;
map< int,map<int,int> > weight;
set<int> nodes;
map<int, map<int,pair<int,int> > > forward_table_list;

void initialize(char* inputfile);
void Distance_Vector();
void fill_output(char* msgfile);
void do_change(char* msgfile, char* changefile);

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./distvec topofile messagefile changesfile\n");
        return -1;
    }

    fpOut.open("output.txt");
    forward_table_list.clear();
    initialize(argv[1]);

    
    Distance_Vector();
    fill_output(argv[2]);
    
    do_change(argv[2],argv[3]);
    
    fpOut.close();

    return 0;
}



void initialize(char* inputfile){
    
    ifstream inf;
    inf.open(inputfile);
    int a,b,c;
    while(inf>>a>>b>>c){
        nodes.insert(a);
        nodes.insert(b);
        weight[a][b] = c;
        weight[b][a] = c;
        // cout<<a<<b<<c<<endl;
    }
    // for (int v: nodes){
    //     cout<<v<<endl;
    //     for(pair<int,int> p: weight[v]){
    //         cout<<v<<" "<< p.first<<" "<<p.second<<endl;
    //     }
    // }
    inf.close();
    cout<<"initialize nodes and weight information success!"<<endl;
    set<int>::iterator iter = nodes.begin();
	while (iter!=nodes.end())
	{
        int src_node=*iter;
        set<int>::iterator dest_iter = nodes.begin();
	    while (dest_iter!=nodes.end()){
            int des_node=*dest_iter;
            if(des_node==src_node){
                weight[src_node][des_node]=0;
            }
            if(weight.count(src_node)==0){
                weight[src_node][des_node]=INF;
            }
            else{
                if(weight[src_node].count(des_node)==0){
                    weight[src_node][des_node]=INF;
                }
            }
            forward_table_list[src_node][des_node]=make_pair(des_node, weight[src_node][des_node]);
            dest_iter++;
        }
        iter++;
    }

    
    //check weight
    // set<int>::iterator iter2 = nodes.begin();
	// while (iter2!=nodes.end()){

    //     int cur_node=*iter2;
    //     map<int,int>::iterator w_iter = weight[cur_node].begin();
    //     while (w_iter!=weight[cur_node].end()){
    //         cout<<cur_node<<" "<<w_iter->first<<" "<<w_iter->second<<endl;
    //         w_iter++;
    //     }
    //     iter2++;

    // }

    
    
}

void Distance_Vector(){
    // cout<<"distance_vector"<<endl;
    int size= nodes.size();
    for(int i=0;i<size;i++){
        set<int>::iterator iter = nodes.begin();
	    while (iter!=nodes.end())
	    {  
            int scr_node=*iter;
            set<int>::iterator dest_iter = nodes.begin();
	        while (dest_iter!=nodes.end()){
                int des_node=*dest_iter;
                
                int min_node=forward_table_list[scr_node][des_node].first;
                int min_cost=forward_table_list[scr_node][des_node].second;
                set<int>::iterator iter3 = nodes.begin();
	            while (iter3!=nodes.end()){
                    int cur_node=*iter3;
                    if(weight[scr_node][cur_node]>=0 && forward_table_list[cur_node][des_node].second >=0){
                        if(min_cost<0 ||(weight[scr_node][cur_node]+ forward_table_list[cur_node][des_node].second)<min_cost){
                            min_node=cur_node;
                            min_cost=weight[scr_node][cur_node]+ forward_table_list[cur_node][des_node].second;
                        }
                    }
                    forward_table_list[scr_node][des_node] = make_pair(min_node,min_cost);
                    iter3++;

                }
                dest_iter++;
            }
            iter++;

        } 

    }
    
}

void fill_output(char* msgfile){
    
    

    // cout<<"write"<<endl;
    
    set<int>::iterator iter = nodes.begin();
	while (iter!=nodes.end())
	{  
        int scr_node=*iter;
        set<int>::iterator dest_iter = nodes.begin();
	    while (dest_iter!=nodes.end()){
            int des_node=*dest_iter;

            fpOut << des_node<<" "<<forward_table_list[scr_node][des_node].first<<" "<<forward_table_list[scr_node][des_node].second<<endl;
            dest_iter++;

        }
        // fpOut<<endl;

        iter++;
    }
    string s;
    ifstream inf;
    int a,b;
    char c,d;
    inf.open(msgfile);
    while (getline(inf, s)){
        int len=s.length();
        c=s[0];
        d=s[2];
        a=int(c-'0');
        b=int(d-'0');
        char msg[len-4];
        for(int i=0;i<len;i++){
            if(i>3){
                msg[i-4]=s[i];
            }

        }
        // cout<<typeid(a).name()<<endl;
        // cout<<c<<d<<endl;
        // cout<<a<<b<<endl;
        
        
        // cout<<msg<<endl;

        if(nodes.count(a) == 0|| nodes.count(b) == 0||forward_table_list[a][b].second == INF){
            fpOut<< "from "<<a<<" to "<<b<<" cost infinite hops unreachable message "<<msg<<endl;
            continue;
        }
        queue<int> temp;
        int cur = a;
        while(cur!=b){
            temp.push(cur);
            cur = forward_table_list[cur][b].first;
        }
        fpOut<<"from "<<a<<" to "<<b<<" cost "<<forward_table_list[a][b].second<<" hops ";
        // cout<<"from "<<a<<" to "<<b<<" cost "<<forward_table_list[a][b].second<<" hops ";
        while(temp.size()>0){
            fpOut<<temp.front()<<" ";
            // cout<<temp.front()<<" ";
            temp.pop();
        }
        fpOut<<"message "<<msg<<endl;
        // cout<<msg<<endl;




    }
    inf.close();




  



}

void do_change(char* msgfile, char* changefile){
    ifstream inf;
    inf.open(changefile);
    int a,b,c;
    while(inf>>a>>b>>c){
        // cout<<a<<b<<c<<endl;
        weight[a][b] = c;
        weight[b][a] = c;
        nodes.insert(a);
        nodes.insert(b);
        set<int>::iterator iter = nodes.begin();
	    while (iter!=nodes.end())
	    {  
            int scr_node=*iter;
            set<int>::iterator dest_iter = nodes.begin();
	        while (dest_iter!=nodes.end()){
                int des_node=*dest_iter;

                forward_table_list[scr_node][des_node] = make_pair(des_node,weight[scr_node][des_node]);
                dest_iter++;

            }
 

            iter++;
        }
        
        
        Distance_Vector();
        
        fill_output(msgfile);


    }
    inf.close();
    
}
