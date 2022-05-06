#include<bits/stdc++.h> 
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
void creatPCB();
void block(int id);

// 打印输出PCB链表，直观展现各个PCB状态 
void showPCB();
// 初始化PCB
void initPCB();
// 检查ReadyList中是否还有未执行的PCB
pPCBNode checkReadyList();
// 运行 
void Running();


// mutex mtx;
int monitor;
int pcbID;
int mycount ;
pPCBNode ReadyList = new PCBNode;

int main() {
	initPCB();
	thread myThread ( Running );
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
		}else if(out == "moni"){
			monitor = 1 ;
		}else if(out == "cls"){
			monitor = 0 ;
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
        temp->pcb.arrivedTime = rand() % 2 + i*1;
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
pPCBNode checkReadyList() {
//    mtx.lock();
	pPCBNode p = ReadyList->next;
	while(p) {
		if (strcmp(p->pcb.state, "Ready") == 0) {
			return p;
		}
		p = p->next;
	}
//    mtx.unlock();
	return NULL;
}


// 运行 
void Running() {
	pPCBNode p = NULL;
	while(1) {
        p = checkReadyList();
		// checkPCB(p);
		if (p != NULL) {
            // cout << "第" <<mycount++<<"次切换"<<endl;
			// printf("此次要执行的PCB的id=%d\n", p->pcb.id);
			// printf("执行中......\n");
			// showPCB();

			strcpy(p->pcb.state, "Running");
            int remainingTime = p->pcb.requiredTime - p->pcb.usedTime;
			if (remainingTime <= TIME) {
				Sleep(remainingTime * 1000);
				strcpy(p->pcb.state, "Dead");
				p->pcb.usedTime = p->pcb.requiredTime;
				if(monitor == 1) printf("\nid=%d执行完毕，PCB状态转为“Dead”",  p->pcb.id);
//TO DO 封装成进程终止原语
				kill(p);

//TO DO 进程占用的内存空间的回收

			} else {
                Sleep(TIME * 1000);
				strcpy(p->pcb.state, "Ready");
				p->pcb.usedTime += TIME;
                //当前未执行完毕的PCB连接到链表尾
				if (p->next) {
//TO DO 封装成进程切换原语
					schedule(p);
				}
				if(monitor == 1) printf("\n时间片到，id=%d的还未执行完毕，PCB状态转为“Ready”", p->pcb.id);
			}
		}
		// showPCB();
	}
}

void creatPCB(){
//    mtx.lock();
    pPCBNode temp = new PCBNode;
    //指针域赋值
    temp->before = ReadyList->before;
    temp->next = NULL;
    ReadyList->before->next = temp;
    ReadyList->before = temp;
    //数据域赋值
    // PCB pcb = temp->pcb;
    temp->pcb.id = pcbID;
    temp->pcb.arrivedTime = rand() % 2 + pcbID*3;
    temp->pcb.requiredTime = rand() % 10 + 1;
    temp->pcb.usedTime = 0;
    strcpy(temp->pcb.state, "Ready");
    //头结点的before用来记录当前链表的尾结点（避免还要单独建一个tail标记尾结点） 
    pcbID++;
//    mtx.unlock();
	printf("id    |到达时间    |所需时间    |已用时间    |状态\n");
    printf("---------------------------------------------------\n"); 
	PCB pcb = temp->pcb;
	printf("%-6d|%-12d|%-12d|%-12d|%-12s\n", 
		pcb.id, pcb.arrivedTime, pcb.requiredTime, pcb.usedTime, pcb.state);
	puts("");
}

void schedule(pPCBNode &p){
    p->before->next = p->next;
    p->next->before = p->before;
    p->before = ReadyList->before;
    ReadyList->before->next = p;
    ReadyList->before = p;
    p->next = NULL;    
}

void kill(pPCBNode &p){
	if (p->next) {
	    p->before->next = p->next;
	    p->next->before = p->before;
	    delete p;
	}else{
		p->before->next = p->next;
		ReadyList->before = ReadyList;
		delete p;
	}

}

void block(int id){
    pPCBNode p = ReadyList->next;
    while(p) {
		if(p->pcb.id == id){
            strcpy(p->pcb.state, "Blocked");
            printf("id=%d被阻塞\n", id);
            return;
        }
		else{
            p = p->next;
        }
	}
    //不存在指定要阻塞的进程
	puts("error");
}
