#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <X11/Xlib.h>

#define HTTP_HEADER_LEN 100

using namespace std;
extern void echo(string, int);

typedef struct sockaddr    saddr;
typedef struct sockaddr_in saddr_in;
typedef socklen_t slen;
typedef struct timeval tval;

struct BMP_HEADER{
  char format[2];
  char filesize[4];
  char reserved1[2];
  char reserved2[2];
  char headersize[4];
  char infosize[4];
  char width[4];
  char height[4];
  char planes[2];
  char bit_p_px[2];
  char compresstype[4];
  char compresssize[4];
  char px_p_m_x[4];
  char px_p_m_y[4];
  char usecolors[4];
  char importantcolors[4];
};
typedef struct BMP_HEADER BMP_HEADER;

struct RGB{
  unsigned char b;
  unsigned char g;
  unsigned char r;
};
typedef struct RGB RGB;

int send_init(int *sid_lstn, saddr_in *saddr_srvr, slen *saddr_size, int port){
  if((*sid_lstn = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    echo("[ERROR] make socket error", 1);
    return 1;
  }
  memset((char *)saddr_srvr, 0, sizeof(*saddr_srvr));
  (*saddr_srvr).sin_family = AF_INET;
  (*saddr_srvr).sin_addr.s_addr = INADDR_ANY;
  (*saddr_srvr).sin_port = htons(port);
  *saddr_size = sizeof(*saddr_srvr);
  if(bind(*sid_lstn, (saddr *)saddr_srvr, *saddr_size) < 0){
    echo("[ERROR] bind failed", 1);
    close(*sid_lstn);
    return 1;
  }
  if(listen(*sid_lstn, 5) < 0){
    echo("[ERROR] listen failed", 1);
    close(*sid_lstn);
    return 1;
  }
  return 0;
}

int sids_slct(int max_sid, fd_set *sid_read, fd_set *sid_slct){
  int slct_stt;
  tval timeout = {0, 500};
  memcpy(sid_read, sid_slct, sizeof(*sid_slct));
  slct_stt = select(max_sid, sid_read, NULL, NULL, &timeout);
  if(slct_stt == 0){
    // echo("[INFO] select time out", 1);
    return 1;
  }
  if(slct_stt < 0){
    echo("[ERROR] select failed", 1);
    return 1;
  }
  return 0;
}

int new_acpt(int sid_lstn, int *sid_acpt, saddr_in *saddr_clnt, slen *saddr_clnt_size, int acpt_lim){
  int i;
  for(i = 0; i < acpt_lim; i++){
    if(sid_acpt[i] < 0) break;
  }
  if(i == acpt_lim){
    echo("[ERROR] free socket is none", 1);
    return -1;
  }
  sid_acpt[i] = accept(sid_lstn, (saddr *)&saddr_clnt[i], &saddr_clnt_size[i]);
  if(sid_acpt[i] < 0){
    echo("[ERROR] accept failed", 1);
    return -1;
  }
  return i;
}

int send_msg(int *sid_acpt, int acpt_lim, fd_set *sid_slct, fd_set *sid_read, char *msg, int len){
  int i, fdsc;
  char buf[6];
  for(i = 0; i < acpt_lim; i++){
    if(sid_acpt[i] < 0){
      continue;
    }
    if(FD_ISSET(sid_acpt[i], sid_read)){
      recv(sid_acpt[i], buf, 6, 0);
      if(!strncmp(buf, "stop", 4)){
        return 1;
      }else if(!strncmp(buf, "GET / ", 6)){
        fdsc = open("index.html", O_RDONLY);
        read(fdsc, msg, len);
        close(fdsc);
        send(sid_acpt[i], msg, strlen(msg), 0);
      }else{
        send(sid_acpt[i], msg, len, 0);
      }
      while(recv(sid_acpt[i], buf, 6, MSG_DONTWAIT) > 0);
      FD_CLR(sid_acpt[i], sid_slct);
      close(sid_acpt[i]);
      sid_acpt[i] = -1;
    }
  }
  return 0;
}

void make_http_header(char *http_h, int *http_h_len){
  snprintf(&http_h[0], 18, "HTTP/1.1 200 OK\r\n");
  snprintf(&http_h[strlen(http_h)], 26, "Content-Type: image/bmp\r\n");
  snprintf(&http_h[strlen(http_h)], 20, "Connection: close\r\n");
  snprintf(&http_h[strlen(http_h)], 22, "Accept-Ranges: none\r\n");
  snprintf(&http_h[strlen(http_h)], 3, "\r\n");
  *http_h_len = strlen(http_h);
}

void make_bmp_header(BMP_HEADER *bmp_h, int *bmp_h_len, XImage *img){
  memset(bmp_h, 0, *bmp_h_len = sizeof(BMP_HEADER));
  int filesize = *bmp_h_len + (img->width * img->height * 3);
  int headersize = *bmp_h_len;
  int width = img->width;
  int height = img->height;
  (*bmp_h).format[0] = 'B';
  (*bmp_h).format[1] = 'M';
  (*bmp_h).filesize[0] = filesize % 256; 
  (*bmp_h).filesize[1] = (filesize /= 256) % 256;
  (*bmp_h).filesize[2] = (filesize /= 256) % 256;
  (*bmp_h).filesize[3] = (filesize /= 256) % 256;
  (*bmp_h).headersize[0] = headersize % 256;
  (*bmp_h).headersize[1] = (headersize /= 256) % 256;
  (*bmp_h).headersize[2] = (headersize /= 256) % 256;
  (*bmp_h).headersize[3] = (headersize /= 256) % 256;
  (*bmp_h).infosize[0] = 40;
  (*bmp_h).width[0] = width % 256;
  (*bmp_h).width[1] = (width /= 256) % 256;
  (*bmp_h).width[2] = (width /= 256) % 256;
  (*bmp_h).width[3] = (width /= 256) % 256;
  (*bmp_h).height[0] = height % 256;
  (*bmp_h).height[1] = (height /= 256) % 256;
  (*bmp_h).height[2] = (height /= 256) % 256;
  (*bmp_h).height[3] = (height /= 256) % 256;
  (*bmp_h).planes[0] = 1;
  (*bmp_h).bit_p_px[0] = 24;
  (*bmp_h).px_p_m_x[0] = 1;
  (*bmp_h).px_p_m_y[0] = 1;
}

int send_img(int *sid_acpt, int acpt_lim, fd_set *sid_slct, fd_set *sid_read, XImage *img){
  int k, l, x, y, http_h_len, data_len, bmp_h_len, msg_len, send_stt;
  char http_h[HTTP_HEADER_LEN], *msg;
  BMP_HEADER bmp_h;
  RGB *data = (RGB *)malloc((data_len = sizeof(RGB) * img->width * img->height));
  make_http_header(http_h, &http_h_len);
  make_bmp_header(&bmp_h, &bmp_h_len, img);
  memset((char *)data, 0, data_len);
  for(y = 0; y < img->height; y++){
    for(x = 0; x < img->width; x++){
      k = (img->height - y - 1) * img->width + x;
      l = (y * img->width + x) * 4;
      data[k].b = img->data[l+0];
      data[k].g = img->data[l+1];
      data[k].r = img->data[l+2];
    }
  }
  msg = (char *)malloc(http_h_len + bmp_h_len + data_len);
  memcpy(&msg[msg_len = 0], http_h, http_h_len);
  memcpy(&msg[msg_len += http_h_len], &bmp_h, bmp_h_len);
  memcpy(&msg[msg_len += bmp_h_len], data, data_len);
  msg_len += data_len;
  send_stt = send_msg(sid_acpt, acpt_lim, sid_slct, sid_read, msg, msg_len);
  free(msg);
  free(data);
  if(send_stt){
    return 1;
  }
  return 0;
}

int send_fin(int sid_lstn, int *sid_acpt, int acpt_lim){
  int i;
  if(sid_lstn > 0){
    close(sid_lstn);
  }
  for(i = 0; i < acpt_lim; i++){
    if(sid_acpt[i] > 0){
      close(sid_acpt[i]);
    }
  }
  return 0;
} 
