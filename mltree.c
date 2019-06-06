#include "main.h"

static void mltree_add_columns();
static GtkTreeModel *mltree_create_items_model();
void mltree_update_item();
static void focus_mltree_item ();
void mltree_cell_data_func();
static void mltree_search_item();
gchar *strip_spc();
void cc_search_text();
void add_address();
void swap_ml();

gboolean flag_make_mltree;

void read_ml(typHLOG *hl){
  gchar *mlfile;
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *tmp_char=NULL, *head=NULL, *tmp_p;
  gchar *tmp;
  gint i_ml=0;

  mlfile=g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,MAIL_LIST,NULL);

  if((fp=fopen(mlfile,"rb"))==NULL){
    fprintf(stderr, "Error: Cannot open \"%s\".\n",mlfile);
    g_free(mlfile);
    if(hl->ml[0].address) g_free(hl->ml[0].address);
    hl->ml[0].address=g_strdup(DEF_MAIL);
    hl->ml[0].year=hl->fr_year;
    hl->ml[0].month=hl->fr_month;
    hl->ml[0].day=hl->fr_day;
    hl->ml_max=1;
    return;
  }
  
  while((buf=fgets_new(fp))!=NULL){
    if(strlen(buf)<10)break;
    tmp_char=(char *)strtok(buf,",");
    if(!tmp_char) break;
    hl->ml[i_ml].address=g_strdup(g_strstrip(tmp_char));
    tmp_char=(char *)strtok(NULL,",");
    if(!tmp_char) break;
    hl->ml[i_ml].year=atoi(tmp_char);
    tmp_char=(char *)strtok(NULL,",");
    if(!tmp_char) break;
    hl->ml[i_ml].month=atoi(tmp_char);
    tmp_char=(char *)strtok(NULL,"\n");
    if(!tmp_char) break;
    hl->ml[i_ml].day=atoi(tmp_char);
    i_ml++;
    g_free(buf);
  }

  hl->ml_max=i_ml;
  
  fclose(fp);
  g_free(mlfile);
}

void popup_ml (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;
  GtkWidget *button;
  GtkWidget *scrwin;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *hbox;
  
  hl->mldialog = gtk_dialog_new();
  gtk_container_set_border_width(GTK_CONTAINER(hl->mldialog),5);
  gtk_window_set_title(GTK_WINDOW(hl->mldialog),"HDS Log Editor : Mail Address Book");
  gtk_window_set_modal(GTK_WINDOW(hl->mldialog),TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(hl->mldialog),GTK_WINDOW(hl->smdialog));

  hbox = gtkut_hbox_new (FALSE, 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hl->mldialog))),
		     hbox,FALSE, FALSE, 0);
  hl->mltree_search_i=0;
  hl->mltree_search_imax=0;

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name(NULL,"edit-find");
#else
  button=gtkut_button_new_from_stock(NULL,GTK_STOCK_FIND);
#endif
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
		    G_CALLBACK (mltree_search_item), (gpointer)hl);
#ifdef __GTK_TOOLTIP_H__
  gtk_widget_set_tooltip_text(button,"Find MAIL Address");
#endif

  entry = gtk_entry_new ();
  gtk_box_pack_start(GTK_BOX(hbox), entry,FALSE, FALSE, 0);
  gtk_editable_set_editable(GTK_EDITABLE(entry),TRUE);
  gtk_entry_set_width_chars(GTK_ENTRY(entry),20);
  g_signal_connect (entry, "changed", G_CALLBACK(cc_search_text), (gpointer)hl);
  g_signal_connect (entry, "activate", G_CALLBACK(mltree_search_item), (gpointer)hl);

  hl->mltree_search_label = gtk_label_new ("     ");
  gtk_box_pack_start(GTK_BOX(hbox), hl->mltree_search_label, FALSE, FALSE, 0);

  scrwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrwin),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_ALWAYS);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hl->mldialog))),
		     scrwin,FALSE, FALSE, 0);

  make_mltree(hl);
  gtk_container_add (GTK_CONTAINER (scrwin), hl->mltree);
  gtk_widget_set_size_request(scrwin, 400,400);
  g_signal_connect (hl->mltree, "cursor-changed",
		    G_CALLBACK (focus_mltree_item), (gpointer)hl);

  hbox = gtkut_hbox_new (FALSE, 5);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(hl->mldialog))),
		     hbox,TRUE, TRUE, 5);

  label = gtk_label_new ("     ");
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Append To:","list-add");
#else
  button=gtkut_button_new_from_stock("Append To:",GTK_STOCK_ADD);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(add_address), 
		   (gpointer)hl);
  
