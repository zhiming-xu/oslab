#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
char cmd[10000];
int ind=1;
int main() 
{
    printf("\033[40;33mWelcome to this little repl for C\nType exit to quit\033[0m\n");
    char *name=tmpnam(NULL);
    char *srcname=(char*)malloc(sizeof(char)*(strlen(name)+3));
    strcpy(srcname, name);
    strcat(srcname, ".c");
    char compile[200]="gcc -shared -fPIC -o ";
    char libname[20]="mylib.so ";
    strcat(compile, libname);
    strcat(compile, srcname);
    strcat(compile, " -ldl");
    while(1)
    {
        printf("\033[40;37m> \033[0m");
        if((fgets(cmd, 10000, stdin)==NULL)||(strcmp(cmd, "exit\n")==0))
            break;
        cmd[strlen(cmd)-1]='\0';
        FILE *fp=fopen(srcname, "a");
        if(cmd[0]=='i'&&cmd[1]=='n'&&cmd[2]=='t')
        {
            fprintf(fp, "%s\n", cmd);
            fclose(fp);
            system(compile);
            perror("GCC");
        }
        else
        {
            char f[100]="tmpfunc";
            char t[5]="0";
            sprintf(t, "%d", ind++);
            strcat(f, t);
            fprintf(fp, "int %s(){ return %s; }\n", f, cmd);
            fclose(fp);
            system(compile);
            perror("GCC");
            void *handle;
            int (*func)();
            handle=dlopen("./mylib.so", RTLD_LAZY);
            if(!handle)
            {
                fprintf(stderr, "%s\n", dlerror());
                exit(1);
            } 
            func=dlsym(handle, f);
            char *error;
            if((error=dlerror())!=NULL)
            {
                fprintf(stderr, "%s\n", error);
                exit(1);
            }
            printf("\033[40;37m> \033[0m");
            printf("\033[40;32m%s = %d\033[0m\n", cmd, func());
            dlclose(handle);
        }
    }
    char clean[100];
    strcpy(clean, "rm "); strcat(clean, srcname); system(clean); 
    strcpy(clean, "rm "); strcat(clean, libname); system(clean);
    return 0;
}
