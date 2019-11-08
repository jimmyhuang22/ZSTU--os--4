#include "windows.h"
#include <conio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <string>
#include <regex>
using namespace std;

void Create_ProcInfo(); // 建立进程调度需要的数据
void Display_ProcInfo();   // 显示当前系统全部进程的状态
void Scheduler_FF();
void Cpu_Sched();
void IO_Sched();
void DisData();
void DisResult();
void Read_Process_Info();

int   RunPoint;   // 运行进程指针，-1时为没有运行进程
int   WaitPoint;  // 阻塞队列指针，-1时为没有阻塞进程
int   ReadyPoint; // 就绪队列指针，-1时为没有就绪进程
long  ClockNumber;   // 系统时钟
int   ProcNumber;    // 系统中模拟产生的进程总数
int   FinishedProc;    // 系统中模拟产生的进程总数


//进程信息结构
struct ProcStruct
{
  int  p_pid;         // 进程的标识号
  char p_state;       // 进程的状态，C--运行  R--就绪  W--组塞  B--后备  F--完成
  int  p_rserial[16]; // 模拟的进程执行的CPU和I/O时间数据序列，间隔存储，0项存储有效项数
  int  p_pos;         // 当前进程运行到的序列位置
  int  p_starttime;   // 进程建立时间
  int  p_endtime;     // 进程运行结束时间
  int  p_cputime;     // 当前运行时间段进程剩余的需要运行时间
  int  p_iotime;      // 当前I/O时间段进程剩余的I/O时间
  int  p_next;        // 进程控制块的链接指针
} proc[10];

////////////////////////////////////////////////////////////////////////////
//
//  随机生成进程数量和每个CPU--I/O时间数据序列，进程数量控制在5到10之间， //
//  数据序列控制在6到12之间，CPU和I/O的时间数据值在5到15的范围           //
//
////////////////////////////////////////////////////////////////////////////

void Create_ProcInfo(void )
{
  int s,i,j;

    srand(GetTickCount());                        // 初始化随机数队列的"种子"
  ProcNumber = ((float) rand() / 32767) * 5 + 5;  // 随机产生进程数量5~10
  FinishedProc=0;

  for(i=0;i<ProcNumber;i++)    // 生成进程的CPU--I/O时间数据序列
  {
    proc[i].p_pid=((float) rand() / 32767) * 1000;
    proc[i].p_state='B';   // 初始都为后备状态

    s=((float) rand() / 32767) *6 + 6; // 产生的进程数据序列长度在6~12间
    proc[i].p_rserial[0]=s; // 第一项用于记录序列的长度
    for(j=1;j<=s;j++)  // 生成时间数据序列，数值在10~30间
      proc[i].p_rserial[j]=((float) rand() / 32767) *10 + 5;
    // 赋其他字段的值
    proc[i].p_pos=1;
    proc[i].p_starttime=((float) rand() / 32767) *49+1;
    proc[i].p_endtime=0;
    proc[i].p_cputime=proc[i].p_rserial[1];
    proc[i].p_iotime=proc[i].p_rserial[2];
    proc[i].p_next=-1;
     }
  printf("\n---------------------------\n    建立了%2d 个进程数据序列\n\n", ProcNumber);
  DisData();
  printf("\nPress Any Key To Continue.......");
  _getch() ;
  return ;
}

////////////////////////////////////////////////////////////////////////

//                        显示系统当前状态

////////////////////////////////////////////////////////////////////////

