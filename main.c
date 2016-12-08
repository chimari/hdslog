//    HDS LOG Editor
//   
//                                           2005.12.08  A.Tajitsu

#include"main.h"    // 設定ヘッダ
#include"version.h"
#include"configfile.h"

// #define DEBUG

gboolean flag_make_frame_list=FALSE;
gboolean flag_make_top=FALSE;
gboolean flagChildDialog=FALSE;
GtkWidget *frame_table;
guint entry_height=24;

void ChildTerm();
static void cc_get_adj ();
static void cc_get_entry ();
static void cc_get_note ();
static void cc_get_toggle();
static void cc_file_head ();
static void refresh_table ();
gboolean create_lock ();
static void remove_lock ();
static void wait_lock ();
static void save_note ();
static void load_note ();

void select_color();
void make_frame_list();
void update_frame_list();
int printfits();
void ext_play();
gint scan_command();
gint printdir();
void gui_init();


// Ya is temporary (using Yb setting)
const SetupEntry setups[] = {
  {"StdUb",  "BLUE",  17100}, 
  {"StdUa",  "BLUE",  17820}, 
  {"StdBa",  "BLUE",  19260}, 
  {"StdBc",  "BLUE",  19890}, 
  {"StdYa",  "BLUE",  21960}, 
  {"StdI2b", "RED",   14040}, 
  {"StdYd",  "RED",   15480}, 
  {"StdYb",  "RED",   15730}, 
  {"StdYc",  "RED",   16500}, 
  {"StdI2a", "RED",   18000}, 
  {"StdRa",  "RED",   18455}, 
  {"StdRb",  "RED",   19080}, 
  {"StdNIRc","RED",   21360}, 
  {"StdNIRb","RED",   22860}, 
  {"StdNIRa","RED",   25200}, 
  {"StdHa",  "MIRROR",0}
};


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

static void cc_get_adj (GtkWidget *widget, gint * gdata)
{
  *gdata=GTK_ADJUSTMENT(widget)->value;
}

