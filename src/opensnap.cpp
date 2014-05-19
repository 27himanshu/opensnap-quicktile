#include <stdio.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <cstdlib>
#include <string.h>
#include "xdo_functions.h"
#include "opensnap.h"
#include <iostream> //C strings are annoying.


void printHelp()
{
	printf("Usage: opensnap-quicktile <OPTION>\n");
	printf("\n");
	printf("Options:\n");
	printf("\n");
	printf("  -d, --daemon               run opensnap-quicktile as daemon\n");
	printf("  -s, --screens              specify number of (physical) screens. Default: autodetect.\n");
	printf("  -o, --offset <PIXEL>       offset in pixel\n");
	printf("  -v, --verbose              be verbose\n");
	printf("  -V, --version              print opensnap-quicktile version number\n");
	printf("  -h, --help                 print this help\n");
	printf("\n");
	printf("Example:\n");
	printf("\n");
	printf("opensnap-quicktile -d\n\n");
}


int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);                     
	Display *dsp = XOpenDisplay( NULL );
	if( !dsp ) { return 1; }

	screens scrinfo;
	getScreens(&scrinfo);
	int verbose=0;
	int isdrag=0;
	int isinitialclick=1;
	int offset=10;
	mousestate mousepos;
	mousestate relativeMousepos;
	XEvent event;
	
	std::string args = "0";

	struct option longopts[] = {
		{"offset",  1, NULL, 'o'},
		{"daemon",  0, NULL, 'd'},
		{"info",    0, NULL, 'i'},
		{"verbose", 0, NULL, 'v'},
		{"help",    0, NULL, 'h'},
		{"version", 0, NULL, 'V'},
		{0, 0, 0, 0}};

	int opt=0;
	
	
	while((opt = getopt_long(argc,argv,"c:o:divVh",longopts,NULL)) != -1)
	{
		switch(opt)
		{
		case 'd':
			if(daemon(0,0) == -1)
			{
				perror("daemon");
				exit(EXIT_FAILURE);
			}
			break;
			
		case 'i':
			dumpInfo(&scrinfo);
			exit(EXIT_SUCCESS);
			break;

			case 'v':
			verbose=1;
			break;
			
		case 'V':
			printf("opensnap-quicktile 1.1");
			exit(EXIT_SUCCESS);
			
		case 'o':
			offset=atoi(optarg);
			break;
			
		case 'h':
		case '?':
			printHelp();
			exit(EXIT_FAILURE);
			break;
		}
	}

	while(1)
	{
		getMousePosition(dsp, &event, &mousepos);

		int scrnn;
		scrnn = gdk_screen_get_monitor_at_point(gdk_screen_get_default(), mousepos.x, mousepos.y);

		//make mouse coordinates relative to screen
		relativeMousepos.x=mousepos.x-scrinfo.screens[scrnn].x;
		relativeMousepos.y=mousepos.y-scrinfo.screens[scrnn].y;
		if(verbose)
		{
			printf("Mouse Coordinates: %d %d %d\n", mousepos.x, mousepos.y, mousepos.state );
		}
		
		if((LEFTCLICK & mousepos.state)==LEFTCLICK)
		{
			if (isdrag)
			{			
				if(relativeMousepos.x<=offset)
				{

					if(relativeMousepos.y<=offset) { args = "top-left"; }
				
					else if(relativeMousepos.y>=scrinfo.screens[scrnn].height-offset-1) { args = "bottom-left"; }
					
					else { args = "left"; }
				}
				
				else if(relativeMousepos.x>=scrinfo.screens[scrnn].width-offset-1)
				{
					if(relativeMousepos.y<=offset) { args = "top-right"; }
				
					else if(relativeMousepos.y>=scrinfo.screens[scrnn].height-offset-1) { args = "bottom-right"; }
					
					else { args = "right"; }
				}
				
				else if(relativeMousepos.y>=scrinfo.screens[scrnn].height-offset-1) { args = "bottom"; } //Prevent minimize.

				else if(relativeMousepos.y<=offset) { args = "top"; }
			}
			
			else
			{
				if(!isdrag && isinitialclick)
				{
					if(isTitlebarHit(dsp, &mousepos) || mousepos.state & 8)
					{
						isdrag=1;
					}
				}
			}
			isinitialclick=false;
		}
		
		if(verbose)
		{
			printf("isdrag is: %d\n",isdrag);
		}
		
		if(((16 & mousepos.state) == mousepos.state || (24 & mousepos.state) == mousepos.state) && isdrag)
		{
			if(args != "0")
			{
				std::string tempAction = "quicktile ";
				tempAction += args;
				system(tempAction.c_str());
				args = "0";
			}
		}
		
		if((LEFTCLICK & mousepos.state) != LEFTCLICK)
		{
			isdrag=0;
			isinitialclick=1;
		}
		usleep(10000);
	}
	
	XCloseDisplay(dsp);
	free(scrinfo.screens);
	return 0;
}