void Display_ProcInfo(void)
{  int i,n;

   system("cls") ;
   printf("\n        当前系统模拟%2d 个进程的运行    时钟：%ld\n\n", ProcNumber,ClockNumber);

   printf("        就绪指针=%d, 运行指针=%d, 阻塞指针=%d\n\n",ReadyPoint,RunPoint,WaitPoint );
   if(RunPoint!= -1)
   {
     printf(" .............Running Process .............\n No.%d ID：%d(%2d,%2d)", RunPoint,proc[RunPoint].p_pid,proc[RunPoint].p_rserial[0],proc[RunPoint].p_starttime);

     printf(" 总CPU时间=%d, 剩余CPU时间=%d\n",proc[RunPoint].p_rserial[proc[RunPoint].p_pos],proc[RunPoint].p_cputime);
   }
   else
       printf("No Process Running !\n");

   n=WaitPoint;
   printf("\n .............Waiting Process ............. \n");
   while(n!=-1) // 显示阻塞进程信息
   {
     printf(" No.%d ID:%5d(%2d,%2d), I/O执行到序列中的第%d个,总I/O时间=%d, 剩余I/O时间=%d\n",n,proc[n].p_pid,proc[n].p_rserial[0],proc[n].p_starttime,proc[n].p_pos,proc[n].p_rserial[proc[n].p_pos],proc[n].p_iotime);
     printf("\tserial:");
     for(int j=1; j<=proc[n].p_rserial[0];j++)
            printf("%4d",proc[n].p_rserial[j]);
       printf("\n");
     n=proc[n].p_next;
   }

   n=ReadyPoint;
   printf("\n............. Ready Process ............. \n");
   while(n!=-1) // 显示就绪进程信息
   {
     printf(" No.%d ID:%5d(%2d,%2d),第%d个/总时间=%d",n,proc[n].p_pid,proc[n].p_rserial[0],proc[n].p_starttime,proc[n].p_pos,proc[n].p_rserial[proc[n].p_pos] );
       printf("\t\t\tserial:");
     for(int j=1; j<=proc[n].p_rserial[0];j++)
            printf("%4d",proc[n].p_rserial[j]);
       printf("\n");
     n=proc[n].p_next;
   }



   printf("\n=================== 后备进程 ====================\n");
   for(i=0; i<ProcNumber; i++)
   if (proc[i].p_state=='B')
     printf("     No.%d ID:%5d(%2d,%2d)\n",i,proc[i].p_pid,proc[i].p_rserial[0],proc[i].p_starttime);
         printf("\n");


   printf("\n================ 已经完成的进程 =================\n");
   for(i=0; i<ProcNumber; i++)
   if (proc[i].p_state=='F')
     printf("No.%d ID:%5d(%2d,%2d),Endtime=%d\n",i,proc[i].p_pid,proc[i].p_rserial[0],proc[i].p_starttime,proc[i].p_endtime);


}
////////////////////////////////////////////////////////////////////////

//              显示模拟执行的结果

////////////////////////////////////////////////////////////////////////
void DisResult(void)
{  int i;
   printf("\n---------------------------------\n");
   printf("标识号-时间序列-建立时间-结束时间-周转时间\n");
   for(i=0; i<ProcNumber; i++)
    {
       printf("ID=%4d> %2d     ",proc[i].p_pid ,proc[i].p_rserial[0] );
         printf("%4d,     %4d      ",proc[i].p_starttime,proc[i].p_endtime);
     printf("%4d",proc[i].p_endtime-proc[i].p_starttime);
       printf("\n" );
    }
}

////////////////////////////////////////////////////////////////////////

//              显示进程数据序列

////////////////////////////////////////////////////////////////////////
void DisData(void)
{  int i,j;
   ofstream outFile;
   outFile.open("Process_Info.txt");
   for(i=0; i<ProcNumber; i++)
    {
       printf("ID:%4d(len= %2d , start=%2d):",proc[i].p_pid ,proc[i].p_rserial[0],proc[i].p_starttime );
         for(j=1; j<=proc[i].p_rserial[0];j++)
            printf("%4d",proc[i].p_rserial[j]);
      printf("\n" );
    }
    outFile.close();
}
////////////////////////////////////////////////////////////////////////

//              选择下一个可以运行的进程

////////////////////////////////////////////////////////////////////////
void NextRunProcess(void)
{
  if (ReadyPoint==-1) { RunPoint = -1; return;}  // 就绪队列也没有等待的进程

     proc[ReadyPoint].p_state ='C';
     RunPoint=ReadyPoint;
       proc[ReadyPoint].p_cputime =proc[ReadyPoint].p_rserial[proc[ReadyPoint].p_pos] ;
     ReadyPoint=proc[ReadyPoint].p_next;
     proc[RunPoint].p_next = -1;

}

////////////////////////////////////////////////////////////////////////

//              CPU调度

////////////////////////////////////////////////////////////////////////

