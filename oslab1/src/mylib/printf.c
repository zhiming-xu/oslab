#include <stdarg.h>
#include <am.h>

static int printf_d(int d, int len_flag, int forced_len);
static int printf_x(int x, int len_flag, int forced_len);
static int printf_c(char c);
static int printf_s(char *s);

int printf(const char *fmt, ...)
{
    int ret=0;
    va_list ap;
    va_start(ap, fmt);
    while(*fmt)
    {
        if(*fmt!='%')
        {
            _putc(*fmt);
            fmt++;
            ret++;
        }
        else
        {
            int fmt_len=0;//此字段的长度
            int len_flag=0;//是否限制输出长度
            char forced_str[10];//%[08]d/x, etc
            int str_index=0;
            int forced_len=0;//限制输出的长度
            fmt++;//%[forced length]d/x, %c, %s, etc
            while(*fmt>='0'&&*fmt<='9')
            {
                if(len_flag==0)
                    len_flag=1;
                forced_str[str_index]=*fmt;
                str_index++;
                fmt++;
            }
            int base=1;
            for(int i=str_index-1;i>=0;--i)
            {
                forced_len+=base*(forced_str[i]-'0');
                base*=10;
            }
            switch(*fmt)
            {
                case 'd':
                {
                    int d_val;
                    d_val=va_arg(ap, int);
                    fmt_len=printf_d(d_val, len_flag, forced_len); 
                    fmt++;
                    break;

                }
                case 'x':
                {
                    int x_val;
                    x_val=va_arg(ap, int);
                    fmt_len=printf_x(x_val, len_flag, forced_len);
                    fmt++;
                    break;
                }
                case 'c':
                {
                    char c_val;
                    c_val=va_arg(ap, int);
                    fmt_len=printf_c(c_val);
                    fmt++;
                    break;
                }
                case 's':
                {
                    char* s_val;
                    s_val=va_arg(ap, char*);
                    fmt_len=printf_s(s_val);
                    fmt++;
                    break;
                }
            }
            ret+=fmt_len;
        }
    }
    return ret;
}
static int printf_d(int d, int len_flag, int forced_len) 
{
    int ret=0;
    if(d<0)
    {
        _putc('-');
        d=-d;
        ret++;
    }
    else if(d==0)
    {
        ret++;
    }
    int base=1;
    while((d/base))
    {
        base*=10;
        ret++;
    }
    base=base==1?base:base/10;
    if(len_flag&&(forced_len>ret))
        for(int i=0;i<forced_len-ret;++i)
            _putc('0');
    while(base)
    {
        _putc('0'+(d/base));
        d%=base;
        base/=10;
    }
    ret=forced_len>ret?forced_len:ret;
    return ret;
}
static int printf_x(int x, int len_flag, int forced_len)
{
    char hex[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    int ret=0;
    if(x<0)
    {
        _putc('-');
        x=-x;
        ret++;
    }
    else if(x==0)
    {
        ret++;
    }
    int base=1;
    while((x/base))
    {
        base*=16;
        ret++;
    }
    base=base==1?base:base/16;
    if(len_flag&&(forced_len>ret))
        for(int i=0;i<forced_len-ret;++i)
            _putc('0');
    while(base)
    {
        _putc(hex[x/base]);
        x%=base;
        base/=16;
    }
    ret=forced_len>ret?forced_len:ret;
    return ret;
}
static int printf_c(char c)
{
    int ret=1;
    _putc(c);
    return ret;
}
static int printf_s(char* s)
{
    int ret=0;
    while(*s)
    {
        _putc(*s);
        s++;
        ret++;
    }
    return ret;
}
