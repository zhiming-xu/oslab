#include "game.h"

//#define SCREEN_STRETCH

static uint32_t canvas[H][W];

extern char font8x8_basic[128][8];
int over_flag = 0;

char *itoa(int n)  {  
    static char s[64];
    int i = sizeof(s) - 1;
    do {
        s[--i] = n % 10 + '0';  
        n /= 10;
    } while(n > 0);  
    return &s[i];
}  

static inline void draw_character(char ch, int x, int y, int color) {
    int i, j;
    char *p = font8x8_basic[(int)ch];
    for (i = 0; i < 8; i ++) 
        for (j = 0; j < 8; j ++) 
            if ((p[i] >> j) & 1)
                if (x + j < W && y + i < H)
                    canvas[y + i][x + j] = color;
}

static inline void draw_string(const char *str, int x, int y, int color) {
    while (*str) {
    draw_character(*str ++, x, y, color);
    if (x + 8 >= W) {
        y += 8; x = 0;
    } 
    else {
      x += 8;
    }
  }
}
int t1 = 0;//程序开始的时间
int t2 = 0;//绘图时的时间
int countdown = 60;//倒计时
int cd_color = 0x00ff00;//倒计时的颜色
int rand_color[] = {0xffffff, 0xff0000, 0xffff00, 0x0000ff};
extern int mode;

void redraw_screen() {
    if(countdown == 0)
    {
        over_flag = 1;
        return;
    }
    fly_t it;
    const char *hit, *miss;
    int swidth = screen_width();
    int sheight = screen_height();
    t2 = uptime();
    if((t2 - t1) / 1000 + countdown >= 60)
    {
        countdown--;
        if(countdown >= 40)
            cd_color += 0x0c0000;
        else if(countdown >= 20)
            cd_color -= 0x000c00;
        else
            cd_color += 1;
    }
    //绘制下落的字符
    for (it = characters(); it != NULL; it = it->_next) {
        static char buf[2];
        int tmp = it->text; 
        if(tmp > 100)
            buf[0] = tmp -100 + 'a';
        else
            buf[0] = tmp + 'A';
        buf[1] = 0;
        if(mode & 0x100)
            draw_string(buf, it->x, it->y, rand_color[uptime() % 4]);
        else
            draw_string(buf, it->x, it->y, 0xffffff);
    }
    //倒计时，渐变色：绿->黄->红，最后10秒闪烁
    const char *key = itoa(countdown);
    if(countdown <= 10 && countdown % 2)
        key = '\0';
    draw_string(key, 0, H - 8, cd_color);
  //命中数，绿色
    hit = itoa(get_hit());
    draw_string(hit, W - strlen(hit) * 8, 0, 0x00ff00);
    //miss数，红色
    miss = itoa(get_miss());
    draw_string(miss, W - strlen(miss) * 8, H - 8, 0xfa5858);
    //FPS，黄色
    const char *fps = itoa(get_fps());
    draw_string(fps, 0, 0, 0xf3f781);
    draw_string("FPS", strlen(fps) * 8, 0, 0xf3f781);

#ifdef SCREEN_STRETCH
    int w = swidth, h = sheight;
    for (int x = 0; x < w; x ++)
        for (int y = 0; y < h; y ++) {
            draw_rect(&canvas[y * H / h][x * W / w], x, y, 1, 1);
    }
#else
    int x = (swidth - W) / 2;
    int y = (sheight - H) / 2;
    draw_rect(&canvas[0][0], x, y, W, H);
#endif

    draw_sync();
    for (int y = 0; y < H; y ++)
        for (int x = 0; x < W; x ++)
            canvas[y][x] = 0x6a005f;
}

int mode = 0;

void draw_welcome()
{
    for (int y = 0; y < H; y ++)
        for (int x = 0; x < W; x ++)
            canvas[y][x] = 0x6a005f;
    
    char w1[] = "Welcome to this naive type game!";
    draw_string(w1, (W - strlen(w1) * 8) / 2, H / 3, 0xffffff);
    char w2[] = "You can choose different mode, from simple to extremely hard";
    draw_string(w2, (W - strlen(w2) * 8) / 2, H / 3 + 36, 0xffffff);
    char w3[] = "Press S for simple, M for medium, H for hard, and E for extremely hard";
    draw_string(w3, (W - strlen(w3) * 8) / 2, H / 3 + 64, 0xffffff);
    int swidth = screen_width();
    char w4[] = "Any other key will lead to simple mode";
    draw_string(w4, (W - strlen(w4) * 8) / 2, H / 3 + 80, 0xffffff);
    int sheight = screen_height();
    int x = (swidth - W) / 2;
    int y = (sheight - H) / 2;
    draw_rect(&canvas[0][0], x, y, W, H);
    
    while(1)
    {
        _KbdReg *kbd = readkey();
        if(kbd->keycode != _KEY_NONE)
        {
            switch(kbd->keycode)
            {
                case _KEY_M:
                    {
                        mode = 0x1;
                        break;
                    }
                case _KEY_H:
                    {
                        mode = 0x11;
                        break;
                    }
                case _KEY_E:
                    {
                        mode = 0x111;
                        break;
                    }
                case _KEY_S:
                default:
                    {
                        mode = 0x0;
                        break;
                    }
            }
            break;
        }
    }
}

void draw_gameover(int score)
{
    for (int y = 0; y < H; y ++)
        for (int x = 0; x < W; x ++)
            canvas[y][x] = 0x6a005f;
    char o1[] = "GAME OVER!";
    draw_string(o1, (W - strlen(o1) * 8) / 2, H/3 + 16, 0xffff00);
    char o2[] = "Congratulations, your score is";
    draw_string(o2, (W - strlen(o2) * 8) / 2, H/3 + 48, 0xffff00);
    char *num = itoa(score);
    draw_string(num, 312, H/3 + 60, 0xffff00);
    char o3[] = "Press any key to quit";
    draw_string(o3, (W - strlen(o2) * 8) / 2, H/3 + 80, 0xffff00);
    
    int swidth = screen_width();
    int sheight = screen_height();
    int x = (swidth - W) / 2;
    int y = (sheight - H) / 2;
    draw_rect(&canvas[0][0], x, y, W, H);
    
    while(1)
    {
        _KbdReg *kbd = readkey();
        if(kbd->keycode != _KEY_NONE)
            break;
    }
}
