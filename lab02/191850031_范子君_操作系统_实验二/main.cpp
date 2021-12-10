#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<string>
#include<sstream>
#include<vector>
#include<map>

extern "C" { void asm_print(const char *, const int);}

using namespace std;

typedef unsigned char u8;	//1字节
typedef unsigned short u16;	//2字节
typedef unsigned int u32;	//4字节

int  BytsPerSec;	//每扇区字节数
int  SecPerClus;	//每簇扇区数
int  RsvdSecCnt;	//Boot记录占用的扇区数,即引导扇区所占用扇区数
int  NumFATs;	//FAT表个数
int  RootEntCnt;	//根目录最大文件数
int  FATSz;	//FAT扇区数
string output;

#pragma pack (1) //指定按1字节对齐

//偏移11个字节
struct Head {
    u16  BPB_BytsPerSec;	//每扇区字节数
    u8   BPB_SecPerClus;	//每簇扇区数
    u16  BPB_RsvdSecCnt;	//Boot记录占用的扇区数
    u8   BPB_NumFATs;	//FAT表个数
    u16  BPB_RootEntCnt;	//根目录最大文件数
    u16  BPB_TotSec16;		//扇区总数
    u8   BPB_Media;		//介质描述符
    u16  BPB_FATSz16;	//FAT扇区数
    u16  BPB_SecPerTrk;  //	每磁道扇区数（Sector/track）
    u16  BPB_NumHeads;	//磁头数（面数）
    u32  BPB_HiddSec;	//隐藏扇区数
    u32  BPB_TotSec32;	//如果BPB_FATSz16为0，该值为FAT扇区数
};

//根目录条目
struct FATEntry {
    char DIR_Name[11];      //文件名
    u8   DIR_Attr;		//文件属性
    char reserved[10];
    u16  DIR_WrtTime;
    u16  DIR_WrtDate;
    u16  DIR_FstClus;	//开始簇号
    u32  DIR_FileSize;      //文件大小
};

class Node{
public:
    string name;
    string path;
    vector<Node*> children;
    int dirCount;
    int fileCount;
    bool isDir;
    bool isRoot;
    char content[10000];
    int size;
    Node(string name,bool isDir){
        this->name = name;
        this->dirCount = 0;
        this->fileCount = 0;
        this->isDir = isDir;
        this->isRoot = false;
    }
};
void fillBPB(FILE * fat12, struct Head* bpb_ptr);	//载入BPB
void readFile(FILE * fat12,Node* root);
void readContent(FILE * fat12,Node * file,int current);
void readChildren(FILE*	fat12,Node *node,int current);
int nextFat(FILE *fat12,int currentVal);
void showResult(Node * root,map<string,Node*>* mapNode);
void split(const string &s, vector<string> &sv, const char flag = ' ');
void showNode(Node * root);
void showDetailNode(Node * root);
void myPrint(const char* p);

