#include "main.h"

#define BUFFSIZE 256

gint flat_ow_check;

void check_db_dir(gchar *wdir){
  gchar *db_dir, *com;
  db_dir=g_strconcat(wdir,
		     G_DIR_SEPARATOR_S,
		     "database",
		     NULL);
  if(access(db_dir, F_OK)!=0){
    com=g_strconcat("mkdir ",
		    db_dir,
		    NULL);
    system(com);
    g_free(com);
  }
  
  g_free(db_dir);
}

void db_check(typHLOG *hl, gint cal){
  gboolean ret=FALSE;
  gchar *fp_cal, *fp_db, *w_db;

  // CAL file check
  switch(cal){
  case CAL_AP:
    fp_cal=g_strconcat(hl->wdir,
		       G_DIR_SEPARATOR_S,
		       (hl->iraf_col==COLOR_R) ? 
		       hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
		       ".fits",
		       NULL);
    break;

  case CAL_THAR:
    fp_cal=g_strconcat(hl->wdir,
		       G_DIR_SEPARATOR_S,
		       (hl->iraf_col==COLOR_R) ? 
		       hl->thar_red[hl->iraf_hdsql_r] : hl->thar_blue[hl->iraf_hdsql_b],
		       ".fits",
		       NULL);
    break;

  case CAL_FLAT:
    fp_cal=g_strconcat(hl->wdir,
		       G_DIR_SEPARATOR_S,
		       (hl->iraf_col==COLOR_R) ? 
		       hl->thar_red[hl->iraf_hdsql_r] : hl->thar_blue[hl->iraf_hdsql_b],
		       ".fits",
		       NULL);
    break;
  }

  if(access(fp_cal, F_OK)==0){
    switch(cal){
    case CAL_AP:
      fp_db=g_strconcat(hl->wdir,
			G_DIR_SEPARATOR_S,
			"database",
			G_DIR_SEPARATOR_S,
			"ap",
			(hl->iraf_col==COLOR_R) ? 
			hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			NULL);
      break;

    case CAL_THAR:
      fp_db=g_strconcat(hl->wdir,
			G_DIR_SEPARATOR_S,
			"database",
			G_DIR_SEPARATOR_S,
			"ec",
			(hl->iraf_col==COLOR_R) ? 
			hl->thar_red[hl->iraf_hdsql_r] : hl->thar_blue[hl->iraf_hdsql_b],
			NULL);
      break;
    }

    switch(cal){
    case CAL_AP:
    case CAL_THAR:
      if(access(fp_db, F_OK)==0){
	ret=TRUE;
      }
      g_free(fp_db);
      break;

    case CAL_FLAT:
      ret=TRUE;
      break;
    }
  }

  g_free(fp_cal);

  switch(cal){
  case CAL_AP:
    if(hl->iraf_col==COLOR_R){
      hl->flag_ap_red[hl->iraf_hdsql_r]=ret;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_red), 
				   ret);
      gtk_widget_set_sensitive(hl->button_flat_red,ret);
      gtk_widget_set_sensitive(hl->button_thar_red,ret);
      gtk_widget_set_sensitive(hl->check_auto_red,ret);
      if(!ret){
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_red), 
				     ret);
      }
    }
    else{
      hl->flag_ap_blue[hl->iraf_hdsql_b]=ret;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_blue), 
				   ret);
      gtk_widget_set_sensitive(hl->button_flat_blue,ret);
      gtk_widget_set_sensitive(hl->button_thar_blue,ret);
      gtk_widget_set_sensitive(hl->check_auto_blue,ret);
      if(!ret){
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_blue), 
				     ret);
      }
    }
    break;

  case CAL_THAR:
    if(hl->iraf_col==COLOR_R){
      hl->flag_thar_red[hl->iraf_hdsql_r]=ret;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_red), 
				   ret);
    }
    else{
      hl->flag_thar_blue[hl->iraf_hdsql_b]=ret;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_blue), 
				   ret);
    }
    break;

  case CAL_FLAT:
    get_flat_scnm(hl);
    break;
  }
}

void get_flat_scnm(typHLOG *hl){
  gchar *c;
  gint ret;

  if(hl->iraf_col==COLOR_R){
    c=hl->flat_red[hl->iraf_hdsql_r];
  }
  else{
    c=hl->flat_blue[hl->iraf_hdsql_b];
  }

  if(!c){
    ret=FLAT_EX_NO;
  }
  else{
    if(g_strstr_len(c,-1,".sc.nm")){
      ret=FLAT_EX_SCNM;
    }
    else if(g_strstr_len(c,-1,".sc.fl")){
      ret=FLAT_EX_SCFL;
    }
    else if(g_strstr_len(c,-1,".sc")){
      ret=FLAT_EX_SC;
    }
    else if(c){
    ret=FLAT_EX_1;
    }
    else{
      ret=FLAT_EX_NO;
    }
  }

  if(hl->iraf_col==COLOR_R){
    hl->ex_flat_red[hl->iraf_hdsql_r]=ret;
    gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_red),
			     ret);
  }  
  else{
    hl->ex_flat_blue[hl->iraf_hdsql_b]=ret;
    gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_blue),
			     ret);
  }
}

void my_file_chooser_add_filter (GtkWidget *dialog, const gchar *name, ...)
{
  GtkFileFilter *filter;
  gchar *name_tmp;
  va_list args;
  gchar *pattern, *ptncat=NULL, *ptncat2=NULL;

  filter=gtk_file_filter_new();

  va_start(args, name);
  while(1){
    pattern=va_arg(args, gchar*);
    if(!pattern) break;
    gtk_file_filter_add_pattern(filter, pattern);
    if(!ptncat){
      ptncat=g_strdup(pattern);
    }
    else{
      if(ptncat2) g_free(ptncat2);
      ptncat2=g_strdup(ptncat);
      if(ptncat) g_free(ptncat);
      ptncat=g_strconcat(ptncat2,",",pattern,NULL);
    }
  }
  va_end(args);

  name_tmp=g_strconcat(name," [",ptncat,"]",NULL);
  gtk_file_filter_set_name(filter, name_tmp);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
  if(name_tmp) g_free(name_tmp);
  if(ptncat) g_free(ptncat);
  if(ptncat2) g_free(ptncat2);
}


void set_ap_label(typHLOG *hl){
  gchar *tmp;

  if(hl->iraf_col==COLOR_R){
    if(hl->ap_red[hl->iraf_hdsql_r]){
      tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			  hl->ap_red[hl->iraf_hdsql_r]);
    }
    else{
      tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
    }
  }
  else{
    if(hl->ap_blue[hl->iraf_hdsql_b]){
      tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			  hl->ap_blue[hl->iraf_hdsql_b]);
    }
    else{
      tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
    }
  }

  gtk_label_set_markup(GTK_LABEL(hl->label_edit_ap), tmp);
  g_free(tmp);
}

void set_flat_label(typHLOG *hl){
  gchar *tmp;

  if(hl->iraf_col==COLOR_R){
    if(hl->flat_red[hl->iraf_hdsql_r]){
      tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			  hl->flat_red[hl->iraf_hdsql_r]);
    }
    else{
      tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
    }
  }
  else{
    if(hl->flat_blue[hl->iraf_hdsql_b]){
      tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			  hl->flat_blue[hl->iraf_hdsql_b]);
    }
    else{
      tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
    }
  }

  gtk_label_set_markup(GTK_LABEL(hl->label_edit_flat), tmp);
  g_free(tmp);
}

void set_thar_label(typHLOG *hl){
  gchar *tmp;

  if(hl->iraf_col==COLOR_R){
    if(hl->thar_red[hl->iraf_hdsql_r]){
      tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			  hl->thar_red[hl->iraf_hdsql_r]);
    }
    else{
      tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
    }
  }
  else{
    if(hl->thar_blue[hl->iraf_hdsql_b]){
      tmp=g_strdup_printf("<span font_family=\"monospace\">%s.fits</span>",
			  hl->thar_blue[hl->iraf_hdsql_b]);
    }
    else{
      tmp=g_strdup("<span color=\"#999999\"><i>(Not set)</i></span>");
    }
  }

  gtk_label_set_markup(GTK_LABEL(hl->label_edit_thar), tmp);
  g_free(tmp);
}

gchar *get_refname(gchar *fp_file){
  gchar *bname=NULL, *ret, *c;

  bname=g_path_get_basename(fp_file);
  c=(char *)strrchr(bname,'.');
  ret=g_strndup(bname,strlen(bname)-strlen(c));
  if(bname) g_free(bname);

  return(ret);
}

gchar *get_refname_db(gchar *fp_file){
  gchar *bname=NULL, *ret, *c;

  bname=g_path_get_basename(fp_file);
  ret=g_strdup(bname+2);

  return(ret);
}

void set_setname(typHLOG *hl, gint i_sel){
  gchar *bin, *tbin, *setup;

  setup=get_setname_short(hl, i_sel);

  if(hl->iraf_col==COLOR_R){
    if(hl->setname_red[hl->iraf_hdsql_r]) 
      g_free(hl->setname_red[hl->iraf_hdsql_r]);

    //if(hl->ap_red[hl->iraf_hdsql_r]){
      bin=g_strdup_printf("%dx%d", 
			  hl->frame[i_sel].bin1,
			  hl->frame[i_sel].bin2);
      hl->setname_red[hl->iraf_hdsql_r]=g_strconcat(setup,
						    bin,
						    NULL);
      tbin=g_strdup_printf("?x%d", 
			  hl->frame[i_sel].bin2);
      hl->tharname_red[hl->iraf_hdsql_r]=g_strconcat(setup,
						     tbin,
						     NULL);
      //}
      //else{
      //hl->setname_red[hl->iraf_hdsql_r]=NULL;
      //}
  }
  else{
    if(hl->setname_blue[hl->iraf_hdsql_b]) 
      g_free(hl->setname_blue[hl->iraf_hdsql_b]);

    //if(hl->ap_blue[hl->iraf_hdsql_b]){
      bin=g_strdup_printf("%dx%d", 
			  hl->frame[i_sel].bin1,
			  hl->frame[i_sel].bin2);
      hl->setname_blue[hl->iraf_hdsql_b]=g_strconcat(setup,
						     bin,
						     NULL);
      tbin=g_strdup_printf("?x%d", 
			  hl->frame[i_sel].bin2);
      hl->tharname_blue[hl->iraf_hdsql_r]=g_strconcat(setup,
						      tbin,
						      NULL);
      //}
      //else{
      //hl->setname_blue[hl->iraf_hdsql_b]=NULL;
      //}
  }

  g_free(bin);
  g_free(tbin);
  g_free(setup);
}

void set_ql_frame_label(typHLOG *hl, GtkWidget *w, gboolean frame_flag){
  gchar *tmp;

  if(hl->iraf_col==COLOR_R){
    if(hl->ap_red[hl->iraf_hdsql_r]){
      tmp=g_strconcat(FRAME_QL_RED_LABEL,
		      "  :  <b>",
		      hl->setname_red[hl->iraf_hdsql_r],
		      "</b>",
		      NULL);
    }
    else{
      tmp=g_strdup(FRAME_QL_RED_LABEL);
    }
  }
  else{
    if(hl->ap_blue[hl->iraf_hdsql_b]){
      tmp=g_strconcat(FRAME_QL_BLUE_LABEL,
		      "  :  <b>",
		      hl->setname_blue[hl->iraf_hdsql_b],
		      "</b>",
		      NULL);
    }
    else{
      tmp=g_strdup(FRAME_QL_BLUE_LABEL);
    }
  }
  if(frame_flag){
    gtkut_frame_set_label(w, tmp);
  }
  else{
    gtk_label_set_markup(GTK_LABEL(w), tmp);
  }

  g_free(tmp);
}

void set_cal_frame_red(typHLOG *hl){
  gboolean ap_flag=hl->flag_ap_red[hl->iraf_hdsql_r];

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_red), ap_flag);
  gtk_widget_set_sensitive(hl->button_flat_red, ap_flag);
  gtk_widget_set_sensitive(hl->button_thar_red, ap_flag);
  gtk_widget_set_sensitive(hl->check_auto_red,ap_flag);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_red), FALSE);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_red), 
			       hl->flag_thar_red[hl->iraf_hdsql_r]);
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_red),
			   hl->ex_flat_red[hl->iraf_hdsql_r]);
}

void set_cal_frame_blue(typHLOG *hl){
  gboolean ap_flag=hl->flag_ap_blue[hl->iraf_hdsql_b];

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_blue), ap_flag);
  gtk_widget_set_sensitive(hl->button_flat_blue, ap_flag);
  gtk_widget_set_sensitive(hl->button_thar_blue, ap_flag);
  gtk_widget_set_sensitive(hl->check_auto_blue,ap_flag);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_blue), FALSE);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_blue), 
			       hl->flag_thar_blue[hl->iraf_hdsql_b]);
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_blue),
			   hl->ex_flat_blue[hl->iraf_hdsql_b]);
}

gchar *get_work_dir_sumda(typHLOG *hl){
  gchar *ret, *pdir;
  
  
  if(hl->upd_flag){
    ret=g_strconcat(G_DIR_SEPARATOR_S,
		    "work",
		    G_DIR_SEPARATOR_S,
		    hl->uname,
		    NULL);
  }
  else{
    pdir=g_path_get_dirname(hl->ddir);
    ret=g_strconcat(pdir,
		    G_DIR_SEPARATOR_S,
		    "work",
		    NULL);
    g_free(pdir);
    if(access(ret, F_OK)!=0){
      g_free(ret);
      ret=g_strdup(hl->ddir);
    }
  }
  return(ret);
}


gchar *get_share_dir_sumda(typHLOG *hl){
  gchar *ret, *pdir;
  
  
  if(hl->upd_flag){
    ret=g_strdup(SHARE_DIR);
  }
  else{
    ret=g_strconcat(g_get_home_dir(),
		    G_DIR_SEPARATOR_S,
		    "share",
		    G_DIR_SEPARATOR_S,
		    "HDS",
		    NULL);
    if(access(ret, F_OK)!=0){
      g_free(ret);
    }
  }
  return(ret);
}


gchar *get_data_dir_sumda(typHLOG *hl){
  gchar *ret;
  
  ret=g_strconcat(G_DIR_SEPARATOR_S,
		  "data",
		  G_DIR_SEPARATOR_S,
		  hl->uname,
		  NULL);
  return(ret);
}


