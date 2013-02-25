/**************************************************/
/* ASSIGNMENT #1 :           Mini Disk Scheduler  */
/* STUDENTID     :                    1999110093  */
/* NAME          :                      배 철 희  */
/* DATE          :                   2001. 3. 30  */
/*                                                */
/* COMPILE       : Sparc SUN SOLARIS g++ ver2.8.1 */
/*               : MS VC++ 6.0                    */
/*               : Turbo C++ ver3.0               */
/*               : Linux GNU                      */
/**************************************************/
#include <fstream.h>
#include <string.h>
#include <stdlib.h>

const int    TRACK  =         20;
const int    SECTOR =          8;
const int    ARMMOVETIME =     1;
const double SECREADTIME =   0.5;


class File;
class Node;
class Queue;
class LinkedList;
class FCFS;
class SCAN;
class Scheduler;

//////////////////////////////////////////////////////////////////////////////////
//  <File CLASS>                                                                //
//  파일 열기, 파일 닫기, 읽은 파일의 줄 단위 버퍼화, 각 데이타 별로 자르기     //
//  "display" 문자열 체크, 출력파일 생성, 출력.                                 //
//////////////////////////////////////////////////////////////////////////////////
class File
{
    private:
        int l;
        char Buf[64];
        char *filename;
        ifstream fin;
        ofstream fout;
    public:
        File(char *fn)           { filename = fn; }
        void FileOutOpen()       { fout.open("output.txt"); }
        void FileOutClose()      { fout.close(); }
        void FileOut(char *buf)  { fout << buf; }
        void FileOut(double t)   { fout << t; }
        void FileOut(int t)      { fout << t; }
        char * GetLine(int l);
        void Token(double &, int &, double &, char*);
        int isDisplay(char *line);
};
/********************************************************************************/
/*  char * GetLine(int line)  <class File>                                      */
/*  input파일을 변수로 읽어서 파일을 열고 getline()함수로 line단위로 버퍼화     */
/*  시키는 함수. 인수로 원하는 라인번호를 주어 원하는 라인을 뽑아 낼 수 있다.   */
/*  단점은 받아들인 인수만큼 getline()함수가 실행됨.                            */
/********************************************************************************/      
char * File::GetLine(int l)
{
    int i=0;
    
    fin.open(filename);
    if( fin == NULL )
    {  // input파일의 존재 유무 체크
       cout << filename << " file not found.\n"; 
       exit(1);
    }
    while( !fin.eof() ) //파일의 끝을 체크하며 루프를 돌며 인수와 비교.
    {
       fin.getline(Buf, 64); 
       if( !strlen(Buf) ) 
            continue;   //중간에 공백으로된 라인을 만났을 경우.
       if( i == l )
       {
           fin.close();
           return Buf;  //일치할 경우 return.
       }
       i++;
    }
    fin.close();
    return NULL;
}

/********************************************************************************/
/*  int isDisplay(char *line_buf)  <class File>                                 */
/*  "display" 문자열 체크.                                                      */
/*  문자열 버퍼가 "display와 일치하면 1, 틀리면 0을 리턴한다.                   */
/********************************************************************************/
int File::isDisplay(char *line)
{
    if( strlen(line) < 5 )
    {
        cout << "input file has not available strings. check it out!\n";
        exit(0);
    }
    if( strncmp(line, "display", 7) == 0 )
        return 1;

    return 0;
}