int main() {
    FILE* fat12;
    fat12 = fopen("a.img", "rb");	//打开FAT12的映像文件

    //S1：获取扇区基本信息
    struct Head head;  //创建引导扇区实体
    struct Head* head_ptr = &head; //创建引导扇区指针
    fillBPB(fat12, head_ptr);  //载入BPB：将实际引导扇区映射到Head结构中
    BytsPerSec = head_ptr->BPB_BytsPerSec;	//每扇区字节数
    SecPerClus = head_ptr->BPB_SecPerClus;	//每簇扇区数
    RsvdSecCnt = head_ptr->BPB_RsvdSecCnt;	//Boot记录占用的扇区数
    NumFATs = head_ptr->BPB_NumFATs;   //FAT表个数
    RootEntCnt = head_ptr->BPB_RootEntCnt;	//根目录最大文件数
    FATSz = head_ptr->BPB_FATSz16; //每FAT扇区数
	
    Node root("/",true);
    root.isRoot = true;
    root.path = "/";
    Node* root_ptr = &root;
    
    readFile(fat12,root_ptr);
    
    // cout<<"结束!"<<endl;
    map<string,Node*> mapNode ;
    mapNode.insert(pair<string,Node*>(root.path,root_ptr));
    showResult(&root,&mapNode);
    Node* temp;
    
    while(true){
        // cout<<">";
        myPrint(">");
        string input;
        vector<string> input_list;
        getline(cin,input);
        split(input, input_list, ' ');
        for (auto it = input_list.begin(); it != input_list.end();) {//删除数组中空格产生的空位置
            if (*it=="") {
                it = input_list.erase(it);
            }
            else {
                it++;
            }
        }
        if(input_list[0].compare("exit") == 0){
            // cout<<"退出程序"<<endl;
            myPrint("退出程序\n");
            fclose(fat12);
            return 0;
        }
        else if(input_list[0].compare("ls") == 0){
            if(input_list.size() == 1){
                auto iter = mapNode.find("/");
                if(iter != mapNode.end()){
                    temp = iter->second;
                }
                showNode(temp);	
            }
            else{
            //ls -l
                if(input_list[1].compare("-l")==0||input_list[1].compare("-ll")==0){
                    if(input_list.size() == 2){
                        auto iter = mapNode.find("/");
                        if(iter != mapNode.end()){
                            temp = iter->second;
                            showDetailNode(temp);
                        }
                    }
                    else{
                        auto iter = mapNode.find(input_list[2]+"/");
                        if(iter != mapNode.end()){
                            temp = iter->second;
                        }
                        showDetailNode(temp);
                    }
                }
                else if(input_list.size()==3 && (input_list[2].compare("-   l")==0 || input_list[2].compare("-ll")==0)){
                    auto iter = mapNode.find(input_list[1]+"/");
                    if(iter != mapNode.end()){
                        temp = iter->second;
                        showDetailNode(temp);
                    }
                }
                // auto iter = mapNode.find(input_list[1]);
                // if(iter != mapNode.end()){
                // 	temp = iter->second;
                // 	cout<<"喜喜"<<endl;
                // }
                // else{
                // 	cout<<"找不到该目录"<<endl;
                // }
            }
        }
        else if(input_list[0].compare("cat") == 0){
            if(input_list.size() == 1){
            // cout<<"缺少文件名"<<endl;
            myPrint("缺少文件名\n");
            }
            else{
                temp = mapNode[input_list[1]];
                // cout<<temp->content<<endl;	
                output = temp->content;
                myPrint(output.c_str());
            }
        }
    }
    return 0 ;
}

//载入BPB：将实际引导扇区映射到Head结构中
/*
 FILE* fat12 : 文件输入流，从引导扇区0偏移处起始
 struct Head* head_ptr ：引导扇区中的BPB映射到的Head结构的指针
 */
void fillBPB(FILE* fat12, struct Head* head_ptr) {  
    // 暂存函数返回值	
    int temp;

    //BPB从引导扇区偏移11个字节处开始,因为开头的跳转指令和厂商名占用11个字节
    /*
    int fseek(FILE *stream, long int offset, int whence查找字节数)
    相对 whence 的偏移量，以字节为单位。
    whence -- 这是表示开始添加偏移 offset 的位置。它一般指定为下列常量之一：
    SEEK_SET	文件的开头
    */
    temp = fseek(fat12, 11, SEEK_SET);

    //BPB长度为25字节
    /*
    size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
    ptr -- 这是指向带有最小尺寸 size*nmemb 字节的内存块的指针。
    size -- 这是要读取的每个元素的大小，以字节为单位。
    nmemb -- 这是元素的个数，每个元素的大小为 size 字节。
    stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输入流。
    */
    temp = fread(head_ptr, 1, 25, fat12);
    
}

