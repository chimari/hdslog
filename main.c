//    HDS LOG Editor
//   
//                                           2005.12.08  A.Tajitsu

#include"main.h"    // 設定ヘッダ
#include"version.h"
#include"configfile.h"

// #define DEBUG

gboolean flag_make_top=FALSE;
gboolean flagChildDialog=FALSE;
GtkWidget *frame_table;
guint entry_height=24;

void write_muttrc();
void write_msmtprc();

void ChildTerm();
static void cc_get_note ();

static void refresh_table ();
gboolean create_lock ();
static void remove_lock ();
static void wait_lock ();
static void save_note ();
static void load_note ();

void select_color();
void update_frame_tree();
int printfits();
void ext_play();
gint scan_command();
gint printdir();
void gui_init();
void splot_help();
void show_version();

gboolean check_scan ();

// Ya is temporary (using Yb setting)
const SetupEntry setups[] = {
  {"Ub",  "BLUE",  17100}, 
  {"Ua",  "BLUE",  17820}, 
  {"Ba",  "BLUE",  19260}, 
  {"Bc",  "BLUE",  19890}, 
  {"Ya",  "BLUE",  21960}, 
  {"I2b", "RED",   14040}, 
  {"Yd",  "RED",   15480}, 
  {"Yb",  "RED",   15730}, 
  {"Yc",  "RED",   16500}, 
  {"I2a", "RED",   18000}, 
  {"Ra",  "RED",   18455}, 
  {"Rb",  "RED",   19080}, 
  {"NIRc","RED",   21360}, 
  {"NIRb","RED",   22860}, 
  {"NIRa","RED",   25200}, 
  {"Ha",  "MIRROR",0}
};

void get_hst_day(gint *year, gint *mon, gint *mday){
  struct tm t, *lt;
  time_t timer;
   
  t.tm_year=*year-1900;
  t.tm_mon=*mon-1;
  t.tm_mday=*mday;
  t.tm_hour=0;
  t.tm_min=0;
  t.tm_sec=0;
  
  timer=mktime(&t);
  timer-=20*60*60;
  
  lt=gmtime(&timer);
  
  *year=lt->tm_year+1900;
  *mon =lt->tm_mon+1;
  *mday=lt->tm_mday;
}



gchar* to_utf8(gchar *input){
  gchar *ret;
  ret=g_locale_to_utf8(input,-1,NULL,NULL,NULL);
  if(!ret) ret=g_strdup(input);
  return(ret);
}

gchar* to_locale(gchar *input){
  gchar *ret;
#ifdef USE_WIN32
  ret=g_win32_locale_filename_from_utf8(input);
  //return(x_locale_from_utf8(input,-1,NULL,NULL,NULL,"SJIS"));
#else
  ret=g_locale_from_utf8(input,-1,NULL,NULL,NULL);
#endif
  if(!ret) ret=g_strdup(input);
  return(ret);
}

gboolean my_main_iteration(gboolean may_block){
  return(g_main_context_iteration(NULL, may_block));
}

void write_muttrc(){
  gchar *filename;
  FILE *fp;
  gint i=0;

  filename=g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,
		       MUTT_FILE, NULL);
  if(access(filename, F_OK)==0){
    g_free(filename);
    return;
  }

  fprintf(stderr," Creating MUTTRC file, \"%s\" .\n", filename);

  if((fp=fopen(filename,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", filename);
    exit(1);
  }
  
  while(muttrc_str[i]){
    fprintf(fp, "%s\n", muttrc_str[i]);
    i++;
  }

  fclose(fp);
  g_free(filename);
}

void write_msmtprc(){
  gchar *filename;
  FILE *fp;
  gint i=0;

  filename=g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,
		       MSMTP_FILE, NULL);
  if(access(filename, F_OK)==0){
    g_free(filename);
    return;
  }

  fprintf(stderr," Creating MSMTPRC file, \"%s\" .\n", filename);

  if((fp=fopen(filename,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", filename);
    exit(1);
  }
  
  while(msmtprc_str[i]){
    fprintf(fp, "%s\n", msmtprc_str[i]);
    i++;
  }

  fclose(fp);

  if((chmod(filename, (S_IRUSR | S_IWUSR)))!=0){
    fprintf(stderr," Cannot chmod MSMTPRC file, \"%s\" .\n", filename);
  }
  g_free(filename);
}

gchar *fgets_new(FILE *fp){
  gint c;
  gint i=0, j=0;
  gchar *dbuf=NULL;

  do{
    i=0;
    while(!feof(fp)){
      c=fgetc(fp);
      if((c==0x00)||(c==0x0a)||(c==0x0d)) break;
      i++;
    }
  }while((i==0)&&(!feof(fp)));
  if(feof(fp)){
    if(fseek(fp,(long)(-i+1),SEEK_CUR)!=0) return(NULL);
  }
  else{
    if(fseek(fp,(long)(-i-1),SEEK_CUR)!=0) return(NULL);
  }

  if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(i+2)))==NULL){
    fprintf(stderr, "!!! Memory allocation error in fgets_new().\n");
    fflush(stderr);
    return(NULL);
  }
  if(fread(dbuf,1, i, fp)){
    while( (c=fgetc(fp)) !=EOF){
      if((c==0x00)||(c==0x0a)||(c==0x0d))j++;
      else break;
    }
    if(c!=EOF){
      if(fseek(fp,-1L,SEEK_CUR)!=0) return(NULL);
    }
    dbuf[i]=0x00;
    //printf("%s\n",dbuf);
    return(dbuf);
  }
  else{
    return(NULL);
  }
  
}


void ChildTerm(int dummy){
  int s;

  wait(&s);
  signal(SIGCHLD, ChildTerm);
}

static void close_child_dialog(GtkWidget *w, GtkWidget *dialog)
{
  //gdk_pointer_ungrab(GDK_CURRENT_TIME);

  gtk_main_quit();
}


static void close_dialog(GtkWidget *w, GtkWidget *dialog)
{
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}


void cc_get_adj (GtkWidget *widget, gint * gdata)
{
  *gdata=(int)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
}

void cc_get_entry (GtkWidget *widget, gchar **gdata)
{
  g_free(*gdata);
  *gdata=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
}

void cc_get_dir (GtkWidget *widget, gchar **gdata)
{
  g_free(*gdata);
  *gdata=g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget)));
}

static void cc_get_note (GtkWidget *widget, gpointer gdata)
{
  NOTEpara *nt;

  nt=(NOTEpara *)gdata;

  g_free(nt->txt);
  nt->txt=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
  if(nt->auto_fl){
    nt->auto_fl=FALSE;
  }
  else{
    nt->time=time(NULL);
  }
}

void cc_get_toggle (GtkWidget * widget, gboolean * gdata)
{
  *gdata=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void cc_auto_red (GtkWidget * widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hl->auto_red=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  if(hl->auto_red){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_blue),
				 FALSE);
  }
}

void cc_auto_blue (GtkWidget * widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hl->auto_blue=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  if(hl->auto_blue){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_red),
				 FALSE);
  }
}

void cc_get_combo_box (GtkWidget *widget,  gint * gdata)
{
  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    *gdata=n;
  }
}


void cc_change_set_red (GtkWidget *widget,  gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hl->iraf_col=COLOR_R;
    hl->iraf_hdsql_r=n;
    
    set_cal_frame_red(hl);
    set_ql_frame_label(hl, hl->frame_ql_red, TRUE);
  }
}


void cc_change_set_blue (GtkWidget *widget,  gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  GtkTreeIter iter;
  if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
    gint n;
    GtkTreeModel *model;
    
    model=gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get (model, &iter, 1, &n, -1);

    hl->iraf_col=COLOR_B;
    hl->iraf_hdsql_b=n;
    
    set_cal_frame_blue(hl);
    set_ql_frame_label(hl, hl->frame_ql_blue, TRUE);
  }
}

static void refresh_table (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  struct tm tmpt2;
  gint i;
  
  hl=(typHLOG *)gdata;

  wait_lock(hl);

  gtk_widget_set_sensitive(hl->b_refresh,FALSE);

  hl->fr_year=hl->buf_year;
  hl->fr_month=hl->buf_month;
  hl->fr_day=hl->buf_day;

  tmpt2.tm_year=hl->fr_year-1900;
  tmpt2.tm_mon=hl->fr_month-1;
  tmpt2.tm_mday=hl->fr_day;
  tmpt2.tm_hour=9;
  tmpt2.tm_min=0;
  tmpt2.tm_sec=0;

  if(hl->prop) g_free(hl->prop);
  hl->prop=NULL;
  
  hl->fr_time=mktime(&tmpt2);
  hl->seek_time=hl->fr_time;
  hl->to_time=hl->fr_time+60*60*24;
    

  for(i=0;i<MAX_FRAME;i++){
    hl->frame[i].note.txt=NULL;
    hl->frame[i].note.time=0;
    hl->frame[i].note.auto_fl=FALSE;
  }
  
  hl->num=0;
  hl->num_old=0;

  hl->ech_flag=hl->ech_tmpfl;
  hl->i2_flag=hl->i2_tmpfl;
  hl->is_flag=hl->is_tmpfl;
  hl->camz_flag=hl->camz_tmpfl;
  hl->imr_flag=hl->imr_tmpfl;
  hl->adc_flag=hl->adc_tmpfl;

  hl->i_reduced=0;

  make_frame_tree(hl);
  
  gtk_widget_set_sensitive(hl->b_refresh,TRUE);

  if(!hl->upd_flag){
    start_scan_command((gpointer)hl);
  }
}

gboolean create_lock (typHLOG *hl){
  gchar lockfile[256];

  if(!hl->upd_flag) return;

  sprintf(lockfile,"%s%shdslog-%04d%02d%02d-%s.lock",
	  g_get_tmp_dir(),G_DIR_SEPARATOR_S, 
	  hl->fr_year,hl->fr_month,hl->fr_day,hl->uname);
    
  while(1){
    hl->lock_fp=open(lockfile, O_RDWR | O_CREAT | O_EXCL, 0444);
    if (hl->lock_fp == -1){
      printf ("%d - Lock already present   %s\n",getpid(), lockfile);
      gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			   "<span color=\"#FF0000\"><b>File Lock</b></span>");
      while(my_main_iteration(FALSE));
      sleep(1);
    }
    else{
      gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			   "Scanning...");
      while(my_main_iteration(FALSE));
      hl->lock_flag=TRUE;
      break;
    }
  }
  return(TRUE);
}

