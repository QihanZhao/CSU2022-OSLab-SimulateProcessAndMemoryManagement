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
vector<FreeMemNode> test;



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
//    cout<<"in the allocate func  "<<tmp<<"   "<< memoryNeeded <<endl;
    return tmp;
}

void releaseAndMerge(int toReleasedSize, int startAddress){
	
//	cout<<"in the Merge func  "<<startAddress<<"   "<< toReleasedSize <<endl;
	
    int endAddress = startAddress + toReleasedSize ; 
    
    if( FMPT[0].startAddress  >= startAddress ){
        if(FMPT[0].startAddress  == endAddress){
            FMPT[0].size += toReleasedSize;
            FMPT[0].startAddress -= toReleasedSize;
        }else{
            FreeMemNode tmp = {.size = toReleasedSize, .startAddress = startAddress };
            FMPT.insert(FMPT.begin(),tmp);
        }
    }else if(FMPT[FMPT.size()-1].startAddress < startAddress){
        if(FMPT[FMPT.size()-1].startAddress + FMPT[FMPT.size()-1].size  == startAddress){
            FMPT[FMPT.size()-1].size += toReleasedSize;
        }else{
            FreeMemNode tmp = {.size = toReleasedSize, .startAddress = startAddress };
            FMPT.push_back(tmp);
        }
    }else{
		for( int i= 0; i< FMPT.size()-1;i++ ){
		    	if(FMPT[i].startAddress < startAddress && FMPT[i+1].startAddress > startAddress){
		            if(FMPT[i].startAddress + FMPT[i].size  == startAddress ){
		                //回收区之前 & 之后有相邻的空闲分区
		                if(FMPT[i+1].startAddress == endAddress){
		                    FMPT[i].size = FMPT[i].size + toReleasedSize + FMPT[i+1].size;
		                    FMPT.erase(FMPT.begin() + i+1);
		                }//仅回收区之前 有相邻的空闲分区
		                else{
		                    FMPT[i].size += toReleasedSize;
//		                    puts("success");
		                }
		            }//仅回收区之后有相邻的空闲分区
		            else if(FMPT[i].startAddress  == endAddress){
		                FMPT[i].size += toReleasedSize;
		                FMPT[i].startAddress -= toReleasedSize;
		            }//回收区之前 & 之后都没有相邻的空闲分区
		            else{
		                FreeMemNode tmp = {.size = toReleasedSize, .startAddress = startAddress };
		                FMPT.insert(FMPT.begin()+i+1,tmp);
		            }
		
		        }
		    }	
	}

    
    
}

//总内存1000Mb，内核区
void initMem(){
    FreeMemNode tmp = {.size = MEMORYSIZE - KERNELSIZE, .startAddress = KERNELSIZE };
    FMPT.push_back(tmp);
    for( int i=0;i<15;i++){
        int memSize = rand()%100 +1;
//        cout<<memSize<<endl;
        int startAddress = allocate(memSize);
        FreeMemNode tmp = {.size = memSize, .startAddress = startAddress };
        test.push_back(tmp);
    }
}

void show(){
    for(int i= 0 ;i< FMPT.size();i++ ){
        cout<<"起始位置" << FMPT[i].startAddress << "大小" << FMPT[i].size << endl;
    }
}


int main(){
    initMem();
    show();
	for( int i=1;i<14;i=i+2){
        releaseAndMerge(test[i].size,test[i].startAddress);
    }
 
    cout<<endl;
    show();
    return 0;
}