gchar *get_uparm_dir_sumda(typHLOG *hl){
  gchar *ret;
  
  if(hl->upd_flag){
    ret=g_strconcat(G_DIR_SEPARATOR_S,
		    "home",
		    G_DIR_SEPARATOR_S,
		    hl->uname,
		    G_DIR_SEPARATOR_S,
		    "uparm",
		    NULL);
  }
  else{
    ret=g_strconcat(g_get_home_dir(),
		    G_DIR_SEPARATOR_S,
		    "uparm",
		    NULL);
  }
  return(ret);
}


void iraf_apall(GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  const gchar *c;
  gchar *tmp;
  gint winid;

  tmp=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, tmp);
  g_free(tmp);
  if(winid>0){
    c = gtk_entry_get_text(GTK_ENTRY(hl->entry_ap_id));
    tmp=g_strdup_printf("xdotool type --window %d \'%s\'", 
			winid, 
			c);
    send_xdo(hl, tmp);
    g_free(tmp);

    tmp=g_strdup_printf("xdotool key --window %d Return", winid);
    system(tmp);
    g_free(tmp);
  }
  else{
    xdo_error(hl, winid);
  }
}


void iraf_ecreidentify(GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  const gchar *c;
  gchar *tmp;
  gint winid;

  if(!hl->ref_frame){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "No reference frame is selected for ecreidentify.",
		  NULL);
    return;
  }

  tmp=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, tmp);
  g_free(tmp);
  if(winid>0){
    c = gtk_entry_get_text(GTK_ENTRY(hl->entry_thar_reid));
    tmp=g_strdup_printf("xdotool type --window %d \'%s\'", 
			winid, 
			c);
    send_xdo(hl, tmp);
    g_free(tmp);

    tmp=g_strdup_printf("xdotool key --window %d Return", winid);
    system(tmp);
    g_free(tmp);
  }
  else{
    xdo_error(hl, winid);
  }
}

void iraf_ecidentify(GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  const gchar *c;
  gchar *tmp;
  gint winid;

  tmp=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, tmp);
  g_free(tmp);
  if(winid>0){
    c = gtk_entry_get_text(GTK_ENTRY(hl->entry_thar_id));
    tmp=g_strdup_printf("xdotool type --window %d \'%s\'", 
			winid, 
			c);
    send_xdo(hl, tmp);
    g_free(tmp);

    tmp=g_strdup_printf("xdotool key --window %d Return", winid);
    system(tmp);
    g_free(tmp);
  }
  else{
    xdo_error(hl, winid);
  }
}

void clip_copy(GtkWidget *widget, gpointer gdata){
  GtkWidget *entry;
  GtkClipboard* clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  const gchar *c;

  entry=(GtkWidget *)gdata;

  c = gtk_entry_get_text(GTK_ENTRY(entry));
  gtk_clipboard_set_text (clipboard, c, strlen(c));
}


void edit_uparm(typHLOG *hl, gchar *key, gchar *type, gchar *data){
  FILE *fp_r, *fp_w;
  gchar buf[BUFFSIZE];
  gchar *infile;
  gchar *c;
  gchar *cp_com;
  gboolean str_flag;
  gint i;

  if(strcmp(type, "s")==0){
    str_flag=TRUE;
  }
  else{
    str_flag=FALSE;
  }
  infile=g_strconcat(hl->udir,
		     G_DIR_SEPARATOR_S,
		     "vo",
		     (hl->iraf_col==COLOR_R) ? 
		     hdsql_red[hl->iraf_hdsql_r] : hdsql_blue[hl->iraf_hdsql_b], 
		     ".par",
		     NULL);
  if(access(infile, F_OK)!=0){
    g_free(infile);

    infile=g_strconcat(hl->udir,
		       G_DIR_SEPARATOR_S,
		       "hds",
		       (hl->iraf_col==COLOR_R) ? 
		       hdsql_red[hl->iraf_hdsql_r] : hdsql_blue[hl->iraf_hdsql_b], 
		       ".par",
		       NULL);
    if(access(infile, F_OK)!=0){
      fprintf(stderr, "Cannot access to *hdsql.par in uparm dir. Skipped...\n");
      g_free(infile);
      return;
    }
  }
  
  if((fp_r=fopen(infile, "r"))!=NULL){
    if((fp_w=fopen(hl->uparmtmp, "wb"))!=NULL){
      while(!feof(fp_r)){
	if(fgets(buf,BUFFSIZE-1,fp_r)!=NULL){
	  if(strncmp(buf, key, strlen(key))==0){
	    c=(char *)strrchr(buf,',')+1;
	    if(str_flag){
	      fprintf(fp_w, "%s,%s,h,\"%s\",,,%s", key, type, data, c);
	    }
	    else{
	      fprintf(fp_w, "%s,%s,h,%s,,,%s", key, type, data, c);
	    }
	  }
	  else{
	    fprintf(fp_w, "%s", buf);
	  }
	}
      }

      fclose(fp_w);
    }
    fclose(fp_r);
  }

  cp_com=g_strconcat("cp ",hl->uparmtmp, " ", infile,NULL);
  system(cp_com);
  g_free(cp_com);

  g_free(infile);
}

gboolean close_popup(gpointer data)
{
  GtkWidget *dialog;

  dialog=(GtkWidget *)data;

  gtk_main_quit();
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  return(FALSE);
}

gboolean check_file(gpointer data)
{
  typHLOG *hl=(typHLOG *)data;

  if(access(hl->file_wait, F_OK)==0){
    gtk_main_quit();
    return(FALSE);
  }
  else{
    return(TRUE);
  }
}

gboolean destroy_popup(GtkWidget *w, GdkEvent *event, gint *data)
{
  g_source_remove(*data);
  gtk_main_quit();
  return(FALSE);
}

void popup_message(GtkWidget *parent, gchar* stock_id,gint delay, ...){
  va_list args;
  gchar *msg1;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gint timer;

  va_start(args, delay);

  if(delay>0){
    dialog = gtk_dialog_new();
  }
  else{
    dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Message",
					 GTK_WINDOW(parent),
					 GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					 "_OK",GTK_RESPONSE_OK,
#else
					 GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					 NULL);
  }
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_window_set_title(GTK_WINDOW(dialog),"HDS Log Editor : Message");

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);
#endif

  if(delay>0){
    timer=g_timeout_add(delay*1000, (GSourceFunc)close_popup,
			(gpointer)dialog);
    g_signal_connect(dialog,"delete-event",G_CALLBACK(destroy_popup), &timer);
  }

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtkut_label_new(msg1);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),
		       label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);

  if(delay>0){
    gtk_main();
  }
  else{
    gtk_dialog_run(GTK_DIALOG(dialog));
    if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  }
}


gboolean wait_for_file(typHLOG *hl, gchar *msg){
  gchar *tmp;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gint timer;

  dialog = gtk_dialog_new();

  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_window_set_title(GTK_WINDOW(dialog),"HDS Log Editor : Message");
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

  timer=g_timeout_add(1000, 
		      (GSourceFunc)check_file,
		      (gpointer)hl);
  g_signal_connect(dialog,"delete-event",G_CALLBACK(destroy_popup), &timer);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("dialog-information", 
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_INFO,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label=gtkut_label_new(msg);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  tmp=g_strconcat("    <span font_family=\"monospace\">",
		  hl->file_wait,
		  "</span>",
		  NULL);
  label=gtkut_label_new(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  gtk_widget_show_all(dialog);

  gtk_main();

  if(GTK_IS_WIDGET(dialog)){
    gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
    gtk_widget_destroy(dialog);

    return(TRUE);
  }
  else{
    return(FALSE);
  }
}


gboolean popup_dialog(GtkWidget *parent, gchar* stock_id, ...){
  va_list args;
  gchar *msg1;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gboolean ret=FALSE;

  va_start(args, stock_id);

  dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Dialog",
				       GTK_WINDOW(parent),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_No",GTK_RESPONSE_NO,
				       "_Yes",GTK_RESPONSE_YES,
#else
				       GTK_STOCK_NO,GTK_RESPONSE_NO,
				       GTK_STOCK_YES,GTK_RESPONSE_YES,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO); 

#if !GTK_CHECK_VERSION(2,21,8)
  gtk_dialog_set_has_separator(GTK_DIALOG(dialog),FALSE);
#endif

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  while(1){
    msg1=va_arg(args,gchar*);
    if(!msg1) break;
   
    label=gtkut_label_new(msg1);
#ifdef USE_GTK3
    gtk_widget_set_halign (label, GTK_ALIGN_START);
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
    gtk_box_pack_start(GTK_BOX(vbox),
		       label,TRUE,TRUE,0);
  }

  va_end(args);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
    ret=TRUE;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  return(ret);
}


gboolean write_dialog(typHLOG *hl, gchar* stock_id, gchar* file_str){
  gchar *tmp;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gboolean ret=FALSE;

  dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Dialog",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_Cancel",GTK_RESPONSE_CANCEL,
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name (stock_id,
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (stock_id,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  tmp=g_strdup_printf("Create a file for %s : ", file_str);
  label=gtk_label_new(tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox),
		     hbox,TRUE,TRUE,0);

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox),entry,FALSE,FALSE,0);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),40);
  gtk_entry_set_text(GTK_ENTRY(entry), hl->file_write);
  g_signal_connect (entry,
		    "changed",
		    G_CALLBACK(cc_get_entry),
		    &hl->file_write);

  label=gtkut_label_new("<span font_family=\"monospace\">.fits</span>");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),
		     label,TRUE,TRUE,0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    ret=TRUE;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  return(ret);
}


gboolean ow_check(typHLOG *hl, gchar* file_in){
  gchar *tmp;
  gchar *filename;
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *pixmap;
  GtkWidget *hbox;
  GtkWidget *vbox;
  gboolean ret=FALSE;

  filename=g_strconcat(hl->wdir,
		       G_DIR_SEPARATOR_S,
		       file_in, 
		       ".fits",
		       NULL);
  if(access(filename, F_OK)!=0){
    g_free(filename);
    return(TRUE);
  }

  dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Dialog",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_No",GTK_RESPONSE_NO,
				       "_Yes",GTK_RESPONSE_YES,
#else
				       GTK_STOCK_NO,GTK_RESPONSE_NO,
				       GTK_STOCK_YES,GTK_RESPONSE_YES,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO); 

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

#ifdef USE_GTK3
  pixmap=gtk_image_new_from_icon_name ("dialog-warning", 
				       GTK_ICON_SIZE_DIALOG);
#else
  pixmap=gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING,
				   GTK_ICON_SIZE_DIALOG);
#endif

  gtk_box_pack_start(GTK_BOX(hbox), pixmap,FALSE, FALSE, 0);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
  gtk_box_pack_start(GTK_BOX(hbox),vbox,FALSE, FALSE, 0);

  label=gtk_label_new("The file");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);
  
  tmp=g_strdup_printf("    <span font_family=\"monospace\">%s</span>", 
		      filename);
  label=gtkut_label_new(tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  label=gtk_label_new("already exists.");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  label=gtk_label_new(" ");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  label=gtk_label_new("Do you want to overwrite it?");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(vbox),
		     label,TRUE,TRUE,0);

  gtk_widget_show_all(dialog);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
    unlink(filename);
    ret=TRUE;
  }

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
  while(my_main_iteration(FALSE));

  g_free(filename);
  return(ret);
}


gchar *get_indir(typHLOG *hl){
  gchar*ret;

  ret=g_strconcat(hl->ddir,
		  G_DIR_SEPARATOR_S,
		  "HDSA00",
		  NULL);

  return(ret);
}


// Return Win Number (only 1 window)
//        0  (No window)
//        -1 (2 or more window)
gint get_xdowin(typHLOG *hl, gchar *win_name){
  gchar *tmp;
  FILE *fp;
  gint ret=0, i=0;
  gchar *buf=NULL;
  
  tmp=g_strdup_printf("xdotool search --onlyvisible --all --name \'%s\' > %s", win_name, hl->xdotmp);
  system(tmp);
  g_free(tmp);

  if((fp=fopen(hl->xdotmp, "r"))!=NULL){
    while(!feof(fp)){
      if((buf=fgets_new(fp))!=NULL){
	ret=(gint)g_strtod(buf,NULL);
	i++;
	g_free(buf);
      }
    }
    fclose(fp);
  }
  else{
    return(ret);
  }
  unlink(hl->xdotmp);

  if(i==1){
    return(ret);
  }
  else{
    return(-i);
  }
}


void xdo_error(typHLOG *hl, gint winid){
  gchar *tmp;

  if (winid==0){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "No HDS_IRAF terminal has been found in your desktop.",
		  " ",
		  "Please start up it by",
		  "      <b>IRAF</b> - <b>Check/Restart xgterm</b>",
		  "in the menu.",
		  NULL);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> \"HDS_IRAF\" windows have been found in your desktop.", 
			-winid);

    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please close all unused xgterm,",
		  "except one for your HDS_IRAF terminal.",
		  NULL);
    g_free(tmp);
  }
}


gboolean send_xdo(typHLOG *hl, gchar *command){
  gchar *tmp, *tmp2;
  FILE *fp;
  gint i=0;
  gchar *buf=NULL;
  gboolean ret;

  tmp=g_strconcat(command,
		  " 2> ",
		  hl->xdotmp,
		  NULL);
  system(tmp);
  
  if((fp=fopen(hl->xdotmp, "r"))!=NULL){
    while(!feof(fp)){
      if((buf=fgets_new(fp))!=NULL){
	if(strlen(buf)>2){
	  i++;
	}
	g_free(buf);
      }
    }
    fclose(fp);
  }
  
  // re-atempt
  if(i>0){
    i=0;
    
    system(tmp);
    
    if((fp=fopen(hl->xdotmp, "r"))!=NULL){
      while(!feof(fp)){
	if((buf=fgets_new(fp))!=NULL){
	  if(strlen(buf)>2){
	    i++;
	  }
	  g_free(buf);
	}
      }
      fclose(fp);
    }
    
    if(i>0){
      tmp2=g_strconcat("     <span font_family=\"monospace\">",
		       tmp,
		       "</span>",
		       NULL);
      
      popup_message(hl->w_top, 
#ifdef USE_GTK3
		    "dialog-warning", 
#else
		    GTK_STOCK_DIALOG_WARNING,
#endif
		    -1,
		    "<b>Error</b>: Failed to send the command via \"xdotool\".",
		      " ",
		    tmp2,
		    NULL);
      
      g_free(tmp2);
      
      ret=FALSE;
    }
    else{
      ret=TRUE;
    }
  }
  else{
      ret=TRUE;
  }
  
  g_free(tmp);

  return(ret);
}


