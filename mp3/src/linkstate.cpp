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


using namespace std;

#define INF 99999
ofstream fpOut;
map< int,map<int,int> > weight;
set<int> nodes;
map<int, map<int,pair<int,int> > > forward_table_list;

void initialize(char* inputfile);
void Dijkstra();
void fill_output(char* msgfile);
void do_change(char* msgfile, char* changefile);

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    }

    fpOut.open("output.txt");
    
    
    initialize(argv[1]);
    forward_table_list.clear();
    Dijkstra();

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

}

void Dijkstra(){

    
    set<int>::iterator iter = nodes.begin();
	while (iter!=nodes.end())
	{
        set<int>node_down;
        map<int,int> distance;
        map<int,int> predecessor;
        queue<int> node_list;

        int begin_node = *iter;
        node_down.insert(begin_node);
        // cout<<node<<endl;
        set<int>::iterator inner_iter = nodes.begin();
        while (inner_iter!=nodes.end()){
            int node = *inner_iter;
            // cout<<node<<endl;
            if(weight[begin_node].count(node)){
                distance[node] = weight[begin_node][node];
            }
            else{
                distance[node] = INF;
            }
            predecessor[node] = begin_node;



            inner_iter++;
        }
        
        for(int i=0;i<nodes.size()-1;i++){
            int mdis = INF;
            int mnode = INF;
            set<int>::iterator inner_iter = nodes.begin();
            while (inner_iter!=nodes.end()){
                int node = *inner_iter;
                if(node_down.count(node)){
                    inner_iter++;
                    continue;
                }
                if (distance[node] == mdis){
                    if (node<mnode){
                        mnode=node;
                    }
                }
                else if(distance[node] < mdis){
                    mdis=distance[node];
                    mnode=node;
                }
                
                inner_iter++;
            }
            node_down.insert(mnode);
            node_list.push(mnode);
            

            map<int,int>::iterator w_iter = weight[mnode].begin();
            while (w_iter!=weight[mnode].end()){
                // cout<<w_iter->first<<w_iter->second<<endl;
                int to_node = w_iter->first;
                int w = w_iter->second;
                if(node_down.count(to_node)){
                    w_iter++;
                    continue;
                }
                if(distance[mnode]!=INF){
                    if(distance[mnode]+w<distance[to_node]){
                        distance[to_node] = distance[mnode]+w;
                        predecessor[to_node] = mnode;
                    }
                    else if(distance[mnode]+w == distance[to_node]){
                        if (mnode<predecessor[to_node]){
                            predecessor[to_node] = mnode;
                        }
                    }
                }
                w_iter++;
            }
        }
        map<int,pair<int,int> > forward_table;
        forward_table[begin_node].first=begin_node;
        forward_table[begin_node].second=0;
        while(!node_list.empty()){
            int cur_node = node_list.front();
            node_list.pop();
            if(predecessor[cur_node] == begin_node){
                forward_table[cur_node].first = cur_node;
                forward_table[cur_node].second = distance[cur_node];
            }
            else{
                forward_table[cur_node].first = forward_table[predecessor[cur_node]].first;
                forward_table[cur_node].second = distance[cur_node];

            }
        }
        forward_table_list[begin_node]=forward_table;


		iter++;
	}

}


void fill_output(char* msgfile){
    
    

    // cout<<"write"<<endl;
    for(int i=1;i<=nodes.size();i++){
        for(int j=1;j<=nodes.size();j++){
            if(i==j){
                fpOut <<i<<" "<<i<<" "<<0<<endl;
                continue;
            }
            if(forward_table_list[i][j].second==INF || nodes.count(i)==0 || nodes.count(j)==0){
                
                continue;
            }
            else{
                fpOut << j<<" "<<forward_table_list[i][j].first<<" "<<forward_table_list[i][j].second<<endl;
            }
        }
        // fpOut<<endl;

    }
    string s;
    ifstream inf;
    int a,b;
    string msg;
    inf.open(msgfile);
    while (getline(inf, s)){
        
        // cout<<len<<endl;
        
        sscanf(s.c_str(), "%d %d %*s", &a, &b);
        msg=s.substr(s.find(" "));
        msg = msg.substr(s.find(" ") + 1);

        

        // cout<<a<<b<<msg<<endl;
        
    
        // cout<<msg<<endl;

        // stringstream ss;
        // ss << msg;
        // string message = ss.str();
        // cout<<message<<endl;
        
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
        
        Dijkstra();
        
        fill_output(msgfile);


    }
    inf.close();
    
}


