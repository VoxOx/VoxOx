
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CHUNKSAMPLES 8000
#define CIRCBUFSAMPLES 16000

int audiolatency = 60;
int rate = 8000;
const char *ifilename;
const char *ofilename;
int ichans = 2;
int micchannel = 0;
int spkchannel = 1;
FILE *ifile, *ofile;


struct circbuf
{
	int  cb_rdx;
	int  cb_wrx;
	int  cb_cnt;
	int  cb_siz;
	char *cb_buf;
};

struct circbuf micbuf, spkbuf;


static void cb_put(struct circbuf *cb, char *data, int len);
static void cb_get(struct circbuf *cb, char **chunk1, int *chunk1len,  char **chunk2, int *chhunk2len, int len);
static void cb_zfill(struct circbuf *cb, int count);
static void cb_init(struct circbuf *cb, int size);
static void cb_clean(struct circbuf *cb);






static void process_args(int argc, char * argv[]);
static void open_files();
static int read_input();
static void read_samples(short *mic, short *spk);
static void write_samples(short mic, short spk, short cleansignal);
static void close_files();
static short do_aec(short mic, short spk);



int main(int argc, char * argv[])
{
  int count;

  process_args(argc, argv);

  open_files();


  while(0 <= (count = read_input()))
    {
      short mic, spk, tmp;

      read_samples(&mic, &spk);
      tmp = do_aec(mic, spk);

      write_samples(mic, spk, tmp);

    }

  close_files();
  


}



static void usage()
{
  printf("usage: testec [-rate 8000|16000] [-lat audiolatencymsecs] [-chans inputchannels] infile outfile\n");
}

static void process_args(int argc, char *argv[])
{
  int i;


  for( i = 1; i < argc; i++)
    {
      if (!strcmp(argv[i], "-rate"))
	{
	  rate = atoi(argv[++i]); 
	}
      else  if (!strcmp(argv[i], "-lat"))
	{
	  audiolatency = atoi(argv[++i]); 
	}
      else  if (!strcmp(argv[i], "-chans"))
	{
	  ichans =  atoi(argv[++i]); 
	  if (ichans <= 1)
	    {
	      usage();
	      exit(2);
	    }
	}
      else
	{
	  if (argc - i != 2)
	    {
	      usage();
	      exit(2);
	    }

	  ifilename = argv[i++];
	  ofilename = argv[i];
	  return;
	}
    }

  usage();
  exit(2);

}

static void 
cb_put(struct circbuf *cb, char *data, int len)
{
    int free = cb->cb_siz - cb->cb_cnt;
    int chunk1max, chunk2max;

    if (len > cb->cb_siz)
        len = cb->cb_siz;

    if (len > free)
    {
        /* 
            not enough free space in the buffer,
            we need to free it
        */
        int needmore = len - free;

        /* advance the read pointer over data we're going to override */
        cb->cb_cnt -= needmore;
        cb->cb_rdx += needmore;
        if (cb->cb_rdx >= cb->cb_siz)
            cb->cb_rdx -= cb->cb_siz;
    }

    chunk1max = cb->cb_siz - cb->cb_wrx;
    if (len < chunk1max)
        chunk1max = len;

    chunk2max = len - chunk1max;

    
    memcpy(cb->cb_buf+cb->cb_wrx, data, chunk1max);

    cb->cb_cnt += chunk1max;
    cb->cb_wrx += chunk1max;
    if (cb->cb_wrx == cb->cb_siz)
        cb->cb_wrx = 0;

    
    if (chunk2max <= 0)
        return;

    memcpy(cb->cb_buf+cb->cb_wrx, data+chunk1max, chunk2max);

    cb->cb_cnt += chunk2max;
    cb->cb_wrx += chunk2max;
    return;
}


static void 
cb_get(struct circbuf *cb, char **chunk1, int *chunk1len,  char **chunk2, int *chunk2len, int len)
{
    int chunk1max = cb->cb_siz - cb->cb_rdx;

    if (len > cb->cb_cnt)
        len = cb->cb_cnt;

    *chunk2 = 0;
    *chunk2len = 0;


    *chunk1 = cb->cb_buf + cb->cb_rdx;
    if (len <= chunk1max)
    {
        *chunk1len = len;
        cb->cb_cnt -= len;
        cb->cb_rdx += len;
        if (cb->cb_rdx == cb->cb_siz)
            cb->cb_rdx = 0;
        return;
    }
    
    *chunk1len = chunk1max;
    cb->cb_rdx = 0;
    cb->cb_cnt -= chunk1max;
    
    len -= chunk1max;


    *chunk2 = cb->cb_buf;
    *chunk2len = len;
    cb->cb_cnt -= len;
    cb->cb_rdx += len;
}	


static char zeroes[] =
  {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  };

void 
cb_zfill(struct circbuf *cb, int len)
{
  int x;
  const int mx = sizeof(zeroes);

  while(len > 0)
    {
      x = (len > mx) ? mx : len;

      cb_put(cb, zeroes, mx);
      len -= x;
    }
}


static void 
cb_init(struct circbuf *cb, int size)
{
    memset(cb, 0, sizeof(0));
    cb->cb_buf = (char *) calloc(1, size);
    cb->cb_siz = size;
}

static void 
cb_clean(struct circbuf *cb)
{
    free(cb->cb_buf);
}






void open_files()
{
  int latencysamples;

  ifile = fopen(ifilename, "rb");
  if (!ifile)
    {
      perror("Error opening input file\n");
      exit(3);
    }
  ofile = fopen(ofilename, "wb");
  if (!ofile)
    {
      perror("Error opening output file\n");
      exit(3);
    }

  cb_init(&micbuf, CIRCBUFSAMPLES*sizeof(short));
  cb_init(&spkbuf, CIRCBUFSAMPLES*sizeof(short));

  
  latencysamples = rate/1000*audiolatency;
  cb_zfill(&spkbuf, latencysamples*sizeof(short));

}

static void close_files()
{
  fclose(ifile);
  fclose(ofile);
}


static int read_input()
{
  short *buf;
  int nsamples;

  buf = (short *) alloca(CHUNKSAMPLES*ichans*sizeof(short));

  nsamples = fread(buf, ichans * sizeof(short), CHUNKSAMPLES, ifile);

  if (nsamples > 0)
    {
      int i;

      for( i = 0; i < nsamples; i++)
	{
	  cb_put(&micbuf, (char *) (&buf[i+micchannel]), sizeof(short));
	  cb_put(&spkbuf, (char *) (&buf[i+spkchannel]), sizeof(short));
	  i += ichans;
	}
      return nsamples;
    }

  return 0;

}


static void read_samples(short *mic, short *spk)
{
  int c1len, c2len;
  char *c2;

  cb_get(&micbuf, (char **) mic, &c1len, &c2, &c2len, sizeof(short));
  cb_get(&spkbuf, (char **) spk,  &c1len, &c2, &c2len, sizeof(short));

}


static void write_samples(short mic, short spk, short cleansig)
{
  short data[3] = { mic, spk, cleansig };

  fwrite(data, sizeof(data), 1, ofile);

}

static short do_aec(short mic, short spk)
{
  return 0;
}

