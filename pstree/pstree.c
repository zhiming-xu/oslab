#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
char proc_entries[1000][100];
struct PNode{
    bool exist;
    int PID;
    int PPID;//parent
    int DPID[50];//descendent
    int cnt;//number of descendents
    int level;//distance from top process
    char p_name[20];
}Node[50000];
int cnt=1;//number of processes
void print(int);

int main(int argc, char *argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    assert(argv[i]); // specification
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]); // specification
  //Open /proc directory
  DIR* proc;
  if((proc=opendir("/proc"))==NULL)
      perror("Directory /proc open fails");
  //Use regex to find the proc's directories
  regex_t re;
  char *pattern="[0-9]+";//Match PID number
  if(regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB)!=0)
      perror("Regex compile error");
  
  struct dirent* entry;
  while((entry=readdir(proc))!=NULL)
  {
      if(regexec(&re, entry->d_name, (size_t)0, NULL, 0)==0)    
      {
          strcpy(proc_entries[cnt], "/proc/");
          strcat(proc_entries[cnt], entry->d_name);
          //int index=atoi(entry->d_name);
          Node[cnt].PPID=0;
          Node[cnt].cnt=0;
          Node[cnt].level=0;
          memset(Node[cnt].DPID,0,50*sizeof(int));
          cnt++;
      }
  }
  for(int i=1;i<cnt;i++)
  {
      DIR* subdir;
      if((subdir=opendir(proc_entries[i]))==NULL)
          perror("Open /proc/PID fail");
      else
      {
         struct dirent* content;
         while((content=readdir(subdir))!=NULL)
         {
             if(strcmp(content->d_name, "status")==0)
             {
                 char filename[300];// /proc/PID/status
                 strcpy(filename, proc_entries[i]);
                 strcat(filename, "/status");
                 FILE* fp=fopen(filename, "r");
                 char pid[50];
                 char name[50];
                 int len;
                 assert(fgets(name, 50, fp));
                 len=strlen(name);//此进程的名称
                 for(int j=6;j<len;++j)
                     name[j-6]=name[j];
                 for(int j=len-7;j<len;++j)
                     name[j]='\0';
                 for(int j=1;j<=4;++j)
                     assert(fgets(pid, 50, fp));
                 assert(fgets(pid, 50, fp));
                 //printf("%s\n", pid);
                 len=strlen(pid);
                 for(int j=0;j<len-5;++j)
                     pid[j]=pid[j+5];
                 int p1=atoi(pid);
                 //printf("%d\n", p1);
                 //assert(0); 
                 assert(fgets(pid, 50, fp));
                 len=strlen(pid);
                 for(int j=0;j<len-6;++j)
                     pid[j]=pid[j+6];
                 int p2=atoi(pid);
                 //printf("%d\n", p2);
                 //assert(0);
                 Node[i].PID=p1;
                 Node[i].PPID=p2;
                 strcpy(Node[i].p_name, name);
                 //printf("%d %s\n", i, Node[2].p_name);
                 //printf("%d\n", Node[3].PID);
                 fclose(fp);
             }
         }
         
      }
  }
  for(int i=1;i<cnt;++i)
  {
      int ppid=-1;
      for(int j=1;j<cnt;++j)
      {
          if(Node[j].PID==Node[i].PPID)
          {
              ppid=j;
              break;
          }
      }
      if(ppid==-1)   
          continue;
      int index=Node[ppid].cnt;
      Node[ppid].DPID[index]=Node[i].PID;
      Node[ppid].cnt++;
      
      int tmp=i;
      while(Node[tmp].PPID>0)
      {
          for(int j=1;j<cnt;++j)
          {
              if(Node[j].PID==Node[tmp].PPID)
              {
                  tmp=j;
                  break;
              }
          }
          Node[i].level++;
      }
  }
  Node[2].level=0;
  Node[2].PID=2;
  strcpy(Node[2].p_name, "kthread");
  for(int i=1;i<cnt;++i)
  {
      print(i);
      //printf("%d %s\n",Node[i].PID, Node[i].p_name);
  }
  return 0;
}
void print(int i)
{
    if(Node[i].level!=0)
    {
        for(int j=0;j<Node[i].level;++j)
            printf("        ");
        printf("└--------");
    }
    printf("%d: %s\n", Node[i].PID, Node[i].p_name);
    for(int j=0;j<Node[i].cnt;++j)
    {
        for(int k=1;k<cnt;++k)
        {
            if(Node[k].PID==Node[i].DPID[j])
            {
                print(k);
                break;
            }
        }
    }
}
