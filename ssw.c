#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

struct cxt {
Display *dis;
int screen;
Window win;
GC gc;

XEvent event;
KeySym key;
char text[255];
};

struct cxt context[20];

unsigned long LHEX(char v[7]){
int r, g, b;
r = 16*(v[1] - 48 - (((v[1] - 48)>>5)*39)) + v[2] - 48 - (((v[2] - 48)>>5)*39);
g = 16*(v[3] - 48 - (((v[3] - 48)>>5)*39)) + v[4] - 48 - (((v[4] - 48)>>5)*39);
b = 16*(v[5] - 48 - (((v[5] - 48)>>5)*39)) + v[6] - 48 - (((v[6] - 48)>>5)*39);
return b + (g<<8) + (r<<16);
}

unsigned long HEX(char v[7]){
int r, g, b;
r = 16*(v[1] - 48 - (((v[1] - 48)>>4)*7)) + v[2] - 48 - (((v[2] - 48)>>4)*7);
g = 16*(v[3] - 48 - (((v[3] - 48)>>4)*7)) + v[4] - 48 - (((v[4] - 48)>>4)*7);
b = 16*(v[5] - 48 - (((v[5] - 48)>>4)*7)) + v[6] - 48 - (((v[6] - 48)>>4)*7);
return b + (g<<8) + (r<<16);
}

unsigned long RGB(int r, int g, int b){
return b + (g<<8) + (r<<16);
}

struct cache {
char t;
char b;
int txt;
int x;
int y;
int w;
int h;
};

void XI(char root, const char *title, 
					   const char *subtitle, 
					   int height, int width,
					   int sx, int sy, char d) {
context[d].dis=XOpenDisplay(":0");
context[d].screen=DefaultScreen(context[d].dis);

	if (root) {
	context[d].win=RootWindow(context[d].dis, context[d].screen);
	} else {
	context[d].win=XCreateWindow(context[d].dis, DefaultRootWindow(context[d].dis), sx, sy, height, width, 
	5, 0, 0, 0, 0, (XSetWindowAttributes *)0);
	
	XSetStandardProperties(context[d].dis, context[d].win, title, subtitle, None, NULL, 0, NULL);
	XSelectInput(context[d].dis, context[d].win, ExposureMask|ButtonPressMask|KeyPressMask);
	}

context[d].gc=XCreateGC(context[d].dis, context[d].win, 0, 0);
XClearWindow(context[d].dis, context[d].win);
	if (!root) {
 	XMapRaised(context[d].dis, context[d].win);
	}
}

void Clean(char d){
XFreeGC(context[d].dis, context[d].gc);
XDestroyWindow(context[d].dis, context[d].win);
XCloseDisplay(context[d].dis);
}

void Flush(char d) {
XFlush(context[d].dis);
}

void RegionFill(int x, int y, int h, int w, unsigned long color, char d){
XSetForeground(context[d].dis, context[d].gc, color);
XFillRectangle(context[d].dis, context[d].win, context[d].gc, x , y, h, w);
}

void RegionScarf(int x, int y, int h, int w, unsigned long color, char d){
XSetForeground(context[d].dis, context[d].gc, color);
XDrawRectangle(context[d].dis, context[d].win, context[d].gc, x-1, y-1, h, w);
}

void Point(int x, int y, unsigned long color, char d) {
XSetForeground(context[d].dis, context[d].gc, color);
XDrawPoint(context[d].dis, context[d].win, context[d].gc, x, y);
}

void Eve(struct cache *cc, char d){
char kill = 0; 

cc->t =   0;
cc->b =   0;
cc->txt = 0;
cc->x =   0;
cc->y =   0;

XNextEvent(context[d].dis, &context[d].event);
	if (context[d].event.type==Expose) {
	cc->t = 3;
	}
        if (context[d].event.type==ButtonPress) {
        cc->t = 2;
        cc->b = context[d].event.xbutton.button;
        cc->x = context[d].event.xbutton.x;
        cc->y = context[d].event.xbutton.y;
        }
        if (context[d].event.type==2) {
	XLookupString(&context[d].event.xkey,context[d].text,255,&context[d].key,0);
                if (context[d].text[0]==27) { // failsafe quit
                        kill = 1;
                } else {
                cc->t = 1;
                cc->txt = context[d].key;
                }
        }
	if (kill){
	Clean(d);
	exit(0);
	}
}

unsigned int WW(char d) {
XWindowAttributes attr;
XGetWindowAttributes(context[d].dis, context[d].win, &attr);
return attr.width;
}

