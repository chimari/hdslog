#include "main.h"

static void frame_tree_add_columns();
static GtkTreeModel *frame_tree_create_items_model();
static void focus_frame_tree_item ();
void frame_tree_cell_data_func();
static void cell_editing();
static void cell_canceled();
static void cell_edited();

void make_frame_tree(typHLOG *hl){
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *sw;
  GtkWidget *button;
  GtkTreeModel *items_model;
  
  if(flag_make_frame_tree){
    gtk_widget_destroy(hl->frame_tree);
  }
  else flag_make_frame_tree=TRUE;

  items_model = frame_tree_create_items_model (hl);

  /* create tree view */
  hl->frame_tree = gtk_tree_view_new_with_model (items_model);
  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (hl->frame_tree)),
			       GTK_SELECTION_SINGLE);
  frame_tree_add_columns (hl, GTK_TREE_VIEW (hl->frame_tree), 
		       items_model);

  g_object_unref(items_model);
  
  gtk_container_add (GTK_CONTAINER (hl->scrwin), hl->frame_tree);

  gtk_widget_show_all(hl->frame_tree);
}


static void frame_tree_add_columns (typHLOG *hl,
			     GtkTreeView  *treeview, 
			     GtkTreeModel *items_model)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;  

  /* number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_NUMBER));
  column=gtk_tree_view_column_new_with_attributes ("No.",
						   renderer,
						   "text",
						   COLUMN_FRAME_NUMBER,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif
						   NULL);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_NUMBER),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Frame ID column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_ID));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_ID,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif
						   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Frame ID <span size=\"smaller\">(for Red CCD)</span>");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_ID),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Object Name column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_NAME));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_NAME,
						   "weight", COLUMN_FRAME_WEIGHT,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Object Name");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_NAME),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* HST column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_HST));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_HST,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "HST<sub>start</sub>");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_HST),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* EXPTIME column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_EXPTIME));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_EXPTIME,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Exp.");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_EXPTIME),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* SecZ column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_SECZ));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_SECZ,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "<i>sec</i> <i>Z</i>");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_SECZ),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Filter column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_FIL1));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_FIL1,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Filter");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_FIL1),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Slit column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_SLITWID));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_SLITWID,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Slit <span size=\"smaller\">(WxL)</span>");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_SLITWID),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Corss */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_CROSS));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_CROSS,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_CROSSFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_CROSSFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Cross D.");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_CROSS),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* Echelle */
  if(hl->ech_flag){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_ECHELLE));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_ECHELLE,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "Echelle");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_ECHELLE),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* Binning */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_BIN1));
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_BIN1,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Binning");
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_BIN1),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


  /* I2 */
  if(hl->i2_flag){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_I2));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_I2,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "I<sub>2</sub>-cell");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_I2),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }

  /* Image Slicer */
  if(hl->is_flag){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_IS));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_IS,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "Img Slicer");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_IS),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* CamZ */
  if(hl->camz_flag){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_CAMZ));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_CAMZ,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "Cam Z");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_CAMZ),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }

  /* ImR */
  if(hl->imr_flag){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_IMR));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_IMR,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "ImR");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_IMR),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);


    /* Slit PA */
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_SLTPA));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_SLTPA,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "PA");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_SLTPA),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }


  /* ADC */
  if(hl->adc_flag){
    renderer = gtk_cell_renderer_text_new ();
    g_object_set_data (G_OBJECT (renderer), "column", 
		       GINT_TO_POINTER (COLUMN_FRAME_ADC));
    column=gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "text", 
						     COLUMN_FRAME_ADC,
#ifdef USE_GTK3
						     "foreground-rgba", COLUMN_FRAME_COLFG,
						     "background-rgba", COLUMN_FRAME_COLBG,
#else
						     "foreground-gdk", COLUMN_FRAME_COLFG,
						     "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						     NULL);
    gtkut_tree_view_column_set_markup(column, "ADC");
    gtk_tree_view_column_set_cell_data_func(column, renderer,
					    frame_tree_cell_data_func,
					    GUINT_TO_POINTER(COLUMN_FRAME_ADC),
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
  }

  /* Note */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", 
  		     GINT_TO_POINTER (COLUMN_FRAME_NOTE));
  g_object_set (renderer,
                "editable", TRUE,
                NULL);
  column=gtk_tree_view_column_new_with_attributes (NULL,
						   renderer,
						   "text", 
						   COLUMN_FRAME_NOTE,
#ifdef USE_GTK3
						   "foreground-rgba", COLUMN_FRAME_COLFG,
						   "background-rgba", COLUMN_FRAME_COLBG,
#else
						   "foreground-gdk", COLUMN_FRAME_COLFG,
						   "background-gdk", COLUMN_FRAME_COLBG,
#endif					   
						   NULL);
  gtkut_tree_view_column_set_markup(column, "Note <span size=\"smaller\">(<i>editable</i>)</span>");
  g_signal_connect (renderer, "editing_started",
		    G_CALLBACK (cell_editing), NULL);
  g_signal_connect (renderer, "editing_canceled",
		    G_CALLBACK (cell_canceled), NULL);
  g_signal_connect (renderer, "edited", G_CALLBACK (cell_edited), hl);
  gtk_tree_view_column_set_cell_data_func(column, renderer,
					  frame_tree_cell_data_func,
					  GUINT_TO_POINTER(COLUMN_FRAME_NOTE),
					  NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW (treeview),column);
}