static void remove_lock (typHLOG *hl){
  gchar lockfile[256];
  sprintf(lockfile,"%s%shdslog-%04d%02d%02d-%s.lock",
	  g_get_tmp_dir(),G_DIR_SEPARATOR_S, 
	  hl->fr_year,hl->fr_month,hl->fr_day,hl->uname);

  close(hl->lock_fp);
  unlink(lockfile);
  hl->lock_flag=FALSE;
}

static void wait_lock (typHLOG *hl){
  while(hl->lock_flag){
    gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			 "<span color=\"#FF0000\"><b>File Lock</b></span>");
    //while(my_main_iteration(FALSE));
    sleep(1);
  }
}



static void save_note (typHLOG *hl)
{
  ConfigFile *cfgfile;
  gchar *filename;
  gint i;

  create_lock(hl);

  filename=g_strdup_printf("%s%s%s%s.hdslog-%04d%02d%02d",
			   g_get_home_dir(),G_DIR_SEPARATOR_S,
			   HDSLOG_DIR, G_DIR_SEPARATOR_S,
			   hl->fr_year,hl->fr_month,hl->fr_day);
  cfgfile = xmms_cfg_open_file(filename);
  if (!cfgfile)  cfgfile = xmms_cfg_new();

  for(i=0;i<hl->num;i++){
    if(hl->frame[i].note.txt){
      if(hl->frame[i].note.time>=hl->seek_time){
	xmms_cfg_write_string(cfgfile, hl->frame[i].id,
			      "note",hl->frame[i].note.txt);
	xmms_cfg_write_int(cfgfile, hl->frame[i].id,
			   "time",hl->frame[i].note.time);
      }
    }
  }

  xmms_cfg_write_file(cfgfile, filename);
  xmms_cfg_free(cfgfile);
  g_free(filename);

  remove_lock(hl);
}


static void load_note (typHLOG *hl,gboolean force_fl)
{
  ConfigFile *cfgfile;
  gchar filename[256];
  gchar *c_buf;
  gint i, i_buf;
  struct stat statbuf;

  if(!hl->upd_flag) return;

  sprintf(filename,"%s%s%s%s.hdslog-%04d%02d%02d",
	  g_get_home_dir(), G_DIR_SEPARATOR_S,
	  HDSLOG_DIR, G_DIR_SEPARATOR_S,
	  hl->fr_year,hl->fr_month,hl->fr_day);

  if (!force_fl){
    stat(filename,&statbuf);
    if((statbuf.st_ctime<hl->seek_time)) return;
  }

  create_lock(hl);

  cfgfile = xmms_cfg_open_file(filename);
  if (cfgfile) {

    for(i=0;i<hl->num;i++){
      if(xmms_cfg_read_int(cfgfile, hl->frame[i].id,
			   "time",&i_buf)){
	if(i_buf>hl->frame[i].note.time){
	  if(xmms_cfg_read_string(cfgfile, hl->frame[i].id,
				  "note",&c_buf)){
	    if(hl->frame[i].note.txt) g_free(hl->frame[i].note.txt);
	    hl->frame[i].note.txt=g_strdup(c_buf);
	    hl->frame[i].note.time=i_buf;
	    if( (hl->frame[i].note.txt) && (!force_fl)){
	      hl->frame[i].note.auto_fl=TRUE;
	      //gtk_entry_set_text(GTK_ENTRY(hl->frame[i].w_note),
	      //		 hl->frame[i].note.txt);
	      //printf("Writing %s  to  %s\n",hl->frame[i].note.txt,
 	      //     hl->frame[i].id);
	    }

	  }
	}
      }
    }

    xmms_cfg_free(cfgfile);
  }

  remove_lock(hl);
}


// Checking Stardard Setups
void select_color(FRAMEpara *frame, gint d_cross_b, gint d_cross_r){
  int i_set;
  gchar tmp[32];

  // Blue
  for(i_set=StdUb;i_set<=StdYa;i_set++){
    if(!strcmp(setups[i_set].cross,frame->crossd)){
      if( fabs((gdouble)((frame->crotan-d_cross_b)-setups[i_set].cross_scan)) 
	  < ALLOWED_DELTA_CROSS) {
	frame->setup=g_strdup(setups[i_set].initial);
	return;
      }
    }
  }
  // Red
  for(i_set=StdI2b;i_set<=StdNIRa;i_set++){
    if(!strcmp(setups[i_set].cross,frame->crossd)){
      if( fabs((gdouble)((frame->crotan-d_cross_r)-setups[i_set].cross_scan)) 
	  < ALLOWED_DELTA_CROSS) {
	frame->setup=g_strdup(setups[i_set].initial);
	return;
      }
    }
  }
  // Mirror
  if(!strcmp(setups[StdHa].cross,frame->crossd)){
    if( fabs((gdouble)((frame->crotan)-setups[StdHa].cross_scan)) 
	< ALLOWED_DELTA_CROSS) {
      frame->setup=g_strdup(setups[StdHa].initial);
      return;
    }
  }
  
  sprintf(tmp,"%5d_%s",(gint)frame->crotan,frame->crossd);
  frame->setup=g_strdup(tmp);
  return;
}


gchar* get_setname_short(typHLOG *hl, gint i_sel){
  gchar *ret;

  if(strlen(hl->frame[i_sel].setup)>4){
    ret=g_strdup_printf("NonStd%d_",
			(hl->iraf_col==COLOR_R) ? hl->iraf_hdsql_r+1 : hl->iraf_hdsql_b+1);
  }
  else{
    ret=g_strdup(hl->frame[i_sel].setup);
  }

  return(ret);
}

gchar* get_setname_long(typHLOG *hl, gint i_sel){
  gchar *ret;
  gchar *setup;

  setup=get_setname_short(hl, i_sel);

  ret=g_strdup_printf("%s%dx%d%s_%04d%02d%02d",
		      setup, 
		      hl->frame[i_sel].bin1,hl->frame[i_sel].bin2,
		      (hl->iraf_col==COLOR_R) ? "R" : "B",
		      hl->fr_year,hl->fr_month,hl->fr_day);
  g_free(setup);
  return(ret);
}



void make_top_table(typHLOG *hl){
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *combo;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *check;
  GtkWidget *button;
  int col=0;
  

  if(flag_make_top)  gtk_widget_destroy(hl->top_table);
  else flag_make_top=TRUE;


  hl->top_table = gtkut_table_new (1, 2, FALSE, 5, 5, 5);
  
  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(hl->top_table, hbox, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Current/Next");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HDSA",
		       1, FILE_HDSA, 2, TRUE, -1);
    if(hl->file_head==FILE_HDSA) iter_set=iter;
    
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "hds20",
		       1, FILE_hds, 2, TRUE, -1);
    if(hl->file_head==FILE_hds) iter_set=iter;
    
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_get_combo_box),
		       &hl->file_head);
  }


  // Next ID
  hl->e_next = gtk_entry_new ();
  gtk_editable_set_editable(GTK_EDITABLE(hl->e_next), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),hl->e_next,FALSE,FALSE,0);

  Flag_tree_editing=FALSE;
  // Note
  hl->e_note = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hl->e_note,FALSE,FALSE,0);
  gtk_entry_set_width_chars(GTK_ENTRY(hl->e_note),40);
  g_signal_connect (hl->e_note,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    &hl->next_note);
  
  label = gtk_label_new ("  Start from");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hl->idnum0,
					    0, 99999999,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    &hl->idnum0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i;
    gchar *tmp;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i=0;i<NUM_SCR;i++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, scr_name[i],
			 1, i, -1);
      if(hl->scr_flag==i) iter_set=iter;
    }
    
    hl->scr_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),hl->scr_combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hl->scr_combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hl->scr_combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hl->scr_combo),&iter_set);
    gtk_widget_show(hl->scr_combo);
    g_signal_connect (hl->scr_combo,"changed",G_CALLBACK(cc_get_combo_box),
		       &hl->scr_flag);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtkut_table_attach(hl->top_table, hbox, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label = gtkut_label_new ("Date <span size=\"smaller\">(HST)</span>");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  hl->fr_e = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(hbox),hl->fr_e,FALSE,FALSE,0);
  gtk_editable_set_editable(GTK_EDITABLE(hl->fr_e),FALSE);
  gtk_entry_set_width_chars(GTK_ENTRY(hl->fr_e),12);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"go-down");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_GO_DOWN);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(G_OBJECT(button),"pressed",
		   G_CALLBACK(popup_fr_calendar), 
		   (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Doublue-Click on calendar to select a new date");
#endif

  if(hl->upd_flag){ 
    set_fr_e_date(hl);
  }
  else{
    gtk_widget_set_sensitive(button,FALSE);
  }

  label = gtk_label_new ("  ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  check = gtk_check_button_new_with_label("Ech.");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->ech_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    &hl->ech_tmpfl);

  check = gtk_check_button_new_with_label("I2");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->i2_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    &hl->i2_tmpfl);
  
  check = gtk_check_button_new_with_label("IS");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->is_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    &hl->is_tmpfl);
  
  check = gtk_check_button_new_with_label("CamZ");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->camz_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    &hl->camz_tmpfl);
  
  check = gtk_check_button_new_with_label("ImR");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->imr_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    &hl->imr_tmpfl);
  
  check = gtk_check_button_new_with_label("ADC");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->adc_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    &hl->adc_tmpfl);
  

  label = gtkut_label_new ("  &#x394;Cross <span color=\"#0000FF\">B</span>");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hl->d_cross_b,
					    -1000, 1000,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    &hl->d_cross_b);

  label = gtkut_label_new ("  <span color=\"#FF0000\">R</span>");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hl->d_cross_r,
					    -1000, 1000,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    &hl->d_cross_r);

#ifdef USE_GTK3
  hl->b_refresh=gtkut_button_new_from_icon_name(NULL,"view-refresh");
#else
  hl->b_refresh=gtkut_button_new_from_stock(NULL,GTK_STOCK_REFRESH);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),hl->b_refresh,FALSE,FALSE,0);
  g_signal_connect(hl->b_refresh,"clicked", 
		   G_CALLBACK(refresh_table), 
		   (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(hl->b_refresh,
			      "Set Date & flags, then Remake table");
#endif


  hl->w_status = gtkut_label_new ("Starting...");
  while(my_main_iteration(FALSE));
  gtk_box_pack_start(GTK_BOX(hbox),hl->w_status,TRUE,TRUE,0);


  gtk_widget_show_all(hl->top_table);
}


void update_frame_tree(typHLOG *hl){
  int i, col=0;
  gchar *tmp;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));

#ifdef DEBUG
  fprintf(stderr, "Start Load\n");
