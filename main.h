#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif  

#include<gtk/gtk.h>
#include <gio/gio.h>

#include <unistd.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
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
#include <math.h>

#include <sys/socket.h>
#include <netdb.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif


#include "gui.h"
#include "gtkut.h"
#include "tree.h"
#include "mltree.h"
#include "mail.h"
#include "hdsql.h"

#define HDSLOG_DIR "Log"

//#include "/opt/share/cfitsio/fitsio.h"

//#define SND_CMD "cat /opt/share/hds/kakunin.au > /dev/audio"
#define SND_CMD "/usr/bin/audioplay /opt/share/hds/au/%s"

#define XGTERM_COM "/usr/local/bin/xgterm -sb -sl 2000 -title %s -e \"cd ~;/usr/local/bin/cl\" &"

#define HTTP_CAMZ_HOST "hds.skr.jp"
#define HTTP_CAMZ_PATH  "/CamZ"
#define HTTP_CAMZ_FILE "hdslog_camz.txt"
#define HTTP_DLSZ_FILE   "hdslog_http_dlsz.txt"

#define REMOTE_HOST "hds.skr.jp"
#define REMOTE_USER "rosegray"
#define REMOTE_DIR "/home/rosegray/www/hds/Data/"


#define FRAME_QL_RED_LABEL  "<span color=\"#FF0000\"><b>Red</b></span> <span size=\"smaller\">(= odd frame ID)</span>"
#define FRAME_QL_BLUE_LABEL "<span color=\"#0000FF\"><b>Blue</b></span> <span size=\"smaller\">(= even frame ID)</span>"

enum{CAL_AP, CAL_FLAT, CAL_THAR, NUM_CAL};

enum{FLAT_OW_NONE, FLAT_OW_SKIP, FLAT_OW_GO,  FLAT_OW_ABORT};

enum{OPEN_AP, OPEN_FLAT, OPEN_THAR, 
     REF1_AP, REF2_AP, 
     REF1_THAR, REF2_THAR, OPEN_LOG, NUM_OPEN};

#ifndef G_DIR_SEPARATOR_S
#define G_DIR_SEPARATOR_S "/"
#endif


#ifdef SIGRTMIN
#define SIGHTTPDL SIGRTMIN+1
#else
#define SIGHTTPDL SIGUSR2
#endif

#define POPUP_TIMEOUT 2

#define HDSLOG_HTTP_ERROR_GETHOST  -1
#define HDSLOG_HTTP_ERROR_SOCKET   -2
#define HDSLOG_HTTP_ERROR_CONNECT  -3
#define HDSLOG_HTTP_ERROR_TEMPFILE -4
#define HDSLOG_HTTP_ERROR_SSL -5
#define HDSLOG_HTTP_ERROR_FORK -6

#define DEF_D_CROSS_R (+130)
#define DEF_D_CROSS_B (+130)
#define DEF_CAMZ_R (-325)
#define DEF_CAMZ_B (-350)
#define DEF_ECHELLE0 (+880)
#define ALLOWED_DELTA_CROSS 10

#define MAX_MAIL 1000

#define XDOTMP "/tmp/hdslog_winid_%s.txt"
#define UPARMTMP "/tmp/hdslog_uparm_%s.tmp"

#define RANDOMIZE() srand(time(NULL)+getpid())
#define RANDOM(x)  (rand()%(x))

#define CHECK_INTERVAL 1000
#define READ_INTERVAL 5*1000

#define MAX_FRAME 1000

#define DEF_MAIL "tajitsu@naoj.org"
#define DEF_FROM "HDS Administrator <tajitsu@naoj.org>"
#define MAIL_COMMAND "mutt"
#define MUTT_FILE ".muttrc"
#define MSMTP_FILE ".msmtprc"

static gchar *muttrc_str[]={
  "set charset=\"utf-8\"",
  "set sendmail=\"/usr/bin/msmtp\"",
  "set realname=\"Subaru HDS Observation Log\"",
  "set from=\"" MAIL_ADDRESS "\"",
  NULL
};

static gchar *msmtprc_str[]={
  "# Set default values for all following accounts.",
  "defaults",
  "auth           on",
  "tls            on",
  "tls_trust_file /etc/ssl/certs/ca-bundle.crt",
  "logfile        ~/.msmtp.log",
  " ",
  "# Gmail",
  "account        gmail",
  "host           smtp.gmail.com",
  "port           587",
  "from           " MAIL_ADDRESS,
  "user           " MAIL_ID,
  "password       " MAIL_PASS,
  " ",
  "# Set a default account",
  "account default : gmail",
  NULL
};


