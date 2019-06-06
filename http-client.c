#include "main.h"

#define BUF_LEN 65535             /* バッファのサイズ */
#define BUFFSIZE 65535

void check_msg_from_parent();
static gint fd_check_io();
gint fd_recv();
gint fd_gets();
char *read_line();
void read_response();
gint fd_write();
void write_to_server();
#ifdef USE_SSL
void write_to_SSLserver();
#endif
void error();
void PortReq();
int sftp_c();
int sftp_get_c();
int ftp_c();

int http_c_nonssl();
#ifdef USE_SSL
int http_c_ssl();
#endif

void unchunk();


#ifdef USE_SSL
gint ssl_gets();
gint ssl_read();
gint ssl_peek();
gint ssl_write();
#endif

gboolean progress_timeout();
void httpdl_signal();

void dl_camz_list();

glong get_file_size();
void write_dlsz();
void unlink_dlsz();
glong get_dlsz();

static void cancel_http();

void read_camz();

#ifdef POP_DEBUG
gboolean debug_flg=TRUE;
#else
gboolean debug_flg=FALSE;
#endif

void check_msg_from_parent(){
}

static gint fd_check_io(gint fd, GIOCondition cond)
{
	struct timeval timeout;
	fd_set fds;
	guint io_timeout=60;

	timeout.tv_sec  = io_timeout;
	timeout.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	if (cond == G_IO_IN) {
		select(fd + 1, &fds, NULL, NULL,
		       io_timeout > 0 ? &timeout : NULL);
	} else {
		select(fd + 1, NULL, &fds, NULL,
		       io_timeout > 0 ? &timeout : NULL);
	}

	if (FD_ISSET(fd, &fds)) {
		return 0;
	} else {
		g_warning("Socket IO timeout\n");
		return -1;
	}
}

gint fd_recv(gint fd, gchar *buf, gint len, gint flags)
{
  gint ret;
  
  if (fd_check_io(fd, G_IO_IN) < 0)
    return -1;

  ret = recv(fd, buf, len, flags);
  return ret;
}


gint fd_gets(gint fd, gchar *buf, gint len)
{
  gchar *newline, *bp = buf;
  gint n;
  
  if (--len < 1)
    return -1;
  do {
    if ((n = fd_recv(fd, bp, len, MSG_PEEK)) <= 0)
      return -1;
    if ((newline = memchr(bp, '\n', n)) != NULL)
      n = newline - bp + 1;
    if ((n = fd_recv(fd, bp, n, 0)) < 0)
      return -1;
    bp += n;
    len -= n;
  } while (!newline && len);
  
  *bp = '\0';
  return bp - buf;
}

/*--------------------------------------------------
 * ソケットから1行読み込む
 */
char *read_line(int socket, char *p){
    char *org_p = p;

    while (1){
        if ( read(socket, p, 1) == 0 ) break;
        if ( *p == '\n' ) break;
        p++;
    }
    *(++p) = '\0';
    return org_p;
}


/*--------------------------------------------------
 * レスポンスを取得する。^\d\d\d- ならもう1行取得
 */
void read_response(int socket, char *p){
  int ret;
    do { 
      //read_line(socket, p);
    ret=fd_gets(socket,p,BUF_LEN);
        if ( debug_flg ){
	  fprintf(stderr, "<-- %s", p);fflush(stderr);
        }
    } while ( isdigit(p[0]) &&
	      isdigit(p[1]) && 
	      isdigit(p[2]) &&
	      p[3]=='-' );

}

gint fd_write(gint fd, const gchar *buf, gint len)
{
  if (fd_check_io(fd, G_IO_OUT) < 0)
    return -1;
  
  return write(fd, buf, len);
}

/*--------------------------------------------------
 * 指定されたソケット socket に文字列 p を送信。
 * 文字列 p の終端は \0 で terminate されている
 * 必要がある
 */

void write_to_server(int socket, char *p){
    if ( debug_flg ){
        fprintf(stderr, "--> %s", p);fflush(stderr);
    }
    
    fd_write(socket, p, strlen(p));
}

#ifdef USE_SSL
void write_to_SSLserver(SSL *ssl, char *p){
  if ( debug_flg ){
    fprintf(stderr, "[SSL] <-- %s", p);fflush(stderr);
  }
  
  ssl_write(ssl, p, strlen(p));
}
#endif

void error( char *message ){
  fprintf(stderr, "%s\n", message);
    exit(1);
}

