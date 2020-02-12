void popup_message(GtkWidget *parent, gchar* stock_id,gint delay, ...);
void xdo_error();
gboolean send_xdo();

void edit_ap();
void edit_flat();
void edit_thar();
void ref1_ap();
void ref2_ap();
void ref1_thar();
void ref2_thar();

void iraf_apall();
void iraf_ecreidentify();
void iraf_ecidentify();
void get_flat_scnm();

void set_ql_frame_label();
void set_cal_frame_red();
void set_cal_frame_blue();
gchar *get_work_dir_sumda();
gchar *get_data_dir_sumda();
gchar *get_uparm_dir_sumda();
void edit_uparm();
gchar *get_indir();
void ql_obj_red();
void ql_obj_blue();
void ql_ap_red();
void ql_ap_blue();
void ql_flat_red();
void ql_flat_blue();
void ql_thar_red();
void ql_thar_blue();
void ql_find_red();
void ql_find_blue();
