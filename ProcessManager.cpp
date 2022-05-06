#include<bits/stdc++.h> 
#include <windows.h>

#define MAX 5  // 定义最大进程数
#define TIME 6 // 定义时间片大小，单位：秒（s） 

using namespace std;

int mycount ;
typedef struct pcb {
//	进程id
    int id;
//  到达时间
    int arrivedTime;
//  运行所需要的时间
    int needTime;
//  已使用的时间
    int usedTime;
//  三种状态：Ready、Running、Dead
    char state[6];
}pcb, *pPCB;

//进程组织：链接方式，here采用双向链表（带头节点）
typedef struct PCBNode {
	pPCB pcb;
//	上一个PCB
	PCBNode *before;
//  下一个PCB
    PCBNode *next;
}PCBNode, *pPCBNode;

pPCBNode ReadyList = new PCBNode;

// 打印输出PCB链表，直观展现各个PCB状态 
void showPCB() {
	pPCBNode p = ReadyList->next;
    printf("id    |到达时间    |所需时间    |已用时间    |状态\n");
    printf("---------------------------------------------------\n"); 
	while(p) {
		pPCB pcb = p->pcb;
		printf("%-6d|%-12d|%-12d|%-12d|%-12s\n", 
			pcb->id, pcb->arrivedTime, pcb->needTime, pcb->usedTime, pcb->state);
		p = p->next;
	}
}

// 初始化PCB
void initPCB() {
    pPCBNode p = ReadyList;
    for (int i = 0; i < MAX; i++) {
    	pPCBNode temp = new PCBNode;
    	temp->pcb = (pPCB)malloc(sizeof(pcb));
        temp->pcb->id = i;
        temp->pcb->arrivedTime = rand() % 10 + 1;
        temp->pcb->needTime = rand() % 10 + 1;
        temp->pcb->usedTime = 0;
        strcpy(temp->pcb->state, "Ready");
		temp->before = p;
        temp->next = NULL;
        p->next = temp;
        p = temp;
        //头结点的before用来记录当前链表的尾结点（避免还要单独建一个tail标记尾结点） 
        ReadyList->before = temp;
    }
    printf("初始化后的ReadyList：\n");
    showPCB();
    printf("\n");
}

// 检查ReadyList中是否还有未执行的PCB
pPCBNode checkReadyList() {
	pPCBNode p = ReadyList->next;
	while(p) {
		if (strcmp(p->pcb->state, "Ready") == 0) {
			return p;
		}
		p = p->next;
	}
	return NULL;
}

// 判断当前PCB是否在ReadyList最有优先执行资格
pPCBNode checkPCB(pPCBNode &p) {
	pPCBNode temp = p->next;
	while(temp) {
		if (strcmp(temp->pcb->state, "Ready") == 0 && (temp->pcb->arrivedTime + temp->pcb->usedTime) < (p->pcb->arrivedTime + p->pcb->usedTime)){
			p = temp;
		}
		temp = temp->next;
	}
	return p;
}

// 运行 
void Running() {
	pPCBNode p = NULL;
	while(p = checkReadyList()) {
		// checkPCB(p);
		if (p != NULL) {
			printf("此次要执行的PCB的id=%d\n", p->pcb->id);
			strcpy(p->pcb->state, "Running");
			printf("执行中......\n");
			showPCB();
            cout << mycount++;

            int remainingTime = p->pcb->needTime - p->pcb->usedTime;
			if (remainingTime <= TIME) {
				Sleep(remainingTime * 1000);
				strcpy(p->pcb->state, "Dead");
				p->pcb->usedTime = p->pcb->needTime;
				printf("执行完毕，id=%d的PCB状态转为“Dead”\n", p->pcb->id);
                if (p->next) {
					p->before->next = p->next;
					p->next->before = p->before;
					delete p;
				}
			} else {
                Sleep(TIME * 1000);
				strcpy(p->pcb->state, "Ready");
				p->pcb->usedTime += TIME;
//				当前未执行完毕的PCB连接到链表尾
				if (p->next) {
					p->before->next = p->next;
					p->next->before = p->before;
					p->before = ReadyList->before;
					ReadyList->before->next = p;
					ReadyList->before = p;
					p->next = NULL;
				}
				printf("时间片到，还未执行完毕，id=%d的PCB状态转为“Ready”\n", p->pcb->id);
			}
		}
		// showPCB();
		printf("\n");
	}
}


int main() {
	initPCB();
	Running();
	return 0;
}