/********************************************************************************/
/* void Token(double &time_val, int &track_val, double &second_val, char * line)*/
/* <class File>                                                                 */
/* 노드 리퀘스트들이 담긴 각각의 라인들을 인수로 받고 그 곳에 들어있는 데이타들,*/
/* time, track, sector의 값들을 인자로 다시 받아서 값을 변화 시키는 역할을 함.  */
/********************************************************************************/
void File::Token(double &tm, int &trk, double &sec, char *line)
{
    char str[64];
    char *tmp_tm;
    char *tmp_trk;
    char *tmp_sec;
	    
    strncpy(str, line, 64);
    tmp_tm = strtok(str, " ");
    tmp_trk = strtok(NULL, " ");
    tmp_sec = strtok(NULL, " ");
    
    tm = atof(tmp_tm);
    trk = atoi(tmp_trk);
    sec = atoi(tmp_sec);
}
//////////////////////////////////////////////////////////////////////////////////
// <Node CLASS>                                                                 //
// 디스크상의 리퀘스트들의 발생시간과 track, sector의 위치를 가지고 있는 class  //
// 자신의 track과 sector의 위치 정보를 알 수 있는 멤버함수와 인접 노드의 주소를 //
// 알 수 있는 next 멤버 변수가 있다.                                            //
// linked list를 구성하기위한 Queue, LinkedList클래스들을 friend로 선언.        //
//////////////////////////////////////////////////////////////////////////////////
class Node
{
    friend class Queue;
    friend class LinkedList;
    private:
       double time;
       int track;
       int sector;
       Node *next;
       int modular; //두가지 sorting 옵션에 사용될 변수. 자신의 섹터에 8을 더한다.
    public:
       Node(double tm, int trk, int sec);
       Node();
       ~Node();
       int tracktime() { return track; }    //해당 멤버변수를 단순히 리턴하는 함수.
       int sectortime() { return sector; }
};
/********************************************************************************/
/*  Node(double time, int track, int sector)                                    */
/*  Node객체 초기화하는 생성자 함수 각각 생성되면서 값을 선언할 수 있는 것과    */
/*  0이나 NULL값으로 초기화 시키는 인자없는 생성자로서 2개.                     */
/********************************************************************************/
Node::Node(double tm, int trk, int sec)
{
    time = tm;
    track = trk;
    sector = sec;
    next = NULL;
    modular = 8 + sector;
}
Node::Node()
{
    time = 0;
    track = 0;
    sector = 0;
    next = NULL;
    modular = 8 + sector;    
}
Node::~Node()
{}
		
//////////////////////////////////////////////////////////////////////////////////
// <Queue CLASS>                                                                //
// FCFS 스케쥴러 구현을 위한 FIFO방식의 자료 구조를 다룰 수 있는 클래스.        //
// 리스트의 처음과 끝을 가르킬 수있는 주소를 가지고 있다. 큐에 입력을 위해      //
// 리스트의 끝에 추가시키는 input()함수와 큐의 앞을 지워버리는 output().        //
// 마지막으로 각 노드들간의 주소를 따라가며 각각의 데이타들을 출력하는          //
// show()함수가 있다.                                                       //
////////////////////////////////////////////////////////////////////////////////// 
class Queue
{
    private:
       Node *front;
       Node *rear;
    public:
       Queue();
       ~Queue();
       int input(char *Line);    
       Node * output(); 
       void show(File *);
};
/********************************************************************************/
/* Queue()의 생성자 함수. front와 rear를 NULL로 초기화한다.                     */
/********************************************************************************/
Queue::Queue()
{
    front = NULL;
    rear = NULL;
}    
Queue::~Queue()
{}
/********************************************************************************/
/* int input(char *line_buf)  <class Queue>                                     */
/* 각각의 데이타로 잘려지지 않은 상태의 라인단위의 버퍼를 입력값으로 받은 후에  */
/* track, sector, time을 strtok()로 잘라낸 후 노드를 생성하여 그 데이타로 초기화*/
/* 하고 Queue를 구성하는 함수. 입력될 노드의 track값을 리턴한다.                */
/********************************************************************************/
int Queue::input(char *Line)
{
    char *tm_tmp;
    char *trk_tmp;
    char *sec_tmp;

    tm_tmp = strtok(Line, " ");
    trk_tmp = strtok(NULL, " ");
    sec_tmp = strtok(NULL, " ");

    double tm = atof(tm_tmp);
    int trk = atoi(trk_tmp);
    int sec = atoi(sec_tmp);

    Node *node = new Node( tm, trk, sec);
    if( front == NULL ) //Queue가 비어있는 경우.
    { 
       front = node; //Queue에 집어 넣으면 최초의 하나의 노드가 되므로 그것은
       rear = node;  //front이자 rear가 된다.
    }
    else  //Queue가 비어있지 않은 경우.
    {
       rear->next = node; //맨 끝인 rear의 next에 새 노드를 연결하고 그것을 다시
       rear = node;       //rear로 만든다.
    }
    return node->track;
}