static void cc_get_entry (GtkWidget *widget, gchar **gdata)
{
  g_free(*gdata);
  *gdata=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
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

static void cc_get_toggle (GtkWidget * widget, gboolean * gdata)
{
  *gdata=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void cc_file_head (GtkWidget *widget, gint *gdata)
{
  
  if(!strcmp(gtk_entry_get_text(GTK_ENTRY(widget)),"HDSA")){
    *gdata = FILE_HDSA;
  }
  else if(!strcmp(gtk_entry_get_text(GTK_ENTRY(widget)),"hds20")){
    *gdata = FILE_hds;
  }
}

void change_label(GtkWidget *widget, gchar *txt, GdkColor color){
  GtkStyle *def_style, *style;

  def_style = gtk_widget_get_style(widget);
  style = gtk_style_copy(def_style);

  gtk_label_set(GTK_LABEL(widget),txt);
  style->fg[0] = color;
  gtk_widget_set_style(widget,style);

  while (g_main_iteration(FALSE));
  gdk_flush();
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
  
  hl->fr_time=mktime(&tmpt2);
  hl->seek_time=hl->fr_time;
  hl->to_time=hl->fr_time+60*60*24;
    

  if(flag_make_frame_list)  gtk_widget_destroy(frame_table);
  flag_make_frame_list=FALSE;

  for(i=0;i<MAX_FRAME;i++){
    hl->frame[i].note.txt=NULL;
    hl->frame[i].note.time=0;
    hl->frame[i].note.auto_fl=FALSE;
  }
  
  hl->num=0;
  hl->num_old=0;

  hl->i2_flag=hl->i2_tmpfl;
  hl->is_flag=hl->is_tmpfl;
  hl->camz_flag=hl->camz_tmpfl;
  hl->imr_flag=hl->imr_tmpfl;
  hl->adc_flag=hl->adc_tmpfl;

  make_frame_list(hl);
  
  gtk_widget_set_sensitive(hl->b_refresh,TRUE);
}

gboolean create_lock (typHLOG *hl){
  gchar lockfile[256];

  sprintf(lockfile,"%s/.hdslog-%04d%02d%02d.lock",
	   g_get_home_dir(),hl->fr_year,hl->fr_month,hl->fr_day);
    
  while(1){
    hl->lock_fp=open(lockfile, O_RDWR | O_CREAT | O_EXCL, 0444);
    if (hl->lock_fp == -1){
      printf ("%d - Lock already present\n",getpid());
      //return(FALSE);
      change_label(hl->w_status, "File Lock", red);
      sleep(1);
    }
    else{
      change_label(hl->w_status, "Scanning...", black);
      hl->lock_flag=TRUE;
      //return(TRUE);
      break;
    }
  }
  return(TRUE);
}

static void remove_lock (typHLOG *hl){
  gchar lockfile[256];
  sprintf(lockfile,"%s/.hdslog-%04d%02d%02d.lock",
	   g_get_home_dir(),hl->fr_year,hl->fr_month,hl->fr_day);

  close(hl->lock_fp);
  unlink(lockfile);
  hl->lock_flag=FALSE;
}

static void wait_lock (typHLOG *hl){
  while(hl->lock_flag){
    change_label(hl->w_status, "File Lock", red);
    sleep(1);
  }
}



static void save_note (typHLOG *hl)
{
  ConfigFile *cfgfile;
  gchar filename[256];
  gint i;

  create_lock(hl);

  sprintf(filename,"%s/.hdslog-%04d%02d%02d",
	   g_get_home_dir(),hl->fr_year,hl->fr_month,hl->fr_day);
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

  remove_lock(hl);
}


static void load_note (typHLOG *hl,gboolean force_fl)
{
  ConfigFile *cfgfile;
  gchar filename[256];
  gchar *c_buf;
  gint i, i_buf;
  struct stat statbuf;

  sprintf(filename,"%s/.hdslog-%04d%02d%02d",
	   g_get_home_dir(),hl->fr_year,hl->fr_month,hl->fr_day);

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
	    hl->frame[i].note.txt=g_strdup(c_buf);
	    hl->frame[i].note.time=i_buf;

	    if( (hl->frame[i].note.txt) && (!force_fl)){
	      hl->frame[i].note.auto_fl=TRUE;
	      gtk_entry_set_text(GTK_ENTRY(hl->frame[i].w_note),
				 hl->frame[i].note.txt);
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


void select_color(FRAMEpara *frame, gint d_cross){
  int i_set;
  gchar tmp[32];

  for(i_set=StdUb;i_set<=StdHa;i_set++){
    if(!strcmp(setups[i_set].cross,frame->crossd)){
      if( (((frame->crotan-d_cross)-setups[i_set].cross_scan)> -10)
	  && ((frame->crotan-d_cross)-setups[i_set].cross_scan)< 10) {
	frame->setup=g_strdup(setups[i_set].initial);
	return;
      }
    }
  }
  
  sprintf(tmp,"%5d/%s",(gint)frame->crotan,frame->crossd);
  frame->setup=g_strdup(tmp);
  return;
}


void make_top_table(typHLOG *hl){
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *combo;
  GtkAdjustment *adj;
  GtkWidget *spinner;
  GtkWidget *check;
  int col=0;
  

  if(flag_make_top)  gtk_widget_destroy(hl->top_table);
  else flag_make_top=TRUE;


  hl->top_table = gtk_table_new (1, 2, FALSE);
  gtk_table_set_col_spacings( GTK_TABLE(hl->top_table), 5 );
  gtk_container_set_border_width (GTK_CONTAINER (hl->top_table), 5);
  
  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_table_attach(GTK_TABLE(hl->top_table), hbox, 0, 1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  label = gtk_label_new ("Current/Next : ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  combo = gtk_combo_new();
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(combo)->entry),FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
  
  label = gtk_list_item_new_with_label ("HDSA");
  gtk_container_add (GTK_CONTAINER (GTK_COMBO(combo)->list),
		     label);
  gtk_widget_show(label);
  label = gtk_list_item_new_with_label ("hds20");
  gtk_container_add (GTK_CONTAINER (GTK_COMBO(combo)->list),
			 label);
  gtk_widget_show(label);
  switch(hl->file_head){
  case FILE_HDSA:
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry),
		       "HDSA");
    break;
  case FILE_hds:
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry),
		       "hds20");
    break;
  }
  gtk_widget_set_usize (combo, entry_height*4, -1);
  gtk_signal_connect (GTK_OBJECT(GTK_COMBO(combo)->entry),
		      "changed",
		      GTK_SIGNAL_FUNC (cc_file_head),
		      &hl->file_head);


  // Next ID
  hl->e_next = gtk_entry_new ();
  gtk_entry_set_editable(GTK_ENTRY(hl->e_next), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),hl->e_next,FALSE,FALSE,0);

  // Note
  hl->e_note = gtk_entry_new ();
  gtk_entry_set_editable(GTK_ENTRY(hl->e_note), TRUE);
  gtk_box_pack_start(GTK_BOX(hbox),hl->e_note,FALSE,FALSE,0);
  gtk_widget_set_usize (hl->e_note, entry_height*10, -1);
  gtk_signal_connect (GTK_OBJECT(hl->e_note),
		      "changed",
		      GTK_SIGNAL_FUNC (cc_get_entry),
		      &hl->next_note);
  
  check = gtk_check_button_new_with_label("Auto Scroll");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->scr_flag){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  gtk_signal_connect (GTK_OBJECT (check), "toggled",
		      GTK_SIGNAL_FUNC (cc_get_toggle),
		      &hl->scr_flag);
  

  hbox = gtk_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_table_attach(GTK_TABLE(hl->top_table), hbox, 0, 1, 1, 2,
		   GTK_FILL,GTK_SHRINK,0,0);

  check = gtk_check_button_new_with_label("I2");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->i2_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  gtk_signal_connect (GTK_OBJECT (check), "toggled",
		      GTK_SIGNAL_FUNC (cc_get_toggle),
		      &hl->i2_tmpfl);
  
  check = gtk_check_button_new_with_label("IS");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->is_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  gtk_signal_connect (GTK_OBJECT (check), "toggled",
		      GTK_SIGNAL_FUNC (cc_get_toggle),
		      &hl->is_tmpfl);
  
  check = gtk_check_button_new_with_label("CamZ");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->camz_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  gtk_signal_connect (GTK_OBJECT (check), "toggled",
		      GTK_SIGNAL_FUNC (cc_get_toggle),
		      &hl->camz_tmpfl);
  
  check = gtk_check_button_new_with_label("ImR");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->imr_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  gtk_signal_connect (GTK_OBJECT (check), "toggled",
		      GTK_SIGNAL_FUNC (cc_get_toggle),
		      &hl->imr_tmpfl);
  
  check = gtk_check_button_new_with_label("ADC");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  if(hl->adc_tmpfl){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),TRUE);
  }
  gtk_signal_connect (GTK_OBJECT (check), "toggled",
		      GTK_SIGNAL_FUNC (cc_get_toggle),
		      &hl->adc_tmpfl);
  

  label = gtk_label_new ("Date : ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hl->buf_year,
					    2000, hl->buf_year+10,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  gtk_widget_set_usize (spinner, entry_height*2.5, -1);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
  		      GTK_SIGNAL_FUNC (cc_get_adj),
  		      &hl->buf_year);

  adj = (GtkAdjustment *)gtk_adjustment_new(hl->buf_month,
					    1, 12, 1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  gtk_widget_set_usize (spinner, entry_height*2, -1);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
  		      GTK_SIGNAL_FUNC (cc_get_adj),
  		      &hl->buf_month);
  
  adj = (GtkAdjustment *)gtk_adjustment_new(hl->buf_day,
					    1, 31, 1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  gtk_widget_set_usize (spinner, entry_height*2, -1);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
  		      GTK_SIGNAL_FUNC (cc_get_adj),
  		      &hl->buf_day);

  label = gtk_label_new ("  dCross : ");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new(hl->d_cross,
					    -1000, 1000,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_entry_set_editable(GTK_ENTRY(&GTK_SPIN_BUTTON(spinner)->entry),
			 FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  gtk_widget_set_usize (spinner, entry_height*2.5, -1);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
  		      GTK_SIGNAL_FUNC (cc_get_adj),
  		      &hl->d_cross);

  hl->b_refresh=gtk_button_new_with_label("Refresh");
  gtk_box_pack_start(GTK_BOX(hbox),hl->b_refresh,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(hl->b_refresh),"clicked", 
		     GTK_SIGNAL_FUNC(refresh_table), 
		     (gpointer)hl);


  hl->w_status = gtk_label_new ("Starting...");
  gtk_box_pack_start(GTK_BOX(hbox),hl->w_status,TRUE,TRUE,0);


  gtk_widget_show_all(hl->top_table);
}


