enum
{ COLUMN_FRAME_NUMBER,
  COLUMN_FRAME_ID,
  COLUMN_FRAME_NAME,
  COLUMN_FRAME_HST,
  COLUMN_FRAME_EXPTIME,
  COLUMN_FRAME_SECZ,
  COLUMN_FRAME_FIL1,
  COLUMN_FRAME_FIL2,
  COLUMN_FRAME_SLITWID,
  COLUMN_FRAME_SLITLEN,
  COLUMN_FRAME_CROSS,
  COLUMN_FRAME_CROSSCOL,
  COLUMN_FRAME_ECHELLE,
  COLUMN_FRAME_BIN1,
  COLUMN_FRAME_BIN2,
  COLUMN_FRAME_I2,
  COLUMN_FRAME_IS,
  COLUMN_FRAME_CAMZ,
  COLUMN_FRAME_IMR,
  COLUMN_FRAME_SLTPA,
  COLUMN_FRAME_ADC,
  COLUMN_FRAME_NOTE,
  COLUMN_FRAME_COLFG,
  COLUMN_FRAME_COLBG,
  COLUMN_FRAME_CROSSFG,
  COLUMN_FRAME_WEIGHT,
  NUM_FRAME_COLUMNS
};


void make_frame_tree();
void frame_tree_update_item();
void frame_tree_select_last();

