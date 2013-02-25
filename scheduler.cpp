/**************************************************/
/* ASSIGNMENT #1 :           Mini Disk Scheduler  */
/* STUDENTID     :                    1999110093  */
/* NAME          :                      �� ö ��  */
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
//  ���� ����, ���� �ݱ�, ���� ������ �� ���� ����ȭ, �� ����Ÿ ���� �ڸ���     //
//  "display" ���ڿ� üũ, ������� ����, ���.                                 //
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
/*  input������ ������ �о ������ ���� getline()�Լ��� line������ ����ȭ     */
/*  ��Ű�� �Լ�. �μ��� ���ϴ� ���ι�ȣ�� �־� ���ϴ� ������ �̾� �� �� �ִ�.   */
/*  ������ �޾Ƶ��� �μ���ŭ getline()�Լ��� �����.                            */
/********************************************************************************/      
char * File::GetLine(int l)
{
    int i=0;
    
    fin.open(filename);
    if( fin == NULL )
    {  // input������ ���� ���� üũ
       cout << filename << " file not found.\n"; 
       exit(1);
    }
    while( !fin.eof() ) //������ ���� üũ�ϸ� ������ ���� �μ��� ��.
    {
       fin.getline(Buf, 64); 
       if( !strlen(Buf) ) 
            continue;   //�߰��� �������ε� ������ ������ ���.
       if( i == l )
       {
           fin.close();
           return Buf;  //��ġ�� ��� return.
       }
       i++;
    }
    fin.close();
    return NULL;
}

/********************************************************************************/
/*  int isDisplay(char *line_buf)  <class File>                                 */
/*  "display" ���ڿ� üũ.                                                      */
/*  ���ڿ� ���۰� "display�� ��ġ�ϸ� 1, Ʋ���� 0�� �����Ѵ�.                   */
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
/* ��� ������Ʈ���� ��� ������ ���ε��� �μ��� �ް� �� ���� ����ִ� ����Ÿ��,*/
/* time, track, sector�� ������ ���ڷ� �ٽ� �޾Ƽ� ���� ��ȭ ��Ű�� ������ ��.  */
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
// ��ũ���� ������Ʈ���� �߻��ð��� track, sector�� ��ġ�� ������ �ִ� class  //
// �ڽ��� track�� sector�� ��ġ ������ �� �� �ִ� ����Լ��� ���� ����� �ּҸ� //
// �� �� �ִ� next ��� ������ �ִ�.                                            //
// linked list�� �����ϱ����� Queue, LinkedListŬ�������� friend�� ����.        //
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
       int modular; //�ΰ��� sorting �ɼǿ� ���� ����. �ڽ��� ���Ϳ� 8�� ���Ѵ�.
    public:
       Node(double tm, int trk, int sec);
       Node();
       ~Node();
       int tracktime() { return track; }    //�ش� ��������� �ܼ��� �����ϴ� �Լ�.
       int sectortime() { return sector; }
};
/********************************************************************************/
/*  Node(double time, int track, int sector)                                    */
/*  Node��ü �ʱ�ȭ�ϴ� ������ �Լ� ���� �����Ǹ鼭 ���� ������ �� �ִ� �Ͱ�    */
/*  0�̳� NULL������ �ʱ�ȭ ��Ű�� ���ھ��� �����ڷμ� 2��.                     */
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
// FCFS �����췯 ������ ���� FIFO����� �ڷ� ������ �ٷ� �� �ִ� Ŭ����.        //
// ����Ʈ�� ó���� ���� ����ų ���ִ� �ּҸ� ������ �ִ�. ť�� �Է��� ����      //
// ����Ʈ�� ���� �߰���Ű�� input()�Լ��� ť�� ���� ���������� output().        //
// ���������� �� ���鰣�� �ּҸ� ���󰡸� ������ ����Ÿ���� ����ϴ�          //
// show()�Լ��� �ִ�.                                                       //
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
/* Queue()�� ������ �Լ�. front�� rear�� NULL�� �ʱ�ȭ�Ѵ�.                     */
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
/* ������ ����Ÿ�� �߷����� ���� ������ ���δ����� ���۸� �Է°����� ���� �Ŀ�  */
/* track, sector, time�� strtok()�� �߶� �� ��带 �����Ͽ� �� ����Ÿ�� �ʱ�ȭ*/
/* �ϰ� Queue�� �����ϴ� �Լ�. �Էµ� ����� track���� �����Ѵ�.                */
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
    if( front == NULL ) //Queue�� ����ִ� ���.
    { 
       front = node; //Queue�� ���� ������ ������ �ϳ��� ��尡 �ǹǷ� �װ���
       rear = node;  //front���� rear�� �ȴ�.
    }
    else  //Queue�� ������� ���� ���.
    {
       rear->next = node; //�� ���� rear�� next�� �� ��带 �����ϰ� �װ��� �ٽ�
       rear = node;       //rear�� �����.
    }
    return node->track;
}

