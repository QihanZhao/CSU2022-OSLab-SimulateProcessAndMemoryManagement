//实现内存动态分区分配
#include<bits/stdc++.h> 

#define MEMORYSIZE 1000
#define KERNELSIZE 100

using namespace std;

//空白内存标识：FMPT(FreeMemoryPartitionTable)
typedef struct FreeMemNode {
    int size;
    int startAddress;
}FreeMemNode, *pFreeMemNode;

vector<FreeMemNode> FMPT;

void allocate();
void releaseAndMerge();

//总内存1000Mb，内核区
void initMem(){
    FreeMemNode tmp = {.size = MEMORYSIZE - KERNELSIZE, .startAddress = KERNELSIZE };
    FMPT.push_back(tmp);
}

void show(){
    for(int i= 0 ;i< FMPT.size();i++ ){
        cout<<"起始位置" << FMPT[i].startAddress << "大小" << FMPT[i].size << endl;
    }
}

int main(){
    initMem();
    show();
    return 0;
}

int allocate(int memoryNeeded){
	int tmp = -1;
    for(int i= 0 ;i< FMPT.size();i++ ){
        if(FMPT[i].size >= memoryNeeded){
            tmp = FMPT[i].startAddress;
            FMPT[i].size -= memoryNeeded;
            FMPT[i].startAddress += memoryNeeded;
            break;
        }else{
            puts("Allocation Failure");
        }
    }
    return tmp;
}

void releaseAndMerge(int toReleasedSize, int startAddress){

    int endAddress = startAddress + toReleasedSize + 1; 
    int i= 0;
    for( ; i< FMPT.size();i++ ){
        if(FMPT[i].startAddress + FMPT[i].size +1 == startAddress ){
            //回收区之前 & 之后有相邻的空闲分区
            if(FMPT[i+1].startAddress == endAddress){
                FMPT[i].size = FMPT[i].size + toReleasedSize + FMPT[i+1].size;
                FMPT.erase(FMPT.begin() + i+1);
            }//仅回收区之前 有相邻的空闲分区
            else{
                FMPT[i].size += toReleasedSize;
            }
        }//仅回收区之后有相邻的空闲分区
        else if(FMPT[i].startAddress  == endAddress){
            FMPT[i].size += toReleasedSize;
            FMPT[i].startAddress -= toReleasedSize;
        }
    }
    // 回收区之前 & 之后都没有相邻的空闲分区
    if(i == FMPT.size()){
        FreeMemNode tmp = {.size = toReleasedSize, .startAddress = startAddress };
        FMPT.push_back(tmp);
    }

}