//完成版 & 代码美化
#include<bits/stdc++.h> 
#include <mutex> 
#include <windows.h>
#define MAX 5  // 定义最大进程数
#define TIME 1 // 定义时间片大小，单位：秒（s）
#define MEMORYSIZE 1000
#define KERNELSIZE 100

using namespace std;

//----------------------------------------------------------------------PCB
//进程标识：PCB
typedef struct PCB {
    int id;
    int arrivedTime;
    int requiredTime;
    int usedTime;
    char state[8];

    int size;
    int startAddress;
}PCB, *pPCB;

//进程组织：链接方式，here采用双向链表（带头节点）
//由于规模比较小，这里使用单一PCB链
typedef struct PCBNode {
    PCB pcb;
    PCBNode *before;
    PCBNode *next;
}PCBNode, *pPCBNode;

//进程控制：原语
void schedule(pPCBNode &p);     //进程调度原语
void kill(pPCBNode &p);         //进程终止原语
void creatPCB();                //进程创建原语
void block(int id);             //进程阻塞原语
void wakeup(int id);            //进程唤醒原语

//全局变量forPCB
pPCBNode ReadyList = new PCBNode;   //内核数据结构（PCB链）
mutex mtx;                          //内核数据结构（PCB链）的互斥锁
int pcbID;                          //创建PCB时，用于产生pcb的id
clock_t beginTime;                  //创建PCB时，用于产生达到时间

//----------------------------------------------------------------------Mem
//空白内存标识：FMPT(FreeMemoryPartitionTable)
typedef struct FreeMemNode {
    int size;
    int startAddress;
}FreeMemNode, *pFreeMemNode;

//内存管理
int allocate(int memoryNeeded);                             //内存分配
void releaseAndMerge(int toReleasedSize, int startAddress);  //内存回收

//全局变量forMem
vector<FreeMemNode> FMPT;           //空闲分区表

//----------------------------------------------------------------------OS
//OS中的工具函数
void initPCB();                 // 初始化PCB 
void showPCB();                 // 打印输出PCB链表，直观展现各个PCB状态 
void initMem();                 // 初始化内存 
void showMem();                 // 打印输出空闲分区表 
void initOS();                  // 初始化OS
void startOS();                 // 启动OS 
pPCBNode checkReadyList();      // 检查ReadyList中是否还有未执行的PCB 


int main() {

	initOS();

	beginTime = clock();
	thread myThread ( startOS );
	myThread.detach();
	
	string s;
	while(1){
		printf("\n[HanShell]# ");
		
//	  	cin>>s;		
		getline(cin,s);
		istringstream record(s);

//	  	cout<<s;
		string out;
        record >> out;
        if(out == "creat") {
			creatPCB(); 
		}else if(out == "show"){
			showPCB();
			showMem();
		}else if(out == "block"){
			record >> out;
			int num = atoi(out.c_str());
			block(num);
		}else if(out == "wakeup"){
			record >> out;
			int num = atoi(out.c_str());
			wakeup(num);
		}
		
	}
	
	return 0;
}

// 初始化PCB
void initPCB() {
    pPCBNode p = ReadyList;
    
    for (int i = 0; i < MAX; i++) {
    	pPCBNode temp = new PCBNode;
        //指针域赋值
        temp->before = p;
        temp->next = NULL;
        p->next = temp;
        p = temp;
        //数据域赋值
        // PCB pcb = temp->pcb;
        temp->pcb.id = pcbID++;
        temp->pcb.arrivedTime = 0;
        temp->pcb.requiredTime = rand() % 10 + 1;
        temp->pcb.usedTime = 0;
        strcpy(temp->pcb.state, "Ready");
        temp->pcb.size = rand() % 89 + 1;
        temp->pcb.startAddress = allocate(temp->pcb.size);
//        puts("success");
        //头结点的before用来记录当前链表的尾结点（避免还要单独建一个tail标记尾结点） 
        ReadyList->before = temp;
    }
    
}

// 打印输出PCB链表，直观展现各个PCB状态 
void showPCB() {
	pPCBNode p = ReadyList->next;
    printf("PID    |到达时间    |所需时间    |已用时间    |状态        |内存始址    |占用大小\n");
    printf("----------------------------------------------------------------------------------\n"); 
	while(p) {
		PCB pcb = p->pcb;
		printf("%-6d|%-12d|%-12d|%-12d|%-12s|%-12d|%-12d\n", 
			pcb.id, pcb.arrivedTime, pcb.requiredTime, pcb.usedTime, pcb.state, pcb.startAddress, pcb.size );
		p = p->next;
	}
	puts("");
}



//总内存1000Mb，内核区
void initMem(){

    FreeMemNode tmp = {.size = MEMORYSIZE - KERNELSIZE, .startAddress = KERNELSIZE };
    FMPT.push_back(tmp);
}

void showMem(){
	printf("id    |内存始址    |空闲大小       <-空闲分区表\n");
	printf("-----------------------------\n"); 
	
    for(int i= 0 ;i< FMPT.size();i++ ){
        printf("%-6d|%-12d|%-12d\n", i,FMPT[i].startAddress, FMPT[i].startAddress);
    }
    puts("");
}