static GtkTreeModel *
frame_tree_create_items_model (typHLOG *hl)
{
  gint i_frm = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create list store */
  model = gtk_list_store_new (NUM_FRAME_COLUMNS,
			      G_TYPE_INT,      //COLUMN_FRAME_NUMBER,
			      G_TYPE_STRING,   //COLUMN_FRAME_ID,
			      G_TYPE_STRING,   //COLUMN_FRAME_NAME,
			      G_TYPE_STRING,   //COLUMN_FRAME_HST,
			      G_TYPE_INT,      //COLUMN_FRAME_EXPTIME,
			      G_TYPE_DOUBLE,   //COLUMN_FRAME_SECZ,
			      G_TYPE_STRING,   //COLUMN_FRAME_FIL1,
			      G_TYPE_STRING,   //COLUMN_FRAME_FIL2,
			      G_TYPE_DOUBLE,   //COLUMN_FRAME_SLITWID,
			      G_TYPE_DOUBLE,   //COLUMN_FRAME_SLITLEN,
			      G_TYPE_STRING,   //COLUMN_FRAME_CROSS,
			      G_TYPE_STRING,   //COLUMN_FRAME_CROSSCOL,
			      G_TYPE_DOUBLE,   //COLUMN_FRAME_ECHELLE,
			      G_TYPE_INT,      //COLUMN_FRAME_BIN1,
			      G_TYPE_INT,      //COLUMN_FRAME_BIN2,
			      G_TYPE_STRING,   //COLUMN_FRAME_I2,
			      G_TYPE_STRING,   //COLUMN_FRAME_IS,
			      G_TYPE_INT,      //COLUMN_FRAME_CAMZ,
			      G_TYPE_STRING,   //COLUMN_FRAME_IMR,
			      G_TYPE_DOUBLE,   //COLUMN_FRAME_SLTPA,
			      G_TYPE_STRING,   //COLUMN_FRAME_ADC,
			      G_TYPE_STRING,   //COLUMN_FRAME_NOTE,
#ifdef USE_GTK3
			      GDK_TYPE_RGBA,   //fgcolor
			      GDK_TYPE_RGBA,   //bgcolor
			      GDK_TYPE_RGBA,    //cross fg color
#else
			      GDK_TYPE_COLOR,  //fgcolor
			      GDK_TYPE_COLOR,  //bgcolor
			      GDK_TYPE_COLOR,   //cross fg color
#endif
			      G_TYPE_INT       // weight
			      );

  for (i_frm = 0; i_frm < hl->num; i_frm++){
    gtk_list_store_append (model, &iter);
    frame_tree_update_item(hl, GTK_TREE_MODEL(model), iter, i_frm);
  }
  
  return GTK_TREE_MODEL (model);
}


