#include <am.h>
#include <amdev.h>
#include "mylib.h"
#include "ioe.h"
#include "app.h"
int main()
{
    if(_ioe_init()!=0)
        _halt(1);
    //return init_test();
    //return key_test();
    return type_game();
}