void PortReq(char *IPaddr , int *i1 , int *i2 , int *i3 , int *i4 , int *i5 , int *i6)
{
  int j ;
  char *ip ;
  IPaddr = IPaddr + 3 ;

  while( isdigit(*IPaddr) == 0 ) { IPaddr++ ; }

  ip = strtok(IPaddr,",");
  *i1 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i2 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i3 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i4 = atoi(ip) ;

  ip = strtok(NULL,",");
  *i5 = atoi(ip) ;

  ip = strtok(NULL,",");

  j = 0 ;
  while ( isdigit(*(ip +j)) != 0 ) { j += 1 ; }
  ip[j] = '\0' ;
  *i6 = atoi(ip) ;
}



int get_camz_list(typHLOG *hl){
  waitpid(http_pid,0,WNOHANG);

  if( (http_pid = fork()) <0){
    fprintf(stderr,"fork error\n");
  }
  else if(http_pid ==0) {
    http_c_nonssl(hl);
    kill(getppid(), SIGHTTPDL);  //calling http_signal
    _exit(1);
  }

  return 0;
}

void unchunk(gchar *dss_tmp){
  FILE *fp_read, *fp_write;
  gchar *unchunk_tmp;
  gchar cbuf[BUFFSIZE];
  gchar *dbuf=NULL;
  gchar *cpp;
  gchar *chunkptr, *endptr;
  long chunk_size;
  gint i, read_size=0, crlf_size=0;
  
  if ( debug_flg ){
    fprintf(stderr, "Decoding chunked file \"%s\".\n", dss_tmp);fflush(stderr);
  }
  
  fp_read=fopen(dss_tmp,"r");
  unchunk_tmp=g_strconcat(dss_tmp,"_unchunked",NULL);
  fp_write=fopen(unchunk_tmp,"wb");
  
  while(!feof(fp_read)){
    if(fgets(cbuf,BUFFSIZE-1,fp_read)){
      cpp=cbuf;
      
      read_size=strlen(cpp);
      for(i=read_size;i>=0;i--){
	if(isalnum(cpp[i])){
	  crlf_size=read_size-i-1;
	  break;
	}
	else{
	  cpp[i]='\0';
	}
      }
      chunkptr=g_strdup_printf("0x%s",cpp);
      chunk_size=strtol(chunkptr, &endptr, 0);
      g_free(chunkptr);
      
      if(chunk_size==0) break;
      
      if((dbuf = (gchar *)g_malloc(sizeof(gchar)*(chunk_size+crlf_size+1)))==NULL){
	fprintf(stderr, "!!! Memory allocation error in unchunk() \"%s\".\n", dss_tmp);
	fflush(stderr);
	break;
      }
      if(fread(dbuf,1, chunk_size+crlf_size, fp_read)){
	fwrite( dbuf , chunk_size , 1 , fp_write ); 
	if(dbuf) g_free(dbuf);
      }
      else{
	break;
      }
    }
  }
  
  fclose(fp_read);
  fclose(fp_write);
  
  unlink(dss_tmp);
  
  rename(unchunk_tmp,dss_tmp);
  
  g_free(unchunk_tmp);
}

#ifdef USE_SSL
 gint ssl_gets(SSL *ssl, gchar *buf, gint len)
{
  gchar *newline, *bp = buf;
  gint n;
  gint i;
  
  if (--len < 1)
    return -1;
  do {
    if ((n = ssl_peek(ssl, bp, len)) <= 0)
	return -1;
    if ((newline = memchr(bp, '\n', n)) != NULL)
      n = newline - bp + 1;
    if ((n = ssl_read(ssl, bp, n)) < 0)
      return -1;
    bp += n;
    len -= n;
  } while (!newline && len);
  
  *bp = '\0';
  return bp - buf;
}
#endif

#ifdef USE_SSL
 gint ssl_read(SSL *ssl, gchar *buf, gint len)
{
	gint err, ret;

	if (SSL_pending(ssl) == 0) {
		if (fd_check_io(SSL_get_rfd(ssl), G_IO_IN) < 0)
			return -1;
	}

	ret = SSL_read(ssl, buf, len);

	switch ((err = SSL_get_error(ssl, ret))) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		return 0;
	default:
		g_warning("SSL_read() returned error %d, ret = %d\n", err, ret);
		if (ret == 0)
			return 0;
		return -1;
	}
}
#endif
 