void readFile(FILE * fat12,Node* root){
    // 根目录区地址 = 扇区字节数 × （引导扇区大小 + FAT表个数 × FAT表大小）
    int catalogBase = BytsPerSec*(RsvdSecCnt + NumFATs * FATSz);
    int count = 0;
    int check = 0;
    while(count < RootEntCnt){ //便利每个条目
        check = fseek(fat12,catalogBase+count*32,SEEK_SET);
        /*if(check == -1){
            // cout<<"定位根目录失败了"<<endl;
            myPrint("定位根目录失败了\n");
            break;
        }*/
        FATEntry entry;
        FATEntry *entry_ptr = &entry;
        check = fread(entry_ptr,1,32,fat12);
        /*if(check != 32){
            // cout<<"entry读取失败了"<<endl;
            myPrint("entry读取失败了\n");
        }*/
        count++;
        
        //判断该条目是否为空，如果为空则检查下一个循环
        if(entry_ptr->DIR_Name[0] == '\0') {continue;}
        
        int j;
        int isFileName = 0;  //0 则文件名合法可输出，1则不合法
        for (j = 0; j < 11; j++) {
            if (
              !(//数字
                ((entry_ptr->DIR_Name[j] >= 48) && (entry_ptr->DIR_Name[j] <= 57)) ||
                //大写英文
                ((entry_ptr->DIR_Name[j] >= 65) && (entry_ptr->DIR_Name[j] <= 90)) ||
                //小写英文
                ((entry_ptr->DIR_Name[j] >= 97) && (entry_ptr->DIR_Name[j] <= 122)) ||
                //空格
                (entry_ptr->DIR_Name[j] == ' ')
              )
            ) {
                isFileName = 1;	//非英文及数字、空格
                break;
            }
        }
        if (isFileName == 1){ 
            continue;	//非目标文件不输出
        }
        if((entry_ptr->DIR_Attr & 0x10) == 0){
            //是文件
            char temp[12];
            int temp_index=0;
            int k = 0;
            //entry_ptr->DIR_Name：filename . extension
            //filename
            while(k<12 && entry_ptr->DIR_Name[k] != ' '){
                temp[temp_index++] = entry_ptr->DIR_Name[k++];
            }
            // .
            temp[temp_index++] = '.';
            //跳过空格
            while(entry_ptr->DIR_Name[k] == ' '){
                k++;
            }
            //extension
            while(k<12 && entry_ptr->DIR_Name[k]!=' '){
                temp[temp_index++] = entry_ptr->DIR_Name[k++];
            }
            //添加 \0标志文件名结尾
            while(temp_index<12){temp[temp_index++] = '\0';}
            
            //创建新节点
            Node *f = new Node(temp,false);
            //添加路径
            f->path = root->path+ f->name;
            //将节点连接到根上
            root->children.push_back(f);
            //将根filecount加1
            root->fileCount++;
            readContent(fat12,f,entry_ptr->DIR_FstClus);
        }
        else{
            //目录
            char temp[12];
            int point = 0;
            int k=0;
            while(k<12 && entry_ptr->DIR_Name[k] != ' '){
                temp[point++] = entry_ptr->DIR_Name[k++];
            }
            while(point<12){temp[point++] = '\0';}
                Node *dir = new Node(temp,true);
                dir->path = root->path + dir->name+'/';
                root->children.push_back(dir);
                root->dirCount++;
                // cout<<"目录"<<dir->name<<endl;
                readChildren(fat12,dir,entry_ptr->DIR_FstClus);
            }
	}
}

void readContent(FILE * fat12,Node * file,int current){
    /* 
     数据区基地址 = 
     每扇区字节数 × （
     引导扇区占用扇区数 + FAT表个数 × FAT表大小 + 
    （根目录文件数 × 32 + 每扇区字节数 - 1）/ 每扇区字节数 //根目录所占扇区数
    ）
    */
    int dataBase = BytsPerSec * (RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec);
    int currentClus = current;  //簇数
    int value = 0;		//这里用value来进行不同簇的读取（超过512字节）
    char *p = file->content;
    if (currentClus == 0) {
        return;
    }
    int size = 0;
    while (value < 0xFF8) {
        value = nextFat(fat12, currentClus);//获取下一个簇
        if (value == 0xFF7) {
            cout<<"坏簇，无法读取\n";
            break;
        }
        //创建一个簇大小的空间str
        char* str = (char*)malloc(SecPerClus*BytsPerSec);
        char *content = str;
        // ?currentClus需要-2吗
        int startByte = dataBase + (currentClus-2)*SecPerClus*BytsPerSec;
        int check;
        check = fseek(fat12, startByte, SEEK_SET);
        if (check == -1){
            cout<<"fseek in printChildren failed!";
        }
        check = fread(content, 1, SecPerClus*BytsPerSec, fat12);//提取数据
        if (check != SecPerClus * BytsPerSec){
              cout<<"fread in printChildren failed!";
        }
        int count = SecPerClus * BytsPerSec;
        int loop = 0;
        for (int i = 0; i < count; i++) {//读取赋值
            size++;
            *p = content[i];
            p++;
        }
        //释放空间
        free(str);
        currentClus = value;
    }	
    file->size = strlen(file->content);
}


