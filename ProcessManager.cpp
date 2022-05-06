//实现wakeup原语& 计时器
#include<bits/stdc++.h> 
#include <mutex> 
#include <windows.h>
#define MAX 5  // 定义最大进程数
#define TIME 6 // 定义时间片大小，单位：秒（s）

using namespace std;

//进程标识：PCB
typedef struct PCB {
    int id;
    int arrivedTime;
    int requiredTime;
    int usedTime;
    char state[8];
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

//OS中的工具函数
void initPCB();                 // 初始化PCB
void showPCB();                 // 打印输出PCB链表，直观展现各个PCB状态 
pPCBNode checkReadyList();      // 检查ReadyList中是否还有未执行的PCB
void startOS();                 // 启动OS 

//全局变量
mutex mtx;                          //内核数据结构（PCB链）的互斥锁
pPCBNode ReadyList = new PCBNode;   //内核数据结构（PCB链）
int pcbID;                          //创建PCB时，用于产生pcb的id
clock_t beginTime                   //创建PCB时，用于产生达到时间

int main() {
	
	beginTime = clock();

	initPCB();
	thread myThread ( startOS );
	myThread.detach();
	
	string s;
	while(1){
		printf("\n[myshell]# ");
		
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
		}else if(out == "block"){
			record >> out;
			int num = atoi(out.c_str());
			block(num);
		}
		
	}
	
	return 0;
}


// 打印输出PCB链表，直观展现各个PCB状态 
void showPCB() {
	pPCBNode p = ReadyList->next;
    printf("id    |到达时间    |所需时间    |已用时间    |状态\n");
    printf("---------------------------------------------------\n"); 
	while(p) {
		PCB pcb = p->pcb;
		printf("%-6d|%-12d|%-12d|%-12d|%-12s\n", 
			pcb.id, pcb.arrivedTime, pcb.requiredTime, pcb.usedTime, pcb.state);
		p = p->next;
	}
	puts("");
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
        //头结点的before用来记录当前链表的尾结点（避免还要单独建一个tail标记尾结点） 
        ReadyList->before = temp;
    }
    printf("初始化后的ReadyList：\n");
    showPCB();
    printf("\n");
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
	    strcpy(temp->pcb.state, "Ready");
	    //头结点的before用来记录当前链表的tail 
	    pcbID++;
    mtx.unlock();
    
	printf("id    |到达时间    |所需时间    |已用时间    |状态\n");
    printf("---------------------------------------------------\n"); 
	PCB pcb = temp->pcb;
	printf("%-6d|%-12d|%-12d|%-12d|%-12s\n", 
		pcb.id, pcb.arrivedTime, pcb.requiredTime, pcb.usedTime, pcb.state);
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