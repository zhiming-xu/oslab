#include <am.h>
#include <amdev.h>
#include "ioe.h"
#include "mylib.h"
int key_test()
{
    while(1)
    {
        _KbdReg * kbd = readkey();
        if(kbd->keycode!=_KEY_NONE)
        {
            if(kbd->keydown)
                printf("Keydown %d\n", kbd->keycode);
            else
                printf("Keyup %d\n", kbd->keycode);
        }
    }
    return 0;
}