/********************************************************************************/
/*  Node * output()  <class Queue>                                              */
/*  Queue에 있는 맨 끝의 Node를 제거하는 역할과 제거된 Node의 복사본을 리턴하는 */
/*  함수이다.                                                                   */
/********************************************************************************/
Node * Queue::output()
{
    Node *Onode = new Node; //임의로 지워질 노드를 미리 복사한다.

    if( front == NULL ) return NULL; //지울 것이 하나도 없는 경우.

    Onode->time = front->time;   //지워질 노드의 데이타 복사.
    Onode->track = front->track;
    Onode->sector = front->sector;

    Node *temp = front; //임의의 front주소 복사.

    if( front == rear )  //하나의 노드가 존재할 경우. 
    {
       temp = front;
       front = NULL;     //마지막 하나의 노드를 삭제한다.
       delete temp;
    }
    else   //2개 이상의 노드가 존재할 경우.                  
    {
       front = front->next; //front의 next를 front로 만든 후...
       delete temp;         //복사된 주소를 삭제함으로서 기존의 front를 지운다.
    }
    return Onode; //복사된 노드를 리턴.
}

/********************************************************************************/
/*  void show(File *file_IO_object)  <class Queue>                          */
/*  이 함수가 불려질 당시의 Queue상에 존재하는 모든 노드의 데이타를 주소를 따라 */
/*  가며 output될 파일에 출력해주는 함수.                                       */
/********************************************************************************/ 
void Queue::show(File *f)
{
    for( Node *temp=front;temp!=0;temp=temp->next ) 
    {
        Node *current = temp;
        f->FileOut(current->track);  f->FileOut("  "); 
        f->FileOut(current->sector); f->FileOut("\n");
    }
    f->FileOut("\n");
}
//////////////////////////////////////////////////////////////////////////////////
// <FCFS class>                                                                 //
// Queue클래스 자체를 관리하며 이것을 이용한 FCFS방식의 디스크 스케쥴링을 수행  //
// 할 수 있게 만들어진 클래스. 파일상에서 빼내어진 라인 단위의 버퍼를 입력값으로//
// 받는 Queue의 input()함수를 이용한 Display()함수와 스케쥴링의 주동작을 하는  //
// process 함수로 구성.                                                         //
//////////////////////////////////////////////////////////////////////////////////
class FCFS
{
    public:
       FCFS()  { queue = new Queue; }//생성자 초기화
       ~FCFS();
       int Queueing(char *buf);
       void Display(File *f) { queue->show(f); }
       Node * Process();
    private:
       Queue *queue;

};
FCFS::~FCFS()
{}

/********************************************************************************/
/*  int Queueing(char *line_buf)  <class FCFS>                                  */
/*  Queue의 insert()함수가 그대로 불리워지는 함수.                              */
/********************************************************************************/
int FCFS::Queueing(char *buf)
{
    return queue->input(buf);
}

/********************************************************************************/
/*  Node * Process()  <class FCFS>                                              */
/*  인자없이 수행되며 Queue::output()함수를 이용하여 지워질 노드자체를 리턴받는 */
/*  함수. 이것을 이용하여 지워질 노드의 access time을 구할 수 있다.             */
/********************************************************************************/ 
Node * FCFS::Process()
{
    return queue->output(); //Will processed node;output fucntion called.
}

//////////////////////////////////////////////////////////////////////////////////
//  <LinkedList class>                                                          //
//  SCAN방식의 스케쥴러의 섹터들을 두 가지 방법으로 정렬하기위한 동작을 수행하는//
//  클래스. 앞으로 생성될 20개의 리스트들의 first를 access하기위해 friend       //
//  클래스로 SCAN클래스를 선언함.                                               //                 
//////////////////////////////////////////////////////////////////////////////////
class LinkedList
{   
    friend class SCAN;
    private:
       Node *front, *rear;
       Node *mid_front, *mid_rear;
    public:
       LinkedList();
       ~LinkedList();
       void insert(char *Line, int mode, int proc_sec);     
       void show(File *);
       Node *output();
};
/*******************************************************************************/
/* LinkedList() 생성자 함수. front와 rear의 초기화.                            */
/*******************************************************************************/
LinkedList::LinkedList()
{
    front = rear = NULL;
    mid_front = mid_rear = NULL; 
}
LinkedList::~LinkedList()
{}