gint iraf_quit_tk(typHLOG *hl){
  gint tkid;
  gchar *tmp;

  tkid=get_xdowin(hl, "irafterm");
  if(tkid>0){
    tmp=g_strdup_printf("xdotool key --window %d q", tkid);
    if(!send_xdo(hl, tmp)) return(0);
    g_free(tmp);

    tmp=g_strdup_printf("xdotool key --window %d Return", tkid);
    if(!send_xdo(hl, tmp)) return(0);
    g_free(tmp);

    return(1);
  }
  else if(tkid==0){
    return(0);
  }
  else if(tkid<0){
    tmp=g_strdup_printf("<b>%d</b> \"irafterm\" windows have been found in your desktop.", 
			-tkid);

    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please close all unused irafterm windows,",
		  "except one invoked from your HDS_IRAF terminal (= xgterm).",
		  NULL);
    g_free(tmp);

    return(tkid);
  }
  else{
    return(tkid);
  }
}

gint get_cnt(typHLOG *hl, gint i_file){
  gchar *cnt_file;
  FILE *fp;
  gint ret=-1;

  cnt_file=g_strdup_printf("%s/result/H%d_cnt",
			   hl->wdir, i_file);
  if((fp=fopen(cnt_file, "r"))!=NULL){
    fscanf(fp, "%d", &ret);
    fclose(fp);
  }

  g_free(cnt_file);
  return(ret);
}

void iraf_obj(typHLOG *hl, gint i_sel, gint i_file){
  gint winid;
  gchar *tmp, *tmp1, *tmp2, *tmp3, *indir, *mask;
  gint tkid;
  gboolean splot_flag=FALSE;
  gboolean ap_ret=FALSE, thar_ret=FALSE, flat_ret=FALSE, is_ret=FALSE;
  gboolean sc_inte0, sc_resi0, sc_edit0, sc_fitt0;
  gboolean ap_inte0, ap_resi0, ap_edit0, is_plot0, ge_cnt0;
  gint ap_llim0, ap_ulim0, is_stx0, is_edx0, sp_line0, ge_stx0, ge_edx0;
  gdouble ge_low0, ge_high0;
  gint i_reduced_old;
  gint cnt;

  if(hl->iraf_col==COLOR_R){
    if(hl->flag_ap_red[hl->iraf_hdsql_r]){
      ap_ret=TRUE;
    }
    if(hl->flag_thar_red[hl->iraf_hdsql_r]){
      thar_ret=TRUE;
    }
    if(hl->ex_flat_red[hl->iraf_hdsql_r]>=FLAT_EX_1){
      flat_ret=TRUE;
    }

    sc_inte0=hl->qp_r[hl->iraf_hdsql_r].sc_inte;
    sc_resi0=hl->qp_r[hl->iraf_hdsql_r].sc_resi;
    sc_edit0=hl->qp_r[hl->iraf_hdsql_r].sc_edit;
    sc_fitt0=hl->qp_r[hl->iraf_hdsql_r].sc_fitt;

    ap_inte0=hl->qp_r[hl->iraf_hdsql_r].ap_inte;
    ap_resi0=hl->qp_r[hl->iraf_hdsql_r].ap_resi;
    ap_edit0=hl->qp_r[hl->iraf_hdsql_r].ap_edit;
    ap_llim0=hl->qp_r[hl->iraf_hdsql_r].ap_llim;
    ap_ulim0=hl->qp_r[hl->iraf_hdsql_r].ap_ulim;

    is_plot0=hl->qp_r[hl->iraf_hdsql_r].is_plot;
    is_stx0 =hl->qp_r[hl->iraf_hdsql_r].is_stx;
    is_edx0 =hl->qp_r[hl->iraf_hdsql_r].is_edx;

    ge_cnt0 =hl->qp_r[hl->iraf_hdsql_r].ge_cnt;
    ge_stx0 =hl->qp_r[hl->iraf_hdsql_r].ge_stx;
    ge_edx0 =hl->qp_r[hl->iraf_hdsql_r].ge_edx;
    ge_low0 =hl->qp_r[hl->iraf_hdsql_r].ge_low;
    ge_high0 =hl->qp_r[hl->iraf_hdsql_r].ge_high;

    sp_line0=hl->qp_r[hl->iraf_hdsql_r].sp_line;
  }
  else{
    if(hl->flag_ap_blue[hl->iraf_hdsql_b]){
      ap_ret=TRUE;
    }
    if(hl->flag_thar_blue[hl->iraf_hdsql_b]){
      thar_ret=TRUE;
    }
    if(hl->ex_flat_blue[hl->iraf_hdsql_b]>=FLAT_EX_1){
      flat_ret=TRUE;
    }

    sc_inte0=hl->qp_b[hl->iraf_hdsql_b].sc_inte;
    sc_resi0=hl->qp_b[hl->iraf_hdsql_b].sc_resi;
    sc_edit0=hl->qp_b[hl->iraf_hdsql_b].sc_edit;
    sc_fitt0=hl->qp_b[hl->iraf_hdsql_b].sc_fitt;

    ap_inte0=hl->qp_b[hl->iraf_hdsql_b].ap_inte;
    ap_resi0=hl->qp_b[hl->iraf_hdsql_b].ap_resi;
    ap_edit0=hl->qp_b[hl->iraf_hdsql_b].ap_edit;
    ap_llim0=hl->qp_b[hl->iraf_hdsql_b].ap_llim;
    ap_ulim0=hl->qp_b[hl->iraf_hdsql_b].ap_ulim;

    is_plot0=hl->qp_b[hl->iraf_hdsql_b].is_plot;
    is_stx0 =hl->qp_b[hl->iraf_hdsql_b].is_stx;
    is_edx0 =hl->qp_b[hl->iraf_hdsql_b].is_edx;

    ge_cnt0 =hl->qp_b[hl->iraf_hdsql_b].ge_cnt;
    ge_stx0 =hl->qp_b[hl->iraf_hdsql_b].ge_stx;
    ge_edx0 =hl->qp_b[hl->iraf_hdsql_b].ge_edx;
    ge_low0 =hl->qp_b[hl->iraf_hdsql_b].ge_low;
    ge_high0 =hl->qp_b[hl->iraf_hdsql_b].ge_high;

    sp_line0=hl->qp_b[hl->iraf_hdsql_b].sp_line;
  }

  if(strncmp(hl->frame[i_sel].is, "NONE", strlen("NONE"))!=0){
    if(flat_ret){
      is_ret=TRUE;
    }
  }

  if(!ap_ret){  // No ap
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-error", 
#else
		  GTK_STOCK_DIALOG_ERROR,
#endif
		  -1,
		  "<b>No aperture reference</b> is found for this hdsql.",
		  " ",
		  "Please make an aperture reference to make a quick look of the spectrum.",
		  "If possible, please prepare",
		  "    - (normalied) Flat",
		  "    - wavelength calibrated ThAr",
		  "frames also to make better quick looks.",
		  NULL);
      return;
  }
  

  if((tkid=iraf_quit_tk(hl))<0){
    return;
  }

  tmp=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, tmp);
  g_free(tmp);
  if(winid>0){
    if(hl->file_wait) g_free(hl->file_wait);
    hl->file_wait=g_strdup_printf("%s/result/H%d.fits",
			     hl->wdir, i_file);
    if(access(hl->file_wait, F_OK)==0){
      if(!popup_dialog(hl->w_top, 
#ifdef USE_GTK3
		       "dialog-information", 
#else
		       GTK_STOCK_DIALOG_INFO,
#endif
		       "Found a reduced spectrum in \"result/\" directory.",
		       " ",
		       "Do you want to reprocess it?",
		       NULL)){
	splot_flag=TRUE;
      }
      else{
	unlink(hl->file_wait);
      }
    }


    if(splot_flag){
      tmp=g_strdup_printf("xdotool type --window %d \'splot %s %d\'", 
			  winid, hl->file_wait, sp_line0);
      send_xdo(hl, tmp);
      g_free(tmp);
    }
    else{
      indir=get_indir(hl);
      edit_uparm(hl,"indirec","s",indir);

      mask=g_strdup_printf(MASK_FILE,
			   hl->sdir,
			   G_DIR_SEPARATOR_S,
			   hl->frame[i_sel].bin1,
			   hl->frame[i_sel].bin2,
			   (hl->iraf_col==COLOR_R) ? "R" : "B");
      edit_uparm(hl,"mb_refer","s",mask);

      tmp1=g_strdup_printf("xdotool type --window %d \'%s %d indir=%s overw+ oversca+ biassub- maskbad+ mb_refe=%s linear+ cosmicr+ scatter+ sc_refe=%s sc_inte%s sc_rece- sc_resi%s sc_edit%s sc_fitt%s xtalk-", 
			   winid, 
			   (hl->iraf_col==COLOR_R) ? 
			   hdsql_red[hl->iraf_hdsql_r] : hdsql_blue[hl->iraf_hdsql_b], 
			   i_file, 
			   indir,
			   mask,
			   (hl->iraf_col==COLOR_R) ? 
			   hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			   (sc_inte0) ? "+" : "-",
			   (sc_resi0) ? "+" : "-",
			   (sc_edit0) ? "+" : "-",
			   (sc_fitt0) ? "+" : "-"); 

      if(is_ret){  // Image Slicer
	tmp2=g_strdup_printf(" flat- fl_refe=%s apall- ap_refe=%s ap_rece- isecf+ is_plot%s is_stx=%d is_edx=%d", 
			     (hl->iraf_col==COLOR_R) ? 
			     hl->flat_red[hl->iraf_hdsql_r] : hl->flat_blue[hl->iraf_hdsql_b], 
			     (hl->iraf_col==COLOR_R) ? 
			     hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			     (is_plot0) ? "+" : "-",
			     is_stx0, is_edx0); 
      }
      else{ // Normal Slit
	if(flat_ret){
	  tmp2=g_strdup_printf(" flat+ fl_refe=%s apall+ ap_refe=%s ap_inte%s ap_rece- ap_resi%s ap_edit%s ap_llim=%d ap_ulim=%d isecf-", 
			       (hl->iraf_col==COLOR_R) ? 
			       hl->flat_red[hl->iraf_hdsql_r] : hl->flat_blue[hl->iraf_hdsql_b], 
			       (hl->iraf_col==COLOR_R) ? 
			       hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			       (ap_inte0) ? "+" : "-",
			       (ap_resi0) ? "+" : "-",
			       (ap_edit0) ? "+" : "-",
			       ap_llim0, ap_ulim0);
	}
	else{
	  tmp2=g_strdup_printf(" flat- apall+ ap_refe=%s ap_inte%s ap_rece- ap_resi%s ap_edit%s ap_llim=%d ap_ulim=%d isecf-", 
			       (hl->iraf_col==COLOR_R) ? 
			       hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			       (ap_inte0) ? "+" : "-",
			       (ap_resi0) ? "+" : "-",
			       (ap_edit0) ? "+" : "-",
			       ap_llim0, ap_ulim0);
	}
      }

      if(thar_ret){
	tmp3=g_strdup_printf(" wavecal+ wv_refer=%s remask+ rvcorre+ getcnt%s ge_stx=%d ge_edx=%d ge_low=%.1lf ge_high=%.1lf splot+ sp_line=%d\'", 
			     (hl->iraf_col==COLOR_R) ? 
			     hl->thar_red[hl->iraf_hdsql_r] : hl->thar_blue[hl->iraf_hdsql_b],
			     (ge_cnt0) ? "+" : "-",
			     ge_stx0, ge_edx0,ge_low0,ge_high0,
			     sp_line0);
      }
      else{
	tmp3=g_strdup_printf(" wavecal- remask- rvcorre- getcnt%s ge_stx=%d ge_edx=%d  ge_low=%.1lf ge_high=%.1lf splot+ sp_line=%d\'", 
			     (ge_cnt0) ? "+" : "-",
			     ge_stx0, ge_edx0,ge_low0,ge_high0,
			     sp_line0);
      }

      tmp=g_strconcat(tmp1,tmp2,tmp3,NULL);
      send_xdo(hl, tmp);
      g_free(tmp);
      g_free(tmp1);
      g_free(tmp2);
      g_free(tmp3);
      g_free(indir);
      g_free(mask);
    }

    tmp=g_strdup_printf("xdotool key --window %d Return", winid);
    send_xdo(hl, tmp);
    g_free(tmp);

    if(!wait_for_file(hl, "Waiting for a Quick Look result file...")){
      return;
    }

    i_reduced_old=abs(hl->i_reduced)-1;

    if(hl->iraf_col==COLOR_R){
      hl->frame[i_sel].qlr=TRUE;
      hl->i_reduced=i_sel+1;
    }
    else{
      hl->frame[i_sel].qlb=TRUE;
      hl->i_reduced=-(i_sel+1);
    }

    if(ge_cnt0){
      cnt=get_cnt(hl, i_file);
      hl->frame[i_sel].note.cnt=cnt;
      if(cnt>0) save_note(hl);
    }

    if(i_reduced_old>=0){
      frame_tree_update_ql(hl, i_reduced_old);
    }
    frame_tree_update_ql(hl, i_sel);

    if((hl->remote_flag) && (!splot_flag)){
      scp_write_remote(hl, hl->file_wait);
    }
  }
  else{
    xdo_error(hl, winid);
  }
}


void ql_obj_foreach (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i, i_file;
  gchar *c;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  c=hl->frame[i].id+strlen("HDSA00");
  if(hl->iraf_col==COLOR_R){
    i_file=(gint)g_strtod(c, NULL);
  }
  else{
    i_file=(gint)g_strtod(c, NULL)+1;
  }
  iraf_obj(hl, i, i_file);
}


void ql_obj_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_R;

  gtk_tree_selection_selected_foreach (selection, ql_obj_foreach, (gpointer)hl);

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}

void ql_obj_blue(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_B;

  gtk_tree_selection_selected_foreach (selection, ql_obj_foreach, (gpointer)hl);

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}


///////////////// Ap ///////////////////