void getMousePosition(Display *dsp, XEvent *event, mousestate *cords)
{
	XQueryPointer(dsp, RootWindow(dsp, DefaultScreen(dsp)),
		&event->xbutton.root, &event->xbutton.window,
		&event->xbutton.x_root, &event->xbutton.y_root,
		&event->xbutton.x, &event->xbutton.y,
		&event->xbutton.state);
	cords->x=event->xbutton.x;
	cords->y=event->xbutton.y;
	cords->state=event->xbutton.state;
}


void getScreens(screens *scrinfo)
{
	GdkScreen *screen = gdk_screen_get_default();
	gint nmon = gdk_screen_get_n_monitors(screen);
	scrinfo->screens = (oRectangle*) malloc(sizeof(oRectangle)*nmon);
	scrinfo->amount=nmon;

	for(int i=0; i < nmon; i++)
	{
		GdkRectangle rect;
		gdk_screen_get_monitor_geometry(screen, i, &rect);
		scrinfo->screens[i].x=rect.x;
		scrinfo->screens[i].y=rect.y;
		scrinfo->screens[i].width=rect.width;
		scrinfo->screens[i].height=rect.height;
	}
}

void getFocusedWindow(Display *dsp,Window *w)
{
	int revert;
	XGetInputFocus(dsp,w,&revert);
}

void findParentWindow(Display *dsp, Window *w, Window *parent)
{
	xdo_window_find_client(dsp,*w,parent,XDO_FIND_PARENTS);
}

void getWindowRect(Display *dsp, Window *win, int *x, int *y, unsigned int *w, unsigned int *h)
{
	unsigned int bw,d;
	int junkx, junky;
	Window junkroot =  RootWindow(dsp, 0);
	XGetGeometry(dsp,*win,&junkroot,&junkx,&junky,w,h,&bw,&d);
	XTranslateCoordinates(dsp, *win, junkroot, junkx, junky, x, y, &junkroot);
}

void getNetFrameExtents(Display *dpy, Window *w, int *top)
{
	long *extents;
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned char *data = NULL;
	int result;

	*top = 0;

	result = XGetWindowProperty(
		dpy, *w, XInternAtom(dpy, "_NET_FRAME_EXTENTS", False),
		0, 4, False, AnyPropertyType,
		&actual_type, &actual_format,
		&nitems, &bytes_after, &data);

	if (result == Success)
	{
		if ((nitems == 4) && (bytes_after == 0))
		{
			extents = (long *)data;
			*top = (int) *(extents + 2);
		}
		XFree(data);
	}
}

void dumpInfo(screens *scrinfo)
{
	for(int i=0; i<scrinfo->amount; i++)
	{
		printf("Screen %i\n", i);
		printf("\t Width:\t\t%i\n\t Height:\t%i\n\t X:\t\t%i\n\t Y:\t\t%i\n\n",
		scrinfo->screens[i].width, scrinfo->screens[i].height, scrinfo->screens[i].x, scrinfo->screens[i].y);
	}
}

int isTitlebarHit(Display *dsp, mousestate *mousepos)
{
	int titlebarHeight, x, y, junkx, junky;
	unsigned int wi,h, junkwi, junkh;
	Window activeWindow, parentWin;
	getFocusedWindow(dsp,&activeWindow);
	findParentWindow(dsp,&activeWindow,&parentWin);
	getNetFrameExtents(dsp,&parentWin,&titlebarHeight);
	getWindowRect(dsp, &parentWin, &junkx, &junky, &wi, &h);
	getWindowRect(dsp, &activeWindow, &x, &y, &junkwi, &junkh); // we need the size of the parent win, but the x/y coordinates of the child, don't ask me why, otherwise the values are off a bit

	if(y==junky)
	{
		y-=titlebarHeight; //Qt hack
	}

	if(mousepos->x>=x && mousepos->x <= (x+(int)wi) && mousepos->y >= (y-titlebarHeight) && mousepos->y <= y)
	{
		return 1;
	}
	return 0;
}
