#include <iostream>
#include <netdb.h>
#include <string>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/Xutil.h>

#define ACPT_LIM 16

typedef struct sockaddr    saddr;
typedef struct sockaddr_in saddr_in;
typedef socklen_t slen;

extern int   open_display(Display**, Window*);
extern int      send_init(int*, saddr_in*, socklen_t*, int);
extern int get_screen_img(Display*, XImage**);
extern int      sids_slct(int, fd_set*, fd_set*);
extern int       new_acpt(int, int*, saddr_in*, slen*, int);
extern int       send_img(int*, int, fd_set*, fd_set*, XImage*);
extern int       show_img(Display*, Window*, XImage*);
extern int       send_fin(int, int*, int);
extern int  close_display(Display*, XImage*);

using namespace std;
void echo(string msg, int err){
  if(err){
    cerr << msg << endl;
  }else{
    cout << msg << endl;
  }
}

int main(int argc, char *argv[]){
  int i;
  int run_server = 1;
  int sid_lstn   = -1;
  int sid_acpt[ACPT_LIM];
  int max_sid    = 0;
  int slct_stt, acpt_stt;
  Display *disp;
  Window view;
  XImage *img;
  saddr_in saddr_srvr, saddr_clnt[ACPT_LIM];
  slen saddr_srvr_size, saddr_clnt_size[ACPT_LIM];
  fd_set sid_slct, sid_read;
  if(open_display(&disp, &view)){
    run_server = 0;
  }
  if(argc == 2){
    if(send_init(&sid_lstn, &saddr_srvr, &saddr_srvr_size, stoi(argv[1]))){
      run_server = 0;
    }else{
      max_sid = sid_lstn + 1;
      FD_ZERO(&sid_slct);
      FD_SET(sid_lstn, &sid_slct);
      for(i = 0; i < ACPT_LIM; i++){
        sid_acpt[i] = -1;
      }
    }
  }
  while(run_server){
    if(get_screen_img(disp, &img)){
      continue;
    }
    show_img(disp, &view, img);
    if(sid_lstn < 0){
      XDestroyImage(img);
      continue;
    }
    if(sids_slct(max_sid, &sid_read, &sid_slct)){
      XDestroyImage(img);
      continue;
    }
    if(FD_ISSET(sid_lstn, &sid_read)){
      acpt_stt = new_acpt(sid_lstn, sid_acpt, saddr_clnt, saddr_clnt_size, ACPT_LIM);
      if(acpt_stt >= 0){
        FD_SET(sid_acpt[acpt_stt], &sid_slct);
        echo("[INFO] new connection", 0);
      }
      if(max_sid < sid_acpt[acpt_stt] + 1){
        max_sid = sid_acpt[acpt_stt] + 1;
      } 
    }
    if(send_img(sid_acpt, ACPT_LIM, &sid_slct, &sid_read, img)){
      for(i = 0; i < ACPT_LIM; i++){
        if(sid_acpt[i] > 0){
          close(sid_acpt[i]);
        }
      }
      run_server = 0;
    }
    XDestroyImage(img);
  }
  if(send_fin(sid_lstn, sid_acpt, ACPT_LIM)){
    return 1;
  }
  if(close_display(disp, img)){
    return 1;
  }
  return 0;
}