#endif
  if((hl->num_old==0)&&(hl->num!=0)){
    // New load
    load_note(hl,TRUE);
  }
  else{
    // No change or Appended New frame
      load_note(hl,FALSE);
  }

#ifdef DEBUG
  fprintf(stderr, "End Load\n");
#endif
  
  // No change
  if(hl->num_old==hl->num){
    if(flag_make_frame_tree){
      if(!gtk_tree_model_get_iter_first(model, &iter)) return;
      
      for(i=0;i<hl->num_old;i++){
	if(hl->frame[i].note.auto_fl){
	  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			     COLUMN_FRAME_NOTE, hl->frame[i].note.txt, 
			     -1);
	  hl->frame[i].note.auto_fl=FALSE;
	}
	if(!gtk_tree_model_iter_next(model, &iter)) break;
      }
    }
    //return;
  }
  else{
    if(hl->next_note){
      if(hl->frame[hl->num-1].note.txt) g_free(hl->frame[hl->num-1].note.txt);
      hl->frame[hl->num-1].note.txt=g_strdup(hl->next_note);
      gtk_entry_set_text(GTK_ENTRY(hl->e_note),"");
      g_free(hl->next_note);
      hl->next_note=NULL;
    }

    tmp=g_strdup_printf("%d - %2d (?)",
			hl->frame[hl->num-1].idnum+2,
			hl->frame[hl->num-1].idnum+3-
			((int)((hl->frame[hl->num-1].idnum+3)/100))*100);
    gtk_entry_set_text(GTK_ENTRY(hl->e_next),tmp);
    g_free(tmp);

    for(i=hl->num_old;i<hl->num;i++){
      gtk_list_store_insert (GTK_LIST_STORE (model), &iter, i);
      frame_tree_update_item(hl, GTK_TREE_MODEL(model), iter, i);
    }  

    if(hl->num_old+1==hl->num){
      if((strcmp(hl->frame[hl->num-1].name,"FLAT")!=0)
	 && (strcmp(hl->frame[hl->num-1].name,"COMPARISON")!=0)
	 && (strcmp(hl->frame[hl->num-1].name,"BIAS")!=0)
	 && (strcmp(hl->frame[hl->num-1].name,"DARK")!=0)){
	if(hl->auto_red){
	  frame_tree_select_last(hl);
	  ql_obj_red(NULL, (gpointer)hl);
	}
	else if(hl->auto_blue){
	  frame_tree_select_last(hl);
	  ql_obj_blue(NULL, (gpointer)hl);
	}
      }
    }
  }

}


int npcmp(FRAMEpara *x, FRAMEpara *y){
  return(x->idnum > y->idnum ? 1 :
	 x->idnum < y->idnum ? -1 : 0);
}

int printfits(typHLOG *hl, char *inf){
  fitsfile *fptr; 
  int status=0;
  char obj_name[256];
  char observer[32];
  char prop[32];
  char caldate[32];
  char frame_id[32];
  char hst_str[32];
  char hst[6];
  char crossd[32];
  char filter01[32];
  char filter02[32];
  char i2[32];
  char imr[32];
  char adc[32];
  char *cp;
  long is, isslic;
  gdouble iswid;
  long det_id;
  gdouble exptime,slt_wid,slt_len,crotan,camz,slt_pa;
  static char last_frame_id[256];
  int ret=0;
  float f_buf;
  glong idnum_tmp;
  gboolean prop_ok;
  gchar *tmp;
  glong ltmp;
  gint t_year, t_mon, t_mday;
  


  fits_open_file(&fptr, inf, READONLY, &status);
  fits_read_key_lng(fptr, "DET-ID", &det_id, 0, &status);
  fits_read_key_str(fptr, "FRAMEID", frame_id, 0, &status);

  if(!strncmp(frame_id,"HDSA9999",8)) return;

  if((det_id==1)&&(strcmp(last_frame_id,frame_id)!=0)){  //Red
    idnum_tmp=atol(frame_id+5);

    if(hl->prop){
      fits_read_key_str(fptr, "PROP-ID", prop, 0, &status);
      if(strcmp(hl->prop,prop)==0){
	prop_ok=TRUE;
      }
      else{
	prop_ok=FALSE;
      }
    }
    else{
      prop_ok=TRUE;
    }

    if((idnum_tmp>=hl->idnum0)&&(prop_ok)){
      hl->frame[hl->num].id=g_strdup(frame_id);
      cp=frame_id+5;
      hl->frame[hl->num].idnum=atol(cp);
      
#ifdef DEBUG
      printf("%s",hl->frame[hl->num].id);
#endif
      if(hl->file_head==FILE_HDSA){
	tmp=g_strdup_printf("%s/result/H%d.fits",
			    hl->wdir, hl->frame[hl->num].idnum);
	if(access(tmp, F_OK)==0){
	  hl->frame[hl->num].qlr=TRUE;
	}
	else{
	  hl->frame[hl->num].qlr=FALSE;
	}
	g_free(tmp);

	tmp=g_strdup_printf("%s/result/H%d.fits",
			    hl->wdir, hl->frame[hl->num].idnum+1);
	if(access(tmp, F_OK)==0){
	  hl->frame[hl->num].qlb=TRUE;
	}
	else{
	  hl->frame[hl->num].qlb=FALSE;
	}
	g_free(tmp);
      }

      if(hl->num==0){
	fits_read_key_str(fptr, "OBSERVER", observer, 0, &status);
	hl->observer=g_strdup(observer);

	fits_read_key_str(fptr, "PROP-ID", prop, 0, &status);
	hl->prop=g_strdup(prop);

	if(!hl->upd_flag){
	  fits_read_key_str(fptr, "DATE-OBS", caldate, 0, &status);
	  t_year=atoi(strtok(caldate,"-"));
	  t_mon=atoi(strtok(NULL,"-"));
	  t_mday=atoi(strtok(NULL,"\0"));

	  get_hst_day(&t_year, &t_mon, &t_mday);
	  hl->fr_year=t_year;
	  hl->fr_month=t_mon;
	  hl->fr_day=t_mday;
	  hl->buf_year=hl->fr_year;
	  hl->buf_month=hl->fr_month;
	  hl->buf_day=hl->fr_day;
	  set_fr_e_date(hl);
	}
      }

      
      switch(hl->file_head){
      case FILE_HDSA:
	fits_read_key_str(fptr, "OBJECT", obj_name, 0, &status);
	break;
      case FILE_hds:
	fits_read_key_str(fptr, "DATA-TYP", obj_name, 0, &status);
	break;
      }
      hl->frame[hl->num].name=g_strdup(obj_name);
      
      fits_read_key_flt(fptr, "EXPTIME", &f_buf, 0, &status);
      hl->frame[hl->num].exp=(guint)f_buf;
      
      fits_read_key_str(fptr, "HST-STR", hst_str, 0, &status);
      strncpy(hst,hst_str,5);
      hst[5]='\0';
      hl->frame[hl->num].hst=g_strdup(hst);
      
      fits_read_key_flt(fptr, "SECZ", &f_buf, 0, &status);
      hl->frame[hl->num].secz=(gdouble)f_buf;
      
      fits_read_key_str(fptr, "FILTER01", filter01, 0, &status);
      hl->frame[hl->num].fil1=g_strdup(filter01);
      
      fits_read_key_str(fptr, "FILTER02", filter02, 0, &status);
      hl->frame[hl->num].fil2=g_strdup(filter02);
      
      fits_read_key_flt(fptr, "SLT-WID", &f_buf, 0, &status);
      hl->frame[hl->num].slt_wid=((gdouble)f_buf*2);
      
      fits_read_key_flt(fptr, "SLT-LEN", &f_buf, 0, &status);
      hl->frame[hl->num].slt_len=((gdouble)f_buf*2);
      
      fits_read_key_flt(fptr, "H_CROTAN", &f_buf, 0, &status);
      hl->frame[hl->num].crotan=((gdouble)f_buf*3600.);
      
      fits_read_key_str(fptr, "H_CROSSD", crossd, 0, &status);
      hl->frame[hl->num].crossd=g_strdup(crossd);
      
      // Standard or Non-Standard
      select_color(&hl->frame[hl->num], hl->d_cross_b, hl->d_cross_r);
      
      fits_read_key_flt(fptr, "H_EROTAN", &f_buf, 0, &status);
      hl->frame[hl->num].erotan=((gdouble)f_buf*3600.);
      
      fits_read_key_lng(fptr, "BIN-FCT1", &hl->frame[hl->num].bin1, 0, &status);
      fits_read_key_lng(fptr, "BIN-FCT2", &hl->frame[hl->num].bin2, 0, &status);

      fits_read_key_flt(fptr, "H_FOCUS", &f_buf, 0, &status);
      hl->frame[hl->num].camz=(gint)(f_buf*1000);

      fits_read_key_str(fptr, "H_I2POS", i2, 0, &status);
      hl->frame[hl->num].i2=g_strdup(i2);
      
      fits_read_key_str(fptr, "IMR-TYPE", imr, 0, &status);
      hl->frame[hl->num].imr=g_strdup(imr);
      
      if(fits_read_key_lng(fptr, "H_ISUNIT", &is, 0, &status)!=KEY_NO_EXIST){
	if(is==0){
	  hl->frame[hl->num].is=g_strdup("NONE   ");
	}
	else{
	  fits_read_key_flt(fptr, "H_ISWID", &f_buf, 0, &status);
	  iswid=(gdouble)f_buf;
	  fits_read_key_lng(fptr, "H_ISSLIC", &isslic, 0, &status);
	  hl->frame[hl->num].is=g_strdup_printf("%4.2lfx%d ",(gdouble)iswid*2.,(int)isslic);
	}
      }
      else{
	hl->frame[hl->num].is=g_strdup("UNKNOWN");
      }
      
      fits_read_key_flt(fptr, "SLT-PA", &f_buf, 0, &status);
      hl->frame[hl->num].slt_pa=(gdouble)f_buf;
      
      fits_read_key_str(fptr, "ADC-TYPE", adc, 0, &status);
      hl->frame[hl->num].adc=g_strdup(adc);
    
      
      //printf("%s %s %4.0fs (%s)\n",frame_id,hst,exptime, obj_name);
      strcpy(last_frame_id,frame_id);
      hl->num++;
      ret=1;
    }
  }

  qsort(hl->frame, hl->num, sizeof(FRAMEpara),
//	(int(*)(FRAMEpara*, FRAMEpara*))npcmp);
	(int(*)(const void*, const void*))npcmp);

  fits_close_file(fptr, &status);

  return(ret);
}