void iraf_ap(typHLOG *hl, gint i_sel, gint i_file){
  gint winid;
  gchar *tmp, *set_name, *mask, *indir, *ecfile=NULL;
  gint tkid;

  if((tkid=iraf_quit_tk(hl))<0){
    return;
  }

  tmp=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, tmp);
  g_free(tmp);
  if(winid>0){
    set_name=get_setname_long(hl, i_sel);

    if(hl->file_write) g_free(hl->file_write);
    hl->file_write=g_strconcat("Ap",
			       set_name,
			       NULL);
    g_free(set_name);

    if(!write_dialog(hl, 
#ifdef USE_GTK3
		     "dialog-information", 
#else
		     GTK_STOCK_DIALOG_INFO,
#endif
		     "Aperture reference")){
      return;
    }

    if(ow_check(hl, hl->file_write)){
      mask=g_strdup_printf(MASK_FILE,
			   hl->sdir,
			   G_DIR_SEPARATOR_S,
			   hl->frame[i_sel].bin1,
			   hl->frame[i_sel].bin2,
			   (hl->iraf_col==COLOR_R) ? "R" : "B");
      edit_uparm(hl,"mb_refer","s",mask);
      
      indir=get_indir(hl);
      edit_uparm(hl,"indirec","s",indir);
      
      if(hl->file_wait) g_free(hl->file_wait);
      hl->file_wait=g_strdup_printf("%s/H%dom.fits",
				    hl->wdir,
				    i_file);
      if(access(hl->file_wait, F_OK)==0){
	unlink(hl->file_wait);
      }
      
      tmp=g_strdup_printf("xdotool type --window %d \'%s %d indirec=%s batch- overw+ oversca+ biassub- maskbad+ mb_refer=%s linear- cosmicr- scatter- xtalk- flat- apall- isecf- wavecal- remask- rvcorre- splot-\'", 
			  winid, 
			  (hl->iraf_col==COLOR_R) ? 
			  hdsql_red[hl->iraf_hdsql_r] : hdsql_blue[hl->iraf_hdsql_b], 
			  i_file, 
			  indir,
			  mask);
      send_xdo(hl, tmp);
      g_free(tmp);
      g_free(indir);
      g_free(mask);
      
      tmp=g_strdup_printf("xdotool key --window %d Return", winid);
      system(tmp);
      g_free(tmp);
      
      if(!wait_for_file(hl, "Waiting for an aperture file...")){
	return;
      }

      tmp=g_strdup_printf("xdotool type --window %d \'imcopy H%06dom %s\'", 
			  winid, 
			  i_file,
			  hl->file_write);
      send_xdo(hl, tmp);
      g_free(tmp);
      
      tmp=g_strdup_printf("xdotool key --window %d Return", winid);
      send_xdo(hl, tmp);
      g_free(tmp);
      

      if(hl->iraf_col==COLOR_R){
	if(hl->ap_red[hl->iraf_hdsql_r]) g_free(hl->ap_red[hl->iraf_hdsql_r]);
	hl->ap_red[hl->iraf_hdsql_r]=g_strdup(hl->file_write);
      }
      else{
	if(hl->ap_blue[hl->iraf_hdsql_b]) g_free(hl->ap_blue[hl->iraf_hdsql_b]);
	hl->ap_blue[hl->iraf_hdsql_b]=g_strdup(hl->file_write);
      }
      set_setname(hl, i_sel);
      


      if(hl->iraf_col==COLOR_R){
	set_ql_frame_label(hl, hl->frame_ql_red, TRUE);
	hl->frame[i_sel].qlr=TRUE;
      }
      else{
	set_ql_frame_label(hl, hl->frame_ql_blue, TRUE);
	hl->frame[i_sel].qlb=TRUE;
      }
      frame_tree_update_ql(hl, i_sel);

      ecfile=g_strdup_printf("%s/%s.ec.fits",
			     hl->wdir,
			     hl->file_write);
      if(access(ecfile, F_OK)==0){
	tmp=g_strdup_printf("xdotool type --window %d \'imdelete %s.ec\'", 
			    winid, 
			    hl->file_write);
	send_xdo(hl, tmp);
	g_free(tmp);
	
	tmp=g_strdup_printf("xdotool key --window %d Return", winid);
	send_xdo(hl, tmp);
	g_free(tmp);
      }
      g_free(ecfile);
    }
    else{ // Skip overwrite
      if(hl->iraf_col==COLOR_R){
	if(hl->ap_red[hl->iraf_hdsql_r]) g_free(hl->ap_red[hl->iraf_hdsql_r]);
	hl->ap_red[hl->iraf_hdsql_r]=g_strdup(hl->file_write);
      }
      else{
	if(hl->ap_blue[hl->iraf_hdsql_b]) g_free(hl->ap_blue[hl->iraf_hdsql_b]);
	hl->ap_blue[hl->iraf_hdsql_b]=g_strdup(hl->file_write);
      }

      set_setname(hl, i_sel);

      if(hl->iraf_col==COLOR_R){
	set_ql_frame_label(hl, hl->frame_ql_red, TRUE);
      }
      else{
	set_ql_frame_label(hl, hl->frame_ql_blue, TRUE);
      }
    }
    
  
    // apall commands
    {
      GtkWidget *dialog, *label, *combo, *hbox, *entry, *button, *table, *check;
      gint norm_mode=0;
      
      dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Aperture Trace",
					   GTK_WINDOW(hl->w_top),
					   GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					   "_OK",GTK_RESPONSE_OK,
#else
					   GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					   NULL);
      gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
      gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
      gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			 hbox,FALSE, FALSE, 0);

      label=gtk_label_new("Commands for apall     ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_START);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

      db_check(hl, CAL_AP);

      check = gtk_check_button_new_with_label("Have database?");
      gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				   (hl->iraf_col==COLOR_R)
				   ? hl->flag_ap_red[hl->iraf_hdsql_r]
				   : hl->flag_ap_blue[hl->iraf_hdsql_b]);
      gtk_widget_set_sensitive(check, FALSE);


      table = gtkut_table_new (2, 2, FALSE, 5, 5, 5);
      gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			 table,FALSE, FALSE, 0);

      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtkut_table_attach(table, hbox, 0, 2, 0, 2,
			 GTK_FILL,GTK_SHRINK,0,0);

      hl->entry_ap_id = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hl->entry_ap_id,FALSE,FALSE,0);
      gtk_entry_set_width_chars(GTK_ENTRY(hl->entry_ap_id),120);
      tmp=g_strdup_printf("apall %s output=%s.ec t_order=4 t_niterate=5 width=15 radius=30 recenter+ resize+",
			  hl->file_write, hl->file_write);
      gtk_entry_set_text(GTK_ENTRY(hl->entry_ap_id), tmp);
      g_free(tmp);
      gtk_editable_set_editable(GTK_EDITABLE(hl->entry_ap_id),TRUE);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("work","document-open");
#else
      button=gtkut_button_new_from_stock("work", GTK_STOCK_FIND);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      g_signal_connect (button, "clicked",
     			G_CALLBACK (ref1_ap), (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
      tmp=g_strconcat("Find Ref in ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      NULL);
      gtk_widget_set_tooltip_text(button,tmp);
      g_free(tmp);
#endif
      
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("share","document-open");
#else
      button=gtkut_button_new_from_stock("share", GTK_STOCK_FIND);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      g_signal_connect (button, "clicked",
     			G_CALLBACK (ref2_ap), (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
      tmp=g_strconcat("Find Ref in ",
		      hl->sdir,
		      G_DIR_SEPARATOR_S,
		      NULL);
      gtk_widget_set_tooltip_text(button,tmp);
      g_free(tmp);
#endif

#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name(NULL,"media-playback-start");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_MEDIA_PLAY);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      g_signal_connect (button, "clicked",
			G_CALLBACK (iraf_apall), (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,
				  "Run apall");
#endif

      gtk_widget_show_all(dialog);

      gtk_dialog_run(GTK_DIALOG(dialog));
      if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

      db_check(hl, CAL_AP);
    }
  }
  else{
    xdo_error(hl, winid);
  }
}


void ql_ap_foreach (GtkTreeModel *model, GtkTreePath *path, 
		    GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i, i_file;
  gchar *c;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  c=hl->frame[i].id+strlen("HDSA00");
  if(hl->iraf_col==COLOR_R){
    i_file=(gint)g_strtod(c, NULL);
  }
  else{
    i_file=(gint)g_strtod(c, NULL)+1;
  }

  iraf_ap(hl, i, i_file);
}


void ql_ap_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_R;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_ap_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to make Aperture reference file.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}


void ql_ap_blue(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_B;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_ap_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to make Aperture reference file.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}

///////////////// Flat ///////////////////

void iraf_flat(typHLOG *hl, gint i_sel, gint i_file){
  gchar *tmp, *indir, *set_name;
  gint winid;
  gint tkid;
  gchar *mask;

  if(strcmp(hl->frame[i_sel].name,"FLAT")!=0){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "The selected frame is not \"<b>FLAT</b>\".",
		  "Aborting...\".",
		  NULL);
    flat_ow_check=FLAT_OW_ABORT;
    return;
  }

  switch(flat_ow_check){
  case FLAT_OW_NONE:
    set_name=get_setname_long(hl, i_sel);
    
    if(hl->file_write) g_free(hl->file_write);
    hl->file_write=g_strconcat("Flat",
			       set_name,
			       NULL);
    g_free(set_name);
    
    if(!write_dialog(hl, 
#ifdef USE_GTK3
		     "dialog-information", 
#else
		     GTK_STOCK_DIALOG_INFO,
#endif
		     "Aperture reference")){
      flat_ow_check=FLAT_OW_ABORT;
      return;
    }

    if(hl->iraf_col==COLOR_R){
      if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
      hl->flat_red[hl->iraf_hdsql_r]=g_strdup(hl->file_write);
      hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_1;
      gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_red),
			       FLAT_EX_1);
    }
    else{
      if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
      hl->flat_blue[hl->iraf_hdsql_b]=g_strdup(hl->file_write);
      hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_1;
      gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_blue),
			       FLAT_EX_1);
    }

    if(!ow_check(hl, hl->file_write)){
      flat_ow_check=FLAT_OW_SKIP;
      db_check(hl, CAL_FLAT);
      return;
    }
    else{
      flat_ow_check=FLAT_OW_GO;
    }
    break;
    
  case FLAT_OW_SKIP:
    db_check(hl, CAL_FLAT);
    return;
    break;
    
  case FLAT_OW_GO:
    break;;
  }


  if((tkid=iraf_quit_tk(hl))<0){
    db_check(hl, CAL_FLAT);
    return;
  }

  tmp=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, tmp);
  g_free(tmp);
  if(winid>0){
    mask=g_strdup_printf(MASK_FILE,
			 hl->sdir,
			 G_DIR_SEPARATOR_S,
			 hl->frame[i_sel].bin1,
			 hl->frame[i_sel].bin2,
			 (hl->iraf_col==COLOR_R) ? "R" : "B");
    indir=get_indir(hl);
    
    if(hl->file_wait) g_free(hl->file_wait);
    hl->file_wait=g_strdup_printf("%s/H%domlx.fits",
				  hl->wdir,
				  i_file);
    if(access(hl->file_wait, F_OK)==0){
      unlink(hl->file_wait);
    }

    tmp=g_strdup_printf("xdotool type --window %d \'%s %d indirec=%s batch- overw+ oversca+ biassub- maskbad+ mb_refe=%s linear+ cosmicr- scatter- xtalk+ flat- apall- isecf- wavecal- remask- rvcorre- splot-\'", 
			winid, 
			(hl->iraf_col==COLOR_R) ? 
			hdsql_red[hl->iraf_hdsql_r] : hdsql_blue[hl->iraf_hdsql_b], 
			i_file,
			indir,
			mask);
    send_xdo(hl, tmp);
    g_free(tmp);
    g_free(indir);
    g_free(mask);
      
    tmp=g_strdup_printf("xdotool key --window %d Return", winid);
    send_xdo(hl, tmp);
    g_free(tmp);

    if(!wait_for_file(hl, "Waiting for a flat file...")){
      db_check(hl, CAL_FLAT);
      return;
    }

    tmp=g_strdup_printf("echo %d >> %s", i_file, hl->flattmp1);
    system(tmp);
    g_free(tmp);

    tmp=g_strdup_printf("echo H%domlx >> %s", i_file, hl->flattmp2);
    system(tmp);
    g_free(tmp);

    if(hl->iraf_col==COLOR_R){
      hl->frame[i_sel].qlr=TRUE;
    }
    else{
      hl->frame[i_sel].qlb=TRUE;
    }
    frame_tree_update_ql(hl, i_sel);
  }

  db_check(hl, CAL_FLAT);
}