/* peek at the socket data without actually reading it */
#ifdef USE_SSL
gint ssl_peek(SSL *ssl, gchar *buf, gint len)
{
	gint err, ret;

	if (SSL_pending(ssl) == 0) {
		if (fd_check_io(SSL_get_rfd(ssl), G_IO_IN) < 0)
			return -1;
	}

	ret = SSL_peek(ssl, buf, len);

	switch ((err = SSL_get_error(ssl, ret))) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	case SSL_ERROR_ZERO_RETURN:
		return 0;
	case SSL_ERROR_SYSCALL:
	  // End of file
	  //printf("SSL_ERROR_SYSCALL ret=%d  %d\n",ret,(gint)strlen(buf));
	        return 0;
	default:
		g_warning("SSL_peek() returned error %d, ret = %d\n", err, ret);
		if (ret == 0)
			return 0;
		return -1;
	}
}
#endif

#ifdef USE_SSL
gint ssl_write(SSL *ssl, const gchar *buf, gint len)
{
	gint ret;

	ret = SSL_write(ssl, buf, len);

	switch (SSL_get_error(ssl, ret)) {
	case SSL_ERROR_NONE:
		return ret;
	case SSL_ERROR_WANT_READ:
	case SSL_ERROR_WANT_WRITE:
		errno = EAGAIN;
		return -1;
	default:
		return -1;
	}
}
#endif


int http_c_nonssl(typHLOG *hl)
{
  int command_socket;           /* コマンド用ソケット */
  int size;
  
  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  check_msg_from_parent();
   
  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hl->http_host, "http", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hl->http_host);
    return(HDSLOG_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();
   
  /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(HDSLOG_HTTP_ERROR_SOCKET);
  }
  
  check_msg_from_parent();
   
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hl->http_host);
    return(HDSLOG_HTTP_ERROR_CONNECT);
  }
  
  check_msg_from_parent();
   
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hl->http_path);
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Accept: text/plain,text/html,application/x-gzip\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Accept-Encoding: gzip\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hl->http_host);
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_server(command_socket, send_mesg);

  sprintf(send_mesg, "\r\n");
  write_to_server(command_socket, send_mesg);

  if((fp_write=fopen(hl->http_dlfile,"wb"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hl->http_dlfile);
    return(HDSLOG_HTTP_ERROR_TEMPFILE);
  }

  unlink_dlsz(hl);
  
  while((size = fd_gets(command_socket,buf,BUF_LEN)) > 2 ){
    // header lines
    if(debug_flg){
      fprintf(stderr,"--> Header: %s", buf);
    }
    if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
      chunked_flag=TRUE;
    }
    if(strncmp(buf,"Content-Length: ",strlen("Content-Length: "))==0){
      cp = buf + strlen("Content-Length: ");
      hl->http_dlsz=atol(cp);
    }
  }
  
  write_dlsz(hl);
  
  do{ // data read
    size = recv(command_socket,buf,BUF_LEN, 0);
    fwrite( &buf , size , 1 , fp_write ); 
  }while(size>0);
      
  fclose(fp_write);

  check_msg_from_parent();

  if(chunked_flag) unchunk(hl->http_dlfile);

    if((chmod(hl->http_dlfile,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hl->http_dlfile);
  }
  
  unlink_dlsz(hl);
  close(command_socket);

  return 0;
}

