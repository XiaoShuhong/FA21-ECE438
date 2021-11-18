#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<map>
#include<set>
#include<fstream>
#include<iostream>
#include<queue>
#include <typeinfo> 

#define INF 99999

using namespace std;
ofstream fpOut;
map< int,map<int,int> > weight;
set<int> nodes;
map<int, map<int,pair<int,int> > > forward_table_list;
typedef map<int, pair<int,int> > single_dis_vec;
typedef map<int, single_dis_vec> dis_vec;
map<int,dis_vec > dis_vectors;
queue<pair<int,single_dis_vec> > temp;
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
    initialize(argv[1]);

    forward_table_list.clear();
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
        single_dis_vec self_vec;
        dis_vec node_vec;
        int out_node=*iter;
        set<int>::iterator inner_iter = nodes.begin();
	    while (inner_iter!=nodes.end()){
            int inner_node=*inner_iter;
            // cout<<out_node<<" "<<inner_node<<endl;
            if(out_node==inner_node){
                self_vec[inner_node]= make_pair(0,inner_node);
                inner_iter++;
                continue;
            }
            if(weight[out_node].count(inner_node)){
                self_vec[inner_node]= make_pair(weight[out_node][inner_node],inner_node);
            }
            else{
                self_vec[inner_node]= make_pair(INF,INF);
            }
        

            inner_iter++;

        }
        node_vec[out_node]=self_vec;
        temp.push(make_pair(out_node,self_vec));
        map<int,int>::iterator w_iter = weight[out_node].begin();
        while (w_iter!=weight[out_node].end()){


            int nei_node = w_iter->first;
            single_dis_vec nei_vec;
            // cout<<nei_node<<endl;

            
            set<int>::iterator inner_iter = nodes.begin();
	        while (inner_iter!=nodes.end()){
                int inner_node=*inner_iter;
                nei_vec[inner_node]=make_pair(INF,INF);
                inner_iter++;
            }

            node_vec[nei_node] = nei_vec;
            w_iter++;

        }
        
        dis_vectors[out_node] = node_vec;
        iter++;
    }
    
}

void Distance_Vector(){
    // cout<<"distance_vector"<<endl;
    while(!temp.empty()){
        pair<int,single_dis_vec > cur_node_info = temp.front();
        temp.pop();
        int cur_node = cur_node_info.first;
        single_dis_vec cur_vector_list = cur_node_info.second;
        map<int,int>::iterator w_iter = weight[cur_node].begin();
        while (w_iter!=weight[cur_node].end()){
            int nei_node = w_iter->first;
            vector<int> update;
            set<int>::iterator inner_iter = nodes.begin();
	        while (inner_iter!=nodes.end()){
                int inner_node= *inner_iter;
                if(cur_vector_list[inner_node].first<dis_vectors[nei_node][cur_node][inner_node].first){
                    update.push_back(inner_node);
                }
                inner_iter++;
            }
            if(update.size() == 0){
                w_iter++;
                continue;

            }
            dis_vectors[nei_node][cur_node] = cur_vector_list;
            for(int i=0;i<update.size();i++){
                int cur_update= update[i];
                if(weight[nei_node][cur_node]+cur_vector_list[cur_update].first<=dis_vectors[nei_node][nei_node][cur_update].first){
                    dis_vectors[nei_node][nei_node][cur_update].first = weight[nei_node][cur_node]+cur_vector_list[cur_update].first;
                    if(dis_vectors[nei_node][nei_node][cur_update].second > cur_node){
                        dis_vectors[nei_node][nei_node][cur_update].second = cur_node;
                    }
                }
            }

            temp.push(make_pair(nei_node, dis_vectors[nei_node][nei_node]));
            w_iter++;
        }
        
    }
    set<int>::iterator iter = nodes.begin();
	while (iter!=nodes.end())
	{
        map<int,pair<int,int> > forward_table;
        int start = *iter;
        set<int>::iterator inner_iter = nodes.begin();
        while (inner_iter!=nodes.end()){
            int end = *inner_iter;
            forward_table[end].first = dis_vectors[start][start][end].second;
            forward_table[end].second = dis_vectors[start][start][end].first;
            inner_iter++;
        }
        forward_table_list[start] = forward_table;

        iter++;
    }


}

void fill_output(char* msgfile){
    
    

    // cout<<"write"<<endl;
    for(int i=1;i<=nodes.size();i++){
        for(int j=1;j<=nodes.size();j++){
            if(i==j){
                fpOut <<i<<i<<0<<endl;
                continue;
            }
            if(forward_table_list[i][j].second==INF || nodes.count(i)==0 || nodes.count(j)==0){
                
                continue;
            }
            else{
                fpOut << j<<forward_table_list[i][j].first<<forward_table_list[i][j].second<<endl;
            }
        }
        // fpOut<<endl;

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
        fpOut<<msg<<endl;
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
        if(c == -999){
            weight[a].erase(b);
            weight[b].erase(a);
            if (weight.count(a) == 0){
                nodes.erase(b);
            }
        
            if (weight.count(b) == 0){
                nodes.erase(a);
            }

        }
        else{
            weight[a][b] = c;
            weight[b][a] = c;
            nodes.insert(a);
            nodes.insert(b);
        }
        forward_table_list.clear();
        
        Distance_Vector();
        
        fill_output(msgfile);


    }
    inf.close();
    
}