void frame_tree_update_item(typHLOG *hl, 
			    GtkTreeModel *model, 
			    GtkTreeIter iter, 
			    gint i_frm)
{
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      COLUMN_FRAME_NUMBER, i_frm,
		      COLUMN_FRAME_ID,     hl->frame[i_frm].id,
		      COLUMN_FRAME_NAME,   hl->frame[i_frm].name,
		      COLUMN_FRAME_HST,    hl->frame[i_frm].hst,
		      COLUMN_FRAME_EXPTIME,hl->frame[i_frm].exp,
		      COLUMN_FRAME_SECZ,   hl->frame[i_frm].secz,
		      COLUMN_FRAME_FIL1,   hl->frame[i_frm].fil1,
		      COLUMN_FRAME_FIL2,   hl->frame[i_frm].fil2,
		      COLUMN_FRAME_SLITWID,hl->frame[i_frm].slt_wid,
		      COLUMN_FRAME_SLITLEN,hl->frame[i_frm].slt_len,
		      COLUMN_FRAME_CROSS,  hl->frame[i_frm].setup,
		      COLUMN_FRAME_CROSSCOL,hl->frame[i_frm].crossd,
		      COLUMN_FRAME_ECHELLE,hl->frame[i_frm].erotan,
		      COLUMN_FRAME_BIN1,   hl->frame[i_frm].bin1,
		      COLUMN_FRAME_BIN2,   hl->frame[i_frm].bin2,
		      COLUMN_FRAME_I2,     hl->frame[i_frm].i2,
		      COLUMN_FRAME_IS,     hl->frame[i_frm].is,
		      COLUMN_FRAME_CAMZ,   hl->frame[i_frm].camz,
		      COLUMN_FRAME_IMR,    hl->frame[i_frm].imr,
		      COLUMN_FRAME_SLTPA,  hl->frame[i_frm].slt_pa,
		      COLUMN_FRAME_ADC,    hl->frame[i_frm].adc,
		      COLUMN_FRAME_NOTE,   hl->frame[i_frm].note.txt,
		      COLUMN_FRAME_COLBG, (i_frm%2==0) ? &color_white : &color_lgreen,
		      -1);

  if(strcmp(hl->frame[i_frm].name,"FLAT")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_flat, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_NORMAL,
			-1);
  }
  else if(strcmp(hl->frame[i_frm].name,"COMPARISON")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_comp, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_NORMAL,
			-1);
  }
  else if(strcmp(hl->frame[i_frm].name,"BIAS")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_bias, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_NORMAL,
			-1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_COLFG, &color_black, -1);
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_WEIGHT,
			PANGO_WEIGHT_BOLD,
			-1);
  }

  if(strcmp(hl->frame[i_frm].crossd,"RED")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_CROSSFG, &color_red, -1);
  }
  else if(strcmp(hl->frame[i_frm].crossd,"BLUE")==0){
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_CROSSFG, &color_blue, -1);
  }
  else{
    gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			COLUMN_FRAME_CROSSFG, &color_black, -1);
  }
}

void frame_tree_update_note(typHLOG *hl, 
			    GtkTreeModel *model, 
			    GtkTreeIter iter, 
			    gint i_frm)
{
  gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		      -1);
}

static void
focus_frame_tree_item (GtkWidget *widget, gpointer data)
{
  GtkTreeIter iter;
  typHLOG *hl = (typHLOG *)data;
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(hl->frame_tree));
  GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(hl->frame_tree));
  gint i_frm;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter)){
    GtkTreePath *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_model_get (model, &iter, COLUMN_FRAME_NUMBER, &i_frm, -1);
    gtk_tree_path_free (path);

    hl->frame_tree_i=i_frm;
  }
 
}