#ifdef USE_SSL
int http_c_ssl(typHLOG *hl)
{
  int command_socket;           /* コマンド用ソケット */
  int size;

  char send_mesg[BUF_LEN];          /* サーバに送るメッセージ */
  char buf[BUF_LEN+1];
  
  FILE *fp_write;
  FILE *fp_read;

  struct addrinfo hints, *res;
  struct addrinfo dhints, *dres;
  struct in_addr addr;
  int err, ret;

  gboolean chunked_flag=FALSE;
  gchar *cp;

  gchar *rand16=NULL;
  gint plen;

  SSL *ssl;
  SSL_CTX *ctx;

   
  check_msg_from_parent();

  /* ホストの情報 (IP アドレスなど) を取得 */
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hl->http_host, "https", &hints, &res)) !=0){
    fprintf(stderr, "Bad hostname [%s]\n", hl->http_host);
    return(HDSLOG_HTTP_ERROR_GETHOST);
  }

  check_msg_from_parent();

    /* ソケット生成 */
  if( (command_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
    fprintf(stderr, "Failed to create a new socket.\n");
    return(HDSLOG_HTTP_ERROR_SOCKET);
  }

  check_msg_from_parent();
  
  /* サーバに接続 */
  if( connect(command_socket, res->ai_addr, res->ai_addrlen) == -1){
    fprintf(stderr, "Failed to connect to %s .\n", hl->http_host);
    return(HDSLOG_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent();

  SSL_load_error_strings();
  SSL_library_init();

  ctx = SSL_CTX_new(SSLv23_client_method());
  ssl = SSL_new(ctx);
  err = SSL_set_fd(ssl, command_socket);
  while((ret=SSL_connect(ssl))!=1){
    err=SSL_get_error(ssl, ret);
    if( (err==SSL_ERROR_WANT_READ)||(err==SSL_ERROR_WANT_WRITE) ){
      g_usleep(100000);
      g_warning("SSL_connect(): try again\n");
      continue;
    }
    g_warning("SSL_connect() failed with error %d, ret=%d (%s)\n",
	      err, ret, ERR_error_string(ERR_get_error(), NULL));
    return (HDSLOG_HTTP_ERROR_CONNECT);
  }

  check_msg_from_parent();
  
  // AddrInfoの解放
  freeaddrinfo(res);

  // HTTP/1.1 ではchunked対策が必要
  sprintf(send_mesg, "GET %s HTTP/1.1\r\n", hl->http_path);
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Accept: application/xml, application/json\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "User-Agent: Mozilla/5.0\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Host: %s\r\n", hl->http_host);
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "Connection: close\r\n");
  write_to_SSLserver(ssl, send_mesg);

  sprintf(send_mesg, "\r\n");
  write_to_SSLserver(ssl, send_mesg);

  if((fp_write=fopen(hl->http_dlfile,"wb"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", hl->http_dlfile);
    return(HDSLOG_HTTP_ERROR_TEMPFILE);
  }

  while((size = ssl_gets(ssl, buf, BUF_LEN)) > 2 ){
    // header lines
    if(debug_flg){
      fprintf(stderr,"[SSL] --> Header: %s", buf);
    }
    if(NULL != (cp = strstr(buf, "Transfer-Encoding: chunked"))){
      chunked_flag=TRUE;
      }
  }
  do{ // data read
    size = SSL_read(ssl, buf, BUF_LEN);
    fwrite( &buf , size , 1 , fp_write ); 
  }while(size >0);
      
  fclose(fp_write);

  check_msg_from_parent();

  if(chunked_flag) unchunk(hl->http_dlfile);

    if((chmod(hl->http_dlfile,(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH ))) != 0){
    g_print("Cannot Chmod Temporary File %s!  Please check!!!\n",hl->http_dlfile);
  }

  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  ERR_free_strings();
  
  close(command_socket);

  return 0;
}
#endif  //USE_SSL



gboolean progress_timeout( gpointer data ){
  typHLOG *hl=(typHLOG *)data;
  glong sz=-1;
  gchar *tmp;
  gdouble frac;

  if(!hl->http_ok){
    return(FALSE);
  }

  if(gtk_widget_get_realized(hl->pbar)){
    sz=get_file_size(hl->http_dlfile);

    if(sz>0){  // After Downloading Started to get current dlsz
      if(hl->http_dlsz<0){
	hl->http_dlsz=get_dlsz(hl);
      }
    }

    if(hl->http_dlsz>0){
      frac=(gdouble)sz/(gdouble)hl->http_dlsz;
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hl->pbar),
				    frac);

      if(sz>1024*1024){
	tmp=g_strdup_printf("%d%% Downloaded (%.2lf / %.2lf MB)",
			    (gint)(frac*100.),
			    (gdouble)sz/1024./1024.,
			    (gdouble)hl->http_dlsz/1024./1024.);
      }
      else if(sz>1024){
	tmp=g_strdup_printf("%d%% Downloaded (%ld / %ld kB)",
			    (gint)(frac*100.),
			    sz/1024,
			    hl->http_dlsz/1024);
      }
      else{
	tmp=g_strdup_printf("%d%% Downloaded (%ld / %ld bytes)",
			    (gint)(frac*100.),
			    sz, hl->http_dlsz);
      }
    }
    else{
      gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hl->pbar));

      if(sz>1024*1024){
	tmp=g_strdup_printf("Downloaded %.2lf MB",
			    (gdouble)sz/1024./1024.);
      }
      else if(sz>1024){
	tmp=g_strdup_printf("Downloaded %ld kB", sz/1024);
      }
      else if (sz>0){
	tmp=g_strdup_printf("Downloaded %ld bytes", sz);
      }
      else{
	tmp=g_strdup_printf("Waiting for HTTP server response ...");
      }
    }
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hl->pbar),
			      tmp);
    g_free(tmp);
  }
  
  return TRUE;
}


