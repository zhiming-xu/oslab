#include <am.h>
#include <amdev.h>
#include "mylib.h"
#include "ioe.h"
#include "game.h"
extern int over_flag;
extern int t1;
static int real_fps;
static int hit = 0, miss = 0;

void set_fps(int value) {
    real_fps = value;
}

int get_fps() {
    return real_fps;
}

int type_game(){
    draw_welcome();
    t1 = uptime();
    int num_draw = 0, frames = 0;
    unsigned long next_frame = 0, next_refresh = 0;
    while(1) {
        unsigned long t;
        bool redraw = false;
        while (1) {
            t = uptime();
            if (t >= next_frame) 
                break;                           
        }
        frames ++;
        if (t > next_refresh) {
            redraw = true;
            next_refresh += 1000 / FPS;
        }                
        next_frame += 1000 / HZ;
        while (keyboard_event());
        while (update_keypress());
        if (frames % (HZ / CHARACTER_PER_SECOND) == 0) {
            create_new_letter();
        }
        if (frames % (HZ / UPDATE_PER_SECOND) == 0) {
            update_letter_pos();
        }
        if(redraw) {
            num_draw ++;
            set_fps(num_draw * 1000 / t);
            redraw_screen();
        }
        if(over_flag)
        {
            draw_gameover(hit);
            break;
        }   
    }
    return 0;
}

LINKLIST_IMPL(fly, 1000)

static fly_t head = NULL;

int get_hit(){
    return hit;
}

int get_miss(){
    return miss;
}

fly_t characters(){
    return head;
}

extern int mode;

void create_new_letter(){
    if(head == NULL){
        head = fly_new();      
    }
    else{
        fly_t now = fly_new();
        fly_insert(NULL, head, now);
        head = now;              
    }
    head->y = 0;
    head->x = rand() % (W / 8 - 2) * 8 + 8;
    head->v = mode & 0x1?(rand() % 1001)/(250) + 1 : 2;
    //head->text = rand() % 26;
    if(mode & 0x10)
    {
        head->text = rand() % 26;
        if(uptime() % 2)
            head->text += 100;
    }
    else
    {
        head->text = rand() % 26;
    }
    release_key(head->text);
}

void update_letter_pos() {
    fly_t it;
    for(it = head;it != NULL;){
        fly_t next = it->_next;
        it->y += it->v;
        if (it->y < 0 || it->y + 8 > H){
            if(it->y < 0)
                hit++;
            else
                miss++;
            fly_remove(it);
            fly_free(it);
            if(it == head)  
                head = next;
        }
        it = next;
    }
}

bool update_keypress() {
    fly_t it,target = NULL;
    int min = -100;
    for(it = head; it != NULL; it = it->_next){
        if(it->v > 0 && it->y > min && query_key(it->text)){
            min = it->y;
            target = it;                    
        }      
    }
    if(target != NULL){
        release_key(target->text);
        target->v = -3;
        return true;
    }
    return false;
}