int nextFat(FILE *fat12,int FATEntryNum){
    //FAT表基地址
    int fatBase = RsvdSecCnt * BytsPerSec;
    //FATEntry的偏移字节
    int fatPos = fatBase + FATEntryNum * 3 / 2;
    //奇偶FAT项处理方式不同，分类进行处理，从0号FAT项开始
    int type = 0;
    if (FATEntryNum % 2 == 0) {
        type = 0;
    }
    else {
        type = 1;
    }
    //先读出FAT项所在的两个字节
    u16 bytes;
    u16* bytes_ptr = &bytes;
    int check;
    check = fseek(fat12, fatPos, SEEK_SET);
    check = fread(bytes_ptr, 1, 2, fat12);
    if (type == 1) {
        //type为1，取byte2和byte1的高4位构成的值
        return bytes >> 4;
    }
    else {
        //type为0，偶项，取byte2的低4位和byte1构成的值
        bytes = bytes << 4;  
        return (bytes >> 4);
    }
}



void showNode(Node * root){
	output = root->path;
	output += ":\n";
	myPrint(output.c_str());
	// cout<<(root->path)<<":"<<endl;
	if(!root->isRoot){
		output = "\033[31m";
		output += ". .. ";
		output += "\033[0m";
		myPrint(output.c_str());
		// cout<<"\033[31m"<<"."<<" "<<".."<<"\033[0m"<<" ";
	}
	vector<Node*>children = root->children;
	for(int i=0;i<children.size();i++){
		if(children[i]->isDir){
			output = "\033[31m";
			output += children[i]->name;
			output += "\033[0m";
			output += " ";
			myPrint(output.c_str());
			// cout<<"\033[31m"<<children[i]->name<<"  "<<"\033[0m";//这里需要红色输出
		}
		else{
			output = children[i]->name;
			output+=" ";
			myPrint(output.c_str());
			// cout<<children[i]->name<<"  ";
		}
	}	
	cout<<endl;
	for(int i=0;i<children.size();i++){
		if(children[i]->isDir){
			showNode(children[i]);
		}
	}
}

void showDetailNode(Node * root){
	output = root->path +" "+ to_string(root->dirCount)+" "+to_string(root->fileCount)+":\n";
	// cout<<(root->path+" "+ to_string(root->dirCount)+" "+to_string(root->fileCount)+":\n");
	myPrint(output.c_str());
	vector<Node*>children = root->children;
	if(!root->isRoot){
		output = "\033[31m";
		output += ".\n..";
		output += "\033[0m";
		output += "\n";
		myPrint(output.c_str());
		// cout<<"\033[31m"<<"."<<endl;
		// cout<<".."<<"\033[0m"<<endl;
	}
	for(int i=0;i<children.size();i++){
		if(children[i]->isDir){
			output = "\033[31m";
			output += children[i]->name;
			output += "\033[0m";
			output += " ";
			output += to_string(children[i]->dirCount);
			output += " ";
			output += to_string(children[i]->fileCount);
			output += "\n";
			myPrint(output.c_str());
			// cout<<"\033[31m"<<children[i]->name<<"  "<<"\033[0m";//这里需要红色输出
			// cout<<children[i]->dirCount<<" "<<children[i]->fileCount<<endl;
		}
		else{
			output = children[i]->name;
			output += "  ";
			output += to_string(children[i]->size);
			output += "\n";
			myPrint(output.c_str());
			// cout<<children[i]->name<<"  ";
			// cout<<children[i]->size<<endl;
		}
	}
	cout<<endl;
	for(int i=0;i<children.size();i++){
		if(children[i]->isDir){
			showDetailNode(children[i]);
		}
	}	
	

}

