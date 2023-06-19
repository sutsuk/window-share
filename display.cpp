#include <string>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/Xutil.h>

#define WIN_WDT 1024
#define WIN_HGT 750

using namespace std;
extern void echo(string, int);

int get_screen_img(Display *disp, XImage **img){
  int      wdt, hgt;
  Screen  *scr;
  Drawable draw;
  if((scr = XDefaultScreenOfDisplay(disp)) == NULL){
    echo("[ERROR] Get Default Screen Failed", 1);
    return 1;
  }
  wdt = scr->width - scr->width % 4;
  hgt = scr->height - scr->height % 4;
  draw = XDefaultRootWindow(disp);
  *img = XGetImage(disp, draw, 0, 0, wdt, hgt, AllPlanes, ZPixmap);
  return 0;
}

int create_window(Display *disp, Window *view, int wdt, int hgt){
  unsigned long black, white;
  Window root;
  XSetWindowAttributes attrs;
  black = BlackPixel(disp, 0);
  white = WhitePixel(disp, 0);
  root  = RootWindow(disp, 0);
  *view = XCreateSimpleWindow(disp, root, 0, 0, wdt, hgt, 1, black, white);
  attrs.override_redirect = True;
  XChangeWindowAttributes(disp, *view, CWBackingStore, &attrs);
  XStoreName(disp, *view, "Window");
  XMapWindow(disp, *view);
  XSync(disp, False);
  return 0;
}

int show_img(Display *disp, Window *view, XImage *img){
  GC gc_view;
  gc_view = XCreateGC(disp, *view, 0, 0);
  XPutImage(disp, *view, gc_view, img, 0, 0, 0, 0, img->width, img->height);
  XFlush(disp);
  return 0;
}

int open_display(Display **disp, Window *view){
  if((*disp = XOpenDisplay(NULL)) == NULL){
    echo("[ERROR] XDisplay Connection Error", 1);
    return 1;
  }
  if(create_window(*disp, view, WIN_WDT, WIN_HGT)){
    return 1;
  }
  return 0;
}
  
int close_display(Display *disp, XImage *img){
  XCloseDisplay(disp);
  return 0;
}