/*******************************************************************************/
/*  void insert(char *buf_line, int sort_mode, int processing_sector)          */
/*  <class LinkedList>                                                         */
/*  Queue클래스와는 다른 enqueueing을 수행한다. 삭제되어 나가는 곳은 한 곳이나 */
/*  삽입되는 곳은 일반적인 sorting과 같은 트랙에서의 회전을 고려한 섹터의      */
/*  access sorting을 하기위한 sorting mode가 인자로 추가되었다.                */
/*  역시 FCFS의 노드 삽입처럼 라인단위의 버퍼를 입력받아 노드생성과 함깨 입력이*/
/*  이루어진다. 세번째 인자는 현재 수행되었던 섹터의 번호이다.                 */
/*******************************************************************************/
void LinkedList::insert(char *Line, int mode, int proc_sec)
{
    char *tm_tmp;
    char *trk_tmp;
    char *sec_tmp;
    
    int mod_sec;

    tm_tmp = strtok(Line, " ");    //라인 단위 문자열을 token한다.
    trk_tmp = strtok(NULL, " ");
    sec_tmp = strtok(NULL, " ");

    double tm = atof(tm_tmp);      //각각에 맞는 실제의 데이타 타입으로 변환.
    int trk = atoi(trk_tmp);
    int sec = atoi(sec_tmp);
    
    Node *temp, *temp2;
    //Node *mid_temp, *mid_temp2;
    temp = front;
    //mid_temp = mid_front;
    
    Node *node = new Node( tm, trk, sec );
    if( node == NULL )
    {
    	  cout << "memory not available.\n";
    	  exit(1);
    }
    
    if( front == NULL ) // 삽입이 처음 이루어지거나 비어 있는 경우.
    { 
        front = rear = node;
        return;
    }    

    // 트랙이 같고 현재 수행중인 섹터보다 작을 경우 8을 더해서 비교한다.
    if( (mode == 1) && (proc_sec >= node->sector) )
    	    mod_sec = node->modular;   
    else  // 그렇지 않을 경우 그대로 sector값을 넣는다.
    	  mod_sec = node->sector;
    	      
    if( front->sector > mod_sec )
    {
        node->next = front;        //front를 밀어내고 front가 되는 경우.
        front = node;
        return;
    }
    while( temp->sector < mod_sec )   
    {
	      if(temp->next == NULL ) // rear보다 큰 데이타를 가진 노드.
	      {
	           temp->next = node;
	           rear = node; //다시 자신이 rear가 된다.
	           return;
	      }              
	      temp2 = temp; // next로만 링크된 노드의 previous 노드의 주소를 알기위해.
        temp = temp->next;  // temp ++
    }
    node->next = temp2->next; //중간인 경우
    temp2->next = node;
}   

/*******************************************************************************/
/*  Node * output()  <class LinkedList>                                        */
/*  linked list의 삭제가 이루어지는 함수로서 특이한 점은 지워질 노드와 똑같은  */
/*  데이타를 가진 다른 주소의 객체를 리턴한다는 것이다.                        */
/*******************************************************************************/
Node * LinkedList::output()
{
    if( front == NULL )       // 리스트가 비었을 경우.
        return NULL;
    Node *Onode = new Node(front->time, front->track, front->sector);

    Node *temp = front;
       
    if( front == rear )       //남아있는 노드가 단 하나일 경우.
    {
        temp = front;
        front = rear = NULL;
        delete temp;
    }
    else                     //두개 이상의 노드.
    {	 
     	front = front->next;
        delete temp;
    }
    return Onode;            //복사시킨 노드형 객체를 리턴.
}

/*******************************************************************************/
/*  void show(File *file_object)  <class LinkedList>                       */
/*  LinkedList용 파일로의 출력 함수. traverse하며 처음으로 시작하여 끝에 도달  */
/*  할 때까지 프린트.                                                          */
/*******************************************************************************/
void LinkedList::show(File *f)
{
    f->FileOut(front->tracktime()); f->FileOut(" ");
    for( Node *temp=front;temp!=0;temp=temp->next ) 
    {
        Node *current = temp;
        f->FileOut(current->sectortime()); f->FileOut(" "); 
    }
    f->FileOut("\n");
}