void httpdl_signal(int sig){
  pid_t child_pid=0;

  gtk_main_quit();

  do{
    int child_ret;
    child_pid=waitpid(http_pid, &child_ret,WNOHANG);
  } while((child_pid>0)||(child_pid!=-1));
}

void popup_dl_camz_list(GtkWidget *w, gpointer gdata){
  typHLOG *hl = (typHLOG *)gdata;
  
  dl_camz_list(hl, TRUE);
}

void dl_camz_list(typHLOG *hl,  gboolean flag_popup){
  GtkWidget *dialog, *vbox, *label, *button, *bar, *hbox;
  gint timer=-1;
  static struct sigaction act;
  gchar *tmp;

  hl->http_ok=TRUE;
  
  if(hl->http_host) g_free(hl->http_host);
  hl->http_host=g_strdup(HTTP_CAMZ_HOST);

  if(hl->http_path) g_free(hl->http_path);
  hl->http_path=g_strdup(HTTP_CAMZ_PATH);

  if(hl->http_dlfile) g_free(hl->http_dlfile);
  hl->http_dlfile=g_strdup_printf("%s%s%s-%d",
				g_get_tmp_dir(), G_DIR_SEPARATOR_S,
				HTTP_CAMZ_FILE,   getuid());

  if(access(hl->http_dlfile, F_OK)==0) unlink(hl->http_dlfile);

  dialog = gtk_dialog_new();
  
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(dialog),5);
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),5);
  gtk_window_set_title(GTK_WINDOW(dialog),"Subaru HDS LOG : Downloading Latest CamZ Values");
  gtk_window_set_decorated(GTK_WINDOW(dialog),TRUE);

  label=gtkut_label_new("Downloading the latest CamZ ...");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,TRUE,TRUE,0);
  gtk_widget_show(label);
  
  hl->http_dlsz=-1;
  hl->pbar=gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hl->pbar,TRUE,TRUE,0);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(hl->pbar));
#ifdef USE_GTK3
  gtk_orientable_set_orientation (GTK_ORIENTABLE (hl->pbar), 
				  GTK_ORIENTATION_HORIZONTAL);
  css_change_pbar_height(hl->pbar,15);
  gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(hl->pbar), TRUE);
#else
  gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (hl->pbar), 
				    GTK_PROGRESS_RIGHT_TO_LEFT);
#endif
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(hl->pbar),0.05);
  gtk_widget_show(hl->pbar);
  
#ifdef USE_GTK3
  bar = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
  bar = gtk_hseparator_new();
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     bar,FALSE, FALSE, 0);

  label=gtkut_label_new("Checking the latest CamZ values ...");
#ifdef USE_GTK3
  gtk_widget_set_halign (label, GTK_ALIGN_START);
  gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
#else
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
#endif
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     label,FALSE,FALSE,0);


  hbox = gtkut_hbox_new (FALSE, 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		     hbox,TRUE, TRUE, 0);

#ifdef USE_GTK3
  button=gtkut_button_new_from_icon_name("Cancel","process-stop");
#else
  button=gtkut_button_new_from_stock("Cancel",GTK_STOCK_CANCEL);
#endif
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE,0),
  g_signal_connect(button,"pressed",
		   G_CALLBACK(cancel_http), 
		    (gpointer)hl);

  gtk_widget_show_all(dialog);

  timer=g_timeout_add(100, 
		      (GSourceFunc)progress_timeout,
		      (gpointer)hl);
  
  act.sa_handler=httpdl_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags=0;
  if(sigaction(SIGHTTPDL, &act, NULL)==-1){
    fprintf(stderr,"Error in sigaction (SIGHTTPDL).\n");
  }

  gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
  
  get_camz_list(hl);
  gtk_main();
  
  gtk_window_set_modal(GTK_WINDOW(dialog),FALSE);
  if(timer!=-1) g_source_remove(timer);

  if(GTK_IS_WIDGET(dialog)) gtk_widget_destroy(dialog);

  if(access(hl->http_dlfile, F_OK)==0){
    read_camz(hl);
  }
  else{
    tmp=g_strconcat(hl->http_host,
		    hl->http_path,
		    NULL);
    fprintf(stderr, "Error : Failed to download CamZ file \"%s\"\n", tmp);
    g_free(tmp);
  }
}