void make_frame_list(typHLOG *hl){
  GtkWidget *label;
  int col=0;
  

  if(flag_make_frame_list)  gtk_widget_destroy(frame_table);
  else flag_make_frame_list=TRUE;


  frame_table = gtk_table_new (10, hl->num+3, FALSE);
  gtk_table_set_col_spacings( GTK_TABLE(frame_table), 5 );
  gtk_container_set_border_width (GTK_CONTAINER (frame_table), 5);
  
  gtk_scrolled_window_add_with_viewport
    (GTK_SCROLLED_WINDOW (hl->scrwin), frame_table);

  label = gtk_label_new ("No.");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("Frame-ID");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("Object Name");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("HST");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("Exp.");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("secZ");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("Filter");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("Slit");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("Cross");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  label = gtk_label_new ("Bin");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);
  col++;

  if(hl->camz_flag){
    label = gtk_label_new ("CamZ");
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;
  }

  if(hl->i2_flag){
    label = gtk_label_new ("I2");
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;
  }

  if(hl->is_flag){
    label = gtk_label_new ("IS");
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;
  }

  if(hl->imr_flag){
    label = gtk_label_new ("ImR");
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    label = gtk_label_new ("SlitPA");
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;
  }

  if(hl->adc_flag){
    label = gtk_label_new ("ADC");
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

  }

  label = gtk_label_new ("Note");
  gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1, 0, 1,
		   GTK_FILL,GTK_SHRINK,0,0);

  gtk_widget_show_all(frame_table);
}