/////////////////////////////////////////////////////////////////////////////////
// SCAN class                                                                  //
// 디스크 스케쥴러방식중 SCAN 알고리즘을 위한 클래스.  Queue클래스와 비슷한    //
// 멤버 함수와 변수들이 있지만 트랙별로 LinkedList를 가질 수 있는 배열과       //
// 각각의 배열리스트들의 first노드의 주소를 엑세스해 볼 수 있는 함수가 있다.   // 
/////////////////////////////////////////////////////////////////////////////////
class SCAN
{
    public:
        SCAN(); 
        ~SCAN();
        void Display(File *); 
        void Listing(int track, char *buf, int i, int sec) 
                              { LL[track-1]->insert(buf, i, sec);} 
        Node * Process(int);
        int isAllNull();
    private:
        LinkedList *LL[20];    
};

/********************************************************************************/
/* SCAN() 생성자함수, 20개의 트랙에 해당하는 LinkedList배열 포인터를 Heap에 생성*/
/********************************************************************************/
SCAN::SCAN()
{    // LinkedList 20개 생성.
    for(int i=0;i<20;i++)
        LL[i] = new LinkedList;
}   	
SCAN::~SCAN()
{}

/********************************************************************************/
/*  void Display(File *file_object)  <class SCAN>                              */
/*  LinkedList클래스의 멤버함수 show()를 호출하되 각각의 트랙에 대하여      */
/*  모두 수행한다. 하나의 트랙당 더이상 노드연결이 없을 때까지 20번 수행한다.   */
/********************************************************************************/  	  
void SCAN::Display(File *f)
{ //Queue에서의 Display 함수와는 다른 것이 바로 배열을 따라가며 20번 찍는다는 것이다.
   for(int i=0;i<20;i++)
       if( LL[i]->front != NULL )
            LL[i]->show(f);
}

/********************************************************************************/
/*  Node * Process(int track_number)  <class SCAN>                              */
/*  트랙넘버를 입력으로 받아서(1~20) 배열 변위(0~19)로 변환해서 해당하는        */
/*  변위의 LinkedList의 first 노드를 삭제한다. 삭제한 노드는 이전에 복사해서    */
/*  리턴한다.                                                                   */ 
/********************************************************************************/
Node * SCAN::Process(int trk)
{
    Node *n;
    
    if( (n = LL[trk-1]->output()) == NULL)
        return NULL;  //더이상 삭제되지 않는 경우.
    else
        return n;
}

/********************************************************************************/
/*  int isAllNull()   <class SCAN>                                              */
/*  모든 트랙에 해당하는 20개의 LinkedList모두가 하나라도 리퀘스트 노드가       */
/*  남아있는지 아닌지를 체크한다.                                               */
/********************************************************************************/
int SCAN::isAllNull()
{
    int Al=0;
    for(int i=0;i<20;i++)
    {
       if( LL[i]->front == NULL )
           Al++;   //NULL인 경우를 카운트.
    }
    if(Al == 20 ) return 0;
    else          return 1;
}   	     
//////////////////////////////////////////////////////////////////////////////////
//  Scheduler class                                                             //
//  FCFS방식과 SCAN방식 스케쥴러의 전체적인 기능을 수행할 수 있도록하는 함수    //
//  와 현재 리스트나 큐에 남아있는 노드들을 좇아 파일에 출력하는 함수를         //
//  가지는 단순한 클래스.                                                       //
//////////////////////////////////////////////////////////////////////////////////
class Scheduler
{
    public:      
       void fcfsprocess(File *);
       void scanprocess(File *);
       void Traversing(File *);
    private:
       FCFS fcfs;
       SCAN scan;
};

