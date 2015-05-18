#include "runJob.h"

runJob::runJob()
{
    //ctor
    MPI_Comm_size(MPI_COMM_WORLD,&Num_p);
    Num_v = Num_e = 0;
    memset(Id_Pat,-1,sizeof(Id_Pat));
}

runJob::~runJob()
{
    //dtor
}

void runJob::OutPattern(int myid){
    string name;
    char buff[10];
    sprintf(buff,"%d",myid);
    name = buff;
    name += "pattern";
    name = "../out/" + name;
    ofstream pattern(name.c_str());
    Node_P *tn;
    List_P *list;
    /*
    for(list = Map_P;list != NULL; list = list->next){
        for(tn = list -> first;tn != NULL;tn = tn->next){
            pattern << list->VID << "(" << list->VPOS << ")    " << tn->EID << "(" << tn->EPOS << ")   "<< tn->VID << endl;
        }
    }
*/

    for(int i = 0;i < Index_P.size();++ i){
        for(tn = Index_P[i] -> first;tn != NULL;tn = tn->next){
            pattern << Index_P[i]->VID << "(" << Index_P[i]->type << ")    " << tn->EID << "(" << tn->EPOS << ")   "<< tn->VID << "(" << tn->type << ")" <<  endl;
        }
    }
}

void runJob::OutPut(int myid){
    string name;
    char buff[10];
    sprintf(buff,"%d",myid);
    name = buff;
    name += "converted";
    name = "../out/" + name;
    ofstream convert(name.c_str());
    Node *tn;
    List *list;
    //output the con index 
    for(int i = 0;i < Index_con.size();++ i){
        for(tn = Index_con[i] -> first; tn != NULL; tn = tn -> next){
            convert << Index_con[i]->VID << "("<< Index_con[i]->Num_Child << ")   " << tn->EID <<"    " << tn -> VID << endl;
        }
    }
    /*
    //output the map
    for(list = Map;list != NULL;list = list -> next){
        for(tn = list->first;tn != NULL;tn = tn->next){
            convert << list->VID << "    " << tn->EID << "   "<< tn->VID << endl;
        }
    }
    for(int i = 0;i < Index.size();++ i){
        for(tn = Index[i] -> first; tn != NULL; tn = tn -> next){
            convert << Index[i]->VID << "("<< Index[i]->Num_Child << ")   " << tn->EID <<"    " << tn -> VID << endl;
        }
    }
     */

}

/***************************************************
  if the VID of a node >= Index.size()
  that means this node hasn't been included by the total map
  which means it's a  node with a '?'
  *************************************************/
bool runJob::InsertNode(int FVID,int EID,int TVID,bool Data){//edge to vertex
    List *list;
    List_P *list_p;
    Node *node;
    Node_P *node_p;
    if(Data) list = Index[FVID];
    else list_p = GetList(FVID);
    if(Data){
        node = new Node();
        node->VID = TVID;
        node->EID = EID;
        node->next = list->first;
        list->first = node;
        (list->Num_Child) ++;
        list = Index_con[TVID];
        node = new Node();
        node->VID = FVID;
        node->EID = EID;
        node->next = list->first;
        list->first = node;
        (list->Num_Child) ++;
    }else{
        node_p = new Node_P();
        if(TVID >= Index.size()){
            node_p->type = 0;
        }
        node_p->EPOS = Num_e;
        node_p->VID = TVID;
        node_p->EID = EID;
        node_p->next = list_p->first;
        list_p->first = node_p;
        (list_p->Num_Child) ++;
        Num_e ++;
    }

    return true;
}

bool runJob::InsertList(int VID){
    if(Map == NULL){//The Map is empty
        Map = new List();
        Map->VID = VID;
        Index.push_back(Map);
        Map_con = new List();
        Map_con->VID = VID;
        Index_con.push_back(Map_con);
    }else{
        List *list = new List();
        list->VID = VID;
        list->next = Map->next;
        Map->next = list;
        Index.push_back(list);
        list = new List();
        list->VID = VID;
        list->next = Map_con->next;
        Map_con->next = list;
        Index_con.push_back(list);
    }
    return true;
}