void update_frame_list(typHLOG *hl){
  int i, col=0;
  GtkWidget *label;
  GtkWidget *entry;
  gchar tmp[64];

  //// Current Condition

#ifdef DEBUG
    fprintf(stderr, "Start Load\n");
#endif
  if((hl->num_old==0)&&(hl->num!=0)){
    load_note(hl,TRUE);
  }
  else{
    load_note(hl,FALSE);
  }
#ifdef DEBUG
    fprintf(stderr, "End Load\n");
#endif

  for(i=hl->num_old;i<hl->num;i++){
    col=0;

    // Num
    sprintf(tmp,"%3d.",i+1);
    label = gtk_label_new (tmp);
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // ID
    label = gtk_label_new (hl->frame[i].id);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // Name
    label = gtk_label_new (hl->frame[i].name);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // HST
    label = gtk_label_new (hl->frame[i].hst);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // EXPTIME
    sprintf(tmp,"%4ds",hl->frame[i].exp);
    label = gtk_label_new (tmp);
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // SECZ
    sprintf(tmp,"%4.2f",hl->frame[i].secz);
    label = gtk_label_new (tmp);
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // FILTER
    sprintf(tmp,"%s/%s",hl->frame[i].fil1,hl->frame[i].fil2);
    label = gtk_label_new (tmp);
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // SLIT
    sprintf(tmp,"%4.2fx%5.2f",hl->frame[i].slt_wid,hl->frame[i].slt_len);
    label = gtk_label_new (tmp);
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // CROSS
    label = gtk_label_new (hl->frame[i].setup);
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // BINNING
    sprintf(tmp,"%1ldx%1ld",hl->frame[i].bin1,hl->frame[i].bin2);
    label = gtk_label_new (tmp);
    gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
    gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		     GTK_FILL,GTK_SHRINK,0,0);
    col++;

    // CAMZ
    if(hl->camz_flag){
      sprintf(tmp,"%4d",hl->frame[i].camz);
      label = gtk_label_new (tmp);
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
      col++;
    }

    // I2
    if(hl->i2_flag){
      label = gtk_label_new (hl->frame[i].i2);
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
      col++;
    }

    // ImgSlicer
    if(hl->is_flag){
      label = gtk_label_new (hl->frame[i].is);
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
      col++;
    }

    // ImR
    if(hl->imr_flag){
      label = gtk_label_new (hl->frame[i].imr);
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
      col++;

      sprintf(tmp,"%+6.2f",hl->frame[i].slt_pa);
      label = gtk_label_new (tmp);
      gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
      gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
      col++;

    }

    // ADC
    if(hl->adc_flag){
      label = gtk_label_new (hl->frame[i].adc);
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      gtk_table_attach(GTK_TABLE(frame_table), label, col, col+1,  i+1, i+2,
		       GTK_FILL,GTK_SHRINK,0,0);
      col++;
    }

    // Note
    hl->frame[i].w_note = gtk_entry_new ();
    gtk_table_attach(GTK_TABLE(frame_table), hl->frame[i].w_note,
		     col, col+1, i+1, i+2,
		     GTK_SHRINK,GTK_SHRINK,0,0);
    gtk_signal_connect (GTK_OBJECT(hl->frame[i].w_note),
			"changed",
			GTK_SIGNAL_FUNC (cc_get_note),
			(gpointer)&hl->frame[i].note);
    if(hl->frame[i].note.txt){
      gtk_entry_set_text(GTK_ENTRY(hl->frame[i].w_note),
			 hl->frame[i].note.txt);
    }

    if(hl->num_old==hl->num-1){
      if(hl->next_note){
	hl->frame[i].note.txt=g_strdup(hl->next_note);
	gtk_entry_set_text(GTK_ENTRY(hl->frame[i].w_note),
			   hl->next_note);
	gtk_entry_set_text(GTK_ENTRY(hl->e_note),"");
	hl->next_note=NULL;
      }
    }

    if(i==hl->num-1){
      sprintf(tmp,"%d - %2d (?)",
	      hl->frame[i].idnum+2,
	      hl->frame[i].idnum+3-
	      ((int)((hl->frame[i].idnum+3)/100))*100);
      gtk_entry_set_text(GTK_ENTRY(hl->e_next),tmp);
    }
  }

  if(hl->scr_flag){
    gtk_adjustment_set_value(
     gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(hl->scrwin)),
     gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(hl->scrwin))->upper-gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(hl->scrwin))->page_size);
  }

  gtk_widget_show_all(frame_table);
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
  float iswid;
  long det_id;
  float exptime,slt_wid,slt_len,crotan,camz,slt_pa;
  static char last_frame_id[256];
  int ret=0;


  fits_open_file(&fptr, inf, READONLY, &status);
  fits_read_key_lng(fptr, "DET-ID", &det_id, 0, &status);
  fits_read_key_str(fptr, "FRAMEID", frame_id, 0, &status);

  if(!strncmp(frame_id,"HDSA9999",8)) return;

  if((det_id==1)&&(strcmp(last_frame_id,frame_id)!=0)){  //Red
    hl->frame[hl->num].id=g_strdup(frame_id);
    cp=frame_id+5;
    hl->frame[hl->num].idnum=atol(cp);

#ifdef DEBUG
    printf("%s",hl->frame[hl->num].id);
#endif

    if(hl->num==0){
      fits_read_key_str(fptr, "OBSERVER", observer, 0, &status);
      hl->observer=g_strdup(observer);

      fits_read_key_str(fptr, "PROP-ID", prop, 0, &status);
      hl->prop=g_strdup(prop);
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

    fits_read_key_flt(fptr, "EXPTIME", &exptime, 0, &status);
    hl->frame[hl->num].exp=(guint)exptime;

    fits_read_key_str(fptr, "HST-STR", hst_str, 0, &status);
    strncpy(hst,hst_str,5);
    hst[5]='\0';
    hl->frame[hl->num].hst=g_strdup(hst);

    fits_read_key_flt(fptr, "SECZ", &hl->frame[hl->num].secz, 0, &status);

    fits_read_key_str(fptr, "FILTER01", filter01, 0, &status);
    hl->frame[hl->num].fil1=g_strdup(filter01);

    fits_read_key_str(fptr, "FILTER02", filter02, 0, &status);
    hl->frame[hl->num].fil2=g_strdup(filter02);

    fits_read_key_flt(fptr, "SLT-WID", &slt_wid, 0, &status);
    hl->frame[hl->num].slt_wid=(slt_wid*2);

    fits_read_key_flt(fptr, "SLT-LEN", &slt_len, 0, &status);
    hl->frame[hl->num].slt_len=(slt_len*2);
    
    fits_read_key_flt(fptr, "H_CROTAN", &crotan, 0, &status);
    hl->frame[hl->num].crotan=(crotan*3600);
    
    fits_read_key_str(fptr, "H_CROSSD", crossd, 0, &status);
    hl->frame[hl->num].crossd=g_strdup(crossd);


    select_color(&hl->frame[hl->num], hl->d_cross);

    fits_read_key_lng(fptr, "BIN-FCT1", &hl->frame[hl->num].bin1, 0, &status);
    fits_read_key_lng(fptr, "BIN-FCT2", &hl->frame[hl->num].bin2, 0, &status);

    fits_read_key_flt(fptr, "H_FOCUS", &camz, 0, &status);
    hl->frame[hl->num].camz=(gint)(camz*1000);

    fits_read_key_str(fptr, "H_I2POS", i2, 0, &status);
    hl->frame[hl->num].i2=g_strdup(i2);
    
    fits_read_key_str(fptr, "IMR-TYPE", imr, 0, &status);
    hl->frame[hl->num].imr=g_strdup(imr);

    if(fits_read_key_lng(fptr, "H_ISUNIT", &is, 0, &status)!=KEY_NO_EXIST){
      if(is==0){
	hl->frame[hl->num].is=g_strdup("NONE   ");
      }
      else{
	fits_read_key_flt(fptr, "H_ISWID", &iswid, 0, &status);
	fits_read_key_lng(fptr, "H_ISSLIC", &isslic, 0, &status);
	hl->frame[hl->num].is=g_strdup_printf("%4.2fx%d ",(float)iswid*2.,(int)isslic);
      }
    }
    else{
      hl->frame[hl->num].is=g_strdup("UNKNOWN");
    }

    fits_read_key_flt(fptr, "SLT-PA", &hl->frame[hl->num].slt_pa, 0, &status);
    
    fits_read_key_str(fptr, "ADC-TYPE", adc, 0, &status);
    hl->frame[hl->num].adc=g_strdup(adc);
    

    //printf("%s %s %4.0fs (%s)\n",frame_id,hst,exptime, obj_name);
    strcpy(last_frame_id,frame_id);
    hl->num++;
    ret=1;
  }

  qsort(hl->frame, hl->num, sizeof(FRAMEpara),
//	(int(*)(FRAMEpara*, FRAMEpara*))npcmp);
	(int(*)(const void*, const void*))npcmp);

  fits_close_file(fptr, &status);

  return(ret);
}

