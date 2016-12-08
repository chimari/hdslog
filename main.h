#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif  

#include<gtk/gtk.h>

#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>
#include<pwd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<errno.h>
#include<fcntl.h>

#include<unistd.h>
#include<dirent.h>
#include<string.h>

#include <fitsio.h>
//#include "/opt/share/cfitsio/fitsio.h"

//#define SND_CMD "cat /opt/share/hds/kakunin.au > /dev/audio"
#define SND_CMD "/usr/bin/audioplay /opt/share/hds/au/%s"

#define DELTA_CROSS (+130)

#define RANDOMIZE() srand(time(NULL)+getpid())
#define RANDOM(x)  (rand()%(x))

#define READ_INTERVAL 5*1000

#define MAX_FRAME 1000

#define DEF_MAIL "tajitsu@naoj.org"
#define DEF_FROM "HDS Administrator <tajitsu@naoj.org>"
#define MAIL_COMMAND "mutt"

// Setup
enum{ StdUb, StdUa, StdBa, StdBc, StdYa, StdI2b, StdYd, StdYb, StdYc, StdI2a, StdRa, StdRb, StdNIRc, StdNIRb, StdNIRa, StdHa} StdSetup;

// FileHead
enum{ FILE_HDSA, FILE_hds} FileHead;


// Color for GUI
GdkColor red   = {0, 0xffff, 0x0000, 0x0000};
GdkColor black = {0, 0x0000, 0x0000, 0x0000};


typedef struct _SetupEntry SetupEntry;
struct _SetupEntry{
  gchar *initial;
  gchar *cross;
  gfloat   cross_scan;
};


typedef struct _NOTEpara NOTEpara;
struct _NOTEpara{
  gchar *txt;
  time_t time;
  gboolean auto_fl;
};


typedef struct _FRAMEpara FRAMEpara;
struct _FRAMEpara{
  gchar *id;
  glong idnum;
  gchar *name;
  
  guint exp;
  gint  repeat;

  gchar *hst;

  gfloat secz;

  gchar *fil1;
  gchar *fil2;

  gfloat slt_wid;
  gfloat slt_len;

  gfloat crotan;
  gchar *crossd;

  glong bin1;
  glong bin2;

  gint  camz;
  gchar *i2;

  NOTEpara note;

  gchar *setup;

  gfloat slt_pa;
  gchar *adc;
  gchar *imr;

  gchar *is;

  GtkWidget *w_note;
};

typedef struct _typHLOG typHLOG;
struct _typHLOG{
  GtkWidget *w_top;
  GtkWidget *w_box;
  GtkWidget *scrwin;
  GtkWidget *top_table;
  GtkWidget *e_next;
  GtkWidget *e_note;
  GtkWidget *b_refresh;
  GtkWidget *w_status;

  gchar *data_dir;

  gchar *mail;

  guint timer;

  gint num;
  gint num_old;

  gint lock_fp;
  gboolean lock_flag;

  gboolean scr_flag;

  guint file_head;
  
  gchar *observer;
  gchar *prop;

  gchar *next_note;

  gint buf_year;
  gint buf_month;
  gint buf_day;
  gint fr_year;
  gint fr_month;
  gint fr_day;

  time_t fr_time;
  time_t seek_time;
  time_t to_time;

  gboolean i2_tmpfl;
  gboolean i2_flag;
  gboolean is_tmpfl;
  gboolean is_flag;
  gboolean camz_tmpfl;
  gboolean camz_flag;
  gboolean imr_tmpfl;
  gboolean imr_flag;
  gboolean adc_tmpfl;
  gboolean adc_flag;

  FRAMEpara frame[MAX_FRAME];

  gint d_cross;
};