#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Close","window-close");
#else
  button=gtkut_button_new_from_stock("Close",GTK_STOCK_CLOSE);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  g_signal_connect(button,"pressed",
		   G_CALLBACK(gtk_main_quit), 
		   NULL);

  
  gtk_widget_show_all(hl->mldialog);
  gtk_main();

  gtk_widget_destroy(hl->mltree);
  flag_make_mltree=FALSE;
  gtk_widget_destroy(hl->mldialog);
}


void make_mltree(typHLOG *hl){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *sw;
  GtkWidget *button;
  GtkTreeModel *items_model;
  
  if(flag_make_mltree){
    gtk_widget_destroy(hl->mltree);
  }
  else flag_make_mltree=TRUE;

  items_model = mltree_create_items_model (hl);

  /* create tree view */
  hl->mltree = gtk_tree_view_new_with_model (items_model);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hl->mltree)),
			       GTK_SELECTION_SINGLE);
  mltree_add_columns (hl, GTK_TREE_VIEW (hl->mltree), 
		      items_model);

  g_object_unref(items_model);
  
  gtk_widget_show_all(hl->mltree);
}


static void mltree_add_columns (typHLOG *hl,
				GtkTreeView  *treeview, 
				GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  
  
  /* Address column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ML_ADDRESS));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_ML_ADDRESS,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_ML_COLFG,
						   "background-rgba", COLUMN_ML_COLBG,
#else
						   "foreground-gdk", COLUMN_ML_COLFG,
						   "background-gdk", COLUMN_ML_COLBG,
#endif
						   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Mail Address");
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_ML_ADDRESS);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Last Date column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_ML_DATE));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_ML_DATE,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_ML_COLFG,
						   "background-rgba", COLUMN_ML_COLBG,
#else
						   "foreground-gdk", COLUMN_ML_COLFG,
						   "background-gdk", COLUMN_ML_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Last Date");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  mltree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_ML_DATE),
					  NULL);
  gtk_tree_view_column_set_sort_column_id(column,COLUMN_ML_DATE);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}

static GtkTreeModel *
mltree_create_items_model (typHLOG *hl)
{
  gint i_ml = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_ML_COLUMNS,
			      G_TYPE_INT,      //COLUMN_ML_NUMBER,
			      G_TYPE_STRING,   //COLUMN_ML_ADDRESS,
			      G_TYPE_INT,      //COLUMN_ML_DATE,
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,   //fgcolor
			      GDK_TYPE_RGBA   //bgcolor
#else
			      GDK_TYPE_COLOR,  //fgcolor
			      GDK_TYPE_COLOR  //bgcolor
#endif
			      );

  for (i_ml = 0; i_ml < hl->ml_max; i_ml++){
    gtk_list_store_append (model, &iter);
    mltree_update_item(hl, GTK_TREE_MODEL(model), iter, i_ml);
  }
  
  return GTK_TREE_MODEL (model);
}


void mltree_update_item(typHLOG *hl, 
			GtkTreeModel *model, 
			GtkTreeIter iter, 
			gint i_ml)
{
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_ML_NUMBER, i_ml,
		      COLUMN_ML_ADDRESS,   hl->ml[i_ml].address,
		      COLUMN_ML_DATE,      hl->ml[i_ml].year*10000
		      +hl->ml[i_ml].month*100
		      +hl->ml[i_ml].day,
		      COLUMN_ML_COLFG, &color_black,
		      COLUMN_ML_COLBG, (i_ml%2==0) ? &color_white : &color_lgreen,
		      -1);
}

static void
focus_mltree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHLOG *hl = (typHLOG *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hl->mltree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->mltree));
  gint i_ml;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_ML_NUMBER, &i_ml, -1);
    gtk_tree_path_free (path);

    hl->mltree_i=i_ml;
  }
}


void mltree_cell_data_func(GtkTreeViewColumn *col , 
			   GtkCellRenderer *renderer,
			   GtkTreeModel *model, 
			   GtkTreeIter *iter,
			   gpointer user_data)
{
  const guint index = GPOINTER_TO_UINT(user_data);
  guint64 size;
  gint i_buf, i_buf2;
  gdouble d_buf, d_buf2;
  gchar *c_buf, *str, *c_buf2;
  gint year, month, day;

  switch (index) {
  case COLUMN_ML_DATE:
    gtk_tree_model_get (model, iter, 
			index, &i_buf,
			-1);
    year=i_buf/10000;
    month=(i_buf-year*10000)/100;
    day=i_buf-year*10000-month*100;

    str=g_strdup_printf("%d/%02d/%02d",year,month,day);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


static void mltree_search_item (GtkWidget *widget, gpointer data)
{
  gint i;
  gchar *label_text;
  typHLOG *hl = (typHLOG *)data;
  gchar *up_text1, *up_text2, *up_obj1, *up_obj2;

  if(!hl->mltree_search_text) return;

  if(strlen(hl->mltree_search_text)<1){
    hl->mltree_search_imax=0;
    hl->mltree_search_i=0;

    gtk_label_set_text(GTK_LABEL(hl->mltree_search_label),"      ");
    return;
  }

  if(hl->mltree_search_imax==0){
    up_text1=g_ascii_strup(hl->mltree_search_text, -1);
    up_text2=strip_spc(up_text1);
    g_free(up_text1);
    for(i=0; i<hl->ml_max; i++){
      up_obj1=g_ascii_strup(hl->ml[i].address, -1);
      up_obj2=strip_spc(up_obj1);
      g_free(up_obj1);
      if(g_strstr_len(up_obj2, -1, up_text2)!=NULL){
	hl->mltree_search_iaddr[hl->mltree_search_imax]=i;
	hl->mltree_search_imax++;
      }
      g_free(up_obj2);
    }
    g_free(up_text2);
  }
  else{
    hl->mltree_search_i++;
    if(hl->mltree_search_i>=hl->mltree_search_imax) hl->mltree_search_i=0;
  }

  if(hl->mltree_search_imax!=0){
    label_text=g_strdup_printf("%d/%d   ",
			       hl->mltree_search_i+1,
			       hl->mltree_search_imax);
    {
      gint i_list;
      GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->mltree));
      GtkTreePath *path;
      GtkTreeIter  iter;

      path=gtk_tree_path_new_first();
      
      for(i=0;i<hl->ml_max;i++){
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, COLUMN_ML_NUMBER, &i_list, -1);
	
	if(i_list==hl->mltree_search_iaddr[hl->mltree_search_i]){
	  gtk_tree_view_set_cursor(GTK_TREE_VIEW(hl->mltree), path, NULL, FALSE);
	  hl->mltree_i=i_list;
	  break;
	}
	else{
	  gtk_tree_path_next(path);
	}
      }
      gtk_tree_path_free(path);
    }
  }
  else{
    label_text=g_strdup_printf("%d/%d   ",
			       hl->mltree_search_i,
			       hl->mltree_search_imax);
  }
  gtk_label_set_text(GTK_LABEL(hl->mltree_search_label),label_text);
  g_free(label_text);
}


gchar *strip_spc(gchar * obj_name){
  gchar *tgt_name, *ret_name;
  gint  i_str=0,i;

  tgt_name=g_strdup(obj_name);
  for(i=0;i<strlen(tgt_name);i++){
    if((obj_name[i]!=0x20)
       &&(obj_name[i]!=0x0A)
       &&(obj_name[i]!=0x0D)
       &&(obj_name[i]!=0x09)){
      tgt_name[i_str]=obj_name[i];
      i_str++;
    }
  }
  tgt_name[i_str]='\0';
  
  ret_name=g_strdup(tgt_name);
  if(tgt_name) g_free(tgt_name);
  return(ret_name);
}


void cc_search_text (GtkWidget *widget, gpointer gdata)
{
  typHLOG *hl=(typHLOG *)gdata;

  if(hl->mltree_search_text) g_free(hl->mltree_search_text);
  hl->mltree_search_text=g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));

  hl->mltree_search_i=0;
  hl->mltree_search_imax=0;

  gtk_label_set_text(GTK_LABEL(hl->mltree_search_label),"      ");
}



void add_address(GtkWidget *w, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gchar *tmp_mail, *cp;


  if(hl->ml[hl->mltree_i].address){
    if(strlen(hl->mail)>5){
      tmp_mail=g_strdup(hl->mail);
      g_free(hl->mail);
      hl->mail=g_strdup_printf("%s, %s",tmp_mail, hl->ml[hl->mltree_i].address);
      g_free(tmp_mail);
    }
    else{
      hl->mail=g_strdup(hl->ml[hl->mltree_i].address);
    }
    gtk_entry_set_text(GTK_ENTRY(hl->address_entry), hl->mail);

    gtk_editable_set_position (GTK_EDITABLE(hl->address_entry),-1);
  }
}


void parse_address(typHLOG *hl){
  gchar *mlfile;
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *tmp_char=NULL, *head=NULL, *tmp_p;
  gchar *tmp;
  gint i_ml=0, i_sent=0, i_sent_max=0, i_tgt, j_ml;
  gint fr_date, ml_date;
  
  gchar *addr[MAX_MAIL];

  mlfile=g_strconcat(g_get_home_dir(),G_DIR_SEPARATOR_S,MAIL_LIST,NULL);
  fr_date=hl->fr_year*10000+hl->fr_month*100+hl->fr_day;
  
  cp=(gchar *)strtok(hl->mail,",");
  if(!cp) return;
  addr[i_sent]=g_strdup(g_strstrip(cp));

  for(;;){
    i_sent++;
    if((cp=(gchar *)strtok(NULL,","))==NULL){
      break;
    }
    else{
      addr[i_sent]=g_strdup(g_strstrip(cp));
    }
  }
  i_sent_max=i_sent;

  for(i_sent=0;i_sent<i_sent_max;i_sent++){
    i_tgt=-1;
    for(i_ml=0;i_ml<hl->ml_max;i_ml++){
      if(strcmp(addr[i_sent],hl->ml[i_ml].address)==0){
	i_tgt=i_ml;
	break;
      }
    }
    
    if(i_tgt<0){ // Add as a new address

      if(hl->ml[hl->ml_max].address) g_free(hl->ml[hl->ml_max].address);
      hl->ml[hl->ml_max].address=g_strdup(addr[i_sent]);
      hl->ml[hl->ml_max].year=hl->fr_year;
      hl->ml[hl->ml_max].month=hl->fr_month;
      hl->ml[hl->ml_max].day=hl->fr_day;

      for(j_ml=hl->ml_max-1;j_ml>=0;j_ml--){
	ml_date=hl->ml[j_ml].year*10000+hl->ml[j_ml].month*100
	  +hl->ml[j_ml].day;
	if(fr_date>ml_date){
	  swap_ml(&hl->ml[j_ml+1], &hl->ml[j_ml]);
	}
	else{
	  break;
	}
      }

      hl->ml_max++;
      
    }
    else{
      if(fr_date>ml_date){
	hl->ml[i_tgt].year=hl->fr_year;
	hl->ml[i_tgt].month=hl->fr_month;
	hl->ml[i_tgt].day=hl->fr_day;

	for(j_ml=i_tgt-1;j_ml>=0;j_ml--){
	  ml_date=hl->ml[j_ml].year*10000+hl->ml[j_ml].month*100
	    +hl->ml[j_ml].day;
	  if(fr_date>ml_date){
	    swap_ml(&hl->ml[j_ml+1], &hl->ml[j_ml]);
	  }
	  else{
	    break;
	  }
	}
      }
    }
  }

  

  if((fp=fopen(mlfile,"wb"))==NULL){
    fprintf(stderr, "Error: Cannot open \"%s\".\n",mlfile);
    g_free(mlfile);
    return;
  }

  for(i_ml=0;i_ml<hl->ml_max;i_ml++){
    fprintf(fp,"%s,%d,%d,%d\n",
	    hl->ml[i_ml].address,
	    hl->ml[i_ml].year,
	    hl->ml[i_ml].month,
	    hl->ml[i_ml].day);
  }

  fclose(fp);
  g_free(mlfile);
}


void swap_ml(MAILpara *ml1, MAILpara *ml2){
  MAILpara temp;
  
  temp=*ml2;
  *ml2=*ml1;
  *ml1=temp;
}