void SendMail(GtkWidget *w, gpointer gdata){
  typHLOG *hl;
  FILE *fp;
  gchar filename[256];
  gchar sub[256];
  gchar command_line[512];
  gint i;

  hl=(typHLOG *)gdata;

  sprintf(filename,  "%s/hdslog-%04d%02d%02d.txt",
	  g_get_home_dir(),hl->fr_year,hl->fr_month,hl->fr_day);

  if((fp=fopen(filename,"w"))==NULL){
    fprintf(stderr," File Read Error  \"%s\" \n", filename);
    exit(1);
  }
  //fprintf(fp,"From: HDS administrator <tajitsu@subaru.naoj.org>\n");
  //fprintf(fp,"To: %s\n",hl->mail);
  //fprintf(fp,"Subject: HDS Obs LOG  %04d-%02d-%02d (HST)\n\n",
  //	  hl->fr_year,hl->fr_month,hl->fr_day);

  fprintf(fp,"HDS Observation LOG  %04d-%02d-%02d (HST)\n\n",
	  hl->fr_year,hl->fr_month,hl->fr_day);
  sprintf(sub,"HDS Observation LOG  %04d-%02d-%02d (HST)",
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
    
    fprintf(fp,"%4d. %-15s %-20s %5s %4ds %4.2f %6s/%6s %4.2f/%5.2f %12s %1dx%1d",
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
    if(hl->i2_flag){
      fprintf(fp," %-7s",hl->frame[i].i2);
    }
    if(hl->is_flag){
      fprintf(fp," %-7s",hl->frame[i].is);
    }
    if(hl->imr_flag){
      fprintf(fp," %-4s %+6.2f",hl->frame[i].imr,hl->frame[i].slt_pa);
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

  fclose(fp);

  sprintf(command_line,"cat %s | %s -s \"%s\" %s",
	  filename, MAIL_COMMAND, sub, hl->mail);
  //  sprintf(command_line,"cat %s | %s -r \"%s\" -s \"%s\" %s",
  //	  filename, MAIL_COMMAND, DEF_FROM, sub, hl->mail);
  //  fprintf(stderr,"%s\n",command_line);
  ext_play(command_line);

  gtk_main_quit();

}


void ext_play(gchar *exe_command)
{
  static pid_t pid;
  gchar *cmdline;
  
  waitpid(pid,0,WNOHANG);
  if(strcmp(exe_command,"\0")!=0){
    cmdline=g_strdup(exe_command);
    if( (pid = fork()) == 0 ){
      system(cmdline);
      _exit(-1);
      signal(SIGCHLD,ChildTerm);
    }
  }
}


void do_mail (gpointer gdata, guint callback_action, GtkWidget *widget)
{
  typHLOG *hl;
  GtkWidget *dialog;
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

  dialog = gtk_dialog_new();
  gtk_container_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Send Mail");
  
  hbox=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
		     hbox,TRUE,TRUE,0);

  label=gtk_label_new("Mail Adresses");
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,TRUE,TRUE,0);
  gtk_widget_set_usize (entry, entry_height*20, -1);
  gtk_entry_set_text(GTK_ENTRY(entry),hl->mail);
  gtk_signal_connect (GTK_OBJECT(entry),
		      "changed",
		      GTK_SIGNAL_FUNC (cc_get_entry),
		      (gpointer)&hl->mail);


  button=gtk_button_new_with_label("Send");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(button),"pressed",
		     GTK_SIGNAL_FUNC(SendMail), 
		     (gpointer)hl);

  GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);

  button=gtk_button_new_with_label("Cancel");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(button),"pressed",
		     GTK_SIGNAL_FUNC(close_child_dialog), 
		     GTK_WIDGET(dialog));

  
  gtk_widget_show_all(dialog);
  gtk_main();

  gtk_widget_destroy(dialog);
  flagChildDialog=FALSE;
}