/*****************************************************
  this function is used to insert pattern list
  if type == 0 means this node include a ?
  ***************************************************/
bool runJob::InsertList_P(int VID,int type){
    if(Map_P == NULL){//The Map is empty
        Map_P = new List_P();
        if(type == 0){
            Map_P->type = 0;
        }
        Map_P->VID = VID;
        Map_P->VPOS = Num_v;
        Index_P.push_back(Map_P);
    }else{
        List_P *list = new List_P();
        if(type == 0){
            list->type = 0;
        }
        list->VID = VID;
        list->VPOS = Num_v;
        list->next = Map_P->next;
        Map_P->next = list;
        Index_P.push_back(list);
    }
    Num_v ++;
    return true;
}

bool runJob::LoadFile(){
    int V1,V2,E;
    int now = 0;
    int buf[3];
    int msgtag = MAPTAG;
    while(1){
        MPI_Recv(&buf,3,MPI_INT,0,msgtag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        if(buf[0] == FIN) break;
        V1 = buf[0];
        E = buf[1];
        V2 = buf[2];
        while(now <= V1) InsertList(now ++);//insert list as many as possible
        while(now <= V2) InsertList(now ++);
        InsertNode(V1,E,V2,true);
    }
    return true;
}

void runJob::GetIdPat(){
    //next we try to get the VID_VPOS map
    for(int i = 0;i < Index_P.size();++ i){
        VID_VPOS.insert(pair<int,int>(Index_P[i]->VID,i));
    }
    //next we try to get the metrx of the edges
    List_P * list;
    Node_P * tn;
    for(list = Map_P;list != NULL; list = list->next){
        for(tn = list -> first;tn != NULL; tn = tn -> next){
            Id_Pat[list->VPOS][VID_VPOS.find(tn->VID)->second] = tn->EID;       
        }
    }
}

bool runJob::LoadPattern(){
    int V1,V2,E;
    int now = 0;
    int buf[5];
    int msgtag = PATTAG;
    while(1){
        MPI_Recv(&buf,5,MPI_INT,0,msgtag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        if(buf[0] == FIN) break;
        V1 = buf[0];
        E = buf[2];
        V2 = buf[3];
        if(GetList(V1) == NULL) InsertList_P(V1,buf[1]);//insert list as many as possible
        if(GetList(V2) == NULL) InsertList_P(V2,buf[4]);
        InsertNode(V1,E,V2,false);
    }
    GetIdPat();
    return true;
}

/************************************************************************
  The massage tag of vertex in run is 3
  Firstly we get the partly finished graph
  we judge the degree of the selected point
  we check the edge of the point
  we generate new partly finished graph
  -10 means this query has been finished
  **********************************************************************/

/**************************************
  Get the pointer of a List
 *************************************/
List_P * runJob::GetList(int VID){
    List_P *list = Map_P;
    while(list != NULL){
        if(list -> VID == VID) return list;
        list = list -> next;
    }
    return NULL;
}


int runJob::GetEdge(int from,int to){
    List *list;
    list = Index[from];
    Node *node;
    for(node = list -> first; node != NULL; node = node->next){
        if(node->VID == to) return node->EID;
    }
    return -1;
}

int runJob::GetEPOS(int from, int to){
    List_P *list;
    list = Index_P[VID_VPOS.find(from)->second];
    Node_P *node;
    for(node = list -> first; node != NULL; node = node->next){
        if(node->VID == to) return node->EPOS;
    }
}

void runJob::OutPutAns(vector<int> vertex, vector<int> edge){
    ofstream output(OUTPUTPATH,ios::app);
    for(int i = 0;i < vertex.size();++ i)
        cout << vertex[i] << "  ";
    cout << endl;
    for(int i = 0;i < edge.size();++ i)
        cout << edge[i] << "    ";
    cout << endl;
    output.close();
}

void runJob::GiveOthers(vector<int> vertex, vector<int> edge){
    bool finish = true;
    int aim = 1;
    int buf[MAX_Q_V + MAX_Q_E];
    for(int i = 0;i < vertex.size();++ i){
        if(vertex[i] == NotFinished) {
            finish = false;
            break;
        }
    }
    if(finish) OutPutAns(vertex,edge);
    else{
        //the usable machine is Num_p - 1
        aim = next_machine % (Num_p - 1);
        next_machine ++;
        //except for the machine 0
        aim ++;
        //the machine couldn't be the running machine
        if(aim == myid) aim ++;
        if(next_machine > Num_p * 1000) next_machine = 0;

        for(int i = 0;i < vertex.size();++ i){
            buf[i] = vertex[i];
        }
        for(int i = 0;i < edge.size();++ i){
            buf[i + vertex.size()] = edge[i];
        }
        MPI_Send(&buf,MAX_Q_V + MAX_Q_E,MPI_INT,aim,DATATAG,MPI_COMM_WORLD);
    }
}

void runJob::GetAnswer(int VID,int vpos,vector<int> vertex,vector<int> edge){
    int temp;
    //Firstly we check wether the degree of this node is enough
    if(Index_P[vpos]->Num_Child > Index[VID]->Num_Child) return ;
    //secondly we check wether every edges are matched
    for(int i = 0;i < vpos;++ i){
        if(vertex[i] == NotFinished) 
            break;
        temp = GetEdge(vertex[i],VID);
        if(Id_Pat[i][vpos] != -1){//the Pattern has a edge
            if(temp == -1) return ;//but the data don't has one 
            if(Id_Pat[i][vpos] != 0 && Id_Pat[i][vpos] != temp) return ;//the edge in data doesn't match the edge in pattern
        }
        temp = GetEdge(VID,vertex[i]);
        if(Id_Pat[vpos][i] != -1){//the Pattern has a edge
            if(temp == -1) return ;//but the data don't has one 
            if(Id_Pat[vpos][i] != 0 && Id_Pat[vpos][i] != temp) return ;//the edge in data doesn't match the edge in pattern
        }
    }
    vertex[vpos] = VID;//Now we know that this node is usable
    //so now we need to generate new edges to get next vector to be caculated by other machines
    for(int i = 0;i < vpos;++ i){
        if(Id_Pat[i][vpos] != -1){// there is a edge between the two nodes
            temp = GetEPOS(Index_P[i]->VID, Index_P[vpos]->VID);
            if(Id_Pat[i][vpos] == 0){
                edge[temp] = GetEdge(vertex[i],VID);
            }else{
                edge[temp] = Id_Pat[i][vpos];
            }
        }
        if(Id_Pat[vpos][i] != -1){// there is a edge between the two nodes
            temp = GetEPOS(Index_P[vpos]->VID, Index_P[i]->VID);
            if(Id_Pat[vpos][i] == 0){
                edge[temp] = GetEdge(VID,vertex[i]);
            }else{
                edge[temp] = Id_Pat[vpos][i];
            }
        }
    }
    GiveOthers(vertex,edge);
}

void runJob::Run(){
    int buf[MAX_Q_V + MAX_Q_E];
    vector<int> vertex;
    vector<int> edge;
    int vpos;
    List_P * waitnode;
    int waittype;
    int aim = 0;
    next_machine = 0;
    while(1){
        MPI_Recv(&buf,MAX_Q_V + MAX_Q_E,MPI_INT,MPI_ANY_SOURCE,DATATAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        /*
        for(int i = 0;i < 5;++ i){
            cout << buf[i] << " ";
        }
        cout << endl;
        */
        vertex.clear();
        edge.clear();
        //if a number == NotFinished means this node hasn't been finished 
        //recieve the task that needed to be handled and if buf[0] == FIN means this select has been finished
        if(buf[0] == FIN) return ;
        for(int i = 0;i < Num_v;++ i){
            vertex.push_back(buf[i]);
        }
        for(int i = 0;i < Num_v;++ i){
            if(buf[i] == NotFinished) {
                vpos = i;
                break;
            }
        }
        for(int i = 0;i < Num_e;++ i){
            edge.push_back(buf[i + Num_v]);
        }
        waitnode = Index_P[vpos];
        waittype = waitnode->type;
        if(waittype != 0) GetAnswer(waitnode->VID,vpos,vertex,edge);
        else {
            for(int i = 0;i < Index.size();++ i){
                GetAnswer(Index[i]->VID,vpos,vertex,edge);
            }
        }
    }    
        
}
