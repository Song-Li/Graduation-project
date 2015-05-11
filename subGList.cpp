#include "master.h"
#include "runJob.h"

int main(int argc, char *argv[]){
    int myid;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    if(myid == 0){
        master Ma;
        string InputName;
        cin >> InputName;
        Ma.MapID(InputName.c_str());
        Ma.Distribute();
        Ma.RunJob.OutPut(myid);
        if(!Ma.GetPattern()) Ma.Finish();
        Ma.RunJob.GetIdPat();
        Ma.RunJob.OutPattern(myid);
        Ma.RunJob.Run();
         
    }else {
        runJob RunJob;
        RunJob.myid = myid;
        RunJob.LoadFile();
        RunJob.LoadPattern();
        RunJob.OutPut(myid);
        RunJob.OutPattern(myid);
        RunJob.Run();
    }
    MPI_Finalize();
    return 0;
}