void showResult(Node * root, map<string,Node*>* mapNode){
	mapNode->insert(pair<string,Node*>(root->path,root));
	if(root->isDir){
		// cout<<"文件夹"<<root->name<<endl;
		vector<Node *> nodeList = root->children;
		for(int i=0;i<nodeList.size();i++){
			showResult(nodeList[i],mapNode);
		}
	}

}

void readChildren(FILE*	fat12,Node *node,int current){
	int dataBase = BytsPerSec * (RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec);
	int currentClus = current;
	int count = 0;
	int check = 0;
	int value = 0;
	while(value<0xFF8){
		value = nextFat(fat12, currentClus);//获取下一个簇
		if (value == 0xFF7
			) {
			cout<<"坏簇，读取失败!\n";
			break;
		}
		while(count<BytsPerSec){
			int startByte = dataBase+(currentClus-2)*SecPerClus*BytsPerSec+count;
			check = fseek(fat12,startByte,SEEK_SET);
			if(check == -1){
				cout<<"子文件夹定位错误";
			}
			FATEntry entry;
			FATEntry *entry_ptr = &entry;
			check = fread(entry_ptr,1,32,fat12);
			if(check != 32){
				cout<<"子文件夹读取错误";
			}
			count+=32;


			if(entry_ptr->DIR_Name[0] == '\0') {continue;}
			int j;
			int boolean = 0;
			for (j = 0; j < 11; j++) {
				if (!(((entry_ptr->DIR_Name[j] >= 48) && (entry_ptr->DIR_Name[j] <= 57)) ||
				((entry_ptr->DIR_Name[j] >= 65) && (entry_ptr->DIR_Name[j] <= 90)) ||
				((entry_ptr->DIR_Name[j] >= 97) && (entry_ptr->DIR_Name[j] <= 122)) ||
				(entry_ptr->DIR_Name[j] == ' '))) {
				boolean = 1;	//非英文及数字、空格
				break;
			}
		}
			if (boolean == 1) continue;	//非目标文件不输出

			if((entry_ptr->DIR_Attr & 0x10) == 0){
				//文件
				char temp[12];
				int point=0;
				int k = 0;
				while(k<12 && entry_ptr->DIR_Name[k] != ' '){
					temp[point++] = entry_ptr->DIR_Name[k++];
				}
				temp[point++] = '.';
				while(entry_ptr->DIR_Name[k] == ' '){
						k++;
				}
		
				while(k<12 && entry_ptr->DIR_Name[k]!=' '){
					temp[point++] = entry_ptr->DIR_Name[k++];
				}
				while(point<12){temp[point++] = '\0';}
				Node *f = new Node(temp,false);
				f->path = node->path+ f->name;
				node->children.push_back(f);
				node->fileCount++;
				readContent(fat12,f,entry_ptr->DIR_FstClus);
				// cout<<"子文件夹文件"<<f->name<<endl;
				// cout<<"子文件夹内容"<<f->content<<endl;
			}
			else{//目录
				char temp[12];
				int point = 0;
				int k=0;
				while(k<12 && entry_ptr->DIR_Name[k] != ' '){
					temp[point++] = entry_ptr->DIR_Name[k++];
				}
				while(point<12){temp[point++] = '\0';}
				Node *dir = new Node(temp,true);
				dir->path = node->path + dir->name+'/';
				node->children.push_back(dir);
				node->dirCount++;
				// cout<<"子文件夹目录"<<dir->name<<endl;
				readChildren(fat12,dir,entry_ptr->DIR_FstClus);
			}
			

		}
	}
	return;


}



void split(const string &s, vector<string> &sv, const char flag ) {
	sv.clear();
	istringstream iss(s);
	string temp;
	while (getline(iss, temp, flag)) {
		sv.push_back(temp);
	}
	return;
}

void myPrint(const char* p){
	asm_print(p,strlen(p));
}
