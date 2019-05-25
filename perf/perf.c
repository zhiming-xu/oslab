//默认跟踪ls命令，使用./perf xx可跟踪其他命令或程序
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <math.h>
regex_t re[2];//进程名: xxx(*)/进程所用时间<x.xxx>
char pattern1[30]="[a-zA-z_]+[a-zA-z]*[\(]";
char pattern2[30]="[0-9]+[.][0-9]+";
char result[10000];
struct{
    char name[20];
    double time;
}Syscall[128];//存放syscall和时间的数组
int num_of_syscall=0;//到目前syscall的个数
char thiscall[20];//正在处理的syscall
double all_time=0;//到目前的总时间

int main(int argc, char *argv[]) 
{
    //先处理正则表达式
    if(regcomp(&re[0], pattern1, REG_EXTENDED)!=0)
        perror("Regex1 compile error");
    if(regcomp(&re[1], pattern2, REG_EXTENDED)!=0)
        perror("Regex2 compile error");
    
    //创建管道
    int fd[2];
    int* write_p=&fd[1];
    int* read_p=&fd[0];
    if(pipe(fd)==-1)
    {
        perror("Failed to create pipe");
        exit(-1);
    }

    //创建子进程
    pid_t pid;
    pid=fork();
    if(pid==-1)
    {
        perror("Fork fails!");
    }
    else if(pid==0)//Child, exec strace
    {
        close(*read_p);
        dup2(*write_p, 2);
        
        int null=open("/dev/null", O_WRONLY | O_APPEND);
        dup2(null, 1);
        
        char* exe_argv[]={"strace", "-T", "ls", NULL};//默认跟踪ls
        
        if(argc>1)
        {    
            for(int i=1;i<argc;++i)
                exe_argv[i+1]=argv[i];
            exe_argv[argc+1]=NULL;
        }
        char* exe_envp[]={"PATH=/bin", NULL};
        
        execve("/usr/bin/strace", exe_argv, exe_envp);
        perror("Error");
    }
    else//Parent, parse output of strace
    {
        wait(NULL);
        close(*write_p);
        if(read(*read_p, result, sizeof(result))<=0)//输出的长度
            exit(-1);
        regmatch_t pmatch;
        int position=0;//在输出结果中的位置
        while (result[position]!='\0') 
        {
            int flag=0;
            for(int i=0;i<2;++i)
            {
                if (regexec(&re[i], result + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) 
                {
                    flag=1;
                    //char *substr_start = result + position;
                    //printf("Match case %d!Length is %d\n", i, pmatch.rm_eo);
                    int substr_len = pmatch.rm_eo;
                    char tmp[100];
                    int j;
                    for(j=0;j<substr_len;++j)
                        tmp[j]=result[position+j];
                    
                    if(i==0)
                    {
                        tmp[j-1]='\0';
                        strcpy(thiscall, tmp);
                    }
                    else
                    {
                        tmp[j]='\0';
                        double tt=atof(tmp);
                        all_time+=tt;
                        int found=0;
                        for(int k=0;k<num_of_syscall;++k)
                        {
                            if(strcmp(thiscall, Syscall[k].name)==0)
                            {
                                found=1;
                                Syscall[k].time+=tt;
                            }
                        }
                        if(found==0)
                        {
                            strcpy(Syscall[num_of_syscall].name, thiscall);
                            Syscall[num_of_syscall].time+=tt;
                            num_of_syscall++;
                        }
                    }
                    position+=substr_len;
                }

            }
            if(!flag)
                position++;
            system("clear");
            for(int i=0;i<num_of_syscall;++i)
            {    
                double ratio=Syscall[i].time/all_time*100;
                int l=(int)(ratio+1);
                printf("%s:", Syscall[i].name);
                int m=strlen(Syscall[i].name);
                for(int k=0;k<20-m;++k)
                    printf(" ");
                printf("%.2f%\n", ratio);
                for(int j=0;j<l;++j)
                    printf("\033[44;34m \033[0m");
                printf("\n");
            }
            usleep(1000);
        }
    }
    return 0;
}