void make_flat(typHLOG *hl){
  gint winid;
  gchar *tmp;
  gchar *scfile=NULL, *nmfile=NULL;
  gint tkid;
  gint i_sel;

  if((tkid=iraf_quit_tk(hl))<0){
    return;
  }

  tmp=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, tmp);
  g_free(tmp);
  if(winid>0){
    if(flat_ow_check==FLAT_OW_GO){
      tmp=g_strdup_printf("cp %s %s/%s.lst", hl->flattmp1, hl->wdir, hl->file_write);
      system(tmp);
      g_free(tmp);
      unlink(hl->flattmp1);
      
      tmp=g_strdup_printf("cp %s %s/%s_omlx.lst", hl->flattmp2, hl->wdir, hl->file_write);
      system(tmp);
      g_free(tmp);
      unlink(hl->flattmp2);

      if(hl->file_wait) g_free(hl->file_wait);
      hl->file_wait=g_strconcat(hl->wdir,
				G_DIR_SEPARATOR_S,
				hl->file_write,
				".fits",
				NULL);

      if(access(hl->file_wait, F_OK)==0){
	unlink(hl->file_wait);
      }

      tmp=g_strdup_printf("xdotool type --window %d \'imcombine @%s_omlx.lst %s combine=ave reject=minmax\'", 
			  winid, 
			  hl->file_write,
			  hl->file_write);
      send_xdo(hl, tmp);
      g_free(tmp);
      
      tmp=g_strdup_printf("xdotool key --window %d Return", winid);
      send_xdo(hl, tmp);
      g_free(tmp);

      if(!wait_for_file(hl, "Waiting for an imcombined flat file...")){
	return;
      }
    }

    if(hl->iraf_col==COLOR_R){
      if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
      hl->flat_red[hl->iraf_hdsql_r]=g_strdup(hl->file_write);
      hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_1;
      gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_red),
			       FLAT_EX_1);
    }
    else{
      if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
      hl->flat_blue[hl->iraf_hdsql_b]=g_strdup(hl->file_write);
      hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_1;
      gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_blue),
			       FLAT_EX_1);
    }
    edit_uparm(hl,"fl_refer","s",hl->file_write);


    //apscatter
    scfile=g_strconcat(hl->file_write, ".sc", NULL);

    if(hl->file_wait) g_free(hl->file_wait);
    hl->file_wait=g_strconcat(hl->wdir,
			      G_DIR_SEPARATOR_S,
			      scfile,
			      ".fits",
			      NULL);

    if(ow_check(hl, scfile)){
      if(access(hl->file_wait, F_OK)==0){
	unlink(hl->file_wait);
      }

      tmp=g_strdup_printf("xdotool type --window %d \'apscatter %s %s find- edit+ resize+ recenter- trace- refer=%s\'", 
			  winid, 
			  hl->file_write,
			  scfile,
			  (hl->iraf_col==COLOR_R) ? 
			  hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b]);
      send_xdo(hl, tmp);
      g_free(tmp);
      
      tmp=g_strdup_printf("xdotool key --window %d Return", winid);
      send_xdo(hl, tmp);
      g_free(tmp);
    }

    if(!wait_for_file(hl, "Waiting for an apscattered flat file...")){
      return;
    }

    if(hl->iraf_col==COLOR_R){
      if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
      hl->flat_red[hl->iraf_hdsql_r]=g_strdup(scfile);
      hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_SC;
      gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_red),
			       FLAT_EX_SC);
    }
    else{
      if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
      hl->flat_blue[hl->iraf_hdsql_b]=g_strdup(scfile);
      hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_SC;
      gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_blue),
			       FLAT_EX_SC);
    }
    edit_uparm(hl,"fl_refer","s",scfile);
    
    
    // apnormalize or apflatten
    {
      GtkWidget *dialog, *label, *combo;
      gint norm_mode=0;
      
      dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Dialog",
					   GTK_WINDOW(hl->w_top),
					   GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					   "_Cancel",GTK_RESPONSE_CANCEL,
					   "_OK",GTK_RESPONSE_OK,
#else
					   GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					   NULL);
      gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
      gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
      gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

      label=gtk_label_new("Normalize Flat image by");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_START);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			 label,FALSE, FALSE, 0);

      {
	GtkListStore *store;
	GtkTreeIter iter, iter_set;	  
	GtkCellRenderer *renderer;
      
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "apnormalize",
			   1, 0, -1);
	if(norm_mode==0) iter_set=iter;
    
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, "apflatten",
			   1, 1, -1);
	if(norm_mode==1) iter_set=iter;
   
	combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			   combo,FALSE, FALSE, 0);
	g_object_unref(store);
    
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    	
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
	gtk_widget_show(combo);
	g_signal_connect (combo,"changed",G_CALLBACK(cc_get_combo_box),
			  &norm_mode);
      }

      gtk_widget_show_all(dialog);

      if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
	if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

	if(norm_mode==0){ // apnormalize
	  nmfile=g_strconcat(scfile, ".nm", NULL);

	  if(hl->file_wait) g_free(hl->file_wait);
	  hl->file_wait=g_strconcat(hl->wdir,
				    G_DIR_SEPARATOR_S,
				    nmfile,
				    ".fits",
				    NULL);
	  if(ow_check(hl, nmfile)){
	    if(access(hl->file_wait, F_OK)==0){
	      unlink(hl->file_wait);
	    }
	    tmp=g_strdup_printf("xdotool type --window %d \'apnormalize %s %s find- edit+ resize+ recenter- trace- refer=%s order=15 niterate=5\'", 
				winid, 
				scfile,
				nmfile,
				(hl->iraf_col==COLOR_R) ? 
				hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b]);
	    send_xdo(hl, tmp);
	    g_free(tmp);
      
	    tmp=g_strdup_printf("xdotool key --window %d Return", winid);
	    send_xdo(hl, tmp);
	    g_free(tmp);
	  }

	  if(!wait_for_file(hl, "Waiting for an apnormalized flat file...")){
	    return;
	  }

	  if(hl->iraf_col==COLOR_R){
	    if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
	    hl->flat_red[hl->iraf_hdsql_r]=g_strdup(nmfile);
	    hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_SCNM;
	    gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_red),
				     FLAT_EX_SCNM);
	  }
	  else{
	    if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
	    hl->flat_blue[hl->iraf_hdsql_b]=g_strdup(nmfile);
	    hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_SCNM;
	    gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_blue),
				     FLAT_EX_SCNM);
	  }  
	  edit_uparm(hl,"fl_refer","s",nmfile);
	}
	else{ // apflatten
	  nmfile=g_strconcat(scfile, ".fl", NULL);

	  if(hl->file_wait) g_free(hl->file_wait);
	  hl->file_wait=g_strconcat(hl->wdir,
				    G_DIR_SEPARATOR_S,
				    nmfile,
				    ".fits",
				    NULL);
	  if(ow_check(hl, nmfile)){
	    if(access(hl->file_wait, F_OK)==0){
	      unlink(hl->file_wait);
	    }
	    tmp=g_strdup_printf("xdotool type --window %d \'apflatten %s %s find- edit+ resize+ recenter- trace- refer=%s order=15 niterate=5\'", 
				winid, 
				scfile,
				nmfile,
				(hl->iraf_col==COLOR_R) ? 
				hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b]);
	    send_xdo(hl, tmp);
	    g_free(tmp);
      
	    tmp=g_strdup_printf("xdotool key --window %d Return", winid);
	    send_xdo(hl, tmp);
	    g_free(tmp);
	  }

	  if(!wait_for_file(hl, "Waiting for an apflattened flat file...")){
	    return;
	  }

	  if(hl->iraf_col==COLOR_R){
	    if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
	    hl->flat_red[hl->iraf_hdsql_r]=g_strdup(nmfile);
	    hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_SCFL;
	    gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_red),
				     FLAT_EX_SCFL);
	  }
	  else{
	    if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
	    hl->flat_blue[hl->iraf_hdsql_b]=g_strdup(nmfile);
	    hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_SCFL;
	    gtk_combo_box_set_active(GTK_COMBO_BOX(hl->combo_flat_blue),
				     FLAT_EX_SCFL);
	  }
	  edit_uparm(hl,"fl_refer","s",nmfile);
	}
      }
      else{
	if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
      }
    }

    if(scfile) g_free(scfile);
    if(nmfile) g_free(nmfile);
  }
  else{
    xdo_error(hl, winid);
  }
}

void ql_flat_foreach (GtkTreeModel *model, GtkTreePath *path, 
		      GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i, i_file;
  gchar *c;

  if(flat_ow_check==FLAT_OW_ABORT) return;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  c=hl->frame[i].id+strlen("HDSA00");
  if(hl->iraf_col==COLOR_R){
    i_file=(gint)g_strtod(c, NULL);
  }
  else{
    i_file=(gint)g_strtod(c, NULL)+1;
  }

  iraf_flat(hl, i, i_file);
}

void ql_flat_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_R;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  flat_ow_check=FLAT_OW_NONE;

  gtk_tree_selection_selected_foreach (selection, ql_flat_foreach, (gpointer)hl);

  if(flat_ow_check!=FLAT_OW_ABORT) make_flat(hl);

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}

void ql_flat_blue(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_B;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  flat_ow_check=FLAT_OW_NONE;
  
  gtk_tree_selection_selected_foreach (selection, ql_flat_foreach, (gpointer)hl);

  if(flat_ow_check!=FLAT_OW_ABORT) make_flat(hl);

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}


///////////////// ThAr /////////////////

void iraf_thar(typHLOG *hl, gint i_sel, gint i_file){
  gint winid;
  gchar *tmp, *set_name, *mask, *indir, *ecfile=NULL;
  gint tkid;
  gint ap_sz=16;
  

  if(strcmp(hl->frame[i_sel].name,"COMPARISON")!=0){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "The selected frame is not \"<b>COMPARISON</b>\".",
		  NULL);
    return;
  }

  if((tkid=iraf_quit_tk(hl))<0){
    return;
  }

  tmp=g_strdup_printf("HDS_IRAF_%s",hl->uname);
  winid=get_xdowin(hl, tmp);
  g_free(tmp);
  if(winid>0){
    set_name=get_setname_long(hl, i_sel);
    ap_sz/=hl->frame[i_sel].bin1;

    if(hl->file_write) g_free(hl->file_write);
    hl->file_write=g_strconcat("ThAr",
			       set_name,
			       NULL);
    g_free(set_name);

    if(!write_dialog(hl, 
#ifdef USE_GTK3
		     "dialog-information", 
#else
		     GTK_STOCK_DIALOG_INFO,
#endif
		     "Wavelength reference")){
      return;
    }

    if(ow_check(hl, hl->file_write)){   
      if(hl->iraf_col==COLOR_R){
	hl->flag_thar_red[hl->iraf_hdsql_r]=FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_red), 
				     FALSE);
      }
      else{
	hl->flag_thar_blue[hl->iraf_hdsql_b]=FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_blue), 
				     FALSE);
      }
      
      mask=g_strdup_printf(MASK_FILE,
			   hl->sdir,
			   G_DIR_SEPARATOR_S,
			   hl->frame[i_sel].bin1,hl->frame[i_sel].bin2,
			   (hl->iraf_col==COLOR_R) ? "R" : "B");
      edit_uparm(hl,"mb_refer","s",mask);
      
      indir=get_indir(hl);
      edit_uparm(hl,"indirec","s",indir);
      
      if(hl->file_wait) g_free(hl->file_wait);
      hl->file_wait=g_strdup_printf("%s/H%dom_ec.fits",
				    hl->wdir,
				    i_file);
      if(access(hl->file_wait, F_OK)==0){
	unlink(hl->file_wait);
      }
      
      tmp=g_strdup_printf("xdotool type --window %d \'%s %d indirec=%s batch- overw+ oversca+ biassub- maskbad+ mb_refer=%s linear- cosmicr- scatter- xtalk- flat- apall+ ap_refer=%s ap_inte+ ap_resi- ap_rece- ap_edit+ ap_trac- ap_llim=-%d ap_ulim=%d isecf- wavecal- remask- rvcorre- splot-\'", 
			  winid, 
			  (hl->iraf_col==COLOR_R) ? 
			  hdsql_red[hl->iraf_hdsql_r] : hdsql_blue[hl->iraf_hdsql_b], 
			  i_file, 
			  indir,
			  mask,
			  (hl->iraf_col==COLOR_R) ? 
			  hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			  ap_sz,
			  ap_sz);
      send_xdo(hl, tmp);
      g_free(tmp);
      g_free(indir);
      g_free(mask);
      
      tmp=g_strdup_printf("xdotool key --window %d Return", winid);
      system(tmp);
      g_free(tmp);
      
      if(!wait_for_file(hl, "Waiting for a ThAr file...")){
	return;
      }
      
      if(hl->iraf_col==COLOR_R){
	hl->frame[i_sel].qlr=TRUE;
      }
      else{
	hl->frame[i_sel].qlb=TRUE;
      }
      frame_tree_update_ql(hl, i_sel);
      
      tmp=g_strdup_printf("xdotool type --window %d \'imcopy H%06dom_ec %s\'", 
			  winid, 
			  i_file,
			  hl->file_write);
      send_xdo(hl, tmp);
      g_free(tmp);
      
      tmp=g_strdup_printf("xdotool key --window %d Return", winid);
      send_xdo(hl, tmp);
      g_free(tmp);
      
      
      if(hl->iraf_col==COLOR_R){
	if(hl->thar_red[hl->iraf_hdsql_r]) g_free(hl->thar_red[hl->iraf_hdsql_r]);
	hl->thar_red[hl->iraf_hdsql_r]=g_strdup(hl->file_write);
      }
      else{
	if(hl->thar_blue[hl->iraf_hdsql_b]) g_free(hl->thar_blue[hl->iraf_hdsql_b]);
	hl->thar_blue[hl->iraf_hdsql_b]=g_strdup(hl->file_write);
      }
    }
    else{  // Skip overwrite
      if(hl->iraf_col==COLOR_R){
	if(hl->thar_red[hl->iraf_hdsql_r]) g_free(hl->thar_red[hl->iraf_hdsql_r]);
	hl->thar_red[hl->iraf_hdsql_r]=g_strdup(hl->file_write);
      }
      else{
	if(hl->thar_blue[hl->iraf_hdsql_b]) g_free(hl->thar_blue[hl->iraf_hdsql_b]);
	hl->thar_blue[hl->iraf_hdsql_b]=g_strdup(hl->file_write);
      }
    }


    // ecidentify commands
    {
      GtkWidget *dialog, *label, *combo, *hbox, *entry, *button, *table, *check;
      gint norm_mode=0;
      
      dialog = gtk_dialog_new_with_buttons("HD SLog Editor : Dialog",
					   GTK_WINDOW(hl->w_top),
					   GTK_DIALOG_MODAL,
#ifdef USE_GTK3
					   "_OK",GTK_RESPONSE_OK,
#else
					   GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
					   NULL);
      gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
      gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
      gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			 hbox,FALSE, FALSE, 0);

      label=gtk_label_new("Commands for Wavelength identification   ");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_START);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

      db_check(hl, CAL_THAR);

      check = gtk_check_button_new_with_label("Have database?");
      gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				   (hl->iraf_col==COLOR_R)
				   ? hl->flag_thar_red[hl->iraf_hdsql_r]
				   : hl->flag_thar_blue[hl->iraf_hdsql_b]);
      gtk_widget_set_sensitive(check, FALSE);

      table = gtkut_table_new (2, 2, FALSE, 5, 5, 5);
      gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
			 table,FALSE, FALSE, 0);

      label=gtk_label_new("ecreidentify w/ reference :");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_START);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtkut_table_attach(table, label, 0, 1, 0, 1,
			 GTK_FILL,GTK_SHRINK,0,0);
  
      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtkut_table_attach(table, hbox, 1, 2, 0, 1,
			 GTK_FILL,GTK_SHRINK,0,0);
      
      if(hl->ref_frame) g_free(hl->ref_frame);
      hl->ref_frame=NULL;

      hl->entry_thar_reid = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hl->entry_thar_reid,FALSE,FALSE,0);
      gtk_entry_set_width_chars(GTK_ENTRY(hl->entry_thar_reid),80);
      tmp=g_strdup_printf("ecreidentify %s shift=0 refer=",
			  hl->file_write);
      gtk_entry_set_text(GTK_ENTRY(hl->entry_thar_reid), tmp);
      g_free(tmp);
      gtk_editable_set_editable(GTK_EDITABLE(hl->entry_thar_reid), TRUE);

