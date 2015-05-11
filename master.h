#ifndef MASTER_H
#define MASTER_H

#include <stdio.h>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <stack>
#include <string.h>
#include <mpi.h>
#include "runJob.h"
#define MAX_Ver 1000000
#define MAX_Edg 100000
using namespace std;

class master
{
    public:
        runJob RunJob;

        //to get the map between name and id
        map<string, int> VertexMap;

        //to get the map between edge and id
        map<string, int> EdgeMap;

        //to get the map between graph and id
        map<string, int> GraphMap;

        //the id map
        List *Map = NULL;

        vector<string> VertexName;
        vector<string> EdgeName;
        vector<List *>Index;

        int Num_p;
        int Num_WenHao;
        master();
        virtual ~master();
        bool MapID(string FileName);
        bool InsertList(int VID);
        bool InsertNode(int FVID,int EID,int TVID);
        void OutPut();
        void Distribute();
        bool GetPattern();
        bool Finish();
        int GetPatternID(string name);


    protected:
    private:

};

#endif // MASTER_H