// 检查ReadyList中是否还有未执行的PCB
//Tip:多出口的函数，在互斥量的控制时要非常注意，在每个出口前都要unlock 
pPCBNode checkReadyList() {
    
	pPCBNode p = ReadyList->next;
	while(p) {
		if (strcmp(p->pcb.state, "Ready") == 0) {
			mtx.unlock();
			return p;
		}
		p = p->next;
	}
    
	return NULL;
}

void initOS(){
    //初始化内存
    initMem();
    //模拟一个随机分布的空白内存表
    vector<FreeMemNode> test;
    for( int i=0 ; i<15 ; i++){
        int memSize = rand()%100 +1;
        // cout<<memSize<<endl;
        int startAddress = allocate(memSize);
        FreeMemNode tmp = {.size = memSize, .startAddress = startAddress };
        test.push_back(tmp);
    }
    for( int i=1 ; i<14 ; i=i+2){
        releaseAndMerge(test[i].size,test[i].startAddress);
    }
    puts("\n**********初始内存分配**********"); 
    showMem();
    
    //初始化PCB
    initPCB();
    puts("\n**********初始提交的任务清单**********"); 
    showPCB();
}

void startOS() {
	pPCBNode p = NULL;
	while(1) {
        p = checkReadyList();
		if (p != NULL) {
			strcpy(p->pcb.state, "Running");
            int remainingTime = p->pcb.requiredTime - p->pcb.usedTime;
			if (remainingTime <= TIME) {
				Sleep(remainingTime * 1000);
				strcpy(p->pcb.state, "Dead");
				p->pcb.usedTime = p->pcb.requiredTime;
                //进程终止原语
				kill(p);
//TO DO 进程占用的内存空间的回收

			} else {			
                Sleep(TIME * 1000);
				strcpy(p->pcb.state, "Ready");
				p->pcb.usedTime += TIME;
				if (p->next) {
                    //进程切换原语
					schedule(p);
				}
			}
		}
        

	}
}

void creatPCB(){
	
    mtx.lock();
	    pPCBNode temp = new PCBNode;
	    //指针域赋值
	    temp->before = ReadyList->before;
	    temp->next = NULL;
	    ReadyList->before->next = temp;
	    ReadyList->before = temp;
	    //数据域赋值
	    // PCB pcb = temp->pcb; 典型错误：这是深拷贝，无法修改母体
	    temp->pcb.id = pcbID;
        temp->pcb.arrivedTime = ((clock() - beginTime) / CLOCKS_PER_SEC );
	    temp->pcb.requiredTime = rand() % 10 + 1;
	    temp->pcb.usedTime = 0;
	    temp->pcb.size = rand() % 49 + 1;
		temp->pcb.startAddress = allocate(temp->pcb.size);
	    strcpy(temp->pcb.state, "Ready");
	    //头结点的before用来记录当前链表的tail 
	    pcbID++;
    mtx.unlock();
    
	printf("id    |到达时间    |所需时间    |已用时间    |状态        |内存始址    |占用大小\n");
	printf("----------------------------------------------------------------------------------\n"); 
	PCB pcb = temp->pcb;
	printf("%-6d|%-12d|%-12d|%-12d|%-12s|%-12d|%-12d\n", 
		pcb.id, pcb.arrivedTime, pcb.requiredTime, pcb.usedTime, pcb.state, pcb.startAddress, pcb.size );
	puts("");
	
}

void schedule(pPCBNode &p){
	mtx.lock();
	    p->before->next = p->next;
	    p->next->before = p->before;
	    p->before = ReadyList->before;
	    ReadyList->before->next = p;
	    ReadyList->before = p;
	    p->next = NULL;
	mtx.unlock();    
}

void kill(pPCBNode &p){
	mtx.lock();
		releaseAndMerge(p->pcb.size,p->pcb.startAddress);
		if (p->next) {
		    p->before->next = p->next;
		    p->next->before = p->before;
		    delete p;
		}else{
			p->before->next = p->next;
			ReadyList->before = ReadyList;
			delete p;
		}
	mtx.unlock();  

}

//Tip:多出口的函数，在互斥量的控制时要非常注意，在每个出口前都要unlock 
void block(int id){
	mtx.lock();
	    pPCBNode p = ReadyList->next;
	    while(p) {
			if(p->pcb.id == id){
	            strcpy(p->pcb.state, "Blocked");
	            printf("id=%d被阻塞\n", id);
                mtx.unlock();
	            return;
	        }
			else{
	            p = p->next;
	        }
		}
	    //不存在指定要阻塞的进程
		puts("error");
	mtx.unlock();   
}


void wakeup(int id){
	mtx.lock();   
	    pPCBNode p = ReadyList->next;
	    while(p) {
			if(p->pcb.id == id){
	            strcpy(p->pcb.state, "Ready");
	            printf("id=%d被唤醒\n", id);
                mtx.unlock();
	            return;
	        }
			else{
	            p = p->next;
	        }
		}
	    //不存在指定要唤醒的进程
		puts("error");
	mtx.unlock();   
}

int allocate(int memoryNeeded){
	int tmp = -1;
    for(int i= 0 ;i< FMPT.size();i++ ){
        if(FMPT[i].size >= memoryNeeded){
            tmp = FMPT[i].startAddress;
            FMPT[i].size -= memoryNeeded;
            FMPT[i].startAddress += memoryNeeded;
            break;
        }
    }
    // cout<<"in the allocate func  "<<tmp<<"   "<< memoryNeeded <<endl;
	if(tmp==-1) puts("Allocation Failure");
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
		                    // puts("success");
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