gint scan_command(gpointer gdata){
  typHLOG *hl;
//  static pid_t pid;

  hl=(typHLOG *)gdata;
  gtk_timeout_remove(hl->timer);
  
  //waitpid(pid,0,WNOHANG);
  //if( (pid=fork())==0){
    printdir(hl);
  //  exit(-1);
  //  signal(SIGCHLD,ChildTerm);
  //}

  hl->timer=gtk_timeout_add(READ_INTERVAL, scan_command, (gpointer)hl);
}

gint printdir(typHLOG *hl){
  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;
  int newflag=0;
  int i,n;

  change_label(hl->w_status, "Scanning...", black);

  
  if((dp = opendir(hl->data_dir)) == NULL){
    fprintf(stderr, "cannot open directory: %s\n",hl->data_dir);
    return;
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
	if( (statbuf.st_ctime>=hl->seek_time)
	    && (statbuf.st_ctime < hl->to_time)){
	  if(statbuf.st_ctime-hl->seek_time<5) sleep(5);
#ifdef DEBUG
//	  printf("0: %d %d %d %s",hl->fr_time,statbuf.st_ctime,hl->to_time,
//	   	 asctime(localtime(&hl->fr_time)));
	  printf("1: %s %s",entry->d_name,
	   	 asctime(localtime(&hl->fr_time)));
#endif
	  newflag+=printfits(hl,entry->d_name);
	}
      }
    }
    else if(hl->file_head==FILE_hds){
      if(!strncmp(entry->d_name,"hds20",5)){
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
    }
  }
    
  chdir("..");
  closedir(dp);

