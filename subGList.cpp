#include "master.h"
#include "runJob.h"

int main(int argc, char *argv[]){
    int myid;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    srand((unsigned int)time(NULL));
    if(myid == 0){
        master Ma;
        string InputName;
        cin >> InputName;
        Ma.MapID(InputName.c_str());
        Ma.OutPut();
        cout << "INFO:  Output Finished" << endl;
        Ma.Distribute();
        if(!Ma.GetPattern()) Ma.Finish();
        Ma.StartRun();
    }else {
        runJob RunJob;
        RunJob.myid = myid;
        RunJob.LoadFile();
        RunJob.LoadPattern();
        RunJob.Run();
    }
    MPI_Finalize();
    return 0;
}
