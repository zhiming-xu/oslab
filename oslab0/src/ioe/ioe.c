#include <am.h>
#include <amdev.h>
#include "ioe.h"
#include "mylib.h"

static _Device *find_device(int id)
{
    for(int i=1;;++i)
    {
        _Device *dev=_device(i);
        if(!dev)
            break;
        else if(dev->id==id)
        {
            return dev;
        }
    }
    return NULL;
}
uint32_t uptime()
{
    _UptimeReg upt;
    _Device *dev=find_device(_DEV_TIMER);
    dev->read(_DEVREG_TIMER_UPTIME, &upt, sizeof(upt));
    return upt.lo;
}
_KbdReg *readkey()
{
    _KbdReg *kbd=NULL;
    _Device *dev=find_device(_DEV_INPUT);
    dev->read(_DEVREG_INPUT_KBD, kbd, sizeof(kbd));
    return kbd;
}
int screen_width()
{
    _Device *dev=find_device(_DEV_VIDEO);
    _VideoInfoReg rinfo;
    dev->read(_DEVREG_VIDEO_INFO, &rinfo, sizeof(rinfo));
    return rinfo.width;
}

int screen_height()
{
    _Device *dev=find_device(_DEV_VIDEO);
    _VideoInfoReg rinfo;
    dev->read(_DEVREG_VIDEO_INFO, &rinfo, sizeof(rinfo));
    return rinfo.height;
}
void draw_rect(uint32_t *pixels, int x, int y, int w, int h)
{
    _Device *dev=find_device(_DEV_VIDEO);
    _FBCtlReg ctl;
    ctl.pixels=pixels;
    ctl.x=x;    ctl.y=y;
    ctl.w=w;    ctl.h=h;
    ctl.sync=1;
    dev->write(_DEVREG_VIDEO_FBCTL, &ctl, sizeof(ctl));
}
void draw_sync()
{
}