/********************************************************************************/
/*  Node * output()  <class Queue>                                              */
/*  Queue�� �ִ� �� ���� Node�� �����ϴ� ���Ұ� ���ŵ� Node�� ���纻�� �����ϴ� */
/*  �Լ��̴�.                                                                   */
/********************************************************************************/
Node * Queue::output()
{
    Node *Onode = new Node; //���Ƿ� ������ ��带 �̸� �����Ѵ�.

    if( front == NULL ) return NULL; //���� ���� �ϳ��� ���� ���.

    Onode->time = front->time;   //������ ����� ����Ÿ ����.
    Onode->track = front->track;
    Onode->sector = front->sector;

    Node *temp = front; //������ front�ּ� ����.

    if( front == rear )  //�ϳ��� ��尡 ������ ���. 
    {
       temp = front;
       front = NULL;     //������ �ϳ��� ��带 �����Ѵ�.
       delete temp;
    }
    else   //2�� �̻��� ��尡 ������ ���.                  
    {
       front = front->next; //front�� next�� front�� ���� ��...
       delete temp;         //����� �ּҸ� ���������μ� ������ front�� �����.
    }
    return Onode; //����� ��带 ����.
}

/********************************************************************************/
/*  void show(File *file_IO_object)  <class Queue>                          */
/*  �� �Լ��� �ҷ��� ����� Queue�� �����ϴ� ��� ����� ����Ÿ�� �ּҸ� ���� */
/*  ���� output�� ���Ͽ� ������ִ� �Լ�.                                       */
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
// QueueŬ���� ��ü�� �����ϸ� �̰��� �̿��� FCFS����� ��ũ �����층�� ����  //
// �� �� �ְ� ������� Ŭ����. ���ϻ󿡼� �������� ���� ������ ���۸� �Է°�����//
// �޴� Queue�� input()�Լ��� �̿��� Display()�Լ��� �����층�� �ֵ����� �ϴ�  //
// process �Լ��� ����.                                                         //
//////////////////////////////////////////////////////////////////////////////////
class FCFS
{
    public:
       FCFS()  { queue = new Queue; }//������ �ʱ�ȭ
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
/*  Queue�� insert()�Լ��� �״�� �Ҹ������� �Լ�.                              */
/********************************************************************************/
int FCFS::Queueing(char *buf)
{
    return queue->input(buf);
}

/********************************************************************************/
/*  Node * Process()  <class FCFS>                                              */
/*  ���ھ��� ����Ǹ� Queue::output()�Լ��� �̿��Ͽ� ������ �����ü�� ���Ϲ޴� */
/*  �Լ�. �̰��� �̿��Ͽ� ������ ����� access time�� ���� �� �ִ�.             */
/********************************************************************************/ 
Node * FCFS::Process()
{
    return queue->output(); //Will processed node;output fucntion called.
}

//////////////////////////////////////////////////////////////////////////////////
//  <LinkedList class>                                                          //
//  SCAN����� �����췯�� ���͵��� �� ���� ������� �����ϱ����� ������ �����ϴ�//
//  Ŭ����. ������ ������ 20���� ����Ʈ���� first�� access�ϱ����� friend       //
//  Ŭ������ SCANŬ������ ������.                                               //                 
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
/* LinkedList() ������ �Լ�. front�� rear�� �ʱ�ȭ.                            */
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
/*  QueueŬ�����ʹ� �ٸ� enqueueing�� �����Ѵ�. �����Ǿ� ������ ���� �� ���̳� */
/*  ���ԵǴ� ���� �Ϲ����� sorting�� ���� Ʈ�������� ȸ���� ����� ������      */
/*  access sorting�� �ϱ����� sorting mode�� ���ڷ� �߰��Ǿ���.                */
/*  ���� FCFS�� ��� ����ó�� ���δ����� ���۸� �Է¹޾� �������� �Ա� �Է���*/
/*  �̷������. ����° ���ڴ� ���� ����Ǿ��� ������ ��ȣ�̴�.                 */
/*******************************************************************************/
void LinkedList::insert(char *Line, int mode, int proc_sec)
{
    char *tm_tmp;
    char *trk_tmp;
    char *sec_tmp;
    
    int mod_sec;

    tm_tmp = strtok(Line, " ");    //���� ���� ���ڿ��� token�Ѵ�.
    trk_tmp = strtok(NULL, " ");
    sec_tmp = strtok(NULL, " ");

    double tm = atof(tm_tmp);      //������ �´� ������ ����Ÿ Ÿ������ ��ȯ.
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
    
    if( front == NULL ) // ������ ó�� �̷�����ų� ��� �ִ� ���.
    { 
        front = rear = node;
        return;
    }    

    // Ʈ���� ���� ���� �������� ���ͺ��� ���� ��� 8�� ���ؼ� ���Ѵ�.
    if( (mode == 1) && (proc_sec >= node->sector) )
    	    mod_sec = node->modular;   
    else  // �׷��� ���� ��� �״�� sector���� �ִ´�.
    	  mod_sec = node->sector;
    	      
    if( front->sector > mod_sec )
    {
        node->next = front;        //front�� �о�� front�� �Ǵ� ���.
        front = node;
        return;
    }
    while( temp->sector < mod_sec )   
    {
	      if(temp->next == NULL ) // rear���� ū ����Ÿ�� ���� ���.
	      {
	           temp->next = node;
	           rear = node; //�ٽ� �ڽ��� rear�� �ȴ�.
	           return;
	      }              
	      temp2 = temp; // next�θ� ��ũ�� ����� previous ����� �ּҸ� �˱�����.
        temp = temp->next;  // temp ++
    }
    node->next = temp2->next; //�߰��� ���
    temp2->next = node;
}   

/*******************************************************************************/
/*  Node * output()  <class LinkedList>                                        */
/*  linked list�� ������ �̷������ �Լ��μ� Ư���� ���� ������ ���� �Ȱ���  */
/*  ����Ÿ�� ���� �ٸ� �ּ��� ��ü�� �����Ѵٴ� ���̴�.                        */
/*******************************************************************************/
Node * LinkedList::output()
{
    if( front == NULL )       // ����Ʈ�� ����� ���.
        return NULL;
    Node *Onode = new Node(front->time, front->track, front->sector);

    Node *temp = front;
       
    if( front == rear )       //�����ִ� ��尡 �� �ϳ��� ���.
    {
        temp = front;
        front = rear = NULL;
        delete temp;
    }
    else                     //�ΰ� �̻��� ���.
    {	 
     	front = front->next;
        delete temp;
    }
    return Onode;            //�����Ų ����� ��ü�� ����.
}

/*******************************************************************************/
/*  void show(File *file_object)  <class LinkedList>                       */
/*  LinkedList�� ���Ϸ��� ��� �Լ�. traverse�ϸ� ó������ �����Ͽ� ���� ����  */
/*  �� ������ ����Ʈ.                                                          */
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
// ��ũ �����췯����� SCAN �˰����� ���� Ŭ����.  QueueŬ������ �����    //
// ��� �Լ��� �������� ������ Ʈ������ LinkedList�� ���� �� �ִ� �迭��       //
// ������ �迭����Ʈ���� first����� �ּҸ� �������� �� �� �ִ� �Լ��� �ִ�.   // 
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
/* SCAN() �������Լ�, 20���� Ʈ���� �ش��ϴ� LinkedList�迭 �����͸� Heap�� ����*/
/********************************************************************************/
SCAN::SCAN()
{    // LinkedList 20�� ����.
    for(int i=0;i<20;i++)
        LL[i] = new LinkedList;
}   	
SCAN::~SCAN()
{}

/********************************************************************************/
/*  void Display(File *file_object)  <class SCAN>                              */
/*  LinkedListŬ������ ����Լ� show()�� ȣ���ϵ� ������ Ʈ���� ���Ͽ�      */
/*  ��� �����Ѵ�. �ϳ��� Ʈ���� ���̻� ��忬���� ���� ������ 20�� �����Ѵ�.   */
/********************************************************************************/  	  
void SCAN::Display(File *f)
{ //Queue������ Display �Լ��ʹ� �ٸ� ���� �ٷ� �迭�� ���󰡸� 20�� ��´ٴ� ���̴�.
   for(int i=0;i<20;i++)
       if( LL[i]->front != NULL )
            LL[i]->show(f);
}

/********************************************************************************/
/*  Node * Process(int track_number)  <class SCAN>                              */
/*  Ʈ���ѹ��� �Է����� �޾Ƽ�(1~20) �迭 ����(0~19)�� ��ȯ�ؼ� �ش��ϴ�        */
/*  ������ LinkedList�� first ��带 �����Ѵ�. ������ ���� ������ �����ؼ�    */
/*  �����Ѵ�.                                                                   */ 
/********************************************************************************/
Node * SCAN::Process(int trk)
{
    Node *n;
    
    if( (n = LL[trk-1]->output()) == NULL)
        return NULL;  //���̻� �������� �ʴ� ���.
    else
        return n;
}

/********************************************************************************/
/*  int isAllNull()   <class SCAN>                                              */
/*  ��� Ʈ���� �ش��ϴ� 20���� LinkedList��ΰ� �ϳ��� ������Ʈ ��尡       */
/*  �����ִ��� �ƴ����� üũ�Ѵ�.                                               */
/********************************************************************************/
int SCAN::isAllNull()
{
    int Al=0;
    for(int i=0;i<20;i++)
    {
       if( LL[i]->front == NULL )
           Al++;   //NULL�� ��츦 ī��Ʈ.
    }
    if(Al == 20 ) return 0;
    else          return 1;
}   	     
//////////////////////////////////////////////////////////////////////////////////
//  Scheduler class                                                             //
//  FCFS��İ� SCAN��� �����췯�� ��ü���� ����� ������ �� �ֵ����ϴ� �Լ�    //
//  �� ���� ����Ʈ�� ť�� �����ִ� ������ ���� ���Ͽ� ����ϴ� �Լ���         //
//  ������ �ܼ��� Ŭ����.                                                       //
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
/*  File ������Ʈ�� ���ڷ� �޾Ƽ� �� �Լ��� ���� "output.txt"���Ͽ� ����Ѵ�.   */
/*  �������� ������ ������ �Ѵ�.                                                */
/********************************************************************************/
void Scheduler::fcfsprocess(File *file)
{
    int l = 0, p = 1, a = 0;

    double temp_time = 0;  //Access Time�� �Ѽ���ð��� ���ϱ����� ������.
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

    Node *n;  //ó���� ����� ���纻�� ���� ��ü ����.

    file->FileOut("FCFS Algorithm\n\n");
    while( (Line = file->GetLine(l)) ) //"input.txt"������ �� �پ� �о����.
    {	
        if( !(file->isDisplay(Line)) ) //"display"���ڿ��� �������� �˻�.
        {
            if(a == 0)  //�� �ѹ��� �Է¹��� �� �ֵ��� flag����.
                first_move = next_move = fcfs.Queueing(Line); //ù��° �Է�.

            n = fcfs.Process(); //ù��° ������Ʈ���� �����Ͽ� ���������� ó��.

            next_time = n->tracktime(); //ó���Ǿ��� ����� ���纻���� track��������.
            sect_time = next_sect_time = n->sectortime(); //sector���� ����.

            //����Ʈ���� ����Ʈ�������� 0�̸� �� ������Ʈ�� ���� Ʈ���� ����.
            if( (temp_time = prev_time - next_time) == 0 ) 
            {   //������ ���� 0���� ũ�� 8-������ ���� ���Ѵ�.
                if( (sect_time = prev_sect_time - next_sect_time) > 0 )
                    sect_time = (SECTOR - sect_time) * SECREADTIME;
                //������ ���� ���� �����̸� ����� �ٲ� �� ����Ѵ�.
                else if( (sect_time = prev_sect_time - next_sect_time) < 0 )
                    sect_time = -(sect_time * SECREADTIME);
                //�Ȱ��� Ʈ�� �Ȱ��� ���Ϳ��� �߻��� ���ɼ�...
                else sect_time = 0;
            } 
            else //�� Ʈ������ ���� ���� ��.
            {
            	  if( temp_time < 0 )
                    temp_time = -temp_time; //�� Ʈ������ �Ÿ��� ������ ���.
                sect_time = sect_time * SECREADTIME + 0.5;
            }  //�ٸ� Ʈ�����̹Ƿ� 0.5�� �ջ�.

            prev_time = next_time;  //���� �������ට ���� �ð��� �ٷ� �� �ð��� �ȴ�.
            prev_sect_time = next_sect_time; // ���Ϳ����� ��.
            
            // �����Ǿ� �ջ�Ǵ� �� ���� �ð�.
            tot_time = tot_time + temp_time + sect_time;

            // "output.txt"���Ͽ� ���.
            file->FileOut("Track number : ");
            file->FileOut( next_time ); 
            file->FileOut(" Sector number : ");
            file->FileOut( next_sect_time ); 
            file->FileOut(" Access time : ");
            file->FileOut( temp_time + sect_time ); // ������ access time.
            file->FileOut("\n");

            //������ request�߻� �ð��� ���� �ð� ������ ���Ͽ� �󸶸�ŭ ť��
            //������ �������� �����ϴ� ����.
            while(each_time <= tot_time)  
            { 
	              Line = file->GetLine(p+l); // �ʱ� p+l�� 0. 0������ ������ ù��° ����
                if( Line == NULL ) break;  // GetLine�Լ��� NULL�� �����Ѵٴ� ���� 
	                                         // ���̻��� �Է��� ���� ����̴�.
	              p++;  // ���� ������Ʈ�� �б����� ��������.
                if( file->isDisplay(Line) )  //"display" ���ڿ��� ������ ���.
                {                   
                   file->FileOut("\nDisplay time : ");
	                 file->FileOut(each_time); file->FileOut(" ms\n");
	                 fcfs.Display(file);
                   continue; //�ٽ� ����ĭ�� �б����� ������ ó������ �ö�.
                }
                next_move = fcfs.Queueing(Line); // ���Խ���. ���� ���Ե� Ʈ��������. 
                if( (move = prev_move - next_move) < 0 ) // ������԰��� ���� ���� 
                     move = -move;                       // ���� ���� ���Ѵ�.
                prev_move = next_move; // ������ ���� ���簪�� ����.
                tot_move += move; //Ʈ���̵� �Ÿ� ����.
                each_time = atof( strtok(Line, " ") ); //������ ���Ե� �����
                a = 1;//�÷��� ��ȯ.                   //������Ʈ�� �̾Ƴ�.
            }
        }
	      l++; p--;// l�� ó���ϱ����� �о���� ������ ����, p�� ���Խ�ų ��������.
	               // �� �������� p-- �� ���� ������ ��춧���̴�. ���� �ȿ��� p��
	               // l�� �ջ�ǹǷ� p������ ������ ���� �Ѷ����� skip�ϰԵȴ�.
    }
    file->FileOut("\nDisk Arm move : "); file->FileOut(tot_move);
    file->FileOut("     Total time : "); file->FileOut(tot_time);    
}

/********************************************************************************/
/*  void Traversing(File *file_object)  <class Scheduler>                       */
/*  File������Ʈ�� �޾Ƽ� "output.txt"���Ͽ� temp.Show�Լ��� �̿��Ͽ� �ּҸ�    */
/*  �����ϸ鼭 ������ ��� ����Ÿ�� ����ϴ� �Լ��̴�.                          */
/********************************************************************************/
void Scheduler::Traversing(File *f)
{ scan.Display(f); }
	  
/********************************************************************************/
/*  void scanprocess(File *file)  <class Scheduler>                             */
/*  SCAN��� �����췯�� �ֿ� ���� �Լ��μ� ���� ������ ���ڸ� �޾� ������       */
/*  ������ �� ����� "output.txt"���Ͽ� �ѷ��ش�.                               */
/*  �ּ� ó���� �ڵ�κе��� ó�� ����� 0track 0sector���� arm�� �����δٴ�    */
/*  �����Ͽ� �ð��� ���� ���� �߻��� request�� ó���Ϸ� ���� �߿� ������ request*/
/*  �� ó���� �� �ְ� �� �κ��̴�.                                              */
/********************************************************************************/
void Scheduler::scanprocess(File *file)
{
	 char *line;      //���� ó���Ǵ� ������Ʈ ����� ����Ÿ ������ ����� ���� ��, 
	                  //�װ��� ��, ���� ���� �� �ִ� ���� �������� ����.
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
   int tNum = 1;   //Ʈ���ѹ� �ʱ�ȭ.
   //int first = 1;  //���� ó�� 
   int first_track_move;
   
   Node *Enode;
  
   file->FileOut("\n\n\nSCAN Algorithm\n\n");
   /*ù��° ��ũ ��û�� �޴� �κ� ����.*/
   line = file->GetLine(cur);
   if( !file->isDisplay(line) )  //ù ���ο� "display"���ڿ��� ������ ���.
   {
      line = file->GetLine(cur++); //�� ��� ����ĭ�� �д´�.
      Traversing(file);
   }

   file->Token(tm, tmp_trk, tmp_sec, line);
   scan.Listing(tmp_trk, line, 0, 0); //ù��° ��û �Է�.
   first_time = tmp_trk + tmp_sec * SECREADTIME + 0.5; //ù��° ��û�� ó�� ���� �ð�
   first_track_move = tmp_trk;
   //Enode = scan.Process(tNum); ù��°�� �Է¸� �ް� ó���� ���߿�..

   do//��� Ʈ���� first�� NULL�� ����
   {
       if (tNum == TRACK ) flag = 0; //Ʈ�� = 20.
       if (tNum == 1 )  flag = 1;   //tNum 1 �� �迭�� 0�� Ʈ��.
       
       while( (Enode = scan.Process(tNum))!= NULL )//�ϳ��� ��ũ�帮��Ʈ ��ü�� ��� ����� ��.
       { 
           //if( first == 0 ) // ó�� ��û�� �ٷ� ó������ �ʵ��� first�� flag�� ����.
           {   
               cur_process_trk = Enode->tracktime();  //����Ʈ�� ������ �� �ΰ��� ���ù���� 
               cur_process_sec = Enode->sectortime(); //�ʿ��� ���ڵ�.
               if( (tmp_trk = pre_trk - Enode->tracktime())) //���� ��û�� �ٸ� track�� �ִ� ���.
               {  
                   tot_sec = Enode->sectortime();
             	 
                   if( tmp_trk < 0 )
             	        tot_trk = -tmp_trk;
                   else    tot_trk = tmp_trk;
                   cur_time = tot_trk + tot_sec * SECREADTIME + 0.5;
               }
               else // ���� ��û�� ���� Ʈ������ �ִ� ���.
               {
                   if( ( tmp_sec = pre_sec - Enode->sectortime() ) < 0 )
                        tot_sec = -tmp_sec;
                   else tot_sec = 8 - tmp_sec;
                   cur_time = tot_sec * SECREADTIME;               
               }               

               pre_trk = Enode->tracktime();       //�������� ������ ������ ������ ���ƿ� �� 
               pre_sec = Enode->sectortime();      //ó���� ����� ������ ��ȯ.
             
               tot_time += cur_time;     //ó�� �ð� ����.
               delete Enode; // �ӽ÷� ���Ϲ޾Ҵ� ���� ó������ ����� ���纻�� �޸𸮸�
                             // �ٽ� Ǯ�� ���´�.
                          
               file->FileOut("Track number : "); file->FileOut(pre_trk);
               file->FileOut("  Sector number : "); file->FileOut(pre_sec);
               file->FileOut("  Accesstime : "); file->FileOut(cur_time);
               file->FileOut("\n");
               
               //arm�� ������ �� �ð� ���.
               tot_arm_move += abs(tmp_trk);
           }   
             
           while(1)  //������ ������Ʈ�� ó���ð� ��ŭ ��带 �����ϴ� ����.
           {
               line = file->GetLine(cur+que);  
               //if( first == 1 ) tot_time = first_time; 
               if( tot_time < each_time ) break; // ���� �� ó���ð��� ������ ������Ʈ �ð� ��
               if( line == NULL ) break;         // �� �ȿ� ������ ��� ������Ʈ�� ����.
               if( file->isDisplay(line) )//"display" �˻�.
               {
                    file->FileOut("Display time : ");
                    file->FileOut(each_time); file->FileOut(" ms\n");                   
                    Traversing(file); 
                    file->FileOut("\n");
                    que++;    //������ ��ŵ�ϰ� �ٽ� ������ ����.
                    continue; 
               }
               file->Token(tm, trk, sec, line); //���� ���������� �� ���� ����Ÿ�� �и��Ѵ�.
               each_time = tm;

               //ó���Ǵ� Ʈ����ȣ�� ���Խ�ų Ʈ����ȣ�� ���ٸ� sorting�� �޸��ؾ��Ѵ�.
               if( (cur_process_trk - trk) == 0 ) //���� Ʈ���� ���Եɰ��.
                   scan.Listing(trk, line, 1, cur_process_sec);
               else scan.Listing(trk, line, 0, cur_process_sec);      //�ٸ� Ʈ���� ����.
               que++;
           }
           //first = 0; //flag ��ȯ.        
       }
       if(flag == 0 ) tNum--; // �÷��װ� 0�� ��� 20 -> 0���� arm �̵�.
       if(flag == 1) tNum++;  // �ݴ�� 1�̸� 0 -> 20���� �̵�.
    }while( scan.isAllNull() );
    file->FileOut("\nTotal access time : "); file->FileOut(tot_time);    
    file->FileOut("\nTotal arm movement: "); file->FileOut(tot_arm_move);
}

/******************************************************************************/
int main(int argc, char *argv[2]) // ���ڷ� �Է� ������ �޴´�.
{
     if( argc != 2 ) // �Է� ���ڰ� 1�� ���ϰų� �̻��� �� ���� ���.
     {
         cout << "Usage: " << argv[0] << " <file_name>\n";
         return 0;
     }
     File *file = new File(argv[1]);  //���� �̸��� ���ڷ� �޾� File ��ü ����.
     Scheduler schd;         //�����췯 Ŭ���� ����.
     file->FileOutOpen();    //"output.txt"������ ����.
     
     schd.fcfsprocess(file); //FCFS schedulling.
     schd.scanprocess(file); //SCAN schedulling.
     
     file->FileOutClose();   //"output.txt"���� close.
     delete file;
     cout << "Schedulling success! : output.txt\n";
     return 1;
}
///////////////////////////////////EndOfFile///////////////////////////////////