unsigned int WH(char d) {
XWindowAttributes attr;
XGetWindowAttributes(context[d].dis, context[d].win, &attr);
return attr.height;
}

int Accommodating() {
int ret;
Display *dis = XOpenDisplay((char *)0);
Window *ignore;
Window nore[2];
XQueryTree(dis, DefaultRootWindow(dis), &nore[0], &nore[1], &ignore, &ret);
XCloseDisplay(dis);
return ret + 1;
}

void Obstruct(struct cache *boxes, char d) {
Window *list;
int x[4], y[4], mx[4], my[4];
Window ignore[2];
XWindowAttributes attr[2];
int i, k, use_n, num;
XGetWindowAttributes(context[d].dis, context[d].win, &attr[0]);
mx[0] = attr[0].x;
mx[1] = attr[0].x + attr[1].width;
mx[2] = attr[0].x + attr[1].width;
mx[3] = attr[0].x;
my[0] = attr[0].y;
my[1] = attr[0].y;
my[2] = attr[0].y + attr[0].height;
my[3] = attr[0].y + attr[0].height;
XQueryTree(context[d].dis, DefaultRootWindow(context[d].dis), &ignore[0], &ignore[1], &list, &num);

int use[num];
use_n = 0;
i = 0;
	if (context[d].win==DefaultRootWindow(context[d].dis)) {
	i = 0;
	} else {
		while (list[i]!=context[d].win) {
		i++;
		}
	}
	while (i < num) {
	XGetWindowAttributes(context[d].dis, list[i], &attr[1]);
		if (attr[1].map_state==2) {
		use[use_n++] = i;
		}
	i++;
	}
	if (!use_n) {
	boxes[0].t = 0;
	return;
	}
i = 0;
	while (i < use_n) {
	XGetWindowAttributes(context[d].dis, list[use[i]], &attr[1]);
	x[0] = attr[1].x;
	x[1] = attr[1].x + attr[1].width;
	x[2] = attr[1].x + attr[1].width;
	x[3] = attr[1].x;
	y[0] = attr[1].y;
	y[1] = attr[1].y;
	y[2] = attr[1].y + attr[1].height;
	y[3] = attr[1].y + attr[1].height;
		while (!(x[k] > mx[0]&&y[k] > my[0]&&x[k] < mx[3]&&y[k] < my[3])) { 
			// check if each point is inside (x,y)(x+width,y)(x+width,y+height)(x,y+height)
		k++;
		}
		if (k==4) {
		use_n--;
		k = i;
			while (k < use_n) {
			use[k] = use[k+1];
			}
		}
	i++;
	}
	if (!use_n) {
	boxes[0].t = 0;
	return;
	}

i = 0;
	while (i < use_n) { // redo with the final knowledge of how many obstructing windows there are
	XGetWindowAttributes(context[d].dis, list[use[i]], &attr[1]);
	x[0] = attr[1].x;
	x[1] = attr[1].x + attr[1].width;
	x[2] = attr[1].x + attr[1].width;
	x[3] = attr[1].x;
	y[0] = attr[1].y;
	y[1] = attr[1].y;
	y[2] = attr[1].y + attr[1].height;
	y[3] = attr[1].y + attr[1].height;
		if (x[0] - 1 < mx[0]&&y[0] - 1 < my[0]&&x[2] + 1 > mx[2]&&y[2] + 1 > my[2]) { // outside window
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = 0;
		boxes[i].y = 0;
		boxes[i].w = mx[2] - mx[0];
		boxes[i].h = my[3] - my[0];
		}

		if (x[0] > mx[0]&&y[0] > my[0]&&x[2] < mx[2]&&y[2] < my[2]) { // inside window
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = x[0] - mx[0];
		boxes[i].y = y[0] - my[0];
		boxes[i].w = x[2] - x[0];
		boxes[i].h = y[2] - y[0];
		}

		if (x[0] < mx[0]&&y[0] < my[0]&&x[2] > mx[0]&&y[2] > my[0]&&
		    x[2] < mx[2]&&y[2] < my[2]) { // top left corner
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = 0;
		boxes[i].y = 0;
		boxes[i].w = x[2] - mx[2];
		boxes[i].h = y[2] - my[2];
		}

		if (x[1] > mx[1]&&y[1] < my[1]&&x[3] > mx[0]&&y[3] > my[0]&&
                    x[3] < mx[2]&&y[3] < my[2]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = x[0] - mx[0];
		boxes[i].y = 0;
		boxes[i].w = mx[0] - x[3];
		boxes[i].h = y[3] - my[0];
		}

		if (x[2] > mx[2]&&y[2] > my[2]&&x[0] > mx[0]&&y[0] > my[0]&&
                    x[0] < mx[2]&&y[0] < my[2]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = x[0] - mx[0];
		boxes[i].y = y[0] - my[0];
		boxes[i].w = mx[2] - x[0];
		boxes[i].h = my[2] - y[0];
		}

		if (x[3] < mx[3]&&y[3] > my[3]&&x[1] > mx[0]&&y[1] > my[0]&&
                    x[1] < mx[2]&&y[1] < my[2]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = 0;
		boxes[i].y = x[0] - mx[0];
		boxes[i].w = x[2] - mx[0];
		boxes[i].h = my[2] - y[0];
		}

		if (x[0] > mx[0]&&x[0] < mx[1]&&y[0] < my[0]&&x[2] > mx[0]&&
		    y[2] > my[0]&&x[2] < mx[2]&&y[2] < my[2]) { // partial segments
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = x[0] - mx[0];
		boxes[i].y = 0;
		boxes[i].w = x[2] - x[0];
		boxes[i].h = y[2] - my[0];
		}

		if (x[0] > mx[0]&&y[0] > my[0]&&x[0] < mx[2]&&y[0] < my[2]&&
		    x[2] > mx[1]&&y[2] > my[1]&&y[2] < my[2]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = x[0] - mx[0];
		boxes[i].y = y[0] - my[0];
		boxes[i].w = mx[2] - x[0];
		boxes[i].h = y[2] - y[0];
		}

		if (x[0] > mx[0]&&y[0] > my[0]&&x[0] < mx[2]&&y[0] < my[2]&&
		    x[2] > mx[3]&&y[2] > my[3]&&x[2] < mx[2]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = x[0] - mx[0];
		boxes[i].y = y[0] - my[0];
		boxes[i].w = x[2] - x[0];
		boxes[i].h = my[2] - y[0];
		}

		if (x[2] > mx[0]&&y[2] > my[0]&&x[2] < mx[2]&&y[2] < my[2]&&
		    x[0] < mx[0]&&y[0] > my[0]&&y[0] < my[2]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = 0;
		boxes[i].y = y[1] - my[0];
		boxes[i].w = x[1] - mx[0];
		boxes[i].h = y[2] - y[0];
		}

		if (x[0] > mx[0]&&y[0] < my[0]&&x[2] > mx[0]&&y[2] > my[2]&&x[2] < mx[2]) { // vert
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = x[0] - mx[0];
		boxes[i].y = 0;
		boxes[i].w = x[2] - x[0];
		boxes[i].h = my[2] - my[0];
		}
		
		if (x[0] < mx[0]&&y[0] > my[0]&&y[2] > my[0]&&x[2] > mx[2]&&y[2] < my[2]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = 0;
		boxes[i].y = y[0] - my[0];
		boxes[i].w = mx[2] - mx[0];
		boxes[i].h = y[2] - y[0];
		}

		if (x[0] < mx[0]&&y[0] < my[0]&&x[2] > mx[2]&&y[2] > my[0]&&y[2] < my[2]) { // segment annihilators
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = 0;
		boxes[i].y = 0;
		boxes[i].w = mx[2] - mx[0];
		boxes[i].h = y[2] - my[0];
		}

		if (x[1] > mx[1]&&y[1] < my[1]&&y[3] > my[3]&&x[3] > mx[3]&&x[3] < mx[1]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = x[0] - mx[0];
		boxes[i].y = 0;
		boxes[i].w = mx[2] - x[0];
		boxes[i].h = my[2] - my[0];
		}

		if (x[2] > mx[2]&&y[2] > my[2]&&x[0] < mx[0]&&y[0] > my[0]&&y[0] < my[2]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = 0;
		boxes[i].y = my[2] - my[0];
		boxes[i].w = mx[2] - mx[0];
		boxes[i].h = my[2] - y[0];
		}

		if (x[3] < mx[3]&&y[3] > my[3]&&y[1] < my[1]&&x[1] < mx[1]&&x[1] > mx[3]) {
		boxes[i].t = use_n;
		boxes[i].b = 0;
		boxes[i].txt = 0;
		boxes[i].x = 0;
		boxes[i].y = 0;
		boxes[i].w = x[2] - mx[0];
		boxes[i].h = my[2] - my[0];
		}

	i++;
	}
return;
}

void Latch (int list_position, char d) {

}

void Force (struct cache options, char d) {

}

int Pend(char d){
return XPending(context[d].dis);
}

void XI_(void){
XInitThreads();
}

void XX(char op) {
exit(op);
}