#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("work","document-open");
#else
      button=gtkut_button_new_from_stock("work", GTK_STOCK_FIND);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      g_signal_connect (button, "clicked",
     			G_CALLBACK (ref1_thar), (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
      tmp=g_strconcat("Find Ref in ",
		      hl->wdir,
		      G_DIR_SEPARATOR_S,
		      NULL);
      gtk_widget_set_tooltip_text(button,tmp);
      g_free(tmp);
#endif
      
#ifdef USE_GTK3
      button=gtkut_button_new_from_icon_name("share","document-open");
#else
      button=gtkut_button_new_from_stock("share", GTK_STOCK_FIND);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      g_signal_connect (button, "clicked",
     			G_CALLBACK (ref2_thar), (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
      tmp=g_strconcat("Find Ref in ",
		      hl->sdir,
		      G_DIR_SEPARATOR_S,
		      NULL);
      gtk_widget_set_tooltip_text(button,tmp);
      g_free(tmp);
#endif

#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name(NULL,"media-playback-start");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_MEDIA_PLAY);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      g_signal_connect (button, "clicked",
			G_CALLBACK (iraf_ecreidentify), (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,
				  "Run ecreidentify");
#endif

      label=gtk_label_new("ecidentify :");
#ifdef USE_GTK3
      gtk_widget_set_halign (label, GTK_ALIGN_START);
      gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
      gtkut_table_attach(table, label, 0, 1, 1, 2,
			 GTK_FILL,GTK_SHRINK,0,0);
      
      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      hbox = gtkut_hbox_new(FALSE,2);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
      gtkut_table_attach(table, hbox, 1, 2, 1, 2,
			 GTK_FILL,GTK_SHRINK,0,0);

      hl->entry_thar_id = gtk_entry_new ();
      gtk_box_pack_start(GTK_BOX(hbox),hl->entry_thar_id,FALSE,FALSE,0);
      gtk_entry_set_width_chars(GTK_ENTRY(hl->entry_thar_id),80);
      tmp=g_strdup_printf("ecidentify %s",
			  hl->file_write);
      gtk_entry_set_text(GTK_ENTRY(hl->entry_thar_id), tmp);
      g_free(tmp);
      gtk_editable_set_editable(GTK_EDITABLE(hl->entry_thar_id),FALSE);

#ifdef USE_GTK3      
      button=gtkut_button_new_from_icon_name(NULL,"media-playback-start");
#else
      button=gtkut_button_new_from_stock(NULL,GTK_STOCK_MEDIA_PLAY);
#endif
      gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
      g_signal_connect (button, "clicked",
			G_CALLBACK (iraf_ecidentify), (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
      gtk_widget_set_tooltip_text(button,
				  "Run ecidentify");
#endif

      gtk_widget_show_all(dialog);

      gtk_dialog_run(GTK_DIALOG(dialog));
      if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

      db_check(hl, CAL_THAR);
    }
  }
  else{
    xdo_error(hl, winid);
  }
}


void ql_thar_foreach (GtkTreeModel *model, GtkTreePath *path, 
		    GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i, i_file;
  gchar *c;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  c=hl->frame[i].id+strlen("HDSA00");
  if(hl->iraf_col==COLOR_R){
    i_file=(gint)g_strtod(c, NULL);
  }
  else{
    i_file=(gint)g_strtod(c, NULL)+1;
  }

  iraf_thar(hl, i, i_file);
}


void ql_thar_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_R;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_thar_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to make Wavelength reference file.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}


void ql_thar_blue(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_B;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_thar_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to make Wavelength reference file.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}

////////////// Find ///////////////////

void hdslog_OpenFile(typHLOG *hl, guint mode){
  GtkWidget *fdialog;
  gchar *tmp;
  gchar *fp_file=NULL;
  gchar **tgt_file;

  switch(mode){
  case OPEN_AP:
    tmp=g_strdup("HDS Log Editor : Select an Aperture File");
    if(hl->iraf_col==COLOR_R){
      if(hl->ap_red[hl->iraf_hdsql_r]){
	fp_file=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    hl->ap_red[hl->iraf_hdsql_r],
			    ".fits",
			    NULL);
      }
    }
    else{
      if(hl->ap_blue[hl->iraf_hdsql_b]){
	fp_file=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    hl->ap_blue[hl->iraf_hdsql_b],
			    ".fits",
			    NULL);
      }
    }
    break;

  case OPEN_FLAT:
    tmp=g_strdup("HDS Log Editor : Select a Flat Image File");
    if(hl->iraf_col==COLOR_R){
      if(hl->flat_red[hl->iraf_hdsql_r]){
	fp_file=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    hl->flat_red[hl->iraf_hdsql_r],
			    ".fits",
			    NULL);
      }
    }
    else{
      if(hl->flat_blue[hl->iraf_hdsql_b]){
	fp_file=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    hl->flat_blue[hl->iraf_hdsql_b],
			    ".fits",
			    NULL);
      }
    }
    break;

  case OPEN_THAR:
    tmp=g_strdup("HDS Log Editor : Select a ThAr File");
    if(hl->iraf_col==COLOR_R){
      if(hl->thar_red[hl->iraf_hdsql_r]){
	fp_file=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    hl->thar_red[hl->iraf_hdsql_r],
			    ".fits",
			    NULL);
      }
    }
    else{
      if(hl->thar_blue[hl->iraf_hdsql_b]){
	fp_file=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    hl->thar_blue[hl->iraf_hdsql_b],
			    ".fits",
			    NULL);
      }
    }
    break;

  case REF1_AP:
    tmp=g_strdup("HDS Log Editor : Select an Aperture Reference File");
    fp_file=g_strconcat(hl->wdir,
			G_DIR_SEPARATOR_S,
			"database",
			NULL);
    break;

  case REF2_AP:
    tmp=g_strdup("HDS Log Editor : Select an Aperture Reference File");
    fp_file=g_strconcat(hl->sdir,
			G_DIR_SEPARATOR_S,
			"database",
			NULL);
    break;

  case REF1_THAR:
    tmp=g_strdup("HDS Log Editor : Select a Wavelength Reference (ThAr) File");
    fp_file=g_strconcat(hl->wdir,
			G_DIR_SEPARATOR_S,
			"database",
			NULL);
    break;

  case REF2_THAR:
    tmp=g_strdup("HDS Log Editor : Select a Wavelength Reference (ThAr) File");
    fp_file=g_strconcat(hl->sdir,
			G_DIR_SEPARATOR_S,
			"database",
			NULL);
    break;

  case OPEN_LOG:
    tmp=g_strdup("HDS Log Editor : Select Observation Log file");
    break;

  }

  tgt_file=&fp_file;
  
  fdialog = gtk_file_chooser_dialog_new(tmp,
					GTK_WINDOW(hl->w_top),
					GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef USE_GTK3
					"_Cancel",GTK_RESPONSE_CANCEL,
					"_Open", GTK_RESPONSE_ACCEPT,
#else
					GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
#endif
					NULL);
  g_free(tmp);
  
  gtk_dialog_set_default_response(GTK_DIALOG(fdialog), GTK_RESPONSE_ACCEPT); 
  if(access(*tgt_file,F_OK)==0){
    switch(mode){
    case REF1_AP:
    case REF2_AP:
    case REF1_THAR:
    case REF2_THAR:
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					   to_utf8(*tgt_file));
      break;

    default:
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fdialog), 
				     to_utf8(*tgt_file));
      gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), 
					to_utf8(*tgt_file));
      break;
    }
  }
  else{
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (fdialog), 
					 hl->wdir);
  }

  switch(mode){
  case OPEN_AP:
    tmp=g_strdup_printf("Ap%s%s*.fits",
			(hl->iraf_col==COLOR_R) ? 
			hl->setname_red[hl->iraf_hdsql_r] : hl->setname_blue[hl->iraf_hdsql_b],
			(hl->iraf_col==COLOR_R) ? "R" : "B");
    my_file_chooser_add_filter(fdialog,"Aperture Reference File",
			       tmp,NULL);
    break;

  case OPEN_FLAT:
    tmp=g_strdup_printf("Flat%s%s*.fits",
			(hl->iraf_col==COLOR_R) ? 
			hl->setname_red[hl->iraf_hdsql_r] : hl->setname_blue[hl->iraf_hdsql_b],
			(hl->iraf_col==COLOR_R) ? "R" : "B");
    my_file_chooser_add_filter(fdialog,"Flat Image File",
			       tmp,NULL);
    break;

  case OPEN_THAR:
    tmp=g_strdup_printf("ThAr%s%s*.fits",
			(hl->iraf_col==COLOR_R) ? 
			hl->tharname_red[hl->iraf_hdsql_r] : hl->tharname_blue[hl->iraf_hdsql_b],
			(hl->iraf_col==COLOR_R) ? "R" : "B");
    my_file_chooser_add_filter(fdialog,"Wavelength Reference File",
			       tmp,NULL);
    break;

  case REF1_AP:
  case REF2_AP:
    tmp=g_strdup_printf("apAp%s%s*",
			(hl->iraf_col==COLOR_R) ? 
			hl->setname_red[hl->iraf_hdsql_r] : hl->setname_blue[hl->iraf_hdsql_b],
			(hl->iraf_col==COLOR_R) ? "R" : "B");
    my_file_chooser_add_filter(fdialog,"Aperture Database File",
			       tmp,NULL);
    break;

  case REF1_THAR:
  case REF2_THAR:
    tmp=g_strdup_printf("ecThAr%s%s*",
			(hl->iraf_col==COLOR_R) ? 
			hl->tharname_red[hl->iraf_hdsql_r] : hl->tharname_blue[hl->iraf_hdsql_b],
			(hl->iraf_col==COLOR_R) ? "R" : "B");
    my_file_chooser_add_filter(fdialog,"Wavelength Database File",
			       tmp,NULL);
    break;
  case OPEN_LOG:
    my_file_chooser_add_filter(fdialog,
			       "Obs Log Text File", "*.eml", ".txt",
			       NULL);
    break;

  default:
    break;
  }
  my_file_chooser_add_filter(fdialog,"All File","*", NULL);

  gtk_widget_show_all(fdialog);

  if (gtk_dialog_run(GTK_DIALOG(fdialog)) == GTK_RESPONSE_ACCEPT) {
    char *fname;
    gchar *dest_file, *fits_file;
    gchar *cpp, *basename0, *basename1;
    
    fname = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fdialog)));
    gtk_widget_destroy(fdialog);

    dest_file=to_locale(fname);

    if(access(dest_file,F_OK)==0){
      if(*tgt_file) g_free(*tgt_file);
      *tgt_file=g_strdup(dest_file);

      switch(mode){
      case OPEN_AP:
	if(hl->iraf_col==COLOR_R){
	  if(hl->ap_red[hl->iraf_hdsql_r]) 
	    g_free(hl->ap_red[hl->iraf_hdsql_r]);
	  hl->ap_red[hl->iraf_hdsql_r]=get_refname(*tgt_file);

	  hl->flag_ap_red[hl->iraf_hdsql_r]=FALSE;
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_red), 
				       FALSE);
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_red),
				       FALSE);
	  gtk_widget_set_sensitive(hl->check_auto_red,FALSE);
	}
	else{
	  if(hl->ap_blue[hl->iraf_hdsql_b]) 
	    g_free(hl->ap_blue[hl->iraf_hdsql_b]);
	  hl->ap_blue[hl->iraf_hdsql_b]=get_refname(*tgt_file);

	  hl->flag_ap_blue[hl->iraf_hdsql_b]=FALSE;
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_ap_blue), 
				       FALSE);
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_auto_blue),
				       FALSE);
	  gtk_widget_set_sensitive(hl->check_auto_blue,FALSE);
	}
	set_ap_label(hl);
	break;

      case OPEN_FLAT:
	if(hl->iraf_col==COLOR_R){
	  if(hl->flat_red[hl->iraf_hdsql_r]) 
	    g_free(hl->flat_red[hl->iraf_hdsql_r]);
	  hl->flat_red[hl->iraf_hdsql_r]=get_refname(*tgt_file);
	}
	else{
	  if(hl->flat_blue[hl->iraf_hdsql_b]) 
	    g_free(hl->flat_blue[hl->iraf_hdsql_b]);
	  hl->flat_blue[hl->iraf_hdsql_b]=get_refname(*tgt_file);
	}
	set_flat_label(hl);
	get_flat_scnm(hl);
	break;

      case OPEN_THAR:
	if(hl->iraf_col==COLOR_R){
	  if(hl->thar_red[hl->iraf_hdsql_r]) 
	    g_free(hl->thar_red[hl->iraf_hdsql_r]);
	  hl->thar_red[hl->iraf_hdsql_r]=get_refname(*tgt_file);

	  hl->flag_thar_red[hl->iraf_hdsql_r]=FALSE;
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_red), 
				       FALSE);
	}
	else{
	  if(hl->thar_blue[hl->iraf_hdsql_b]) 
	    g_free(hl->thar_blue[hl->iraf_hdsql_b]);
	  hl->thar_blue[hl->iraf_hdsql_b]=get_refname(*tgt_file);

	  hl->flag_thar_blue[hl->iraf_hdsql_b]=FALSE;
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hl->check_thar_blue), 
				       FALSE);
	}
	set_thar_label(hl);
	break;

      case REF1_AP:
	if(hl->ref_frame) g_free(hl->ref_frame);
	hl->ref_frame=get_refname_db(*tgt_file);

	tmp=g_strdup_printf("apall %s output=%s.ec t_order=4 t_niterate=5 recenter+ resize+  width=10 radius=30 refer=%s",
			    (hl->iraf_col==COLOR_R) ? 
			    hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			    (hl->iraf_col==COLOR_R) ? 
			    hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			    hl->ref_frame);
	gtk_entry_set_text(GTK_ENTRY(hl->entry_ap_id), tmp);
	g_free(tmp);
	break;

      case REF2_AP:
	if(hl->ref_frame) g_free(hl->ref_frame);
	hl->ref_frame=get_refname_db(*tgt_file);
	fits_file=g_strconcat(hl->sdir,
			      G_DIR_SEPARATOR_S,
			      hl->ref_frame,
			      ".fits",
			      NULL);
	if(access(fits_file, F_OK)==0){
	  tmp=g_strconcat("cp ",
			  fits_file,
			  " ",
			  hl->wdir,
			  G_DIR_SEPARATOR_S,
			  NULL);
	  system(tmp);
	  g_free(tmp);
	  g_free(fits_file);

	  check_db_dir(hl->wdir);

	  tmp=g_strconcat("cp ",
			  *tgt_file,
			  " ",
			  hl->wdir,
			  G_DIR_SEPARATOR_S,
			  "database",
			  G_DIR_SEPARATOR_S,
			  NULL);
	  system(tmp);
	  g_free(tmp);

	  tmp=g_strdup_printf("apall %s output=%s.ec t_order=4 t_niterate=5 recenter+ resize+  width=10 radius=30 refer=%s",
			      (hl->iraf_col==COLOR_R) ? 
			      hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			      (hl->iraf_col==COLOR_R) ? 
			      hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			      hl->ref_frame);
	  gtk_entry_set_text(GTK_ENTRY(hl->entry_ap_id), tmp);
	  g_free(tmp);
	}
	else{
	  popup_message(hl->w_top, 
#ifdef USE_GTK3
			"dialog-warning",
#else
			GTK_STOCK_DIALOG_WARNING, 
#endif
			-1,
			"<b>Error</b>: File cannot be opened.",
			" ",
			fits_file,
			NULL);
	  g_free(fits_file);
	}
	break;

      case REF1_THAR:
	if(hl->ref_frame) g_free(hl->ref_frame);
	hl->ref_frame=get_refname_db(*tgt_file);

	tmp=g_strdup_printf("ecreidentify %s shift=0 refer=%s",
			    (hl->iraf_col==COLOR_R) ? 
			    hl->thar_red[hl->iraf_hdsql_r] : hl->thar_blue[hl->iraf_hdsql_b],
			    hl->ref_frame);
	gtk_entry_set_text(GTK_ENTRY(hl->entry_thar_reid), tmp);
	g_free(tmp);
	break;

      case REF2_THAR:
	if(hl->ref_frame) g_free(hl->ref_frame);
	hl->ref_frame=get_refname_db(*tgt_file);
	fits_file=g_strconcat(hl->sdir,
			      G_DIR_SEPARATOR_S,
			      hl->ref_frame,
			      ".fits",
			      NULL);
	if(access(fits_file, F_OK)==0){
	  tmp=g_strconcat("cp ",
			  fits_file,
			  " ",
			  hl->wdir,
			  G_DIR_SEPARATOR_S,
			  NULL);
	  system(tmp);
	  g_free(tmp);
	  g_free(fits_file);

	  check_db_dir(hl->wdir);

	  tmp=g_strconcat("cp ",
			  *tgt_file,
			  " ",
			  hl->wdir,
			  G_DIR_SEPARATOR_S,
			  "database",
			  G_DIR_SEPARATOR_S,
			  NULL);
	  system(tmp);
	  g_free(tmp);

	  tmp=g_strdup_printf("ecreidentify %s shift=0 refer=%s",
			      (hl->iraf_col==COLOR_R) ? 
			      hl->thar_red[hl->iraf_hdsql_r] : hl->thar_blue[hl->iraf_hdsql_b],
			      hl->ref_frame);
	  gtk_entry_set_text(GTK_ENTRY(hl->entry_thar_reid), tmp);
	  g_free(tmp);
	}
	else{
	  popup_message(hl->w_top, 
#ifdef USE_GTK3
			"dialog-warning",
#else
			GTK_STOCK_DIALOG_WARNING, 
#endif
			-1,
			"<b>Error</b>: File cannot be opened.",
			" ",
			fits_file,
			NULL);
	  g_free(fits_file);
	}
	break;

      case OPEN_LOG:
	{
	  FILE *fp;

	  if((fp=fopen(*tgt_file, "r"))!=NULL){
	  
	    ReadLog(hl, fp);

	    fclose(fp);
	  }
	}
	break;

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
		    "<b>Error</b>: File cannot be opened.",
		    " ",
		    fname,
		    NULL);
    }
    
    g_free(dest_file);
    g_free(fname);
  } else {
    gtk_widget_destroy(fdialog);

    switch(mode){
    case REF1_AP:
    case REF2_AP:
      tmp=g_strdup_printf("apall %s output=%s.ec t_order=4 t_niterate=5  width=10 radius=30 recenter+ resize+",
			  (hl->iraf_col==COLOR_R) ? 
			  hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b],
			  (hl->iraf_col==COLOR_R) ? 
			  hl->ap_red[hl->iraf_hdsql_r] : hl->ap_blue[hl->iraf_hdsql_b]);
      gtk_entry_set_text(GTK_ENTRY(hl->entry_ap_id), tmp);
      g_free(tmp);
      break;
    }

  }
}