void ScpCAL(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  
  scp_write_cal(hl);
  gtk_main_quit();
}


void WriteLog(typHLOG *hl, FILE *fp){
  gint i;

  fprintf(fp,"HDS Observation LOG  %04d-%02d-%02d (HST)\n\n",
	  hl->fr_year,hl->fr_month,hl->fr_day);

  if(hl->observer){
    fprintf(fp," Observer: %s\n",hl->observer);
  }
  if(hl->prop){
    fprintf(fp," Proposal-ID: %s\n",hl->prop);
  }
  
  fprintf(fp,"\nNo.  Frame-ID        Object Name            HST  Exp.  secZ   Filter       Slit       Cross      Bin.");
  if(hl->camz_flag){
    fprintf(fp," CamZ");
  }
  if(hl->ech_flag){
    fprintf(fp," Echelle");
  }
  if(hl->i2_flag){
    fprintf(fp,"  I2    ");
  }
  if(hl->is_flag){
    fprintf(fp,"  IS    ");
  }
  if(hl->imr_flag){
    fprintf(fp," ImR  SlitPA  ");
  }
  if(hl->adc_flag){
    fprintf(fp," ADC ");
  }
  fprintf(fp, "  Note\n");
  
  for(i=0;i<hl->num;i++){
    
    fprintf(fp,"%4d. %-15s %-20s %5s %4ds %4.2lf %6s/%6s %4.2lf/%5.2lf %12s %1dx%1d",
	    i+1,
	    hl->frame[i].id,
	    hl->frame[i].name,
	    hl->frame[i].hst,
	    hl->frame[i].exp,
	    hl->frame[i].secz,
	    hl->frame[i].fil1,hl->frame[i].fil2,
	    hl->frame[i].slt_wid,hl->frame[i].slt_len,
	    hl->frame[i].setup,
	    hl->frame[i].bin1,hl->frame[i].bin2);
    if(hl->camz_flag){
      fprintf(fp," %4d",hl->frame[i].camz);
    }
    if(hl->ech_flag){
      fprintf(fp," %7d",(gint)hl->frame[i].erotan);
    }
    if(hl->i2_flag){
      fprintf(fp," %-7s",hl->frame[i].i2);
    }
    if(hl->is_flag){
      fprintf(fp," %-7s",hl->frame[i].is);
    }
    if(hl->imr_flag){
      fprintf(fp," %-4s %+6.2lf",hl->frame[i].imr,hl->frame[i].slt_pa);
    }
    if(hl->adc_flag){
      fprintf(fp," %-3s",hl->frame[i].adc);
    }
    if(hl->frame[i].note.txt){
      fprintf(fp,"  %s\n",hl->frame[i].note.txt);
    }
    else{
      fprintf(fp,"  \n");
    }
  }
  
  fprintf(fp,"  \n");
  fprintf(fp,"  \n");
  fprintf(fp,"  Using focus measurement on %s\n", hl->camz_date);
  fprintf(fp,"  best CamZ for Red  : %+d\n", hl->camz_r);
  fprintf(fp,"  best CamZ for Blue : %+d\n", hl->camz_b);
  fprintf(fp,"  delta Cross Scan Red  : %+d\n", hl->d_cross_r);
  fprintf(fp,"  delta Cross Scan Blue : %+d\n", hl->d_cross_b);
  fprintf(fp,"  Echelle zero angle : %+d\n", hl->echelle0);
  fprintf(fp,"  \n");
}

void ReadLog(typHLOG *hl,  FILE *fp){
  gchar *buf, *c, *n, *fname=NULL;
  gint i_frm=0, min_line;
  
  while(!feof(fp)){
    if((buf=fgets_new(fp))!=NULL){
      if(strlen(buf)>10){
	c=buf+4+2;
	if(strncmp(c,"HDSA",4)==0){
	  if(i_frm==0){
	    min_line=strlen(buf);
	  }
	  else if(strlen(buf)<min_line){
	    min_line=strlen(buf);
	  }
	  i_frm++;
	}
      }
    }
  }

  fseek(fp, 0L, SEEK_SET);
  
  while(!feof(fp)){
    if((buf=fgets_new(fp))!=NULL){
      if(strlen(buf)>10){
	c=buf+4+2;
	if(strncmp(c,"HDSA",4)==0){
	  if(strlen(buf)>min_line){
	    fname=g_strndup(buf+4+2,12);

	    for(i_frm=0;i_frm<hl->num;i_frm++){
	      if(strcmp(fname,hl->frame[i_frm].id)==0){
		c=buf+min_line;
		if(hl->frame[i_frm].note.txt) g_free(hl->frame[i_frm].note.txt);
		hl->frame[i_frm].note.txt=g_strdup(c);
		hl->frame[i_frm].note.auto_fl=TRUE;
		break;
	      }
	    }
	    g_free(fname);
	  }
	}
      }
    }
  }
}

void SendMail(GtkWidget *w, gpointer gdata){
  typHLOG *hl;
  FILE *fp;
  gchar filename[256];
  gchar sub[256];
  gchar command_line[512];

  hl=(typHLOG *)gdata;

  sprintf(filename,  "%s%s%s%shdslog-%04d%02d%02d.txt",
	  g_get_home_dir(), G_DIR_SEPARATOR_S,
	  HDSLOG_DIR, G_DIR_SEPARATOR_S,
	  hl->fr_year,hl->fr_month,hl->fr_day);

  if((fp=fopen(filename,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", filename);
    exit(1);
  }

  sprintf(sub,"HDS Observation LOG  %04d-%02d-%02d (HST)",
	  hl->fr_year,hl->fr_month,hl->fr_day);

  WriteLog(hl, fp);

  fclose(fp);

  sprintf(command_line,"cat %s | %s -s \"%s\" %s",
	  filename, MAIL_COMMAND, sub, hl->mail);
  ext_play(command_line);

  parse_address(hl);

  gtk_main_quit();

}


void ext_play(gchar *exe_command)
{
  static pid_t pid;
  gchar *cmdline;
  gint ret;
  
  waitpid(pid,0,WNOHANG);
  if(strcmp(exe_command,"\0")!=0){
    cmdline=g_strdup(exe_command);
    if( (pid = fork()) == 0 ){
      ret=system(cmdline);
      _exit(-1);
      signal(SIGCHLD,ChildTerm);
    }
  }
}


void do_iraf (GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gint winid;
  gchar *wtitle, *com, *tmp;

  wtitle=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, wtitle);
  if(winid==0){ // No xgterm
    com=g_strdup_printf(XGTERM_COM, wtitle);
    ext_play(com);
    g_free(com);
    usleep(1e6);

    winid=get_xdowin(hl, wtitle);
    
    if(winid<=0){ // Failed to start xgterm
      popup_message(hl->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    -1,
		    "Failed to start up xgterm.",
		    NULL);
    }
    else{ // Start up xgterm
      tmp=g_strdup_printf("xdotool type --window %d \'cd %s\'", 
			  winid, hl->wdir);
      send_xdo(hl, tmp);
      g_free(tmp);

      tmp=g_strdup_printf("xdotool key --window %d Return", winid);
      send_xdo(hl, tmp);
      g_free(tmp);
    }
  }
  else if(winid<0){ // 2 or more xgterm
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "Found 2 or more xgterm.",
		  " ",
		  "Please close xgterms excep one you want to use.",
		  NULL);
  }
  else{ // 1 xgterm
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "xgterm for HDS quick data analysis is alaready running.",
		  NULL);
  }
  g_free(wtitle);
}


void do_mail (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  read_ml(hl);

  hl->smdialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(hl->smdialog),5);
  gtk_window_set_title(GTK_WINDOW(hl->smdialog),"HDS Log Editor : Send Mail");
  gtk_window_set_modal(GTK_WINDOW(hl->smdialog),TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(hl->smdialog),GTK_WINDOW(hl->w_top));
  
  hbox=gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hl->smdialog))),
		     hbox,FALSE, FALSE, 0);

  label=gtk_label_new("Mail Addresses");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

  hl->address_entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),hl->address_entry,TRUE,TRUE,0);
  gtk_entry_set_width_chars(GTK_ENTRY(hl->address_entry),60);
  gtk_entry_set_text(GTK_ENTRY(hl->address_entry),hl->mail);
  g_signal_connect (hl->address_entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->mail);


  hbox=gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hl->smdialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Address Book","x-office-address-book");
#else
  button=gtkut_button_new_from_stock("Address Book",GTK_STOCK_PASTE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(popup_ml), 
		   (gpointer)hl);

  label=gtk_label_new("  ");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Send","mail-send");
#else
  button=gtkut_button_new_from_stock("Send",GTK_STOCK_NETWORK);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(SendMail), 
		   (gpointer)hl);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(close_child_dialog), 
		   GTK_WIDGET(hl->smdialog));

  
  gtk_widget_show_all(hl->smdialog);
  gtk_main();

  gtk_widget_destroy(hl->smdialog);
  flagChildDialog=FALSE;
}


void do_read_log(GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreePath *path;
  gint i_frm;

  hdslog_OpenFile(hl, OPEN_LOG);

  update_frame_tree(hl);

  path=gtk_tree_path_new_first();

  for(i_frm=0;i_frm<hl->num;i_frm++){
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_NOTE, hl->frame[i_frm].note.txt, -1);
    gtk_tree_path_next(path);
  }
  gtk_tree_path_free(path);
}


