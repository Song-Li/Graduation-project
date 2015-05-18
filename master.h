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
using namespace std;

class master
{
    public:

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
        bool StartRun();
        int GetPatternID(string name);
        double AveDegree;
        int MaxDegree;


    protected:
    private:

};

#endif // MASTER_H