void frame_tree_cell_data_func(GtkTreeViewColumn *col , 
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

  switch (index) {
  case COLUMN_FRAME_ID:
  case COLUMN_FRAME_NAME:
  case COLUMN_FRAME_HST:
  case COLUMN_FRAME_FIL1:
  case COLUMN_FRAME_CROSS:
  case COLUMN_FRAME_I2:
  case COLUMN_FRAME_IS:
  case COLUMN_FRAME_IMR:
  case COLUMN_FRAME_ADC:
  case COLUMN_FRAME_NOTE:
    gtk_tree_model_get (model, iter, 
			index, &c_buf,
			-1);
    break;

  case COLUMN_FRAME_NUMBER:
  case COLUMN_FRAME_EXPTIME:
  case COLUMN_FRAME_BIN1:
  case COLUMN_FRAME_CAMZ:
    gtk_tree_model_get (model, iter, 
			index, &i_buf,
			-1);
    break;

  case COLUMN_FRAME_SECZ:
  case COLUMN_FRAME_SLITWID:
  case COLUMN_FRAME_SLTPA:
  case COLUMN_FRAME_ECHELLE:
    gtk_tree_model_get (model, iter, 
			index, &d_buf,
			-1);
    break;
  }

  switch (index) {
  case COLUMN_FRAME_FIL1:
    if(strcmp(c_buf,"FREE")==0){
      g_free(c_buf);
      c_buf=g_strdup("---");
    }
    gtk_tree_model_get (model, iter, COLUMN_FRAME_FIL2, &c_buf2, -1);

    if(strcmp(c_buf2,"FREE")==0){
      g_free(c_buf2);
      c_buf2=g_strdup("---");
    }
    str=g_strdup_printf("%s/%s",c_buf,c_buf2);
    break;

  case COLUMN_FRAME_IS:
  case COLUMN_FRAME_IMR:
    if(strcmp(c_buf,"NONE")==0){
      str=g_strdup("---");
    }
    else{
      str=g_strdup(c_buf);
    }
    break;

  case COLUMN_FRAME_I2:
    if(strcmp(c_buf,"UNKNOWN")==0){
      str=g_strdup("---");
    }
    else{
      str=g_strdup(c_buf);
    }
    break;

  case COLUMN_FRAME_NUMBER:
    str=g_strdup_printf("%d",i_buf+1);
    break;

  case COLUMN_FRAME_EXPTIME:
    str=g_strdup_printf("%ds",i_buf);
    break;

  case COLUMN_FRAME_BIN1:
    gtk_tree_model_get (model, iter, COLUMN_FRAME_BIN2, &i_buf2, -1);
    str=g_strdup_printf("%dx%d",i_buf,i_buf2);
    break;

  case COLUMN_FRAME_CAMZ:
    str=g_strdup_printf("%d",i_buf);
    break;

  case COLUMN_FRAME_SECZ:
  case COLUMN_FRAME_SLTPA:
    str=g_strdup_printf("%.2lf",d_buf);
    break;

  case COLUMN_FRAME_SLITWID:
    gtk_tree_model_get (model, iter, COLUMN_FRAME_SLITLEN, &d_buf2, -1);
    str=g_strdup_printf("%.2lfx%.2lf",d_buf, d_buf2);
    break;

  case COLUMN_FRAME_ECHELLE:
    str=g_strdup_printf("%.0lf",d_buf);
    break;

  default:
    str=g_strdup(c_buf);
    break;
  }

  g_object_set(renderer, "text", str, NULL);
  if(str)g_free(str);
}


static void cell_editing (GtkCellRendererText *cell)
{
  Flag_tree_editing=TRUE;
}

static void cell_canceled (GtkCellRendererText *cell)
{
  Flag_tree_editing=FALSE;
}

static void cell_edited (GtkCellRendererText *cell,
			 const gchar         *path_string,
			 const gchar         *new_text,
			 gpointer             data)
{
  typHLOG *hl = (typHLOG *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));
  gchar tmp[128];

  gtk_tree_model_get_iter (model, &iter, path);

  switch (column) {
  case COLUMN_FRAME_NOTE:
    {
      gint i;
      gchar *old_text;
      
      gtk_tree_model_get (model, &iter, column, &old_text, -1);
      g_free (old_text);
      
      gtk_tree_model_get (model, &iter, COLUMN_FRAME_NUMBER, &i, -1);
      
      if(hl->frame[i].note.txt) g_free(hl->frame[i].note.txt);
      hl->frame[i].note.txt=g_strdup(new_text);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
			  hl->frame[i].note.txt, -1);
      
      if(hl->frame[i].note.auto_fl){
	hl->frame[i].note.auto_fl=FALSE;
      }
      else{
	hl->frame[i].note.time=time(NULL);
      }
    }
    break;
  }

  Flag_tree_editing=FALSE;
  gtk_tree_path_free (path);
}


void frame_tree_select_last(typHLOG *hl){
  gint i, i_frame;
  GtkTreeModel *model 
    = gtk_tree_view_get_model(GTK_TREE_VIEW(hl->frame_tree));
  GtkTreePath *path;
  GtkTreeIter  iter;
  GtkAdjustment *adj;

  if(Flag_tree_editing) return;
  
  path=gtk_tree_path_new_first();
  
  for(i=0;i<hl->num-1;i++){
      gtk_tree_path_next(path);
  }
  gtk_tree_view_set_cursor(GTK_TREE_VIEW(hl->frame_tree), 
			   path, NULL, FALSE);
  gtk_tree_path_free(path);

  adj=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(hl->scrwin));
  gtk_adjustment_set_value(adj,
			   gtk_adjustment_get_upper(adj)
			   -gtk_adjustment_get_page_size(adj));
}