// Setup
enum{ StdUb, StdUa, StdBa, StdBc, StdYa, StdI2b, StdYd, StdYb, StdYc, StdI2a, StdRa, StdRb, StdNIRc, StdNIRb, StdNIRa, StdHa} StdSetup;

// FileHead
enum{ FILE_HDSA, FILE_hds} FileHead;

// Color
enum{ COLOR_R, COLOR_B} ChipColor;

// Scroll
enum{SCR_AUTO, SCR_SMART, SCR_NONE, NUM_SCR}  ScrMode;
static gchar *scr_name[]={
  "Auto Scroll",
  "Smart Scroll",
  "No Scroll",
  NULL
};

// IRAF
#define NUM_SET 5

static gchar *hdsql_red[]={
  "hdsql",
  "hdsql2",
  "hdsql4",
  "hdsql6",
  "hdsql8",
  NULL
};

static gchar *hdsql_blue[]={
  "hdsql1",
  "hdsql3",
  "hdsql5",
  "hdsql7",
  "hdsql9",
  NULL
};

#define FLAT_TMP1 "/tmp/hdslog_flat_%s.txt"
#define FLAT_TMP2 "/tmp/hdslog_flat_%s_omlx.txt"

enum{  FLAT_EX_NO,
       FLAT_EX_1,
       FLAT_EX_SC,
       FLAT_EX_SCNM,
	 FLAT_EX_SCFL,
	 NUM_FLAT_EX};

static gchar *flat_ex[]={
  "----",
  "ave",
  "sc",
  "sc.nm",
  "sc.fl",
  NULL
};
       


// Color for GUI
#ifdef USE_GTK3
static GdkRGBA color_comment = {0.87, 0.00, 0.00, 1};
static GdkRGBA color_comp  =   {0.48, 0.09, 0.84, 1};
static GdkRGBA color_flat  =   {0.89, 0.36, 0.00, 1};
static GdkRGBA color_bias  =   {0.25, 0.25, 0.25, 1};
static GdkRGBA color_focus =   {0.53, 0.27, 0.00, 1};
static GdkRGBA color_calib =   {0.00, 0.53, 0.00, 1};
static GdkRGBA color_black =   {0.00, 0.00, 0.00, 1};
static GdkRGBA color_red   =   {1.00, 0.00, 0.00, 1};
static GdkRGBA color_blue =    {0.00, 0.00, 1.00, 1};
static GdkRGBA color_white =   {1.00, 1.00, 1.00, 1};
static GdkRGBA color_gray1 =   {0.40, 0.40, 0.40, 1};
static GdkRGBA color_gray2 =   {0.80, 0.80, 0.80, 1};
static GdkRGBA color_pink =    {1.00, 0.40, 0.40, 1};
static GdkRGBA color_pink2 =   {1.00, 0.80, 0.80, 1};
static GdkRGBA color_pale =    {0.40, 0.40, 1.00, 1};
static GdkRGBA color_pale2 =   {0.80, 0.80, 1.00, 1};
static GdkRGBA color_pale3 =   {0.90, 0.90, 1.00, 1};
static GdkRGBA color_yellow3 = {1.00, 1.00, 0.90, 1};
static GdkRGBA color_orange =  {1.00, 0.80, 0.40, 1};
static GdkRGBA color_orange2 = {1.00, 1.00, 0.80, 1};
static GdkRGBA color_orange3 = {0.95, 0.45, 0.02, 1};
static GdkRGBA color_green  =  {0.40, 0.80, 0.80, 1};
static GdkRGBA color_green2 =  {0.80, 1.00, 0.80, 1};
static GdkRGBA color_purple2 = {1.00, 0.80, 1.00, 1};
static GdkRGBA color_com1 =    {0.00, 0.53, 0.00, 1};
static GdkRGBA color_com2 =    {0.73, 0.53, 0.00, 1};
static GdkRGBA color_com3 =    {0.87, 0.00, 0.00, 1};
static GdkRGBA color_lblue =   {0.80, 0.80, 1.00, 1};
static GdkRGBA color_lgreen =  {0.80, 1.00, 0.80, 1};
static GdkRGBA color_lorange=  {1.00, 0.90, 0.70, 1};
static GdkRGBA color_lred   =  {1.00, 0.80, 0.80, 1};
#else
static GdkColor color_comp =    {0, 0x7A00, 0x0000, 0xD500};
static GdkColor color_flat =    {0, 0xE300, 0x5C00, 0x0000};
static GdkColor color_bias =    {0, 0x8000, 0x8000, 0x8000};
static GdkColor color_comment = {0, 0xDDDD, 0x0000, 0x0000};
static GdkColor color_focus = {0, 0x8888, 0x4444, 0x0000};
static GdkColor color_calib = {0, 0x0000, 0x8888, 0x0000};
static GdkColor color_black = {0, 0, 0, 0};
static GdkColor color_red   = {0, 0xFFFF, 0, 0};
static GdkColor color_blue = {0, 0, 0, 0xFFFF};
static GdkColor color_white = {0, 0xFFFF, 0xFFFF, 0xFFFF};
static GdkColor color_gray1 = {0, 0x6666, 0x6666, 0x6666};
static GdkColor color_gray2 = {0, 0xBBBB, 0xBBBB, 0xBBBB};
static GdkColor color_pink = {0, 0xFFFF, 0x6666, 0x6666};
static GdkColor color_pink2 = {0, 0xFFFF, 0xCCCC, 0xCCCC};
static GdkColor color_pale = {0, 0x6666, 0x6666, 0xFFFF};
static GdkColor color_pale2 = {0, 0xCCCC, 0xCCCC, 0xFFFF};
static GdkColor color_pale3 = {0, 0xEEEE, 0xEEEE, 0xFFFF};
static GdkColor color_yellow3 = {0, 0xFFFF, 0xFFFF, 0xEEEE};
static GdkColor color_orange = {0, 0xFFFF, 0xCCCC, 0x6666};
static GdkColor color_orange2 = {0, 0xFFFF, 0xFFFF, 0xCCCC};
static GdkColor color_orange3 = {0, 0xFD00, 0x6A00, 0x0200};
static GdkColor color_green = {0, 0x6666, 0xCCCC, 0x6666};
static GdkColor color_green2 = {0, 0xCCCC, 0xFFFF, 0xCCCC};
static GdkColor color_purple2 = {0, 0xFFFF, 0xCCCC, 0xFFFF};
static GdkColor color_com1 = {0, 0x0000, 0x8888, 0x0000};
static GdkColor color_com2 = {0, 0xBBBB, 0x8888, 0x0000};
static GdkColor color_com3 = {0, 0xDDDD, 0x0000, 0x0000};
static GdkColor color_lblue = {0, 0xBBBB, 0xBBBB, 0xFFFF};
static GdkColor color_lgreen= {0, 0xBBBB, 0xFFFF, 0xBBBB};
static GdkColor color_lorange={0, 0xFFFF, 0xCCCC, 0xAAAA};
static GdkColor color_lred=   {0, 0xFFFF, 0xBBBB, 0xBBBB};
#endif


