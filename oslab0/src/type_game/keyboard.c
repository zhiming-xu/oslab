#include "game.h"

static int letter_code[]={
    _KEY_A, _KEY_B, _KEY_C, _KEY_D, _KEY_E, _KEY_F, _KEY_G,
    _KEY_H, _KEY_I, _KEY_J, _KEY_K, _KEY_L, _KEY_M, _KEY_N, _KEY_O, _KEY_P,
    _KEY_Q, _KEY_R, _KEY_S, _KEY_T, _KEY_U, _KEY_V,
    _KEY_W, _KEY_X, _KEY_Y, _KEY_Z,
};

static bool letter_pressed[26];

void press_key(int code)
{
    for(int i=0;i<26;++i)
    {
        if(letter_code[i]==code)
        {
            letter_pressed[i]=true;
        }
    }
}

void release_key(int ind)
{
    if(ind > 100)
        ind -= 100;
    letter_pressed[ind]=false;
}

bool query_key(int ind)
{
    if(ind > 100)
        ind -= 100;
    return letter_pressed[ind];
}

static int key_code=0;

int last_key_code(void)
{
    return key_code;
}

bool keyboard_event()
{
    _KbdReg* rkey = readkey();
    if(rkey->keycode==_KEY_NONE)
        return false;

    if(rkey->keydown)
    {
        press_key(rkey->keycode);
        return true;
    }
    else
    {
        for(int i=0;i<26;++i)
        {    
            if(rkey->keycode==letter_code[i])
                release_key(i);
        }
        return true;
    }
}