void edit_ap (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hdslog_OpenFile(hl, OPEN_AP);
}

void edit_flat (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hdslog_OpenFile(hl, OPEN_FLAT);
}

void edit_thar (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hdslog_OpenFile(hl, OPEN_THAR);
}

void ref1_ap (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hdslog_OpenFile(hl, REF1_AP);
}

void ref2_ap (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hdslog_OpenFile(hl, REF2_AP);
}

void ref1_thar (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hdslog_OpenFile(hl, REF1_THAR);
}

void ref2_thar (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  hdslog_OpenFile(hl, REF2_THAR);
}

void edit_cal(typHLOG *hl){
  GtkWidget *dialog, *label, *frame, *combo, *hbox, 
    *vbox, *hbox1, *entry, *button, *table;
  gchar *tmp;
      
  dialog = gtk_dialog_new_with_buttons("HDS Log Editor : CAL frames for quick look",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 

  if(hl->iraf_col==COLOR_R){
    tmp=g_strdup_printf("SetUp-%d : <b>%s</b><span color=\"#FF0000\"><b>R</b></span>  <span size=\"smaller\">for %s</span>",
			hl->iraf_hdsql_r+1,
			hl->setname_red[hl->iraf_hdsql_r],
			hdsql_red[hl->iraf_hdsql_r]);
  }
  else{
    tmp=g_strdup_printf("SetUp-%d : <b>%s</b><span color=\"#0000FF\"><b>B</b></span>  <span size=\"smaller\">for %s</span>",
			hl->iraf_hdsql_b+1,
			hl->setname_blue[hl->iraf_hdsql_b],
			hdsql_blue[hl->iraf_hdsql_b]);
  }
  
  label=gtkut_label_new(tmp);
  g_free(tmp);
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE, FALSE, 0);
		      

  frame = gtkut_frame_new ("<b> Aperture reference</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 5);

  hl->label_edit_ap=gtkut_label_new("Ap");
  set_ap_label(hl);

#ifdef USE_GTK3
  gtk_widget_set_halign (hl->label_edit_ap, GTK_ALIGN_START);
  gtk_widget_set_valign (hl->label_edit_ap, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hl->label_edit_ap), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hl->label_edit_ap, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-open");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
  		    G_CALLBACK (edit_ap), (gpointer)hl);

  ////  Flat
  frame = gtkut_frame_new ("<b>Flat image</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 5);

  hl->label_edit_flat=gtkut_label_new("Flat");
  set_flat_label(hl);

#ifdef USE_GTK3
  gtk_widget_set_halign (hl->label_edit_flat, GTK_ALIGN_START);
  gtk_widget_set_valign (hl->label_edit_flat, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hl->label_edit_flat), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hl->label_edit_flat, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-open");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
  		    G_CALLBACK (edit_flat), (gpointer)hl);

  ////  ThAr
  frame = gtkut_frame_new ("<b>Wavelength reference</b>");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtkut_vbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox,FALSE, FALSE, 5);

  hl->label_edit_thar=gtkut_label_new("ThAr");
  set_thar_label(hl);

#ifdef USE_GTK3
  gtk_widget_set_halign (hl->label_edit_thar, GTK_ALIGN_START);
  gtk_widget_set_valign (hl->label_edit_thar, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (hl->label_edit_thar), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), hl->label_edit_thar, FALSE, FALSE, 5);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"document-open");
#else
  button=gtkut_button_new_from_stock(NULL, GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
  		    G_CALLBACK (edit_thar), (gpointer)hl);


  gtk_widget_show_all(dialog);

  gtk_dialog_run(GTK_DIALOG(dialog));
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);
}


void iraf_find(typHLOG *hl, gint i_sel, gint i_file){
  gchar *tmp, *set_name;
  gchar *work_dir;
  gint tkid;
  gchar *ap_fits, *thar_fits, *flat_fits;
  gchar *c;
  gboolean ow_flag;
  

  if((tkid=iraf_quit_tk(hl))<0){
    return;
  }

  set_name=get_setname_long(hl, i_sel);

  // Red
  if(hl->iraf_col==COLOR_R){
    // Ap
    if(!hl->ap_red[hl->iraf_hdsql_r]){
      ap_fits=g_strconcat(hl->wdir,
			  G_DIR_SEPARATOR_S,
			  "Ap",
			  set_name,
			  ".fits",
			  NULL);
      if(access(ap_fits, F_OK)==0){
	if(hl->ap_red[hl->iraf_hdsql_r]) g_free(hl->ap_red[hl->iraf_hdsql_r]);
	hl->ap_red[hl->iraf_hdsql_r]=g_strconcat("Ap",
						 set_name,
						 NULL);
      }
      g_free(ap_fits);
    }

    db_check(hl, CAL_AP);
 
    set_setname(hl, i_sel);
    

    if(hl->iraf_col==COLOR_R){
      set_ql_frame_label(hl, hl->frame_ql_red, TRUE);
    }
    else{
      set_ql_frame_label(hl, hl->frame_ql_blue, TRUE);
    }
    
    
    // ThAr
    if(!hl->thar_red[hl->iraf_hdsql_r]){
      thar_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "ThAr",
			    set_name,
			    ".fits",
			    NULL);
      if(access(thar_fits, F_OK)==0){
	if(hl->thar_red[hl->iraf_hdsql_r]) g_free(hl->thar_red[hl->iraf_hdsql_r]);
	hl->thar_red[hl->iraf_hdsql_r]=g_strconcat("ThAr",
						   set_name,
						   NULL);
      }
      g_free(thar_fits);
    }
    db_check(hl, CAL_THAR);

    // Flat
    hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_NO;
    
    while(1){
      // sc.nm
      flat_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "Flat",
			    set_name,
			    ".sc.nm.fits",
			    NULL);
      if(access(flat_fits, F_OK)==0){
	if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
	hl->flat_red[hl->iraf_hdsql_r]=g_strconcat("Flat",
						   set_name,
						   ".sc.nm",
						   NULL);
	hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_SCNM;
	break;
      }
	
      // sc.fl
      g_free(flat_fits);
      flat_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "Flat",
			    set_name,
			    ".sc.fl.fits",
			    NULL);
      if(access(flat_fits, F_OK)==0){
	if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
	hl->flat_red[hl->iraf_hdsql_r]=g_strconcat("Flat",
						   set_name,
						   ".sc.fl",
						   NULL);
	hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_SCFL;
	break;
      }
	
      // sc
      g_free(flat_fits);
      flat_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "Flat",
			    set_name,
			    ".sc.fits",
			    NULL);
      if(access(flat_fits, F_OK)==0){
	if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
	hl->flat_red[hl->iraf_hdsql_r]=g_strconcat("Flat",
						   set_name,
						   ".sc",
						   NULL);
	hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_SC;
	break;
      }
      
      // ave
      g_free(flat_fits);
      flat_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "Flat",
			    set_name,
			    ".fits",
			    NULL);
      if(access(flat_fits, F_OK)==0){
	if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
	hl->flat_red[hl->iraf_hdsql_r]=g_strconcat("Flat",
						   set_name,
						   NULL);
	hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_1;
	break;
      }
      
      // None
      if(hl->flat_red[hl->iraf_hdsql_r]) g_free(hl->flat_red[hl->iraf_hdsql_r]);
      hl->flat_red[hl->iraf_hdsql_r]=NULL;
      hl->ex_flat_red[hl->iraf_hdsql_r]=FLAT_EX_NO;
      break;
    }
    g_free(flat_fits);

    db_check(hl, CAL_FLAT);
  }
  else{  // Blue CCD
    // Ap
    if(!hl->ap_blue[hl->iraf_hdsql_b]){
      ap_fits=g_strconcat(hl->wdir,
			  G_DIR_SEPARATOR_S,
			  "Ap",
			  set_name,
			  ".fits",
			  NULL);
      if(access(ap_fits, F_OK)==0){
	if(hl->ap_blue[hl->iraf_hdsql_b]) g_free(hl->ap_blue[hl->iraf_hdsql_b]);
	hl->ap_blue[hl->iraf_hdsql_b]=g_strconcat("Ap",
						  set_name,
						  NULL);
      }
      g_free(ap_fits);
    }
    db_check(hl, CAL_AP);
    
    set_setname(hl, i_sel);

    if(hl->iraf_col==COLOR_R){
      set_ql_frame_label(hl, hl->frame_ql_red, TRUE);
    }
    else{
      set_ql_frame_label(hl, hl->frame_ql_blue, TRUE);
    }

    // ThAr
    if(!hl->thar_blue[hl->iraf_hdsql_b]){
      thar_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "ThAr",
			    set_name,
			    ".fits",
			    NULL);
      if(access(thar_fits, F_OK)==0){
	if(hl->thar_blue[hl->iraf_hdsql_b]) g_free(hl->thar_blue[hl->iraf_hdsql_b]);
	hl->thar_blue[hl->iraf_hdsql_b]=g_strconcat("ThAr",
						    set_name,
						    NULL);
      }
      g_free(thar_fits);
    }
    db_check(hl, CAL_THAR);

    // Flat
    hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_NO;
    
    while(1){
      // sc.nm
      flat_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "Flat",
			    set_name,
			    ".sc.nm.fits",
			    NULL);
      if(access(flat_fits, F_OK)==0){
	if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
	hl->flat_blue[hl->iraf_hdsql_b]=g_strconcat("Flat",
						    set_name,
						    ".sc.nm",
						    NULL);
	hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_SCNM;
	break;
      }
      
      // sc.fl
      g_free(flat_fits);
      flat_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "Flat",
			    set_name,
			    ".sc.fl.fits",
			    NULL);
      if(access(flat_fits, F_OK)==0){
	if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
	hl->flat_blue[hl->iraf_hdsql_b]=g_strconcat("Flat",
						    set_name,
						    ".sc.fl",
						    NULL);
	hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_SCFL;
	break;
      }
      
      // sc
      g_free(flat_fits);
      flat_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "Flat",
			    set_name,
			    ".sc.fits",
			    NULL);
      if(access(flat_fits, F_OK)==0){
	if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
	hl->flat_blue[hl->iraf_hdsql_b]=g_strconcat("Flat",
						    set_name,
						    ".sc",
						    NULL);
	hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_SC;
	break;
      }
      
      // ave
      g_free(flat_fits);
      flat_fits=g_strconcat(hl->wdir,
			    G_DIR_SEPARATOR_S,
			    "Flat",
			    set_name,
			    ".fits",
			    NULL);
      if(access(flat_fits, F_OK)==0){
	if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
	hl->flat_blue[hl->iraf_hdsql_b]=g_strconcat("Flat",
						    set_name,
						    NULL);
	hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_1;
	break;
      }
      
      // None
      if(hl->flat_blue[hl->iraf_hdsql_b]) g_free(hl->flat_blue[hl->iraf_hdsql_b]);
      hl->flat_blue[hl->iraf_hdsql_b]=NULL;
      hl->ex_flat_blue[hl->iraf_hdsql_b]=FLAT_EX_NO;
      break;
    }
    g_free(flat_fits);

    db_check(hl, CAL_FLAT);
  }

  g_free(set_name);

  edit_cal(hl);
}