void do_cp_cal(GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *db_dir, *com;
  gint i, ret;

  if(access(hl->sdir, F_OK)==0){
    db_dir=g_strconcat(hl->sdir,
		       G_DIR_SEPARATOR_S,
		       "database",
		       NULL);
    if(access(hl->sdir, F_OK)!=0){
      com=g_strconcat("mkdir ",
		      db_dir,
		      NULL);
      ret=system(com);
      g_free(com);
    }
  }
  else{
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "<b>Error</b>: cannot access to HDS shared directory.",
		  " ",
		  hl->sdir,
		  NULL);
    return;
  }

  for(i=0;i<NUM_SET;i++){
    // Ap Red
    if(hl->flag_ap_red[i]){
      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->ap_red[i],
		      ".fits  ",
		      hl->sdir,
		      G_DIR_SEPARATOR_S,
		      hl->ap_red[i],
		      ".fits  ",
		      NULL);
      ret=system(com);
      g_free(com);

      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      "database",
		      G_DIR_SEPARATOR_S,
		      "ap",
		      hl->ap_red[i],
		      " ",
		      db_dir,
		      G_DIR_SEPARATOR_S,
		      "ap",
		      hl->ap_red[i],
		      NULL);
      ret=system(com);
      g_free(com);
    }

    // Ap Blue
    if(hl->flag_ap_blue[i]){
      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->ap_blue[i],
		      ".fits  ",
		      hl->sdir,
		      G_DIR_SEPARATOR_S,
		      hl->ap_blue[i],
		      ".fits  ",
		      NULL);
      ret=system(com);
      g_free(com);

      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      "database",
		      G_DIR_SEPARATOR_S,
		      "ap",
		      hl->ap_blue[i],
		      " ",
		      db_dir,
		      G_DIR_SEPARATOR_S,
		      "ap",
		      hl->ap_blue[i],
		      NULL);
      ret=system(com);
      g_free(com);
    }

    // ThAr Red
    if(hl->flag_thar_red[i]){
      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->thar_red[i],
		      ".fits  ",
		      hl->sdir,
		      G_DIR_SEPARATOR_S,
		      hl->thar_red[i],
		      ".fits  ",
		      NULL);
      ret=system(com);
      g_free(com);

      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      "database",
		      G_DIR_SEPARATOR_S,
		      "ec",
		      hl->thar_red[i],
		      " ",
		      db_dir,
		      G_DIR_SEPARATOR_S,
		      "ec",
		      hl->thar_red[i],
		      NULL);
      ret=system(com);
      g_free(com);
    }

    // ThAr Blue
    if(hl->flag_thar_blue[i]){
      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      hl->thar_blue[i],
		      ".fits  ",
		      hl->sdir,
		      G_DIR_SEPARATOR_S,
		      hl->thar_blue[i],
		      ".fits  ",
		      NULL);
      ret=system(com);
      g_free(com);

      com=g_strconcat("cp ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      "database",
		      G_DIR_SEPARATOR_S,
		      "ec",
		      hl->thar_blue[i],
		      " ",
		      db_dir,
		      G_DIR_SEPARATOR_S,
		      "ec",
		      hl->thar_blue[i],
		      NULL);
      ret=system(com);
      g_free(com);
    }
  }
}


void do_scp (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *dialog;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;
  GtkWidget *table;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  dialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"HDS Log Editor : Upload CAL files to sumda");
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  

  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);
  
  label=gtk_label_new("User ID : ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),20);
  gtk_entry_set_text(GTK_ENTRY(entry), HDS01_UNAME);
  gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);

  label=gtk_label_new("Password : ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),20);
  if(hl->spass) gtk_entry_set_text(GTK_ENTRY(entry), hl->spass);
  gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->spass);

  hbox=gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Upload","network-transmit");
#else
  button=gtkut_button_new_from_stock("Upload",GTK_STOCK_SAVE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(ScpCAL), 
		   (gpointer)hl);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(close_child_dialog), 
		   dialog);

  
  gtk_widget_show_all(dialog);
  gtk_main();

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


void do_dir (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *dialog;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;
  GtkWidget *table;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Folder Setup",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  

  table = gtkut_table_new (2, 4, FALSE, 2, 2, 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);
  
  label=gtk_label_new("Raw data directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("Work directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("IRAF uparm directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("Shared data directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  button = gtk_file_chooser_button_new ("Select Raw data directory",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button),
                                       hl->ddir);
  gtkut_table_attach(table, button, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button,
		    "file-set",
		    G_CALLBACK(cc_get_dir),
		    (gpointer)&hl->ddir);

  button = gtk_file_chooser_button_new ("Select Work directory",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button),
                                       hl->wdir);
  gtkut_table_attach(table, button, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button,
		    "file-set",
		    G_CALLBACK(cc_get_dir),
		    (gpointer)&hl->wdir);

  button = gtk_file_chooser_button_new ("Select IRAF uparm directory",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button),
                                       hl->udir);
  gtkut_table_attach(table, button, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button,
		    "file-set",
		    G_CALLBACK(cc_get_dir),
		    (gpointer)&hl->udir);

  button = gtk_file_chooser_button_new ("Select Shared data directory",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (button),
                                       hl->sdir);
  gtkut_table_attach(table, button, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button,
		    "file-set",
		    G_CALLBACK(cc_get_dir),
		    (gpointer)&hl->sdir);

  gtk_widget_show_all(dialog);
  gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


void do_remote (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl;
  GtkWidget *dialog;
  GtkWidget *check;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;
  GtkWidget *table;

  hl=(typHLOG *)gdata;
  
  if(flagChildDialog){
    return;
  }
  else{
    flagChildDialog=TRUE;
  }

  dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Remote Upload Setup",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));

  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  

  table = gtkut_table_new (2, 5, FALSE, 2, 2, 2);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     table,FALSE, FALSE, 0);
  

  check = gtk_check_button_new_with_label("Upload reduced spectra after every QL");
  gtkut_table_attach(table, check, 0, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), hl->remote_flag);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    &hl->remote_flag);

  label=gtk_label_new("Host : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("User : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("Password : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtk_label_new("Upload directory : ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 0, 1, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);



  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),60);
  if(hl->remote_host) gtk_entry_set_text(GTK_ENTRY(entry), hl->remote_host);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->remote_host);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),60);
  if(hl->remote_user) gtk_entry_set_text(GTK_ENTRY(entry), hl->remote_user);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->remote_user);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),20);
  if(hl->remote_pass) gtk_entry_set_text(GTK_ENTRY(entry), hl->remote_pass);
  gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->remote_pass);

  entry = gtk_entry_new ();
  gtkut_table_attach(table, entry, 1, 2, 4, 5,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),60);
  if(hl->remote_dir) gtk_entry_set_text(GTK_ENTRY(entry), hl->remote_dir);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    (gpointer)&hl->remote_dir);

  gtk_widget_show_all(dialog);
  gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}


gpointer thread_scan_command(gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  printdir(hl);

  if(hl->scloop) g_main_loop_quit(hl->scloop);
  while(my_main_iteration(FALSE));

  return(NULL);
}

gboolean start_scan_command(gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  if(hl->scloop) return(FALSE);

  //printf("Parent : Start scanning\n");
  gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			 "Scanning...");
  while(my_main_iteration(FALSE));
  
  hl->scloop=g_main_loop_new(NULL, FALSE);
  hl->scthread=g_thread_new("hdslog_scan",
			    thread_scan_command, (gpointer)hl);
  g_main_loop_run(hl->scloop);
  g_thread_join(hl->scthread);
  //printf("Parent : Thread end\n");
  g_main_loop_unref(hl->scloop);
  hl->scloop=NULL;

  update_frame_tree(hl);
  hl->num_old=hl->num;

  gtk_label_set_markup(GTK_LABEL(hl->w_status), 
			 " ");
  while(my_main_iteration(FALSE));
  
  if(hl->num >0){
    switch(hl->scr_flag){
    case SCR_AUTO:
      frame_tree_select_last(hl);
      break;

    case SCR_SMART:
      if(hl->frame_tree_i > hl->num-2){
	frame_tree_select_last(hl);
      }
      break;
    }
  }

  hl->scanning_timer=-1;
  return(FALSE);
}

gboolean check_scan (gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  if(!hl->scanning_flag){
    hl->scanning_timer=g_timeout_add(100, 
				     (GSourceFunc)start_scan_command,
				     (gpointer)hl);
    hl->scanning_flag=TRUE;
  }
  else if (hl->scanning_timer<0){  // 2nd time
    if(hl->upd_flag){
      hl->scanning_timer=g_timeout_add(READ_INTERVAL, 
				       (GSourceFunc)start_scan_command,
				       (gpointer)hl);
    }
  }

  return(TRUE);
}

void check_cal(typHLOG *hl){
  gint i;
  gchar *filename;
  gchar *indir;
  
  if(hl->file_head!=FILE_HDSA) return;


  // Red
  i=hl->iraf_hdsql_r;

  if((!hl->flag_ap_red[i]) && (hl->ap_red[i])){
    filename=g_strconcat(hl->wdir,
			 "database",
			 G_DIR_SEPARATOR_S,
			 "ap",
			 hl->ap_red[i],
			 NULL);
    if(access(filename, F_OK)==0){
      hl->flag_ap_red[i]=TRUE;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_red),
				   TRUE);
      gtk_widget_set_sensitive(hl->button_flat_red,TRUE);
      gtk_widget_set_sensitive(hl->button_thar_red,TRUE);
      gtk_widget_set_sensitive(hl->check_auto_red,TRUE);

      indir=get_indir(hl);
      edit_uparm(hl,"indirec","s",indir);
      edit_uparm(hl,"sc_refer","s",hl->ap_red[i]);
      edit_uparm(hl,"ap_refer","s",hl->ap_red[i]);
    }
    g_free(filename);
  }
  
  // ThAr Red
  if((!hl->flag_thar_red[i]) && (hl->thar_red[i])){
    filename=g_strconcat(hl->wdir,
			 "database",
			 G_DIR_SEPARATOR_S,
			 "ec",
			 hl->thar_red[i],
			 NULL);
    if(access(filename, F_OK)==0){
      hl->flag_thar_red[i]=TRUE;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_red),
				   TRUE);
      edit_uparm(hl,"wv_refer","s",hl->thar_red[i]);
    }
    g_free(filename);
    
  }
  
  // Blue
  i=hl->iraf_hdsql_b;

  if((!hl->flag_ap_blue[i]) && (hl->ap_blue[i])){
    filename=g_strconcat(hl->wdir,
			 "database",
			 G_DIR_SEPARATOR_S,
			 "ap",
			 hl->ap_blue[i],
			 NULL);
    if(access(filename, F_OK)==0){
      hl->flag_ap_blue[i]=TRUE;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_blue),
				   TRUE);
      gtk_widget_set_sensitive(hl->button_flat_blue,TRUE);
      gtk_widget_set_sensitive(hl->button_thar_blue,TRUE);
      gtk_widget_set_sensitive(hl->check_auto_blue,TRUE);

      indir=get_indir(hl);
      edit_uparm(hl,"indirec","s",indir);
      edit_uparm(hl,"sc_refer","s",hl->ap_blue[i]);
      edit_uparm(hl,"ap_refer","s",hl->ap_blue[i]);
    }
    g_free(filename);
  }
  
  if((!hl->flag_thar_blue[i]) && (hl->thar_blue)){
    filename=g_strconcat(hl->wdir,
			 "database",
			 G_DIR_SEPARATOR_S,
			 "ec",
			 hl->thar_blue[i],
			 NULL);
    if(access(filename, F_OK)==0){
      hl->flag_thar_blue[i]=TRUE;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_blue),
				   TRUE);
      edit_uparm(hl,"wv_refer","s",hl->thar_blue[i]);
    }
    g_free(filename);
    
  }
}