#ifdef DEBUG
  fprintf(stderr, "Start Save\n");
#endif
  save_note(hl);
#ifdef DEBUG
  fprintf(stderr, "End Save\n");
#endif
  
  hl->seek_time=time(NULL);
  
  update_frame_list(hl);
  hl->num_old=hl->num;

  change_label(hl->w_status, "", black);

#ifdef DEBUG
  fprintf(stderr, "End of Read: %s\n",hl->data_dir);
#endif

}

void do_quit (gpointer gdata, guint callback_action, GtkWidget *widget)
{
  gtk_main_quit();
}

GtkWidget *make_menu(typHLOG *hl){
  GtkAccelGroup *accel_group;
  GtkItemFactory *item_factory;
  GtkItemFactoryEntry menu_items[] = {
    {"/_File", NULL, NULL, 0, "<Branch>"},
    {"/File/Mail", "<control>M", do_mail, 0, NULL},
    {"/File/sep1", NULL, NULL, 0, "<Separator>"},
    {"/File/Quit", "<control>Q", do_quit, 0, NULL}
  };


  accel_group=gtk_accel_group_new();
  item_factory= gtk_item_factory_new(GTK_TYPE_MENU_BAR,
				     "<main>",
				     accel_group);
  gtk_item_factory_create_items(item_factory,
				(guint)4,
				menu_items, (gpointer)hl);

#ifdef USE_GTK2
//  _gtk_accel_group_attach(accel_group, GTK_OBJECT(hl->w_top));
#else
  gtk_accel_group_attach(accel_group, GTK_OBJECT(hl->w_top));
#endif
  return(gtk_item_factory_get_widget(item_factory, "<main>"));
}


