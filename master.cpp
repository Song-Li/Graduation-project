#include "master.h"
#include "runJob.h"

using namespace std;
#define MAXLEN 10000000
char Map_buf[MAXLEN];

master::master(){
    //ctor
    MPI_Comm_size(MPI_COMM_WORLD,&Num_p);
    VertexMap.insert(pair<string,int>("?",0));
    VertexName.push_back("?");
    AveDegree = 0;
    MaxDegree = 0;
    InsertList(0);
    Num_WenHao = 0;
    EdgeMap.insert(pair<string,int>("?",0));
    GraphMap.insert(pair<string,int>("?",0));
}

master::~master(){
    //dtor
}

bool master::Finish(){}

void master::OutPut(){
    ofstream convert("../out/converted");
    Node *tn;
    for(int i = 0;i < Index.size();++ i){
        for(tn = Index[i]->first;tn != NULL;tn = tn->next){
            convert << Index[i]->VID << " " << tn->EID << " "<< tn->VID << " " << 1 << endl;
        }
    }
    convert.close();
}

int master::GetPatternID(string name){
    if(VertexMap.find(name) == VertexMap.end()){
        VertexMap.insert(pair<string,int>(name,Index.size() + Num_WenHao ++));
    }
    return VertexMap.find(name) -> second;
}

bool master::GetPattern(){//The massage tag of pattern is 2
    int V1,V2,E;
    char buf[100000];
    int buf_int[5];
    string Tri[5];
    int flag = 0;
    int now = 0;
    int type = -1;
    stack<char> comma;
    int len;
    int line = 0;
    ifstream input("../Pattern");
    while(!input.eof()){
        flag = 0;
        line ++;
        for(int i = 0;i < 5;++ i) Tri[i].clear();
        input.getline(buf,MAXLEN);
        if(buf[0] == '#') continue;
        if(strlen(buf) == 0) continue;
        if(input.fail()) {
            cout << "Fail at " << line << endl;
            cout << buf << endl;
            return false;
        }
        len = strlen(buf);
        for(int i = 0;i < len;++ i){
            if(buf[i] == ' '){
                if(comma.empty()){
                    flag ++;
                    continue;
                }
            }
            if(buf[i] == '\\') {
                Tri[flag] += buf[i ++];
                Tri[flag] += buf[i];
                continue;
            }
            if(buf[i] == '\"'){
                if(!comma.empty() && comma.top() == '\"') comma.pop();
                else comma.push(buf[i]);
            }
            Tri[flag] += buf[i];
        }
        for(int i = 0;i < 4;++ i) Tri[i] += Tri[3];
        map<string, int>::iterator it;
        buf_int[1] = buf_int[4] = -1;
        if(Tri[0][0] == '?'){
            V1 = GetPatternID(Tri[0]);
            buf_int[1] = 0;
        }else{
            it = VertexMap.find(Tri[0]);
            if(it == VertexMap.end()) return false;//this label is not found 
            else V1 = it -> second;
        }
        if(Tri[1][0] == '?'){
            E = 0;
        }else{
            it = EdgeMap.find(Tri[1]);
            if(it == EdgeMap.end()) return false;//this label is not found 
            else E = it -> second;
        }
        if(Tri[2][0] == '?'){
            V2 = GetPatternID(Tri[2]);
            buf_int[4] = 0;
        }else{
            it = VertexMap.find(Tri[2]);
            if(it == VertexMap.end()) return false;//this label is not found 
            else V2 = it -> second;
        }
        buf_int[0] = V1;
        buf_int[2] = E;
        buf_int[3] = V2;
        for(int i = 1;i < Num_p;++ i){
            MPI_Send(&buf_int,5,MPI_INT,i,PATTAG,MPI_COMM_WORLD);
        }
    }
    buf_int[0] = FIN;
    buf_int[1] = buf_int[2] = FIN;
    for(int i = 1;i < Num_p;++ i)
        MPI_Send(&buf_int,5,MPI_INT,i,PATTAG,MPI_COMM_WORLD);
    input.close();
    cout << "INFO:  The Number of Unsure Pattern Vertex is " << Num_WenHao << endl;
}

void master::Distribute(){
    Node *tn;
    int msgtag = MAPTAG;
    int buf[3];
    int Num_P = Num_p;
    cout << "INFO:  The number of machine is " << Num_P << endl;
    for(int i = 0;i < Index.size();++ i){
        for(tn = Index[i]->first;tn != NULL;tn = tn->next){
            buf[0] = Index[i]->VID;
            buf[1] = tn->EID;
            buf[2] = tn->VID;
            for(int j = 1;j < Num_P;++ j)
                MPI_Send(&buf,3,MPI_INT,j,msgtag,MPI_COMM_WORLD);
        }
    }
    buf[0] = FIN;
    buf[1] = buf[2] = FIN;
    for(int j = 1;j < Num_P;++ j)
        MPI_Send(&buf,3,MPI_INT,j,msgtag,MPI_COMM_WORLD);
}