void ql_find_foreach (GtkTreeModel *model, GtkTreePath *path, 
		      GtkTreeIter *iter, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  gint i, i_file;
  gchar *c;
    
  gtk_tree_model_get (model, iter, COLUMN_FRAME_NUMBER, &i, -1);

  c=hl->frame[i].id+strlen("HDSA00");
  if(hl->iraf_col==COLOR_R){
    i_file=(gint)g_strtod(c, NULL);
  }
  else{
    i_file=(gint)g_strtod(c, NULL)+1;
  }

  iraf_find(hl, i, i_file);
}

void ql_find_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_R;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_find_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to find prepared CAL files.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}

void ql_find_blue(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp;
  gchar *cmdline;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_rows;
  gint tmp_scr=hl->scr_flag;

  if(hl->file_head!=FILE_HDSA) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   SCR_NONE);

  hl->iraf_col=COLOR_B;

  i_rows=gtk_tree_selection_count_selected_rows (selection);

  if(i_rows==1){
    gtk_tree_selection_selected_foreach (selection, ql_find_foreach, (gpointer)hl);
  }
  else{
    tmp=g_strdup_printf("<b>%d</b> rows are selected in the table.", 
			i_rows);
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  tmp,
		  " ",
		  "Please select only one row to find prepared CAL files.",
		  NULL);
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(hl->scr_combo),
			   tmp_scr);
}


void iraf_param(typHLOG *hl){
  GtkWidget *dialog, *label, *frame, *table, *hbox, *check, *combo, *spinner;
  GtkAdjustment *adj;
  gchar *tmp;
  gint tmp_x;

  if((hl->iraf_col==COLOR_R) ?
     !hl->flag_ap_red[hl->iraf_hdsql_r] :
     !hl->flag_ap_blue[hl->iraf_hdsql_b]){
    popup_message(hl->w_top, 
#ifdef USE_GTK3
		  "dialog-warning", 
#else
		  GTK_STOCK_DIALOG_WARNING,
#endif
		  -1,
		  "No Aperture reference is found in this set.",
		  " ",
		  "Please set and make an Aperture reference by <b>Ap</b> button.",
		  NULL);
    return;
  }


  dialog = gtk_dialog_new_with_buttons("HDS Log Editor : Parameters for hdsql",
				       GTK_WINDOW(hl->w_top),
				       GTK_DIALOG_MODAL,
#ifdef USE_GTK3
				       "_OK",GTK_RESPONSE_OK,
#else
				       GTK_STOCK_OK,GTK_RESPONSE_OK,
#endif
				       NULL);
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 


  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,FALSE, FALSE, 0);

  tmp=g_strdup_printf("Setup-%d : %s : ",
		      (hl->iraf_col==COLOR_R) ?
		      hl->iraf_hdsql_r+1 : hl->iraf_hdsql_b+1,
		      (hl->iraf_col==COLOR_R) ?
		      hdsql_red[hl->iraf_hdsql_r] : hdsql_blue[hl->iraf_hdsql_b]);
  label=gtkut_label_new(tmp);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  label=gtkut_label_new(" ");
  set_ql_frame_label(hl, label, FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE, FALSE, 0);

  // R<-->B
  frame = gtkut_frame_new ("<b>Priority of reduction</b> (sc_*)");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  if(hl->iraf_col==COLOR_R){
    check = gtk_check_button_new_with_label("Red --> Blue");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hl->qp_r[hl->iraf_hdsql_r].ql_1st);
    g_signal_connect (check, "toggled",
		      G_CALLBACK (cc_get_toggle),
		      &hl->qp_r[hl->iraf_hdsql_r].ql_1st);
  }
  else{
    check = gtk_check_button_new_with_label("Blue --> Red");
    gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
				 hl->qp_b[hl->iraf_hdsql_b].ql_1st);
    g_signal_connect (check, "toggled",
		      G_CALLBACK (cc_get_toggle),
		      &hl->qp_b[hl->iraf_hdsql_b].ql_1st);
  }

  // sc
  frame = gtkut_frame_new ("<b>Scattered Light Subtraction</b> (sc_*)");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  check = gtk_check_button_new_with_label("sc_inte");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].sc_inte :
			       hl->qp_b[hl->iraf_hdsql_b].sc_inte);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].sc_inte :
		    &hl->qp_b[hl->iraf_hdsql_b].sc_inte);
  
  check = gtk_check_button_new_with_label("sc_resi");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].sc_resi :
			       hl->qp_b[hl->iraf_hdsql_b].sc_resi);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].sc_resi :
		    &hl->qp_b[hl->iraf_hdsql_b].sc_resi);

  check = gtk_check_button_new_with_label("sc_edit");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].sc_edit :
			       hl->qp_b[hl->iraf_hdsql_b].sc_edit);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].sc_edit :
		    &hl->qp_b[hl->iraf_hdsql_b].sc_edit);

  check = gtk_check_button_new_with_label("sc_fitt");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].sc_fitt :
			       hl->qp_b[hl->iraf_hdsql_b].sc_fitt);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].sc_fitt :
		    &hl->qp_b[hl->iraf_hdsql_b].sc_fitt);


  // ap
  frame = gtkut_frame_new ("<b>Aperture Extraction</b> (ap_*)");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  check = gtk_check_button_new_with_label("ap_inte");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].ap_inte :
			       hl->qp_b[hl->iraf_hdsql_b].ap_inte);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ap_inte :
		    &hl->qp_b[hl->iraf_hdsql_b].ap_inte);
  
  check = gtk_check_button_new_with_label("ap_resi");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].ap_resi :
			       hl->qp_b[hl->iraf_hdsql_b].ap_resi);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ap_resi :
		    &hl->qp_b[hl->iraf_hdsql_b].ap_resi);

  check = gtk_check_button_new_with_label("ap_edit");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].ap_edit :
			       hl->qp_b[hl->iraf_hdsql_b].ap_edit);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ap_edit :
		    &hl->qp_b[hl->iraf_hdsql_b].ap_edit);

  label = gtk_label_new ("  ap_llim");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new((hl->iraf_col==COLOR_R) ?
					    hl->qp_r[hl->iraf_hdsql_r].ap_llim :
					    hl->qp_b[hl->iraf_hdsql_b].ap_llim,
					    -40,40,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ap_llim :
		    &hl->qp_b[hl->iraf_hdsql_b].ap_llim);

  label = gtk_label_new ("  ap_ulim");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new((hl->iraf_col==COLOR_R) ?
					    hl->qp_r[hl->iraf_hdsql_r].ap_ulim :
					    hl->qp_b[hl->iraf_hdsql_b].ap_ulim,
					    -40,40,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ap_ulim :
		    &hl->qp_b[hl->iraf_hdsql_b].ap_ulim);


  // is
  frame = gtkut_frame_new ("<b>Image Slicer</b> (is_*)");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  check = gtk_check_button_new_with_label("is_plot");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].is_plot :
			       hl->qp_b[hl->iraf_hdsql_b].is_plot);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].is_plot :
		    &hl->qp_b[hl->iraf_hdsql_b].is_plot);
  
  label = gtk_label_new ("  is_stx");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new((hl->iraf_col==COLOR_R) ?
					    hl->qp_r[hl->iraf_hdsql_r].is_stx :
					    hl->qp_b[hl->iraf_hdsql_b].is_stx,
					    -40,40,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].is_stx :
		    &hl->qp_b[hl->iraf_hdsql_b].is_stx);

  label = gtk_label_new ("  is_edx");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new((hl->iraf_col==COLOR_R) ?
					    hl->qp_r[hl->iraf_hdsql_r].is_edx :
					    hl->qp_b[hl->iraf_hdsql_b].is_edx,
					    -40,40,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].is_edx :
		    &hl->qp_b[hl->iraf_hdsql_b].is_edx);

  // is
  frame = gtkut_frame_new ("<b>Get Spectrum Count</b> (ge_*)");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  check = gtk_check_button_new_with_label("ge_cnt");
  gtk_box_pack_start(GTK_BOX(hbox),check,FALSE,FALSE,0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
			       (hl->iraf_col==COLOR_R) ?
			       hl->qp_r[hl->iraf_hdsql_r].ge_cnt :
			       hl->qp_b[hl->iraf_hdsql_b].ge_cnt);
  g_signal_connect (check, "toggled",
		    G_CALLBACK (cc_get_toggle),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ge_cnt :
		    &hl->qp_b[hl->iraf_hdsql_b].ge_cnt);
  
  label = gtk_label_new ("  ge_stx");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new((hl->iraf_col==COLOR_R) ?
					    hl->qp_r[hl->iraf_hdsql_r].ge_stx :
					    hl->qp_b[hl->iraf_hdsql_b].ge_stx,
					    1,3000,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ge_stx :
		    &hl->qp_b[hl->iraf_hdsql_b].ge_stx);

  label = gtk_label_new ("  ge_edx");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new((hl->iraf_col==COLOR_R) ?
					    hl->qp_r[hl->iraf_hdsql_r].ge_edx :
					    hl->qp_b[hl->iraf_hdsql_b].ge_edx,
					    1000,4100,
					    1.0, 1.0, 0);
  spinner =  gtk_spin_button_new (adj, 0, 0);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ge_edx :
		    &hl->qp_b[hl->iraf_hdsql_b].ge_edx);

  label = gtk_label_new ("  ge_low");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new((hl->iraf_col==COLOR_R) ?
					    hl->qp_r[hl->iraf_hdsql_r].ge_low :
					    hl->qp_b[hl->iraf_hdsql_b].ge_low,
					    0.5,3.0,
					    0.1, 0.1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj_double),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ge_low :
		    &hl->qp_b[hl->iraf_hdsql_b].ge_low);

  label = gtk_label_new ("  ge_high");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  adj = (GtkAdjustment *)gtk_adjustment_new((hl->iraf_col==COLOR_R) ?
					    hl->qp_r[hl->iraf_hdsql_r].ge_high :
					    hl->qp_b[hl->iraf_hdsql_b].ge_high,
					    0.5,6.0,
					    0.1, 0.1, 0);
  spinner =  gtk_spin_button_new (adj, 1, 1);
  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), FALSE);
  gtk_box_pack_start(GTK_BOX(hbox),spinner,FALSE,FALSE,0);
  g_signal_connect (adj, "value_changed",
		    G_CALLBACK (cc_get_adj_double),
		    (hl->iraf_col==COLOR_R) ?
		    &hl->qp_r[hl->iraf_hdsql_r].ge_high :
		    &hl->qp_b[hl->iraf_hdsql_b].ge_high);


  // splot
  frame = gtkut_frame_new ("<b>Splot</b> (sp_*)");
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     frame,FALSE, FALSE, 0);

  hbox = gtkut_hbox_new(FALSE,2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  label = gtk_label_new ("sp_line");
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

  {
    GtkListStore *store;
    GtkTreeIter iter, iter_set;	  
    GtkCellRenderer *renderer;
    gint i_ord;
    gchar *tmp;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    for(i_ord=1;i_ord<40;i_ord++){
      tmp=g_strdup_printf("order %d", i_ord);
      gtk_list_store_append(store, &iter);
      gtk_list_store_set(store, &iter, 
			 0, tmp,
			 1, i_ord, -1);
      g_free(tmp);
      if(((hl->iraf_col==COLOR_R) ?
	 hl->qp_r[hl->iraf_hdsql_r].sp_line :
	 hl->qp_b[hl->iraf_hdsql_b].sp_line)==i_ord) iter_set=iter;
    }
    
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    gtk_box_pack_start(GTK_BOX(hbox),combo,FALSE,FALSE,0);
    g_object_unref(store);
    
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo),renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text",0,NULL);
    
    gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&iter_set);
    gtk_widget_show(combo);
    g_signal_connect (combo,"changed",G_CALLBACK(cc_get_combo_box),
		      (hl->iraf_col==COLOR_R) ?
		      &hl->qp_r[hl->iraf_hdsql_r].sp_line :
		      &hl->qp_b[hl->iraf_hdsql_b].sp_line);
  }



  gtk_widget_show_all(dialog);


  gtk_dialog_run(GTK_DIALOG(dialog));
  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  if(hl->iraf_col==COLOR_R){
    hl->qp_b[hl->iraf_hdsql_r].ql_1st=!hl->qp_r[hl->iraf_hdsql_r].ql_1st;
    if(hl->qp_r[hl->iraf_hdsql_r].ge_cnt){
      hl->qp_b[hl->iraf_hdsql_r].ge_cnt=FALSE;
    }
    if(hl->qp_r[hl->iraf_hdsql_r].ge_stx > hl->qp_r[hl->iraf_hdsql_r].ge_edx){
      tmp_x=hl->qp_r[hl->iraf_hdsql_r].ge_stx;
      hl->qp_r[hl->iraf_hdsql_r].ge_stx=hl->qp_r[hl->iraf_hdsql_r].ge_edx;
      hl->qp_r[hl->iraf_hdsql_r].ge_edx=tmp_x;
    }
  }
  else{
    hl->qp_r[hl->iraf_hdsql_b].ql_1st=!hl->qp_b[hl->iraf_hdsql_b].ql_1st;
    if(hl->qp_b[hl->iraf_hdsql_b].ge_cnt){
      hl->qp_r[hl->iraf_hdsql_b].ge_cnt=FALSE;
    }
    if(hl->qp_b[hl->iraf_hdsql_b].ge_stx > hl->qp_b[hl->iraf_hdsql_b].ge_edx){
      tmp_x=hl->qp_b[hl->iraf_hdsql_b].ge_stx;
      hl->qp_b[hl->iraf_hdsql_b].ge_stx=hl->qp_b[hl->iraf_hdsql_b].ge_edx;
      hl->qp_b[hl->iraf_hdsql_b].ge_edx=tmp_x;
    }
  }
}

void ql_param_red(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  hl->iraf_col=COLOR_R;

  iraf_param(hl);
}

void ql_param_blue(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;

  hl->iraf_col=COLOR_B;

  iraf_param(hl);
}