void Cpu_Sched(void)
{
  int n;

  if (RunPoint == -1)    // 没有进程在CPU上执行
  {     NextRunProcess(); return;   }

  proc[RunPoint].p_cputime--;      // 还需要CPU时间，下次继续，这次就返回了
  if (proc[RunPoint].p_cputime > 0) return;

  //如果不满足以上>0的条件，就意味着=0，就不会自动返回，接着做以下事情。

  // 进程完成本次CPU后的处理
    if (proc[RunPoint].p_rserial[0]==proc[RunPoint].p_pos) //进程全部序列执行完成
  {
    FinishedProc++;
        proc[RunPoint].p_state ='F';
    proc[RunPoint].p_endtime = ClockNumber;
        RunPoint=-1;  //无进程执行
    NextRunProcess();//找分派程序去，接着做下一个
  }
  else //进行IO操作，进入阻塞队列
  {
    proc[RunPoint].p_pos++;
    proc[RunPoint].p_state ='W';
        proc[RunPoint].p_iotime =proc[RunPoint].p_rserial[proc[RunPoint].p_pos];
        proc[n].p_next == -1;//标记下，就自己一个进程，没带尾巴一起来；否则,当p_next不为-1时，后面的那一串都是被阻塞者
    n=WaitPoint;
    if(n == -1) //是阻塞队列第一个I/O进程
       WaitPoint=RunPoint;
    else
    { do //放入阻塞队列第尾
      {
        if(proc[n].p_next == -1)
        { proc[n].p_next = RunPoint;
          break;
        }
        n=proc[n].p_next;
      } while(n!=-1) ;
    }
        RunPoint=-1;
    NextRunProcess();
  }
  return;
}

////////////////////////////////////////////////////////////////////////

//              I/O调度

////////////////////////////////////////////////////////////////////////

void IO_Sched(void)
{
  int n,bk;

  if (WaitPoint==-1) return;   // 没有等待I/O的进程，直接返回

  proc[WaitPoint].p_iotime--;  // 进行1个时钟的I/O时间
    if (proc[WaitPoint].p_iotime > 0) return; // 还没有完成本次I/O

  // 进程的I/O完成处理
    if (proc[WaitPoint].p_rserial[0]==proc[WaitPoint].p_pos) //进程全部任务执行完成
  {
    FinishedProc++;
        proc[WaitPoint].p_endtime = ClockNumber;
    proc[WaitPoint].p_state ='F';

    if(proc[WaitPoint].p_next==-1)
    { WaitPoint=-1;return ;}
    else //调度下一个进程进行I/O操作
    {
           n=proc[WaitPoint].p_next;
       proc[WaitPoint].p_next=-1;
       WaitPoint=n;
           proc[WaitPoint].p_iotime =proc[WaitPoint].p_rserial[proc[WaitPoint].p_pos] ;
       return ;
    }
  }
  else //进行下次CPU操作，进就绪队列
  {
    bk=WaitPoint;
    WaitPoint=proc[WaitPoint].p_next;

    proc[bk].p_pos++;
        proc[bk].p_state ='R'; //进程状态为就绪
    proc[bk].p_next =-1;

    n=ReadyPoint;
    if(n == -1) //是就绪队列的第一个进程
    {  ReadyPoint=bk; return; }
    else
    {  do
      {
        if(proc[n].p_next == -1) { proc[n].p_next = bk;  break ; }
        n=proc[n].p_next;
      }
      while(n!=-1);
    }
  }
    return ;
}
////////////////////////////////////////////////////////////////////////

//              检查是否有新进程到达，有则放入就绪队列

////////////////////////////////////////////////////////////////////////

void NewReadyProc(void)
{
  int i,n;

  for(i=0; i<ProcNumber; i++)
  {
    if (proc[i].p_starttime == ClockNumber) // 进程进入时间达到系统时间
    {
          proc[i].p_state='R';//  进程状态修改为就绪
      proc[i].p_next=-1;// 该进行即将要挂在队列末尾，它肯定是尾巴，后面没人的，所以先设置next=-1

          if(ReadyPoint==-1) // 如果当前就绪队列无进程
          ReadyPoint=i;
      else      // 如果就绪队列有进程，放入队列尾
      {
          n=ReadyPoint;
        while(proc[n].p_next!=-1) n=proc[n].p_next; //找到原来队伍中的尾巴
              proc[n].p_next=i;//挂在这个尾巴后面
      }
    }
  }
   return;
}


////////////////////////////////////////////////////////////////////////