glong get_file_size(gchar *fname)
{
  FILE *fp;
  long sz;

  fp = fopen( fname, "rb" );
  if( fp == NULL ){
    return -1;
  }

  fseek( fp, 0, SEEK_END );
  sz = ftell( fp );

  fclose( fp );
  return sz;
}


void write_dlsz(typHLOG *hl){
  FILE *fp;
  gchar *tmp_file;

  tmp_file=g_strdup_printf("%s%s%s-%d",
			   g_get_tmp_dir(), G_DIR_SEPARATOR_S,
			   HTTP_DLSZ_FILE, getuid());
  
  if((fp=fopen(tmp_file,"w"))==NULL){
    fprintf(stderr," File Write Error  \"%s\" \n", tmp_file);
    g_free(tmp_file);
    return;
  }

  fprintf(fp, "%ld\n",hl->http_dlsz);
  fclose(fp);
  
  g_free(tmp_file);
  return;
}


void unlink_dlsz(typHLOG *hl){
  gchar *tmp_file;

  hl->http_dlsz=0;
  
  tmp_file=g_strdup_printf("%s%s%s-%d",
			   g_get_tmp_dir(), G_DIR_SEPARATOR_S,
			   HTTP_DLSZ_FILE, getuid());
  
  if(access(tmp_file, F_OK)==0){
    unlink(tmp_file);
  }

  g_free(tmp_file);
  return;
}


glong get_dlsz(typHLOG *hl){
  FILE *fp;
  gchar *tmp_file;
  glong sz=0;
  gchar buf[10];
  
  tmp_file=g_strdup_printf("%s%s%s-%d",
			   g_get_tmp_dir(), G_DIR_SEPARATOR_S,
			   HTTP_DLSZ_FILE, getuid());
  
  if((fp=fopen(tmp_file,"r"))==NULL){
    g_free(tmp_file);
    return(-1);
  }

  if(fgets(buf,10-1,fp)){
    sz = atol(buf);
  }
  fclose(fp);

  unlink(tmp_file);
  
  g_free(tmp_file);
  return (sz);
}


static void cancel_http(GtkWidget *w, gpointer gdata){
  typHLOG *hl = (typHLOG *)gdata;
  pid_t child_pid=0;

  hl->http_ok=FALSE;

  if(http_pid){
    kill(http_pid, SIGKILL);
    gtk_main_quit();

    do{
      int child_ret;
      child_pid=waitpid(http_pid, &child_ret,WNOHANG);
    } while((child_pid>0)||(child_pid!=-1));
 
    http_pid=0;
  }
  else{
    gtk_main_quit();
  }

  unlink_dlsz(hl);
  if(access(hl->http_dlfile, F_OK)==0) unlink(hl->http_dlfile);
}


void read_camz(typHLOG *hl){
  FILE *fp;
  gchar *buf=NULL, *cp, *cpp, *tmp_char=NULL, *head=NULL, *tmp_p;
  gchar *tmp;
  gint i;

  if((fp=fopen(hl->http_dlfile,"rb"))==NULL){
    fprintf(stderr, "Error: Cannot open \"%s\".\n",hl->http_dlfile);
    return;
  }
  
  while((buf=fgets_new(fp))!=NULL){
    tmp_char=(char *)strtok(buf,",");
    
    if(strncmp(tmp_char,"CamZ_B",strlen("CamZ_B"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	hl->camz_b=g_strtod(tmp_p,NULL);
      }
    }
    if(strncmp(tmp_char,"CamZ_R",strlen("CamZ_R"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	hl->camz_r=g_strtod(tmp_p,NULL);
      }
    }
    if(strncmp(tmp_char,"RdCross",strlen("RdCross"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	hl->d_cross_r=g_strtod(tmp_p,NULL);
      }
    }
    if(strncmp(tmp_char,"BdCross",strlen("BdCross"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	hl->d_cross_b=g_strtod(tmp_p,NULL);
      }
    }
    if(strncmp(tmp_char,"Echelle",strlen("Echelle"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	hl->echelle0=g_strtod(tmp_p,NULL);
      }
    }
    if(strncmp(tmp_char,"Date",strlen("Date"))==0){
      if((tmp_p=strtok(NULL,","))!=NULL){
	if(hl->camz_date) g_free(hl->camz_date);
	hl->camz_date=g_strdup(tmp_p);
      }
    }
  }
  fclose(fp);

  unlink(hl->http_dlfile);
}