static const gchar* cal_month[]={"Jan",
				 "Feb",
				 "Mar",
				 "Apr",
				 "May",
				 "Jun",
				 "Jul",
				 "Aug",
				 "Sep",
				 "Oct",
				 "Nov",
				 "Dec"};

static const gchar* day_name[]={"Sun",
				"Mon",
				"Tue",
				"Wed",
				"Thu",
				"Fri",
				"Sat"};


typedef struct _SetupEntry SetupEntry;
struct _SetupEntry{
  gchar *initial;
  gchar *cross;
  gdouble   cross_scan;
};


typedef struct _NOTEpara NOTEpara;
struct _NOTEpara{
  gchar *txt;
  time_t time;
  gboolean auto_fl;
};


typedef struct _MAILpara MAILpara;
struct _MAILpara{
  gchar *address;
  gint  year;
  gint  month;
  gint  day;
};

typedef struct _FRAMEpara FRAMEpara;
struct _FRAMEpara{
  gchar *id;
  glong idnum;
  gchar *name;
  
  guint exp;
  gint  repeat;

  gchar *hst;

  gdouble secz;

  gchar *fil1;
  gchar *fil2;

  gdouble slt_wid;
  gdouble slt_len;

  gdouble crotan;
  gchar *crossd;

  gdouble erotan;

  glong bin1;
  glong bin2;

  gint  camz;
  gchar *i2;

  NOTEpara note;

  gchar *setup;

  gdouble slt_pa;
  gchar *adc;
  gchar *imr;

  gchar *is;

  GtkWidget *w_note;

  gboolean qlr;
  gboolean qlb;
};

typedef struct _QLParam QLParam;
struct _QLParam{
  gboolean sc_inte;
  gboolean sc_resi;
  gboolean sc_edit;
  gboolean sc_fitt;
  
  gboolean ap_inte;
  gboolean ap_resi;
  gboolean ap_edit;
  gint     ap_llim;
  gint     ap_ulim;

  gboolean is_plot;
  gint     is_stx;
  gint     is_edx;

  gint     sp_line;
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