gint printdir(typHLOG *hl){
  GtkTreeIter iter;
  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;
  int newflag=0;
  int i,n;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));

  if((dp = opendir(hl->data_dir)) == NULL){
    fprintf(stderr, "cannot open directory: %s\n",hl->data_dir);
    return (-1);
  }
#ifdef DEBUG
  else{
    fprintf(stderr, "Reading: %s mode=%d\n",hl->data_dir,hl->file_head);
  }
#endif
  
  chdir(hl->data_dir);
  
  
  while((entry=readdir(dp))!=NULL){
    stat(entry->d_name,&statbuf);
      
    if(hl->file_head==FILE_HDSA){
      if(!strncmp(entry->d_name,"HDSA",4)){
	if(hl->upd_flag){
	  if( (statbuf.st_ctime>=hl->seek_time)
	      && (statbuf.st_ctime < hl->to_time)){
	    if(statbuf.st_ctime-hl->seek_time<5) sleep(5);
#ifdef DEBUG
	    printf("1: %s %s",entry->d_name,
		   asctime(localtime(&hl->fr_time)));
#endif
	    newflag+=printfits(hl,entry->d_name);
	  }
	}
	else{
	  newflag+=printfits(hl,entry->d_name);
	}
      }
    }
    else if(hl->file_head==FILE_hds){
      if(!strncmp(entry->d_name,"hds20",5)){
	if(hl->upd_flag){
	  if( (statbuf.st_ctime>=hl->seek_time)
	      && (statbuf.st_ctime < hl->to_time)){
	    if(statbuf.st_ctime-hl->seek_time<5) sleep(5);
#ifdef DEBUG
	    printf("1: %s %s",entry->d_name,
		   asctime(localtime(&hl->fr_time)));
#endif
	    newflag+=printfits(hl,entry->d_name);
	  }
	}
	else{
	  newflag+=printfits(hl,entry->d_name);
	}
      }
    }
  }
    
  chdir("..");
  closedir(dp);

  //check_cal(hl);

#ifdef DEBUG
  fprintf(stderr, "Start Save\n");
#endif
  save_note(hl);
#ifdef DEBUG
  fprintf(stderr, "End Save\n");
#endif
  
  hl->seek_time=time(NULL);
  
#ifdef DEBUG
  fprintf(stderr, "End of Read: %s\n",hl->data_dir);
#endif

  return (0);
}

void do_quit (GtkWidget *widget)
{
  gtk_main_quit();
}

GtkWidget *make_menu(typHLOG *hl){
  GtkWidget *menu_bar;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkWidget *popup_button;
  GtkWidget *bar;
  GtkWidget *image;
  GdkPixbuf *pixbuf, *pixbuf2;
  gint w,h;

  menu_bar=gtk_menu_bar_new();
  gtk_widget_show (menu_bar);

  gtk_icon_size_lookup(GTK_ICON_SIZE_MENU,&w,&h);

  //// File
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("system-file-manager", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "File");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("File");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  if(hl->upd_flag){
    //File/Send Mail
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("mail-send", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Send Mail");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Send Mail");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_mail),(gpointer)hl);
    
    bar =gtk_separator_menu_item_new();
    gtk_widget_show (bar);
    gtk_container_add (GTK_CONTAINER (menu), bar);
    
  //File/Upload CAL
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("network-transmit", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Upload CAL files");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Upload CAL files");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_scp),(gpointer)hl);
  }
  else{
  //File/Open Obs Log
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("document-open", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Open Obs Log");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Open Obs Log");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_read_log),(gpointer)hl);
  }

  bar =gtk_separator_menu_item_new();
  gtk_widget_show (bar);
  gtk_container_add (GTK_CONTAINER (menu), bar);

  //File/Quit
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Quit");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Quit");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate",G_CALLBACK(do_quit),NULL);

  //// IRAF
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("utilites-terminal", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "IRAF");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("IRAF");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

  //IRAF/xgterm
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("utilities-terminal", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Check/Restart xgterm");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Check/Restart xgterm");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate", 
		    G_CALLBACK(do_iraf), (gpointer)hl);

  //IRAF/folder
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("folder", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Folder Setups");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Folder Setups");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate", 
		    G_CALLBACK(do_dir), (gpointer)hl);

  if(hl->upd_flag){
  //// IRAF/Upload
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("network-transmit", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Upload reduced spectra");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Upload reduced spectra");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_remote),(gpointer)hl);
  }
  else{
  //File/Upload CAL
#ifdef USE_GTK3
    image=gtk_image_new_from_icon_name ("edit-copy", GTK_ICON_SIZE_MENU);
    popup_button =gtkut_image_menu_item_new_with_label (image, "Copy CAL files to Share Dir.");
#else
    image=gtk_image_new_from_stock (GTK_STOCK_COPY, GTK_ICON_SIZE_MENU);
    popup_button =gtk_image_menu_item_new_with_label ("Copy CAL files to Share Dir.");
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
    gtk_widget_show (popup_button);
    gtk_container_add (GTK_CONTAINER (menu), popup_button);
    g_signal_connect (popup_button, "activate",G_CALLBACK(do_cp_cal),(gpointer)hl);
  }

#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("help-browser", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "Splot Help");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_HELP, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("Splot Help");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate", 
		    G_CALLBACK(splot_help), (gpointer)hl);

  //// Info
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("user-info", GTK_ICON_SIZE_MENU);
  menu_item =gtkut_image_menu_item_new_with_label (image, "Info");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_MENU);
  menu_item =gtk_image_menu_item_new_with_label ("Info");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_item),image);
#endif
  gtk_widget_show (menu_item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
  
  menu=gtk_menu_new();
  gtk_widget_show (menu);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  
  //Info/About
#ifdef USE_GTK3
  image=gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_MENU);
  popup_button =gtkut_image_menu_item_new_with_label (image, "About");
#else
  image=gtk_image_new_from_stock (GTK_STOCK_ABOUT, GTK_ICON_SIZE_MENU);
  popup_button =gtk_image_menu_item_new_with_label ("About");
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(popup_button),image);
#endif
  gtk_widget_show (popup_button);
  gtk_container_add (GTK_CONTAINER (menu), popup_button);
  g_signal_connect (popup_button, "activate", 
		    G_CALLBACK(show_version), (gpointer)hl);

  gtk_widget_show_all(menu_bar);
  return(menu_bar);
}



void gui_init(typHLOG *hl){
  gchar *tmp;
  GtkWidget *menubar, *label, *table;
  GtkWidget *hbox, *hbox1, *hbox2, *button, *frame, *frame1, *combo;

  // Main Window 
  hl->w_top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(hl->w_top, "destroy",
		   G_CALLBACK(gtk_main_quit),NULL);
  gtk_container_set_border_width(GTK_CONTAINER(hl->w_top),0);
  tmp=g_strdup_printf("HDS Log Editor (%s)",hl->uname);
  gtk_window_set_title(GTK_WINDOW(hl->w_top),tmp);
  g_free(tmp);

  hl->w_box = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (hl->w_top), hl->w_box);

  menubar=make_menu(hl);
  gtk_box_pack_start(GTK_BOX(hl->w_box), menubar,FALSE, FALSE, 0);

  make_top_table(hl);
  gtk_box_pack_start(GTK_BOX(hl->w_box), hl->top_table,FALSE, FALSE, 5);

  hl->scrwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(hl->scrwin),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_ALWAYS);
  gtk_box_pack_start(GTK_BOX(hl->w_box), hl->scrwin,TRUE, TRUE, 5);

  gtk_widget_set_size_request(hl->scrwin,-1,400);


  make_frame_tree(hl);

  hbox = gtkut_hbox_new(FALSE,5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(hl->w_box), hbox,FALSE, FALSE, 5);

  hl->frame_ql_red = gtkut_frame_new (FRAME_QL_RED_LABEL);
  gtk_box_pack_start (GTK_BOX (hbox),hl->frame_ql_red, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hl->frame_ql_red), 5);

  table = gtkut_table_new (5, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (hl->frame_ql_red), table);
  
  label=gtkut_label_new("<b>Obj</b>");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"media-playback-start");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_MEDIA_PLAY);
#endif
  gtkut_table_attach(table, button, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_obj_red), (gpointer)hl);

  label=gtkut_label_new("<span size=\"smaller\">set</span>");
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_ql=0;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
    while(hdsql_red[i_ql]){
      gtk_list_store_append(store, &iter);
      tmp=g_strdup_printf("%d", i_ql+1);
      gtk_list_store_set(store, &iter, 
			 0, tmp,
			 1, i_ql, -1);
      g_free(tmp);
      if(hl->iraf_hdsql_r==i_ql) iter_set=iter;

      i_ql++;
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_change_set_red),
		      (gpointer)hl);
  }

  label=gtkut_label_new("<span size=\"smaller\">auto</span>");
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hl->check_auto_red = gtk_check_button_new();
  gtkut_table_attach(table, hl->check_auto_red, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_red),FALSE);
  gtk_widget_set_sensitive(hl->check_auto_red, FALSE);
  g_signal_connect (hl->check_auto_red,"toggled",
		    G_CALLBACK(cc_auto_red),
		    (gpointer)hl);


  label=gtkut_label_new("<span size=\"smaller\">conf</span>");
  gtkut_table_attach(table, label, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"preferences-system");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_PROPERTIES);