/********************************************************************************/
/*  void fcfsprocess(File *file_object)  <class Scheduler>                      */
/*  File 오브젝트를 인자로 받아서 이 함수를 통해 "output.txt"파일에 출력한다.   */
/*  여러가지 복잡한 수행을 한다.                                                */
/********************************************************************************/
void Scheduler::fcfsprocess(File *file)
{
    int l = 0, p = 1, a = 0;

    double temp_time = 0;  //Access Time과 총수행시간을 구하기위한 변수들.
    double each_time = 0;
    double prev_time = 0;
    double next_time = 0;
    double tot_time = 0;
    double sect_time = 0;
    double next_sect_time = 0;
    double prev_sect_time = 0;

    int first_move = 0;
    int prev_move = 0;
    int next_move = 0;
    int tot_move = 0;
    int move = 0;

    char *Line =0;

    Node *n;  //처리될 노드의 복사본을 받을 객체 선언.

    file->FileOut("FCFS Algorithm\n\n");
    while( (Line = file->GetLine(l)) ) //"input.txt"파일을 한 줄씩 읽어들임.
    {	
        if( !(file->isDisplay(Line)) ) //"display"문자열을 만나는지 검사.
        {
            if(a == 0)  //딱 한번만 입력받을 수 있도록 flag설정.
                first_move = next_move = fcfs.Queueing(Line); //첫번째 입력.

            n = fcfs.Process(); //첫번째 리퀘스트부터 시작하여 순차적으로 처리.

            next_time = n->tracktime(); //처리되어진 노등의 복사본에서 track정보구함.
            sect_time = next_sect_time = n->sectortime(); //sector정보 구함.

            //이전트랙과 현재트랙의차가 0이면 두 리퀘스트는 같은 트랙에 존재.
            if( (temp_time = prev_time - next_time) == 0 ) 
            {   //섹터의 차가 0보다 크면 8-섹터의 차를 구한다.
                if( (sect_time = prev_sect_time - next_sect_time) > 0 )
                    sect_time = (SECTOR - sect_time) * SECREADTIME;
                //섹터의 차가 음의 정수이면 양수로 바꾼 후 계산한다.
                else if( (sect_time = prev_sect_time - next_sect_time) < 0 )
                    sect_time = -(sect_time * SECREADTIME);
                //똑같은 트랙 똑같은 섹터에서 발생할 가능성...
                else sect_time = 0;
            } 
            else //두 트랙간의 차가 있을 때.
            {
            	  if( temp_time < 0 )
                    temp_time = -temp_time; //두 트랙간의 거리를 구한후 계산.
                sect_time = sect_time * SECREADTIME + 0.5;
            }  //다른 트랙간이므로 0.5를 합산.

            prev_time = next_time;  //다음 루프수행때 현재 시간이 바로 전 시간이 된다.
            prev_sect_time = next_sect_time; // 섹터에대해 상동.
            
            // 누적되어 합산되는 총 수행 시간.
            tot_time = tot_time + temp_time + sect_time;

            // "output.txt"파일에 출력.
            file->FileOut("Track number : ");
            file->FileOut( next_time ); 
            file->FileOut(" Sector number : ");
            file->FileOut( next_sect_time ); 
            file->FileOut(" Access time : ");
            file->FileOut( temp_time + sect_time ); // 각각의 access time.
            file->FileOut("\n");

            //각각의 request발생 시간과 수행 시간 누적을 비교하여 얼마만큼 큐로
            //삽입할 것인지를 결정하는 루프.
            while(each_time <= tot_time)  
            { 
	              Line = file->GetLine(p+l); // 초기 p+l은 0. 0라인은 실제의 첫번째 라인
                if( Line == NULL ) break;  // GetLine함수가 NULL을 리턴한다는 것은 
	                                         // 더이상의 입력이 없을 경우이다.
	              p++;  // 다음 리퀘스트를 읽기위해 라인증가.
                if( file->isDisplay(Line) )  //"display" 문자열을 만났을 경우.
                {                   
                   file->FileOut("\nDisplay time : ");
	                 file->FileOut(each_time); file->FileOut(" ms\n");
	                 fcfs.Display(file);
                   continue; //다시 다음칸을 읽기위해 루프의 처음으로 올라감.
                }
                next_move = fcfs.Queueing(Line); // 삽입시작. 현재 삽입될 트랙값리턴. 
                if( (move = prev_move - next_move) < 0 ) // 현재삽입값과 이전 삽입 
                     move = -move;                       // 값의 차를 구한다.
                prev_move = next_move; // 나중을 위해 현재값을 저장.
                tot_move += move; //트랙이동 거리 누적.
                each_time = atof( strtok(Line, " ") ); //각각의 삽입될 노드의
                a = 1;//플래그 전환.                   //리퀘스트를 뽑아냄.
            }
        }
	      l++; p--;// l은 처리하기위해 읽어들일 라인의 증가, p는 삽입시킬 라인증가.
	               // 이 시점에서 p-- 한 것은 예외의 경우때문이다. 루프 안에서 p와
	               // l이 합산되므로 p에대한 연산을 빼면 한라인을 skip하게된다.
    }
    file->FileOut("\nDisk Arm move : "); file->FileOut(tot_move);
    file->FileOut("     Total time : "); file->FileOut(tot_time);    
}

