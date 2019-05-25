#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include <stdint.h>

#define PROC_DIR "/proc/"
#define MAX_COMMAND_LEN 200
#define MAX_SUSPECT_NUM 50

uint32_t suspect_addr[MAX_SUSPECT_NUM];
int suspect_num = 0;
int addr_s = 0, addr_e = 0;
void RegProc(char*);

int main(int argc, char *argv[]) {
    // Preproccess
    if(argc<2)
    {
        printf("\033[31mTwo parameters needed!\033[0m\n");
        return 0;
    }
    else
    {
        for(int i=0; argv[1][i]!='\0'; ++i)
        {
            if(!isdigit(argv[1][i]))
            {
                printf("\033[31mPID should be numbers!\033[0m\n");
                return 0;
            }
        }
    }
    // Use Regex to find target address
    RegProc(argv[1]);
    // Read instructions
    char command[MAX_COMMAND_LEN];   // Store command from input
    char pid[20];   strcpy(pid, argv[1]);   // Store pid
    int tracepid = atoi(pid);   // pid in type int
    int traced = 0;
    char *tmp;
    memset(suspect_addr, 0, MAX_SUSPECT_NUM);
    while(1)
    {
        int exitflag = 0;
        printf("\033[36m>> \033[0m");
        if(!fgets(command, MAX_COMMAND_LEN, stdin))
            perror("fgets error");
        else
        {
            command[strlen(command)-1]='\0';
            switch(command[0])
            {
                // pause
                case 'p':
                if(strcmp("pause", command)==0)
                {
                    if(!traced)
                    {
                        if(ptrace(PTRACE_ATTACH, tracepid, NULL, NULL)==-1)
                            perror("Ptrace error in pause");
                        traced = 1;
                    }
                    printf("\033[34mThe process will pause\033[0m\n");
                }
                else
                    printf("\033[31mCommand %s is ilegal!\033[0m\n", command);
                break;
                // resume
                case 'r':
                if(strcmp("resume", command)==0)
                {
                    if(traced)
                    {
                        if(ptrace(PTRACE_DETACH, tracepid, NULL, NULL)==-1)
                            perror("Ptrace error in resume");
                        traced = 0;
                    }
                    char *argv[5], *envp[] = {"PATH=/bin", NULL};
                    int forkpid = fork();
                    if(forkpid == 0)
                    {
                        argv[0] = "kill";
                        argv[1] = "-CONT";
                        argv[2] = pid;
                        argv[3] = NULL;
                        execve("/bin/kill", argv, envp);
                    }
                    else if(forkpid > 0)
                        printf("\033[34mThe process will resume\033[0m\n");
                    else
                        perror("Fork error in resume");
                }
                else
                    printf("\033[31mCommand %s is ilegal!\033[0m\n", command);
                break;  
                //exit
                case 'e':
                if(strcmp("exit", command)==0)
                {
                    printf("\033[32mThe program will exit\n\033[0m");
                    exitflag = 1;
                }
                else 
                    printf("\033[31mCommand %s is ilegal!\033[0m\n", command);
                break;
                // setup
                case 's': 
                tmp = strtok(command, " ");
                if(strcmp("setup", tmp)==0)
                {
                    tmp = strtok(NULL, " ");
                    if(tmp == NULL)
                    {    
                        printf("\033[31mNot enough parameter!\033[0m\n");
                        break;
                    }
                    uint8_t targetval = atoi(tmp);
                    if(suspect_num > 1)
                        printf("\033[31mThe num of suspect addr is more than 1!\033[0m\n");
                    else if(suspect_num == 0)
                        printf("\033[31mNo skeptical address found, try lookup <number> first!\033[0m\n");
                    else
                    {
                        for(int i=0; i<MAX_SUSPECT_NUM; ++i)
                        {
                            if(suspect_addr[i]>0)
                            {
                                ptrace(PTRACE_POKEDATA, tracepid, suspect_addr[i], targetval);
                                int ret = ptrace(PTRACE_PEEKTEXT, tracepid, suspect_addr[i], NULL);
                                printf("\033[31maddr: %x, val: %d\033[0m\n", suspect_addr[i], ret);
                                break;
                            }
                        }
                    }
                }
                else
                    printf("\033[31mCommand %s is ilegal!\033[0m\n", command);
                break;
                // lookup
                case 'l':
                tmp = strtok(command, " ");
                if(strcmp("lookup", tmp)==0)
                {
                    tmp = strtok(NULL, " ");
                    if(tmp == NULL)
                    {
                        printf("\033[31mNot enough parameter!\033[0m\n");
                        break;
                    }
                    int current_addr[MAX_SUSPECT_NUM];
                    int current_num = 0;
                    uint8_t targetval = atoi(tmp);
                    uint8_t val;
                    wait(NULL);
                    for(int addr = addr_s; addr < addr_e; ++addr)
                    {
                        val = ptrace(PTRACE_PEEKTEXT, tracepid, addr, NULL);
                        if(val == targetval)
                        {
                            printf("\033[34maddr: %x, val: %d\n", addr, val);
                            current_addr[current_num] = addr;
                            current_num++;
                        }
                    }
                    
                    if(suspect_num == 0 && current_num > 0)
                    {
                        for(int i = 0; i < current_num; ++i)
                            suspect_addr[i] = current_addr[i];
                        suspect_num += current_num;
                    }

                    if(suspect_num > 0 && current_num > 0)
                    {
                        for(int i = 0; i < MAX_SUSPECT_NUM; ++i)
                        {
                            int flag = 0;
                            for(int j = 0; j < current_num; ++j)
                            {
                                if(suspect_addr[i] > 0 && suspect_addr[i] == current_addr[j])
                                {
                                    flag = 1;
                                    break;
                                }
                            }
                            if(flag == 0 && suspect_addr[i] != 0)
                            {
                                suspect_addr[i] = 0;
                                suspect_num--;
                            }
                        }
                        printf("\033[32mNow %d skeptical address(es) found\033[0m\n", suspect_num);
                    }
                    if(current_num == 0)
                        printf("\033[31mNo value %d found\033[0m\n", targetval);
                    else if(suspect_num == 1)
                        printf("\033[32mThe value is found, setup command now effective\033[0m\n");
                }
                else
                    printf("\033[31mCommand %s is ilegal!\033[0m\n", command);
                break;

                default:
                printf("\033[31m\nCommand %s is ilegal\033[0m\n", command);
                break;
            }
            if(exitflag)
                break;
        }
    }
    return 0;
}