#endif
  gtkut_table_attach(table, button, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_param_red), (gpointer)hl);

 
  frame1 = gtkut_frame_new ("Calibration");
  gtkut_table_attach(table, frame1, 4, 5, 0, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);

  hbox2 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);
  gtk_container_add (GTK_CONTAINER (frame1), hbox2);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_find_red), (gpointer)hl);

  label=gtk_label_new(" ");
  gtk_box_pack_start(GTK_BOX(hbox2),label,TRUE,TRUE, 0);

  button=gtk_button_new_with_label("Ap");
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_ap_red), (gpointer)hl);

  hl->check_ap_red = gtk_check_button_new();
  gtk_box_pack_start(GTK_BOX(hbox2),hl->check_ap_red,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_red),FALSE);
  gtk_widget_set_sensitive(hl->check_ap_red, FALSE);


  label=gtk_label_new(" ");
  gtk_box_pack_start(GTK_BOX(hbox2),label,TRUE,TRUE, 0);

  hl->button_flat_red=gtk_button_new_with_label("Flat");
  gtk_box_pack_start(GTK_BOX(hbox2),hl->button_flat_red,FALSE, FALSE, 0);
  gtk_widget_set_sensitive(hl->button_flat_red, FALSE);
  g_signal_connect (hl->button_flat_red, "clicked",
		    G_CALLBACK (ql_flat_red), (gpointer)hl);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_ex;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_ex=0;i_ex<NUM_FLAT_EX;i_ex++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, flat_ex[i_ex],
			 1, i_ex, -1);
      if(hl->ex_flat_red[hl->iraf_hdsql_r]==i_ex) iter_set=iter;
    }
    
    hl->combo_flat_red = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox2),hl->combo_flat_red,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hl->combo_flat_red),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hl->combo_flat_red), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hl->combo_flat_red),&iter_set);
    gtk_widget_show(hl->combo_flat_red);
    gtk_widget_set_sensitive(hl->combo_flat_red,FALSE);
  }


  label=gtk_label_new(" ");
  gtk_box_pack_start(GTK_BOX(hbox2),label,TRUE,TRUE, 0);


  hl->button_thar_red=gtk_button_new_with_label("ThAr");
  gtk_box_pack_start(GTK_BOX(hbox2),hl->button_thar_red,FALSE, FALSE, 0);
  gtk_widget_set_sensitive(hl->button_thar_red,FALSE);
  g_signal_connect (hl->button_thar_red, "clicked",
		    G_CALLBACK (ql_thar_red), (gpointer)hl);

  hl->check_thar_red = gtk_check_button_new();
  gtk_box_pack_start(GTK_BOX(hbox2),hl->check_thar_red,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_red),FALSE);
  gtk_widget_set_sensitive(hl->check_thar_red, FALSE);


  hl->frame_ql_blue = gtkut_frame_new (FRAME_QL_BLUE_LABEL);
  gtk_box_pack_start (GTK_BOX (hbox),hl->frame_ql_blue, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hl->frame_ql_blue), 5);

  table = gtkut_table_new (5, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (hl->frame_ql_blue), table);
  
  label=gtkut_label_new("<b>Obj</b>");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"media-playback-start");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_MEDIA_PLAY);
#endif
  label=gtkut_label_new("<b>Obj</b>");
  gtkut_table_attach(table, button, 0, 1, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_obj_blue), (gpointer)hl);
  
  label=gtkut_label_new("<span size=\"smaller\">set</span>");
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_ql=0;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    while(hdsql_blue[i_ql]){
      gtk_list_store_append(store, &iter);
      tmp=g_strdup_printf("%d", i_ql+1);
      gtk_list_store_set(store, &iter, 
			 0, tmp,
			 1, i_ql, -1);
      g_free(tmp);
      if(hl->iraf_hdsql_b==i_ql) iter_set=iter;

      i_ql++;
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtkut_table_attach(table, combo, 1, 2, 1, 2,
		       GTK_FILL,GTK_SHRINK,0,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_change_set_blue),
		      (gpointer)hl);
  }


  label=gtkut_label_new("<span size=\"smaller\">auto</span>");
  gtkut_table_attach(table, label, 2, 3, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  hl->check_auto_blue = gtk_check_button_new();
  gtkut_table_attach(table, hl->check_auto_blue, 2, 3, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_blue),FALSE);
  gtk_widget_set_sensitive(hl->check_auto_blue, FALSE);
  g_signal_connect (hl->check_auto_blue,"toggled",
		    G_CALLBACK(cc_auto_blue),
		    (gpointer)hl);


  label=gtkut_label_new("<span size=\"smaller\">conf</span>");
  gtkut_table_attach(table, label, 3, 4, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"preferences-system");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_PROPERTIES);
#endif
  gtkut_table_attach(table, button, 3, 4, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_param_blue), (gpointer)hl);


  frame1 = gtkut_frame_new ("Calibration");
  gtkut_table_attach(table, frame1, 4, 5, 0, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);

  hbox2 = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);
  gtk_container_add (GTK_CONTAINER (frame1), hbox2);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_find_blue), (gpointer)hl);

  label=gtk_label_new(" ");
  gtk_box_pack_start(GTK_BOX(hbox2),label,TRUE,TRUE, 0);

  button=gtk_button_new_with_label("Ap");
  gtk_box_pack_start(GTK_BOX(hbox2),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (ql_ap_blue), (gpointer)hl);

  hl->check_ap_blue = gtk_check_button_new();
  gtk_box_pack_start(GTK_BOX(hbox2),hl->check_ap_blue,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_blue),FALSE);
  gtk_widget_set_sensitive(hl->check_ap_blue, FALSE);

  label=gtk_label_new(" ");
  gtk_box_pack_start(GTK_BOX(hbox2),label,TRUE,TRUE, 0);

  hl->button_flat_blue=gtk_button_new_with_label("Flat");
  gtk_box_pack_start(GTK_BOX(hbox2),hl->button_flat_blue,FALSE, FALSE, 0);
  gtk_widget_set_sensitive(hl->button_flat_blue, FALSE);
  g_signal_connect (hl->button_flat_blue, "clicked",
		    G_CALLBACK (ql_flat_blue), (gpointer)hl);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_ex;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_ex=0;i_ex<NUM_FLAT_EX;i_ex++){
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, flat_ex[i_ex],
			 1, i_ex, -1);
      if(hl->ex_flat_blue[hl->iraf_hdsql_b]==i_ex) iter_set=iter;
    }
    
    hl->combo_flat_blue = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox2),hl->combo_flat_blue,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(hl->combo_flat_blue),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(hl->combo_flat_blue), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(hl->combo_flat_blue),&iter_set);
    gtk_widget_show(hl->combo_flat_blue);
    gtk_widget_set_sensitive(hl->combo_flat_blue,FALSE);
  }

  label=gtk_label_new(" ");
  gtk_box_pack_start(GTK_BOX(hbox2),label,TRUE,TRUE, 0);

  hl->button_thar_blue=gtk_button_new_with_label("ThAr");
  gtk_box_pack_start(GTK_BOX(hbox2),hl->button_thar_blue,FALSE, FALSE, 0);
  gtk_widget_set_sensitive(hl->button_thar_blue, FALSE);
  g_signal_connect (hl->button_thar_blue, "clicked",
		    G_CALLBACK (ql_thar_blue), (gpointer)hl);

  hl->check_thar_blue = gtk_check_button_new();
  gtk_box_pack_start(GTK_BOX(hbox2),hl->check_thar_blue,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_blue),FALSE);
  gtk_widget_set_sensitive(hl->check_thar_blue, FALSE);

  gtk_widget_show_all(hl->w_top);
}



void splot_help(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  GtkWidget *dialog, *frame, *table, *label, *vbox, *button, *hbox;

  dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(dialog), "HDS Log Editor : Help for splot");

  vbox = gtkut_vbox_new(FALSE,0);
  gtk_container_add (GTK_CONTAINER (dialog), vbox);

  frame = gtkut_frame_new ("<b>Change Order</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>(</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Move to previous");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>)</b>    ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Move to next");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  frame = gtkut_frame_new ("<b>X-axis Scale</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>v</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Velocity scale [km/s] (toggle)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>$</b>    ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Pixel (toggle)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  frame = gtkut_frame_new ("<b>Y-axis Scale</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 1, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>b</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Set base line to zero (toggle)");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
   
  frame = gtkut_frame_new ("<b>Magnify</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>a</b> &amp; <b>a</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Auto expand between cursors");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>c</b>    ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Clear and redraw");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);

  frame = gtkut_frame_new ("<b>Measurement</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 4, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>m</b> &amp; <b>m</b>   ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Avg, RMS, and SNR between cursors");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
 
  label=gtkut_label_new("    <b>k</b> &amp; <b>k</b>   ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Gaussian fit");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>e</b> &amp; <b>e</b>   ");
  gtkut_table_attach(table, label, 0, 1, 2, 3,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Get equivalent width");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 2, 3,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  label=gtkut_label_new("    <b>C</b>   ");
  gtkut_table_attach(table, label, 0, 1, 3, 4,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Get values at cursor position");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 3, 4,
		     GTK_FILL,GTK_SHRINK,0,0);
  
  frame = gtkut_frame_new ("<b>Other Display Commands</b>");
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  
  table = gtkut_table_new (2, 2, FALSE, 2, 2, 2);
  gtk_container_add (GTK_CONTAINER (frame), table);

  label=gtkut_label_new("    <b>s</b>    ");
  gtkut_table_attach(table, label, 0, 1, 0, 1,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Smooth the spectrum by boxcar");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);

  label=gtkut_label_new("    <b>:hist y</b>    ");
  gtkut_table_attach(table, label, 0, 1, 1, 2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
  label=gtkut_label_new("Set line type to Histgram");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtkut_table_attach(table, label, 1, 2, 1, 2,
		     GTK_FILL,GTK_SHRINK,0,0);
   
  
  hbox = gtkut_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Close","window-close");
#else
  button=gtkut_button_new_from_stock("Close",GTK_STOCK_CLOSE);
#endif
  g_signal_connect (button, "clicked",
		    G_CALLBACK (close_dialog), dialog);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);

  gtk_widget_show_all(dialog);
}


void show_version (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *label, *button, *pixmap, *vbox, *hbox;
  GdkPixbuf *pixbuf, *pixbuf2;
#if HAVE_SYS_UTSNAME_H
  struct utsname utsbuf;
#endif
  gchar buf[1024];
  GtkWidget *scrolledwin;
  GtkWidget *text;
  GtkTextBuffer *buffer;
  GtkTextIter iter;
  typHLOG *hl=(typHLOG *) gdata;
  gint result;

  dialog = gtk_dialog_new_with_buttons("HDS Log Editor : About This Program",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
  gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
							   GTK_RESPONSE_OK));
  

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  pixbuf = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  pixbuf2=gdk_pixbuf_scale_simple(pixbuf,
				  96,96,GDK_INTERP_BILINEAR);
  pixmap = gtk_image_new_from_pixbuf(pixbuf2);
  g_object_unref(pixbuf);
  g_object_unref(pixbuf2);

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  gtk_label_set_markup(GTK_LABEL(label), "<span size=\"larger\"><b>Subaru/HDS Log Editor</b></span>   version "VERSION);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  g_snprintf(buf, sizeof(buf),
	     "GTK+ %d.%d.%d / GLib %d.%d.%d",
	     gtk_major_version, gtk_minor_version, gtk_micro_version,
	     glib_major_version, glib_minor_version, glib_micro_version);
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

#if HAVE_SYS_UTSNAME_H
  uname(&utsbuf);
  g_snprintf(buf, sizeof(buf),
	     "Operating System: %s %s (%s)",
	     utsbuf.sysname, utsbuf.release, utsbuf.machine);
#else
  g_snprintf(buf, sizeof(buf),
	     "Operating System: unknown UNIX");
#endif
  label = gtk_label_new (buf);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);


  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

 
  
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL(label), "&#xA9; 2005  Akito Tajitsu");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),label,FALSE, FALSE, 0);

  label = gtk_label_new ("Subaru Telescope, National Astronomical Observatory of Japan");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label=gtk_label_new(NULL);
  gtk_label_set_markup (GTK_LABEL(label), "&lt;<i>tajitsu@naoj.org</i>&gt;");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  label = gtk_label_new ("");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox), label,FALSE, FALSE, 0);

  scrolledwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
				 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwin),
				      GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     scrolledwin, TRUE, TRUE, 0);
  gtk_widget_set_size_request (scrolledwin, 400, 250);
  
  text = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
  gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text), 6);
  gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text), 6);
  gtk_container_add(GTK_CONTAINER(scrolledwin), text);
  
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
  gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
  
  gtk_text_buffer_insert(buffer, &iter,
			 "This program is free software; you can redistribute it and/or modify "
			 "it under the terms of the GNU General Public License as published by "
			 "the Free Software Foundation; either version 3, or (at your option) "
			 "any later version.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "This program is distributed in the hope that it will be useful, "
			 "but WITHOUT ANY WARRANTY; without even the implied warranty of "
			 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
			 "See the GNU General Public License for more details.\n\n", -1);

  gtk_text_buffer_insert(buffer, &iter,
			 "You should have received a copy of the GNU General Public License "
			 "along with this program.  If not, see <http://www.gnu.org/licenses/>.", -1);

  gtk_text_buffer_get_start_iter(buffer, &iter);
  gtk_text_buffer_place_cursor(buffer, &iter);

  gtk_widget_show_all(dialog);

  result= gtk_dialog_run(GTK_DIALOG(dialog));

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}