bool master::InsertList(int VID){
    if(Map == NULL){//The Map is empty
        Map = new List();
        Map->VID = VID;
        Index.push_back(Map);
    }else{
        List *list = new List();
        list->VID = VID;
        list->next = Map->next;
        Map->next = list;
        Index.push_back(list);
    }
    return true;
}

bool master::InsertNode(int FVID,int EID,int TVID){//edge to vertex
    List *list = Index[FVID];
    Node *node = new Node();
    node->VID = TVID;
    node->EID = EID;
    node->next = list->first;
    list->first = node;
    (list->Num_Child) ++;

    return true;
}

bool master::MapID(string FileName){
    ifstream InputFile(FileName.c_str());
    stack<char> comma;
    string Tri[5];
    int flag = 0;
    int len;
    int VID = 1;
    int EID = 1;
    int GID = 1;
    int line = 0;
    while(!InputFile.eof()){
        flag = 0;
        line ++;
        for(int i = 0;i < 5;++ i) Tri[i].clear();
        InputFile.getline(Map_buf,MAXLEN);
        if(Map_buf[0] == '#') continue;
        if(strlen(Map_buf) == 0) continue;
        if(InputFile.fail()) {
            cout << "Fail at " << line << endl;
            cout << strlen(Map_buf) << endl;
            return false;
        }
        len = strlen(Map_buf);
        
        for(int i = 0;i < len;++ i){
            if(Map_buf[i] == '\\') {      
                Tri[flag] += Map_buf[i ++];
                Tri[flag] += Map_buf[i];
                continue;
            }
	        if(comma.empty() && Map_buf[i] == '<' ){
                comma.push(Map_buf[i]);    
            }else if(Map_buf[i] == '>'){
                if(!comma.empty() && comma.top() == '<') 
                    comma.pop();
            }else if(Map_buf[i] == ' '){
                if(comma.empty()){
                    flag ++;
                    continue;
                }
            }else if(Map_buf[i] == '\"'){
                if(!comma.empty() && comma.top() == '\"') comma.pop();
                else comma.push(Map_buf[i]);
            }
            Tri[flag] += Map_buf[i];
        }
        for(int i = 0;i < 4;++ i) Tri[i] += Tri[3];
        if(VertexMap.find(Tri[0]) == VertexMap.end()) {
            VertexMap.insert(pair<string,int>(Tri[0],VID ++));
            //VertexName.push_back(Tri[0]);
            InsertList(VID - 1);
        }
        if(EdgeMap.find(Tri[1]) == EdgeMap.end()) {
            EdgeMap.insert(pair<string,int>(Tri[1],EID ++));
            //EdgeName.push_back(Tri[1]);
        }
        if(VertexMap.find(Tri[2]) == VertexMap.end()) {
            VertexMap.insert(pair<string,int>(Tri[2],VID ++));
           // VertexName.push_back(Tri[2]);
            InsertList(VID - 1);
        }
        if(GraphMap.find(Tri[3]) == GraphMap.end()) GraphMap.insert(pair<string,int>(Tri[3],GID ++));

        int V1 = VertexMap.find(Tri[0])->second;
        int V2 = VertexMap.find(Tri[2])->second;
        int E = EdgeMap.find(Tri[1])->second;
        InsertNode(V1,E,V2);
    }
    for(int i = 0;i < Index.size();++ i){
        if(MaxDegree < Index[i] -> Num_Child) MaxDegree = Index[i] -> Num_Child;
        AveDegree += Index[i] -> Num_Child;
    }
    cout << "INFO:  The total degree is " <<  AveDegree << endl;
    AveDegree /= Index.size() - 1; 
    map<string,int>::iterator it;
    cout << "INFO:  The number of Vertex is " << VertexMap.size() - 1 << endl;
    cout << "INFO:  The number of Edge is " << EdgeMap.size() - 1<< endl;
    cout << "INFO:  The number of Graph is " << GraphMap.size() - 1<< endl;
    cout << "INFO:  The average degree is " << AveDegree << endl;
    cout << "INFO:  The max degree is " << MaxDegree << endl;
    /*for(it = VertexMap.begin();it != VertexMap.end();++ it) output << "Vertex : " << it -> first << "   " << it -> second << endl;
    for(it = EdgeMap.begin();it != EdgeMap.end();++ it) output << "Edge : " << it -> first << "   " << it -> second << endl;
    for(it = GraphMap.begin();it != GraphMap.end();++ it) output << "Graph : " << it -> first << "   " << it -> second << endl;
    output.close();*/
    return true;

}

//start the running
bool master::StartRun(){
    int buf[MAX_Q_V + MAX_Q_E];
    for(int i = 0;i < MAX_Q_V + MAX_Q_E;++ i) buf[i] = NotFinished;
    int aim = 1;
    cout << "INFO:  The first machine is " << aim <<  endl;
    MPI_Send(&buf,MAX_Q_V + MAX_Q_E,MPI_INT,aim,DATATAG,MPI_COMM_WORLD);
    return true;
}