void RegProc(char* pid)
{
    char path[MAX_COMMAND_LEN];
    regex_t reg1, reg2;
    char pattern1[] = "rw-p";
    char pattern2[] = "heap";
    regmatch_t match[1];
    strcpy(path, PROC_DIR);
    strcat(path, pid);
    strcat(path, "/maps");
    FILE *fp;
    if((fp = fopen(path, "r")) == NULL)
        perror(".../maps open fail in RegProc");
    regcomp(&reg1, pattern1, REG_EXTENDED);
    regcomp(&reg2, pattern2, REG_EXTENDED);
    char buf[MAX_COMMAND_LEN];
    while(fgets(buf, MAX_COMMAND_LEN, fp) != NULL)
    {
        if(regexec(&reg2, buf, 1, match, 0) == 0)
            break;
        if(regexec(&reg1, buf, 1, match, 0) == 0 && regexec(&reg2, buf, 1, match, 0) != 0)
        {
            char start[20], end[20];
            uint32_t offset = 0;
            while(buf[offset] != '-')
            {
                start[offset] = buf[offset];
                offset++;
            }
            start[offset++] = '\0';
            int tmp = offset;
            while(buf[offset] != ' ')
            {
                end[offset - tmp] = buf[offset];
                offset++;
            }
            end[offset] = '\0';
            int current_addr_s, current_addr_e;
            sscanf(start, "%x", &current_addr_s);
            sscanf(end, "%x", &current_addr_e);
            if(addr_s == 0 || addr_s > current_addr_s)
                addr_s = current_addr_s;
            if(addr_e == 0 || addr_e < current_addr_e)
                addr_e = current_addr_e;
        }
    }
    regfree(&reg1); regfree(&reg2);
    if(fclose(fp) != 0)
        perror("File close error in RegProc");
    printf("\033[36m0x%x 0x%x\n\033[0m", addr_s, addr_e);
    return;
}