int main(int argc, char* argv[]){
  typHLOG *hl;
  time_t t;
  struct tm *tmpt,tmpt2;
  gint i;
  gchar *filename;
  GdkPixbuf *icon;
  gchar hostname[128];

  if(argc!=2){
    fprintf(stderr, "[Usage] : %% hdslog data-dir\n");
    exit(0);
  }
  else if(access(argv[1], F_OK)!=0){
    fprintf(stderr, " hdslog ERROR : Cannot access to \"%s\".\n", argv[1]);
    exit(-1);
  }

  filename=g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,
		       HDSLOG_DIR, NULL);
  if(access(filename, F_OK)!=0){
    fprintf(stderr, "Creating Directory \"%s\".\n",filename);
    mkdir(filename,(S_IRWXU|S_IRGRP|S_IROTH));
  }
  g_free(filename);

  write_muttrc();
  write_msmtprc();

  hl=g_malloc0(sizeof(typHLOG));

  if(argv[1][strlen(argv[1])-1]=='/'){
    hl->data_dir=g_strndup(argv[1],strlen(argv[1])-1);
  }
  else{
    hl->data_dir=g_strdup(argv[1]);
  }
  hl->num=0;
  hl->idnum0=0;
  hl->prop=NULL;
  hl->file_head=FILE_HDSA;
  hl->mail=g_strdup(DEF_MAIL);

  hl->http_host=NULL;
  hl->http_path=NULL;
  hl->http_dlfile=NULL;

  t = time(NULL);
  tmpt = localtime(&t);

  hl->fr_year=tmpt->tm_year+1900;
  hl->fr_month=tmpt->tm_mon+1;
  hl->fr_day=tmpt->tm_mday;
  if(tmpt->tm_hour<9){
    get_hst_day(&hl->fr_year, &hl->fr_month, &hl->fr_day);
  }

  tmpt2.tm_year=hl->fr_year-1900;
  tmpt2.tm_mon=hl->fr_month-1;
  tmpt2.tm_mday=hl->fr_day;
  tmpt2.tm_hour=9;
  tmpt2.tm_min=0;
  tmpt2.tm_sec=0;
  
  hl->fr_time=mktime(&tmpt2);
  hl->seek_time=hl->fr_time;

  hl->to_time=hl->fr_time+60*60*24;
  
  hl->buf_year=hl->fr_year;
  hl->buf_month=hl->fr_month;
  hl->buf_day=hl->fr_day;

  hl->scr_flag=SCR_SMART;
  hl->ech_tmpfl=FALSE;
  hl->ech_flag=FALSE;
  hl->i2_tmpfl=FALSE;
  hl->i2_flag=FALSE;
  hl->is_tmpfl=FALSE;
  hl->is_flag=FALSE;
  hl->camz_tmpfl=FALSE;
  hl->camz_flag=FALSE;
  hl->imr_tmpfl=FALSE;
  hl->imr_flag=FALSE;
  hl->adc_tmpfl=FALSE;
  hl->adc_flag=FALSE;

  for(i=0;i<MAX_FRAME;i++){
    hl->frame[i].note.txt=NULL;
    hl->frame[i].note.time=0;
    hl->frame[i].note.auto_fl=FALSE;
  }

  for(i=0;i<MAX_MAIL;i++){
    hl->ml[i].address=NULL;
    hl->ml[i].year=0;
    hl->ml[i].month=0;
    hl->ml[i].day=0;
  }
  hl->ml_max=0;
  
  hl->d_cross_b=DEF_D_CROSS_B;
  hl->d_cross_r=DEF_D_CROSS_R;
  hl->camz_b=DEF_CAMZ_B;
  hl->camz_r=DEF_CAMZ_R;
  hl->echelle0=DEF_ECHELLE0;
  hl->camz_date=NULL;
  flag_make_frame_tree=FALSE;

  // Host
  gethostname(hostname, sizeof(hostname));
  if(strncmp(hostname,"sumda",strlen("sumda"))==0){
    hl->upd_flag=TRUE;
  }
  else if(strncmp(hostname,"hdsobcpl",strlen("hdsobcpl"))==0){
    hl->upd_flag=TRUE;
  }
  else{
    hl->upd_flag=FALSE;
  }

  // IRAF
  hl->uname=getenv("USER");
  hl->xdotmp=g_strdup_printf(XDOTMP,hl->uname);
  hl->uparmtmp=g_strdup_printf(UPARMTMP,hl->uname);
  hl->flattmp1=g_strdup_printf(FLAT_TMP1,hl->uname);
  hl->flattmp2=g_strdup_printf(FLAT_TMP2,hl->uname);
  hl->ddir=g_strdup(hl->data_dir);
  hl->wdir=get_work_dir_sumda(hl);
  hl->sdir=get_share_dir_sumda(hl);
  hl->udir=get_uparm_dir_sumda(hl);
  hl->spass=NULL;
  hl->iraf_hdsql_r=0;
  hl->iraf_hdsql_b=0;
  hl->file_write=NULL;
  hl->file_wait=NULL;
  hl->ref_frame=NULL;

  hl->auto_red=FALSE;
  hl->auto_blue=FALSE;

  for(i=0;i<NUM_SET;i++){
    hl->bin1_red[i]=1;
    hl->bin1_blue[i]=1;
    hl->bin2_red[i]=1;
    hl->bin2_blue[i]=1;

    hl->setname_red[i]=NULL;
    hl->setname_blue[i]=NULL;
    hl->ap_red[i]=NULL;
    hl->ap_blue[i]=NULL;
    hl->flat_red[i]=NULL;
    hl->flat_blue[i]=NULL;
    hl->thar_red[i]=NULL;
    hl->thar_blue[i]=NULL;

    hl->flag_ap_red[i]=FALSE;
    hl->flag_ap_blue[i]=FALSE;
    hl->ex_flat_red[i]=FLAT_EX_NO;
    hl->ex_flat_blue[i]=FLAT_EX_NO;
    hl->flag_thar_red[i]=FALSE;
    hl->flag_thar_blue[i]=FALSE;

    hl->qp_r[i].sc_inte=TRUE;
    hl->qp_r[i].sc_resi=FALSE;
    hl->qp_r[i].sc_edit=TRUE;
    hl->qp_r[i].sc_fitt=TRUE;
    hl->qp_r[i].ap_inte=TRUE;
    hl->qp_r[i].ap_resi=TRUE;
    hl->qp_r[i].ap_edit=TRUE;
    hl->qp_r[i].ap_llim=-20;
    hl->qp_r[i].ap_ulim=+20;
    hl->qp_r[i].is_plot=TRUE;
    hl->qp_r[i].is_stx=-20;
    hl->qp_r[i].is_edx=+20;
    hl->qp_r[i].sp_line=5;

    hl->qp_b[i].sc_inte=TRUE;
    hl->qp_b[i].sc_resi=FALSE;
    hl->qp_b[i].sc_edit=TRUE;
    hl->qp_b[i].sc_fitt=TRUE;
    hl->qp_b[i].ap_inte=TRUE;
    hl->qp_b[i].ap_resi=TRUE;
    hl->qp_b[i].ap_edit=TRUE;
    hl->qp_b[i].ap_llim=-20;
    hl->qp_b[i].ap_ulim=+20;
    hl->qp_b[i].is_plot=TRUE;
    hl->qp_b[i].is_stx=-20;
    hl->qp_b[i].is_edx=+20;
    hl->qp_b[i].sp_line=5;
  }
  
  hl->i_reduced=0;

  hl->remote_flag=FALSE;
  hl->remote_host=g_strdup(REMOTE_HOST);
  hl->remote_user=g_strdup(REMOTE_USER);
  hl->remote_pass=NULL;
  hl->remote_dir=g_strdup(REMOTE_DIR);
  
  gtk_init(&argc, &argv);

#ifndef USE_GTK3
  gdk_color_alloc(gdk_colormap_get_system(),&color_red);
  gdk_color_alloc(gdk_colormap_get_system(),&color_black);
#endif

  popup_dl_camz_list(NULL, (gpointer)hl);
  icon = gdk_pixbuf_new_from_resource ("/icons/subaru_icon.png", NULL);
  gtk_window_set_default_icon(icon);

  gui_init(hl);

  hl->scanning_flag=FALSE;
  if(hl->upd_flag){
    hl->timer=g_timeout_add(CHECK_INTERVAL, 
			    (GSourceFunc)check_scan, 
			    (gpointer)hl);
  }
  else{
    do_dir(NULL,(gpointer)hl);
    start_scan_command((gpointer)hl);
  }
  gtk_main();
}