//                         调度模拟算法

////////////////////////////////////////////////////////////////////////

void Scheduler_FF(void)
{
  if(ProcNumber==0)
  {
      cout << "1" <<endl;
    Read_Process_Info();
  }

  ClockNumber=0;// 时钟开始计时, 开始调度模拟
  while(FinishedProc < ProcNumber) // 执行算法
  {
    ClockNumber++;  // 时钟前进1个单位
    NewReadyProc(); // 判别新进程是否到达
    Cpu_Sched();    // CPU调度
      IO_Sched();     // IO调度
    Display_ProcInfo(); //显示当前状态
    Sleep(700); //////////////////////////////////*******************************


  }

   getch();
}

///////////////////////////////////////////////////////////////////

//                         读取上次创建的进程信息

///////////////////////////////////////////////////////////////////

void Read_Process_Info(  )
{

    ifstream inFile;     // 定义打开文件的文件流
    char ch;
    int i,j,k,tmp;

    inFile.open("Process_Info.txt") ; // 打开上次写的txt进行信息文件流

    i=0;
    if(inFile)
    {


        string buffTmp;
        while(getline(inFile, buffTmp))
        {
            cout << buffTmp <<endl;


        /*正则表达式匹配的内容
            ID=55(len=2,start=4):6 5
            ID=338(len=5,start=8):4 7 6 7 6
            ID=409(len=3,start=8):8 9 10
            ID=377(len=2,start=11):4 4
            ID=55(len=2,start=4):6 5
            results[0]      ID=338(len=5,start=8):4 7 6 7 6
            results[1]      338
            results[2]      (
            results[3]      5
            results[4]      8
            results[5]      :
            results[6]      4 7 6 7 6

        */
        regex pattern("^ID=(\\d+)(.*)len=(\\d+),start=(\\d+)\\)(.)(.*)");
        smatch results;
        regex_match(buffTmp, results, pattern);

        string temp_s;
        int temp_int;

        //把正则表达式表达出的数字先赋值给一个string再转成int，再赋值给变量



        try {
            temp_s = results[1];
            temp_int = stoi(temp_s);
            proc[i].p_pid = temp_int;


            temp_s = results[3];
            temp_int = stoi(temp_s);
            proc[i].p_rserial[0] = temp_int;


            temp_s = results[4];
            temp_int = stoi(temp_s);
            proc[i].p_starttime = temp_int;

            int length;
            length = results[6].length();

            string temp = results[6];
            for(int i = 0;i < length;i++)
            {
                if(temp[i]!=' ')
                {
                    proc[i].p_rserial[i+1] = temp[i];
                }

            }
            proc[i].p_state='B';
            proc[i].p_pos=1;
            proc[i].p_endtime=0;
            proc[i].p_next=-1;
            proc[i].p_cputime=proc[i].p_rserial[1];
            proc[i].p_iotime=proc[i].p_rserial[2];
            i++; //本行结束，一个进程信息读完，序号+1, 准备 next process

        }
        catch (std::invalid_argument&){
            int bbbb = 0;
        }



    }
    }
    ProcNumber = i-1;
    inFile.close();//完工后，记得归位，关灯。
    DisResult();


}



///////////////////////////////////////////////////////////////////

//                          主函数

///////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  char ch;

  RunPoint=-1;   // 运行进程指针，-1时为没有运行进程
  WaitPoint=-1;  // 阻塞队列指针，-1时为没有阻塞进程
  ReadyPoint=-1; // 就绪队列指针，-1时为没有就绪进程
  ClockNumber=0;   // 系统时钟
  ProcNumber=0;    // 当前系统中的进程总数

  system("cls") ;
  while ( 1 )
  {
  printf("***********************************\n");
    printf("     1: 建立进程调度数据序列 \n") ;
    printf("     2: 读进程信息，执行调度算法\n") ;
    printf("***********************************\n");
    printf( "Enter your choice (1 ~ 2): ");

  do{   //如果输入信息不正确，继续输入
        ch = (char)_getch() ;
  } while(ch != '1' && ch != '2');

  if(ch == '2') Scheduler_FF();     // 选择2
  if(ch == '1') Create_ProcInfo();  // 选择1

   _getch() ;
     system("cls") ;
  }
  _getch() ;
  return 0;
}


