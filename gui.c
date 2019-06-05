#include "main.h"

void set_fr_e_date(typHLOG *hl){
  gchar *tmp;
    
  tmp=g_strdup_printf("%s %d, %d",
		      cal_month[hl->buf_month-1],
		      hl->buf_day,
		      hl->buf_year);

  gtk_entry_set_text(GTK_ENTRY(hl->fr_e),tmp);
  g_free(tmp);

}

void select_fr_calendar (GtkWidget *widget, gpointer gdata){
  typHLOG *hl=(typHLOG *)gdata;
  gint i_list;

  gtk_calendar_get_date(GTK_CALENDAR(widget),
			&hl->buf_year,
			&hl->buf_month,
			&hl->buf_day);
  hl->buf_month++;

  set_fr_e_date(hl);

  //update_objtree(hg);
  
  gtk_main_quit();
}


void popup_fr_calendar (GtkWidget *widget, gpointer gdata)
{
  GtkWidget *dialog, *calendar;
  GtkAllocation *allocation=g_new(GtkAllocation, 1);
  gint root_x, root_y;
  typHLOG *hl=(typHLOG *)gdata;

  gtk_widget_get_allocation(widget,allocation);

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(hl->w_top));
  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  gtk_window_get_position(GTK_WINDOW(hl->w_top),&root_x,&root_y);

  g_signal_connect(G_OBJECT(dialog),"delete-event",
		   G_CALLBACK(gtk_main_quit),NULL);
  gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);

  calendar=gtk_calendar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     calendar,FALSE, FALSE, 0);

  gtk_calendar_select_month(GTK_CALENDAR(calendar),
			    hl->buf_month-1,
			    hl->buf_year);

  gtk_calendar_select_day(GTK_CALENDAR(calendar),
			  hl->buf_day);

  g_signal_connect(G_OBJECT(calendar),
		   "day-selected-double-click",
		   G_CALLBACK(select_fr_calendar), 
		   (gpointer)hl);

  gtk_window_set_keep_above(GTK_WINDOW(dialog),TRUE);
  gtk_window_move(GTK_WINDOW(dialog),
		  root_x+allocation->x,
		  root_y+allocation->y);
  g_free(allocation);
  gtk_widget_show_all(dialog);
  gtk_main();
  gtk_widget_destroy(dialog);
}