/********************************************************************************/
/*  void Traversing(File *file_object)  <class Scheduler>                       */
/*  File오브젝트를 받아서 "output.txt"파일에 temp.Show함수를 이용하여 주소를    */
/*  추적하면서 각각의 노드 데이타를 출력하는 함수이다.                          */
/********************************************************************************/
void Scheduler::Traversing(File *f)
{ scan.Display(f); }
	  
/********************************************************************************/
/*  void scanprocess(File *file)  <class Scheduler>                             */
/*  SCAN방식 스케쥴러의 주요 수행 함수로서 역시 파일형 인자를 받아 복잡한       */
/*  수행을 한 결과를 "output.txt"파일에 뿌려준다.                               */
/*  주석 처리된 코드부분들은 처음 수행시 0track 0sector에서 arm이 움직인다는    */
/*  가정하에 시간상 제일 먼저 발생한 request를 처리하러 가는 중에 만나는 request*/
/*  를 처리할 수 있게 한 부분이다.                                              */
/********************************************************************************/
void Scheduler::scanprocess(File *file)
{
	 char *line;      //현재 처리되는 리퀘스트 노드의 데이타 정보를 현재와 이전 값, 
	                  //그것의 차, 총합 가질 수 있는 여러 변수들의 선언.
   int que =  0;
   int cur =  0;
   int flag = 1;

   int pre_trk = 0;
   int tot_trk = 0;
   
   double tm, sec;  int trk;
      
   double tot_time =  0;
   double each_time = 0;
   double first_time =0;
   double cur_time =  0;   
   
   double pre_sec = 0;
   double tot_sec = 0;
   double tmp_sec = 0;
      
   int tmp_trk = 0;
   int cur_process_trk;
   int cur_process_sec;
   
   int tot_arm_move = 0;
   int tNum = 1;   //트랙넘버 초기화.
   //int first = 1;  //제일 처음 
   int first_track_move;
   
   Node *Enode;
  
   file->FileOut("\n\n\nSCAN Algorithm\n\n");
   /*첫번째 디스크 요청을 받는 부분 시작.*/
   line = file->GetLine(cur);
   if( !file->isDisplay(line) )  //첫 라인에 "display"문자열이 등장할 경우.
   {
      line = file->GetLine(cur++); //이 경우 다음칸을 읽는다.
      Traversing(file);
   }

   file->Token(tm, tmp_trk, tmp_sec, line);
   scan.Listing(tmp_trk, line, 0, 0); //첫번째 요청 입력.
   first_time = tmp_trk + tmp_sec * SECREADTIME + 0.5; //첫번째 요청의 처리 예상 시간
   first_track_move = tmp_trk;
   //Enode = scan.Process(tNum); 첫번째를 입력만 받고 처리는 나중에..

   do//모든 트랙의 first가 NULL인 조건
   {
       if (tNum == TRACK ) flag = 0; //트랙 = 20.
       if (tNum == 1 )  flag = 1;   //tNum 1 은 배열의 0번 트랙.
       
       while( (Enode = scan.Process(tNum))!= NULL )//하나의 링크드리스트 객체가 모두 비었을 때.
       { 
           //if( first == 0 ) // 처음 요청을 바로 처리하지 않도록 first를 flag로 설정.
           {   
               cur_process_trk = Enode->tracktime();  //리스트에 삽입할 때 두가지 소팅방법에 
               cur_process_sec = Enode->sectortime(); //필요한 인자들.
               if( (tmp_trk = pre_trk - Enode->tracktime())) //다음 요청이 다른 track에 있는 경우.
               {  
                   tot_sec = Enode->sectortime();
             	 
                   if( tmp_trk < 0 )
             	        tot_trk = -tmp_trk;
                   else    tot_trk = tmp_trk;
                   cur_time = tot_trk + tot_sec * SECREADTIME + 0.5;
               }
               else // 다음 요청이 같은 트랙내에 있는 경우.
               {
                   if( ( tmp_sec = pre_sec - Enode->sectortime() ) < 0 )
                        tot_sec = -tmp_sec;
                   else tot_sec = 8 - tmp_sec;
                   cur_time = tot_sec * SECREADTIME;               
               }               

               pre_trk = Enode->tracktime();       //현재노드의 정보를 다음에 루프를 돌아올 때 
               pre_sec = Enode->sectortime();      //처리된 노드의 정보로 전환.
             
               tot_time += cur_time;     //처리 시간 누산.
               delete Enode; // 임시로 리턴받았던 현재 처리중인 노드의 복사본의 메모리를
                             // 다시 풀어 놓는다.
                          
               file->FileOut("Track number : "); file->FileOut(pre_trk);
               file->FileOut("  Sector number : "); file->FileOut(pre_sec);
               file->FileOut("  Accesstime : "); file->FileOut(cur_time);
               file->FileOut("\n");
               
               //arm이 움직인 총 시간 계산.
               tot_arm_move += abs(tmp_trk);
           }   
             
           while(1)  //각각의 리퀘스트당 처리시간 만큼 노드를 삽입하는 루프.
           {
               line = file->GetLine(cur+que);  
               //if( first == 1 ) tot_time = first_time; 
               if( tot_time < each_time ) break; // 누산 된 처리시간과 각각의 리퀘스트 시간 비교
               if( line == NULL ) break;         // 그 안에 들어오는 모든 리퀘스트는 삽입.
               if( file->isDisplay(line) )//"display" 검사.
               {
                    file->FileOut("Display time : ");
                    file->FileOut(each_time); file->FileOut(" ms\n");                   
                    Traversing(file); 
                    file->FileOut("\n");
                    que++;    //라인을 스킵하고 다시 루프를 수행.
                    continue; 
               }
               file->Token(tm, trk, sec, line); //현재 라인정보를 세 개의 데이타로 분리한다.
               each_time = tm;

               //처리되는 트랙번호와 삽입시킬 트랙번호가 같다면 sorting을 달리해야한다.
               if( (cur_process_trk - trk) == 0 ) //같은 트랙에 삽입될경우.
                   scan.Listing(trk, line, 1, cur_process_sec);
               else scan.Listing(trk, line, 0, cur_process_sec);      //다른 트랙에 삽입.
               que++;
           }
           //first = 0; //flag 전환.        
       }
       if(flag == 0 ) tNum--; // 플래그가 0일 경우 20 -> 0으로 arm 이동.
       if(flag == 1) tNum++;  // 반대로 1이면 0 -> 20으로 이동.
    }while( scan.isAllNull() );
    file->FileOut("\nTotal access time : "); file->FileOut(tot_time);    
    file->FileOut("\nTotal arm movement: "); file->FileOut(tot_arm_move);
}

/******************************************************************************/
int main(int argc, char *argv[2]) // 인자로 입력 파일을 받는다.
{
     if( argc != 2 ) // 입력 인자가 1개 이하거나 이상일 때 사용법 출력.
     {
         cout << "Usage: " << argv[0] << " <file_name>\n";
         return 0;
     }
     File *file = new File(argv[1]);  //파일 이름을 인자로 받아 File 객체 생성.
     Scheduler schd;         //스케쥴러 클래스 생성.
     file->FileOutOpen();    //"output.txt"파일을 연다.
     
     schd.fcfsprocess(file); //FCFS schedulling.
     schd.scanprocess(file); //SCAN schedulling.
     
     file->FileOutClose();   //"output.txt"파일 close.
     delete file;
     cout << "Schedulling success! : output.txt\n";
     return 1;
}
///////////////////////////////////EndOfFile///////////////////////////////////


