#include <stdio.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <dbus-cxx.h>
#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include "xdo_functions.h"
#include "opensnap.h"
#include "quickTileProxy.h"

// Constants
#define NAME "opensnap-quicktile"
#define VERSION "1.3"

#define LEFTCLICK 256

// Initialise logging
log4cpp::Category& logger = log4cpp::Category::getRoot();

// Global variables
int dbus = 0;
quickTileProxy * qt_proxy;

/** Execute the given string as the argument of the quicktile binary.
 */
int string2exec(std::string args)
{
	logger.info("Executing quicktile command%s: %s", dbus ? "(via DBUS)" : "", args.c_str());
    int rtn;
    if (dbus) {
        rtn = qt_proxy->doCommand(args);
    } else {
        std::string tempAction = "quicktile " + args;
        rtn = system(tempAction.c_str());
    }
    logger.log(rtn == 0 ? log4cpp::Priority::DEBUG : log4cpp::Priority::WARN, "Command returned %d", rtn);
    return rtn;
}

int main(int argc, char **argv)
{
    // Set up logger
    log4cpp::Appender *console = new log4cpp::OstreamAppender("console", &std::cout);
    console->setLayout(new log4cpp::BasicLayout());
    logger.setAppender(console);

    // Get the OSQT_LOG_LEVEL environment variable
    const char* osqt_log_level = getenv("OSQT_LOG_LEVEL");
    if (osqt_log_level == NULL) {
        // If the OSQT_LOG_LEVEL environment variable isn't set then set to
        // DEFAULT_LOG_PRIORITY
        logger.setPriority(log4cpp::Priority::ERROR);
    } else {
        try {
            logger.setPriority(log4cpp::Priority::getPriorityValue(osqt_log_level));
            logger.info("Log level set by OSQT_LOG_LEVEL environment variable to %d", logger.getPriority());
        } catch (std::invalid_argument) {
            logger.setPriority(log4cpp::Priority::ERROR);
            logger.error("Value of OSQT_LOG_LEVEL environment variable was not a valid value: %s\n"
            "\tUsing default log level of 'ERROR'", osqt_log_level);
        }
    }

    logger.info("Starting up " NAME);

    // Initialise GTK and X
    logger.debug("Initialising GTK and X");
	gtk_init(&argc, &argv);
    logger.debug("GTK initialised");
	Display *dsp = XOpenDisplay( NULL );
	if( !dsp ) {
        logger.fatal("Could not initialise X. Is the X Server running?");
        return EXIT_FAILURE;
    }

	screens scrinfo;
	getScreens(&scrinfo);
	int isdrag=0;
	int isinitialclick=1;
	int offset=10;
	mousestate mousepos;
	mousestate relativeMousepos;
	XEvent event;
	int mode = 0;
	int Continue = 0;

	struct option longopts[] = {
		{"offset",    1, NULL, 'o'},
		{"daemon",    0, NULL, 'd'},
		{"help",      0, NULL, 'h'},
		{"version",   0, NULL, 'v'},
		{"debug",     0, NULL, 'D'},
		{"titledrag", 0, NULL, 't'},
		{"altdrag",   0, NULL, 'a'},
		{"dbus",      0, NULL, 'b'},
		{0, 0, 0, 0}};

	int opt=0;

	// Handle command line options
    logger.info("Parsing command line arguments");
	while((opt = getopt_long(argc,argv,"o:dhvDtab",longopts,NULL)) != -1)
	{
		switch(opt)
		{
		case 'd':
            logger.debug("Daemon mode requested by command line arguments");
			if(daemon(0,0) == -1)
			{
				logger.fatal("Could not daemonise OpenSnap");
				exit(EXIT_FAILURE);
			}
            logger.notice("Now running as daemon");
			break;

		case 'v':
			printf("opensnap-quicktile " VERSION);
			exit(EXIT_SUCCESS);
			break;

		case 'D':
            logger.setPriority(static_cast<int>(logger.getPriority()) + 100);
			break;

		case 'o':
			offset=atoi(optarg);
            logger.notice("Offset has been set to %d pixels");
			break;

		case 't':
			mode = 1;
            logger.notice("Mode has been set to titlebar drag only");
			break;

		case 'a':
			mode = 2;
            logger.notice("Mode has been set to Alt+Dragging only");
			break;

        case 'b':
            dbus = 1;
            logger.notice("DBus mode has been enabled");
            break;

		case 'h':
		case '?':
			printf("Usage: opensnap-quicktile <OPTION>\n\n");
			printf("Options:\n");
			printf("  -d, --daemon            Run opensnap-quicktile as daemon.\n");
			printf("  -o, --offset <PIXEL>    Offset in pixel.\n");
			printf("  -v, --version           Print opensnap-quicktile version number.\n");
			printf("  -h, --help              Print this help.\n");
			printf("  -D, --debug             Increase debug log level\n");
			printf("  -t, --titledrag         Only drags by titlebar.\n");
			printf("  -a, --altdrag           Only drags by Alt+Dragging.\n\n");
			printf("  -b, --dbus              Use dbus to communicate with quicktile daemon.\n\n");

			exit(EXIT_FAILURE);
			break;
		}
	}

    if (dbus)
    {
        logger.info("DBus connection being initiated");
        DBus::init();
        logger.debug("Creating a DBus dispatcher");
        DBus::Dispatcher::pointer dispatcher = DBus::Dispatcher::create();
        logger.debug("Creating a connection to Session Bus");
        DBus::Connection::pointer connection = dispatcher->create_connection(DBus::BUS_SESSION);
        logger.debug("Setting up proxy to QuickTile");
        qt_proxy = new quickTileProxy(connection);
    }

	// Main loop
    logger.info("Waiting for actions");
	while(1)
	{
		getMousePosition(dsp, &event, &mousepos);

		int scrnn;
		scrnn = gdk_screen_get_monitor_at_point(gdk_screen_get_default(), mousepos.x, mousepos.y);

		relativeMousepos.x=mousepos.x-scrinfo.screens[scrnn].x;
		relativeMousepos.y=mousepos.y-scrinfo.screens[scrnn].y;

		/* Check if the window has been dragged to a screen edge. */
		if((LEFTCLICK & mousepos.state)==LEFTCLICK)
		{
			if(relativeMousepos.x<=offset || relativeMousepos.x>=scrinfo.screens[scrnn].width-offset-1 || relativeMousepos.y>=scrinfo.screens[scrnn].height-offset-1 || relativeMousepos.y<=offset) { Continue = 1; }

			else
			{
				if(!isdrag && isinitialclick)
				{
					if(mode == 0 || mode == 2)
					{
						if(mousepos.state & 8)
						{
							isdrag=1;
						}
					}

					if(mode == 0 || mode == 1)
					{
						if(isTitlebarHit(dsp, &mousepos))
						{

							isdrag=1;
						}
					}
				}
				Continue = 0;
			}
			isinitialclick=false;
		}

		if(Continue == 1)
		{
			if(((16 & mousepos.state) == mousepos.state || (24 & mousepos.state) == mousepos.state) && isdrag)
			{
				/* Check to see if window is still over there. This is to prevent the window from snapping after the mouse has been moved away. */
				if(relativeMousepos.x<=offset)
				{
					if(relativeMousepos.y<=offset) { string2exec("top-left"); }

					else if(relativeMousepos.y>=scrinfo.screens[scrnn].height-offset-1) { string2exec("bottom-left"); }

					else { string2exec("left"); }
				}

				else if(relativeMousepos.x>=scrinfo.screens[scrnn].width-offset-1)
				{
					if(relativeMousepos.y<=offset) { string2exec("top-right"); }

					else if(relativeMousepos.y>=scrinfo.screens[scrnn].height-offset-1) { string2exec("bottom-right"); }

					else { string2exec("right"); }
				}

				else if(relativeMousepos.y>=scrinfo.screens[scrnn].height-offset-1) { string2exec("bottom"); }

				else if(relativeMousepos.y<=offset) { string2exec("top"); }

				Continue = 0;
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
    delete qt_proxy;
	return 0;
}

/** Get the current location of the pointer.
*/
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

/** Get information about each screen.
*/
void getScreens(screens *scrinfo)
{
	GdkScreen *screen = gdk_screen_get_default();
	gint nmon = gdk_screen_get_n_monitors(screen);
	scrinfo->screens = (oRectangle*) malloc(sizeof(oRectangle)*nmon);
	scrinfo->amount=nmon;

	for(int i=0; i < nmon; i++)
	{
		gdk_screen_get_monitor_geometry(screen, i, &scrinfo->screens[i]);
	}
}

/** Get the window currently in focus.
*/
void getFocusedWindow(Display *dsp,Window *w)
{
	int revert;
	XGetInputFocus(dsp,w,&revert);
}

/** Find the parent window of the given window.
*/
void findParentWindow(Display *dsp, Window *w, Window *parent)
{
	xdo_window_find_client(dsp,*w,parent,XDO_FIND_PARENTS);
}

/** Get the rectangle that makes up the window area.
*/
void getWindowRect(Display *dsp, Window *win, int *x, int *y, unsigned int *w, unsigned int *h)
{
	unsigned int bw,d;
	int junkx, junky;
	Window junkroot =  RootWindow(dsp, 0);
	XGetGeometry(dsp,*win,&junkroot,&junkx,&junky,w,h,&bw,&d);
	XTranslateCoordinates(dsp, *win, junkroot, junkx, junky, x, y, &junkroot);
}

/** Find the extents of the window frame added by the window manager.
 */
void getNetFrameExtents(Display *dpy, Window *w, int *top)
{
	long *extents;
	Atom actual_type;
	int actual_format;
	unsigned long nitems, bytes_after;
	unsigned char *data = NULL;
	int result;

	*top = 0;

	result = XGetWindowProperty(dpy, *w, XInternAtom(dpy, "_NET_FRAME_EXTENTS", False), 0, 4, False, AnyPropertyType, &actual_type, &actual_format,	&nitems, &bytes_after, &data);

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

/** Is the mouse in the titlebar of the active window?
 */
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
