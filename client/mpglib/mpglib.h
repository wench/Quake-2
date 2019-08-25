#ifndef  MPGlib_H_INCLUDED
#define MPGlib_H_INCLUDED

#include "mpg123.h"

typedef struct buf_s {
        unsigned char *pnt;
	long size;
	long pos;
        struct buf_s *next;
        struct buf_s *prev;
} buf;

typedef struct  {
	buf *buf;
	long pos;
	frame *next;
	frame *prev;
} framebuf;

typedef struct  {
	buf *head,*tail;
	int bsize;
	int framesize;
        int fsizeold;
	 frame fr;
        unsigned char bsspace[2][MAXFRAMESIZE+512]; /* MAXFRAMESIZE */
	real hybrid_block[2][2][SBLIMIT*SSLIMIT];
	int hybrid_blc[2];
	unsigned long header;
	int bsnum;
	real synth_buffs[2][2][0x110];
        int  synth_bo;
} mpstr;

typedef int                 BOOL;

#define MP3_ERR -1
#define MP3_OK  0
#define MP3_NEED_MORE 1


BOOL InitMP3(mpstr *mp);
int decodeMP3(mpstr *mp,char *inmemory,int inmemsize,
     char *outmemory,int outmemsize,int *done);
void ExitMP3(mpstr *mp);

#endif