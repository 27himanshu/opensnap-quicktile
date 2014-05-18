#define LEFTCLICK 256
#define MY_MAXPATH 2048

typedef struct {
    int x;
    int y;
    int state;
} mousestate;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} oRectangle;

typedef struct {
    int amount;
    oRectangle *screens;
} screens;

void getMousePosition(Display *dsp, XEvent *event, mousestate *cords);
void getScreens(screens* scrinfo);
void getFocusedWindow(Display *dsp,Window *w);
void getNetFrameExtents(Display *dpy, Window *w, int *top);
void getWindowRect(Display *dsp, Window *win, int *x, int *y, unsigned int *w, unsigned int *h);
void findParentWindow(Display *dsp, Window *w, Window *parent);
int isTitlebarHit(Display *dsp, mousestate *mousepos);
void dumpInfo(screens *scrinfo);