  gchar *uname;
  gchar *xdotmp;
  gchar *uparmtmp;
  gchar *flattmp1;
  gchar *flattmp2;
  gchar *wdir;
  gchar *sdir;
  gchar *ddir;
  gchar *udir;

  gchar *spass;

  gchar *ref_frame;

  glong idnum0;
  gchar *prop0;

  gchar *data_dir;

  gchar *mail;

  guint timer;

  gint num;
  gint num_old;

  gint lock_fp;
  gboolean lock_flag;

  gint scr_flag;
  GtkWidget *scr_combo;

  guint file_head;
  gboolean upd_flag;
  
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

  gboolean ech_tmpfl;
  gboolean ech_flag;
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

  MAILpara ml[MAX_MAIL];

  gint camz_r;
  gint camz_b;
  gint d_cross_r;
  gint d_cross_b;
  gint echelle0;
  gchar *camz_date;

  GtkWidget *fr_e;

  GtkWidget *frame_tree;
  gint frame_tree_i;

  gchar *http_host;
  gchar *http_path;
  gchar *http_dlfile;
  glong http_dlsz;
  GtkWidget *pdialog;
  GtkWidget *pbar;
  gboolean http_ok;

  GtkWidget *address_entry;
  GtkWidget *smdialog;
  GtkWidget *mldialog;
  GtkWidget *mltree;
  gint mltree_i;
  gint ml_max;
  GtkWidget *mltree_search_label;
  gchar *mltree_search_text;
  gint mltree_search_imax;
  gint mltree_search_i;
  gint mltree_search_iaddr[MAX_MAIL];

  GThread        *scthread;
  GMainLoop      *scloop;
  gint scanning_timer;
  gboolean scanning_flag;

  GThread   *pthread;
  GCancellable   *pcancel;
  GMainLoop *ploop;
  gboolean pabort;

  gint iraf_col;
  gint iraf_hdsql_r;
  gint iraf_hdsql_b;
  //  gint iraf_order_r;
  //  gint iraf_order_b;
  gchar *file_write;
  gchar *file_wait;

  gint bin1_red[NUM_SET];
  gint bin1_blue[NUM_SET];
  gint bin2_red[NUM_SET];
  gint bin2_blue[NUM_SET];

  gboolean auto_red;
  gboolean auto_blue;
  GtkWidget *check_auto_red;
  GtkWidget *check_auto_blue;

  GtkWidget *frame_ql_red;
  GtkWidget *frame_ql_blue;
  gchar *setname_red[NUM_SET];
  gchar *setname_blue[NUM_SET];

  GtkWidget *check_ap_red;
  GtkWidget *check_ap_blue;
  gchar *ap_red[NUM_SET];
  gchar *ap_blue[NUM_SET];
  gboolean flag_ap_red[NUM_SET];
  gboolean flag_ap_blue[NUM_SET];

  GtkWidget *combo_flat_red;
  GtkWidget *combo_flat_blue;
  GtkWidget *button_flat_red;
  GtkWidget *button_flat_blue;
  gchar *flat_red[NUM_SET];
  gchar *flat_blue[NUM_SET];
  gint ex_flat_red[NUM_SET];
  gint ex_flat_blue[NUM_SET];

  GtkWidget *check_thar_red;
  GtkWidget *check_thar_blue;
  GtkWidget *button_thar_red;
  GtkWidget *button_thar_blue;
  gchar *thar_red[NUM_SET];
  gchar *thar_blue[NUM_SET];
  gboolean flag_thar_red[NUM_SET];
  gboolean flag_thar_blue[NUM_SET];

  GtkWidget *label_edit_ap;
  GtkWidget *label_edit_flat;
  GtkWidget *label_edit_thar;

  GtkWidget *entry_ap_id;
  GtkWidget *entry_thar_reid;
  GtkWidget *entry_thar_id;

  gboolean remote_flag;
  gchar *remote_host;
  gchar *remote_user;
  gchar *remote_pass;
  gchar *remote_dir;

  QLParam qp_r[NUM_SET];
  QLParam qp_b[NUM_SET];

  gint i_reduced;
};


pid_t http_pid;
gboolean flag_make_frame_tree;
gboolean Flag_tree_editing;


gchar* to_utf8();
gchar* to_locale();
void ReadLog();

void cc_get_entry ();
void cc_get_adj ();
void cc_get_toggle();
void cc_get_combo_box ();

gchar *fgets_new();

void WriteLog();

void popup_dl_camz_list();
gchar* get_setname_short();
gchar* get_setname_long();

int scp_write_cal();
void hdslog_OpenFile();