void gui_init(typHLOG *hl){
  GtkWidget *menubar;

  // Main Window 
  hl->w_top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(hl->w_top), "destroy",
  		     GTK_SIGNAL_FUNC(gtk_main_quit),NULL);
  gtk_container_border_width(GTK_CONTAINER(hl->w_top),0);
  gtk_window_set_title(GTK_WINDOW(hl->w_top),"HDS Log Editor");

  hl->w_box = gtk_vbox_new(FALSE,0);
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
  gtk_widget_set_usize (hl->w_box, -1, entry_height*20);


  make_frame_list(hl);
  
  gtk_widget_show_all(hl->w_top);

}


int main(int argc, char* argv[]){
  typHLOG *hl;
  time_t t;
  struct tm *tmpt,tmpt2;
  gint i;

  hl=g_malloc0(sizeof(typHLOG));

  hl->data_dir=g_strdup(argv[1]);
  hl->num=0;
  hl->file_head=FILE_HDSA;
  hl->mail=g_strdup(DEF_MAIL);

  hl->d_cross=DELTA_CROSS;

  t = time(NULL);
  tmpt = localtime(&t);

  hl->fr_year=tmpt->tm_year+1900;
  hl->fr_month=tmpt->tm_mon+1;
  if(tmpt->tm_hour<9){
    hl->fr_day=tmpt->tm_mday-1;
  }
  else{
    hl->fr_day=tmpt->tm_mday;
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

  hl->scr_flag=TRUE;
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

  gtk_init(&argc, &argv);

  gdk_color_alloc(gdk_colormap_get_system(),&red);
  gdk_color_alloc(gdk_colormap_get_system(),&black);

  for(i=0;i<MAX_FRAME;i++){
    hl->frame[i].note.txt=NULL;
    hl->frame[i].note.time=0;
    hl->frame[i].note.auto_fl=FALSE;
  }
  
  gui_init(hl);

  hl->timer=gtk_timeout_add(READ_INTERVAL, scan_command, (gpointer)hl);

  gtk_main();
}

