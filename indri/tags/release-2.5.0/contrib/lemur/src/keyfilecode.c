/*                                                               */
/* Copyright 1984,1985,1986,1988,1989,1990,2003,2004,2005,2006   */
/*   by Howard Turtle                                            */

#define boolean int
#define true 1
#define false 0

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "keyerr.h"
#include "keydef.h"
#include "keyprint.h"
#include "keyfile.h"
/* platform specific file io bits */
#include "fileio.h"

#define show_errors true
#define trace_io false

/*#define log_buffers*/

#ifdef log_buffers
FILE
*buffer_log;
#endif

int
  get_buffer_cnt=0,
  replace_buffer_cnt=0,
  get_index_cnt=0,
  hash_chain_cnt=0;

int
  read_cnt=0,
  write_cnt=0;

static level0_pntr
  null0_ptr = {max_segment,0,0},
  dummy_ptr = {0,0,0};
static struct leveln_pntr
  nulln_ptr = {max_segment,0};

static char
  index_caption[3][10]={"user    \0","free_rec\0","free_lc \0"};

void print_buffer_caption();
void print_buffer_MRU_chain();
void print_key_struct();

static int allocate_block();
static boolean allocate_rec();
static void replace_max_key();
static void deallocate_rec();
static void check_ix_block_compression();
static void update_index();
static void index_delete();
static void split_block();
static FILE *file_index();


#define eq_pntr(p1,p2) ((p1.block==p2.block) && (p1.segment==p2.segment))
#define null_pntr(p1) ((p1.segment==max_segment) && (p1.block==0))

/* Miscellaneous procedures */

#ifndef min
static int min(int i, int j)
{
  if ( i<j ) return(i);
  else return(j);
}
#endif

#define mvc(t1,sc1,t2,sc2,lc) memmove((unsigned char *)t2+sc2,(unsigned char *)t1+sc1,(size_t)lc)

/*void mvc(t1,sc1,t2,sc2,lc)
unsigned char t1[],t2[]; int sc1,sc2,lc;
{
  memmove(t2+sc2,t1+sc1,lc);
}*/


static void print_leveln_pntr(char caption[], struct leveln_pntr *pn)
{
  printf("%s%u/",caption,pn->segment);
  printf(UINT64_format,pn->block);
  printf(" ");
}

static void print_level0_pntr(char caption[], level0_pntr *p0)
{
  printf("%s%u/",caption,p0->segment);
  printf(UINT64_format,p0->sc);
  printf("/%u ",p0->lc);
}

/* compressed_int_lc returns the length of a compressed integer */
/*   without actually uncompressing it.  Note that it doesn't   */
/*   care what the size of the uncompressed int would be.       */

static int compressed_int_lc(char buf[])
{int i=0;

  while ( (buf[i] & 128)!=0 ) i++;
  return(i+1);
}

/*static int compressed_int_lc(char buf[], unsigned offset)
{int i=0;

  while ( (buf[offset+i] & 128)!=0 ) i++;
  return(i+1);
}*/

/* uncompress_UINT16 uncompresses an integer compressed in      */
/*    array p and returns the number of bytes consumed to       */
/*    decompress the int.                                       */


static int uncompress_UINT16(UINT16 *i, unsigned char p[])
{int j=0; boolean done=false;

  *i = 0;
  do {
    *i = *i | (p[j] & 127);
    if ( (p[j] & 128)!=0 ) {
      *i = *i << 7;
      j++;
    }
    else done = true;
  } while ( !done );
  return(j+1);
}

/* uncompress_UINT32 uncompresses an integer compressed in      */
/*    array p and returns the number of bytes consumed to       */
/*    decompress the int.                                       */

static int uncompress_UINT32(UINT32 *i, unsigned char p[])
{int j=0; boolean done=false;

  *i = 0;
  do {
    *i = *i | (p[j] & 127);
    if ( (p[j] & 128)!=0 ) {
      *i = *i << 7;
      j++;
    }
    else done = true;
  } while ( !done );
  return(j+1);
}

/* uncompress_UINT64 uncompresses an integer compressed in      */
/*    array p and returns the number of bytes consumed to       */
/*    decompress the int.                                       */

static int uncompress_UINT64(UINT64 *i, unsigned char p[])
{int j=0; boolean done=false;

  *i = 0;
  do {
    *i = *i | (p[j] & 127);
    if ( (p[j] & 128)!=0 ) {
      *i = *i << 7;
      j++;
    }
    else done = true;
  } while ( !done );
  return(j+1);
}

/* compress_UINT32 compresses UINT32 i into a byte string and     */
/*   returns the length of the compressed string. Note that ptr   */
/*   points to the rightmost character in the compressed string,  */
/*   i.e. the int is compressed from ptr to the left.             */


static int compress_UINT32(UINT32 i, unsigned char *ptr)
{unsigned char *p;

  p = ptr;
  do {
    *p = (i & 127) | 128;
    p--;
    i = i>>7;
  } while ( i>0 );
  *ptr = *ptr & 127; /* high bit off */
  return((int)(ptr-p));
}

/* compress_UINT64 compresses UINT64 i into a byte string and     */
/*   returns the length of the compressed string. Note that ptr   */
/*   points to the rightmost character in the compressed string,  */
/*   i.e. the int is compressed from ptr to the left.             */


static int compress_UINT64(UINT64 i, unsigned char *ptr)
{unsigned char *p;

  p = ptr;
  do {
    *p = (i & 127) | 128;
    p--;
    i = i>>7;
  } while ( i>0 );
  *ptr = *ptr & 127; /* high bit off */
  return((int)(ptr-p));
}

/* UINT32_lc_if_compressed returns the length that UINT32 i      */
/*   will occupy once compressed.  This function is used to      */
/*   check the compressed size of pointer segments and lengths.  */
/*   Segments are always short and lengths tend to be short so   */
/*   favoring small values of i makes sense.                     */

static int UINT32_lc_if_compressed(UINT32 i)
{
  if      ( i<128    )    return(1);
  else if ( i<16384  )    return(2);
  else if ( i<2097152 )   return(3);
  else if ( i<268435456 ) return(4);
  else return(5);
}

/* UINT64_lc_if_compressed returns the length that UINT64 i      */
/*   will occupy once compressed.                                */

static int UINT64_lc_if_compressed(UINT64 i)
{
  if      ( i<        UINT64_C(34359738368) ) { /* <=5 */
    if      ( i<                UINT64_C(128) ) return(1);
    else if ( i<              UINT64_C(16384) ) return(2);
    else if ( i<            UINT64_C(2097152) ) return(3);
    else if ( i<          UINT64_C(268435456) ) return(4);
    else                                        return(5);
  }
  else { /* >5 */
    if      ( i<      UINT64_C(4398046511104) ) return(6);
    else if ( i<    UINT64_C(562949953421312) ) return(7);
    else if ( i<  UINT64_C(72057594037927936) ) return(8);
    else if ( i<UINT64_C(9223372036854775808) ) return(9);
    else                                        return(10);
  }
}

/* static int UINT64_lc_if_compressed(UINT64 i)
{
  if      ( i<                UINT64_C(128) ) return(1);
  else if ( i<              UINT64_C(16384) ) return(2);
  else if ( i<            UINT64_C(2097152) ) return(3);
  else if ( i<          UINT64_C(268435456) ) return(4);
  else if ( i<        UINT64_C(34359738368) ) return(5);
  else if ( i<      UINT64_C(4398046511104) ) return(6);
  else if ( i<    UINT64_C(562949953421312) ) return(7);
  else if ( i<  UINT64_C(72057594037927936) ) return(8);
  else if ( i<UINT64_C(9223372036854775808) ) return(9);
  else return(10);
}*/

static unsigned allocation_lc(unsigned lc, unsigned unit)
{
  if ( lc==0 ) return(0);
  else return(((lc-1) / unit + 1) * unit);
}

static unsigned rec_allocation_lc(unsigned lc)
{
  if ( lc==0 ) return(0);
  else return(((lc-1) / rec_allocation_unit + 1) * rec_allocation_unit);
}

/* Error handling */

static void fatal_error(f,err)
struct fcb *f; int err;
{
   f->error_code = err; f->file_ok = false;
}

static void set_error(struct fcb *f, int err, char caption[])
{
  f->error_code = err;
  if ( show_errors ) printf("%s",caption);
}

static void set_error1(struct fcb *f, int err, char caption[], int code)
{
  f->error_code = err;
  if ( show_errors ) printf("%s%d\n",caption,code);
}

/* Error checking.  Three fields in the fcb are used for error */
/*   management.  f->marker is set to keyf when the file is    */
/*   created and is never changed.  Any fcb with a different   */
/*   value is not useable.  f->file_ok is set true when the    */
/*   file is created and is turned off if an error occurs that */
/*   is so serious that the file is probably damaged (call to  */
/*   fatal_error).  f->error_code is set for any error         */
/*   condition.  Some errors are considered transient (e.g.,   */
/*   ateof, atbof, xxnokey,...) and are reset on the next call */
/*   to the package.  All others are considered permanent and  */
/*   are not reset.                                            */

static boolean check_fcb(struct fcb *f)
{ boolean ok;

  ok = (f->marker==keyf) && f->file_ok;
  switch ( f->error_code ) {
    case  0: break;
    case  1: /* badopen_err */      ok = false; break;
    case  2: /* badcreate_err */    ok = false; break;
    case  3: /* smallfcb_err */     ok = false; break;
    case  4: /* dltnokey_err */                 break;
    case  5: /* getnokey_err */                 break;
    case  6: /* notkeyfil_err */    ok = false; break;
    case  7: /* filenotok_err */    ok = false; break;
    case  8: /* badkey_err */                   break;
    case  9: /* maxlevel_err */     ok = false; break;
    case 10: /* ateof_err */                    break;
    case 11: /* atbof_err */                    break;
    case 12: /* longrec_err */                  break;
    case 13: /* longkey_err */                  break;
    case 14: /* version_err */      ok = false; break;
    case 15: /* seek_err */         ok = false; break;
    case 16: /* read_err */         ok = false; break;
    case 17: /* write_err */        ok = false; break;
    case 18: /* segment_open_err */ ok = false; break;
    case 19: /* segment_create_err */ok = false; break;
    case 20: /* bad_name_err */     ok = false; break;
    case 21: /* bad_dlt_err */      ok = false; break;
    case 22: /* max_key_err */      ok = false; break;
    case 23: /* nospace_err */      ok = false; break;
    case 24: /* free_insrt_err */   ok = false; break;
    case 25: /* free_dlt_err */     ok = false; break;
    case 26: /* alloc_rec_err */    ok = false; break;
    case 27: /* dealloc_rec_err */  ok = false; break;
    case 28: /* alloc_buf_err */    ok = false; break;
    case 29: /* move_rec_err */     ok = false; break;
    case 30: /* bad_close_err */    ok = false; break;
    case 31: /* ix_struct_err */    ok = false; break;
    case 32: /* read_only_err */                break;
    case 33: /* repl_max_key_err */ ok = false; break;
    case 34: /* data_lc_err */                  break;
    default: /* illegal_err code */ ok = false; break;
  }
  if ( ok ) f->error_code = no_err;
  return(ok);
}

static boolean set_up(struct fcb *f, unsigned char key[], int key_lc, struct key *k)
{
  if ( !check_fcb(f) ) return(false);
  k->lc = key_lc;
  if ( k->lc>0 && k->lc<maxkey_lc ) {
    memcpy(k->text,key,(size_t)key_lc); return(true);
  }
  else {
    f->error_code = badkey_err; k->lc = 0;
    return(false);
  }
}

/* Pointer manipulation */

static boolean gt_n_pntr(struct leveln_pntr p1, struct leveln_pntr p2)
{
  if ( p1.segment<p2.segment ) return(false);
  else if ( p1.segment>p2.segment ) return(true);
  else return( p1.block>p2.block );
}

/* compress0_pntr.  This version checks to see if the record should */
/*   be packed into the index block (uncompressed, so no byte       */
/*   swapping).  If so, it moves the record and then compresses lc  */
/*   into the block.  Otherwise, it compresses sc, segment, and lc  */
/*   into the block.  On entry, cp points to the first available    */
/*   character.  Note that insertions go from right to left so */
/*   that lc is the first field after the key (followed either by   */
/*   data or the disk_offset.                                       */

static int compress0_pntr(struct fcb *f, level0_pntr *p, unsigned char *cp)
{int lc,lc1,lc2;

  if ( p->lc <= f->data_in_index_lc ) {
    mvc(p->data_rec,0,cp-p->lc+1,0,p->lc);
    lc = compress_UINT32(p->lc,cp-p->lc);
    return( p->lc + lc );
  }
  else {
    lc = compress_UINT64(p->sc,cp);
    lc1 = compress_UINT32(p->segment,cp-lc);
    lc2 = compress_UINT32(p->lc,cp-lc-lc1);
    return( lc+lc1+lc2 );
  }
}

/* unpack0_lc returns the length occupied by a level0_pntr.  It   */
/*   does not return the pointer.                                 */

int unpack0_lc(struct fcb *f, struct ix_block *b, int ix)
{unsigned lc; UINT32 plc;/* UINT16 i; UINT64 j; */ char *cp;

  cp = (char *)b->keys + b->keys[ix].sc + b->keys[ix].lc;
  lc = uncompress_UINT32(&plc,cp);
  if ( plc <= f->data_in_index_lc ) {
    lc = lc + plc;
  }
  else {
    /*    lc = lc + uncompress_UINT16(&i,cp+lc);
	  lc = lc + uncompress_UINT64(&j,cp+lc);*/
    lc = lc + compressed_int_lc(cp+lc);
    lc = lc + compressed_int_lc(cp+lc);
  }
  return(lc);
}

/* unpack0_ptr uncompresses a level0 pointer.  Note   */
/*   cp points to the first character in the compressed    */
/*   string (unlike compress0_pntr).                       */

int unpack0_ptr(struct fcb *f, struct ix_block *b, int ix, level0_pntr *p)
{int lc; char *cp;

  cp = (char *)b->keys + b->keys[ix].sc + b->keys[ix].lc;
  lc = uncompress_UINT32(&(p->lc),cp);
  if ( p->lc <= f->data_in_index_lc ) {
    mvc(cp+lc,0,p->data_rec,0,p->lc);
    lc = lc + p->lc;
  }
  else {
    lc = lc + uncompress_UINT16(&(p->segment),cp+lc);
    lc = lc + uncompress_UINT64(&(p->sc),cp+lc);
  }
  return(lc);
}

/* unpack0_ptr_and_rec unpacks the level0_pntr for buf[ix] and extracts the   */
/*   data into rec (up to max_rec_lc bytes).  The number of bytes decoded to  */
/*   get the pointer is returned as the function value.  The number of data   */
/*   bytes in rec is returned in rec_lc.                                      */
/* If max_rec_lc==0 then no data is returned.  If the data rec is on disk     */
/*   it is read and p will have the normal on-disk segment/lc/sc values.  If  */
/*   the data rec is in the index block it is copied directly, segment is set */
/*   to max_segment and sc is set to zero.                                   */
/* If the caller wants to unpack the pointer but not get the rec then rec     */
/*   should be pointed to the data_rec field in p and max_rec_lc set to       */
/*   f->data_in_index_lc                                                      */

int unpack0_ptr_and_rec(struct fcb *f, struct buffer_type *buf, int ix, level0_pntr *p,
  unsigned char rec[], int *rec_lc, unsigned max_rec_lc)
{int lc; char *cp; size_t size=0; FILE *file;

  cp = (char *)buf->b.keys + buf->b.keys[ix].sc + buf->b.keys[ix].lc;
  lc = uncompress_UINT32(&(p->lc),cp);
  *rec_lc = p->lc;
  if ( (unsigned)*rec_lc>max_rec_lc ) *rec_lc = max_rec_lc;
  if ( p->lc > f->data_in_index_lc ) {
    lc = lc + uncompress_UINT16(&(p->segment),cp+lc);
    lc = lc + uncompress_UINT64(&(p->sc),cp+lc);
    file = file_index(f,p->segment);
    if ( fseeko(file,(FILE_OFFSET)p->sc,0)!=0 ) f->error_code = seek_err;
    else size = fread(rec,(size_t) 1,(size_t) *rec_lc,file);
    if ( size!=(size_t)*rec_lc ) f->error_code = read_err;
  }
  else {
    p->segment = max_segment;
    p->sc = 0;
    memcpy(rec,cp+lc,(size_t)*rec_lc);
    lc = lc + p->lc;
  }
  return(lc);
}

/* compressn_pntr compresses a leveln pointer p into a byte */
/*   string.  cp points to the last byte in the compressed  */
/*   string and compression will proceed from right to left */

static int compressn_pntr(struct leveln_pntr *p, unsigned char *cp)
{int lc,lc1;

  lc = compress_UINT64(p->block,cp);
  lc1 = compress_UINT32(p->segment,cp-lc);
  return( lc+lc1 );
}

static int level0_pntr_lc(struct fcb *f, level0_pntr *p)
{
  if ( p->lc<=f->data_in_index_lc )
    return( UINT32_lc_if_compressed(p->lc) + p->lc );
  else 
    return( UINT32_lc_if_compressed(p->lc) + UINT64_lc_if_compressed(p->sc)
      + UINT32_lc_if_compressed(p->segment) );
}

/* leveln_pntr_lc returns the compressed length of leveln_pntr p */

static int leveln_pntr_lc(struct leveln_pntr *p)
{/*int lc,lc1;

  lc = UINT32_lc_if_compressed(p->block);
  lc1 = UINT32_lc_if_compressed(p->segment);
  return( lc+lc1 );*/
  return( UINT64_lc_if_compressed(p->block) + UINT32_lc_if_compressed(p->segment) );
}

/*static*/ int levelx_pntr_lc(struct fcb *f, levelx_pntr *p, int level)
{
  if ( level==0 ) return(level0_pntr_lc(f,&(p->p0)));
  else return(leveln_pntr_lc(&(p->pn)));
}

/* uncompressn_pntr uncompresses a leveln pointer.  Note   */
/*   cp points to the first character in the compressed    */
/*   string (unlike compressn_pntr).                       */

static int uncompressn_pntr(struct leveln_pntr *p, unsigned char *cp)
{int lc,lc1;

  lc = uncompress_UINT16(&(p->segment),cp);
  lc1 = uncompress_UINT64(&(p->block),cp+lc);
  /*  memcpy(&(p->block),cp+lc,sizeof(int));*/
  return( lc+lc1 );
}

static int packn_ptr(struct ix_block *b, struct leveln_pntr p)
{int lc;

  lc = compressn_pntr(&p,(unsigned char *) b->keys+(keyspace_lc-b->chars_in_use-1));
  return(lc);
}

int unpackn_ptr(struct ix_block *b, int ix, struct leveln_pntr *p)
{int lc;

  lc = uncompressn_pntr(p,(char *)b->keys+(b->keys[ix].sc+b->keys[ix].lc));
  return(lc);
}

static int pack0_ptr(struct fcb *f, struct ix_block *b, level0_pntr *p)
{int lc;

  lc = compress0_pntr(f,p,(unsigned char *) b->keys+(keyspace_lc-b->chars_in_use-1));
  return(lc);
}

/* copy_ptr copies the ix_th pointer in b to the next available */
/*   space in b1 and returns the length copied.  It does not    */
/*   update counts in either b or b1.                           */

static int copy_ptr(struct fcb *f, struct ix_block *b, int ix, struct ix_block *b1)
{unsigned pntr_sc,lc; UINT32 plc; /* UINT16 segment; UINT64 psc; struct leveln_pntr pn;*/

  pntr_sc = b->keys[ix].sc + b->keys[ix].lc;
  if ( b->level==0 ) {
    lc = uncompress_UINT32(&plc,(char *)b->keys+pntr_sc);
    if ( plc<=f->data_in_index_lc ) lc = lc + plc;
    else {
      /*      lc = lc + uncompress_UINT16(&segment,(char *)b->keys+pntr_sc+lc);
	      lc = lc + uncompress_UINT64(&psc,(char *)b->keys+pntr_sc+lc);*/
      lc = lc + compressed_int_lc((char *)b->keys+pntr_sc+lc);
      lc = lc + compressed_int_lc((char *)b->keys+pntr_sc+lc);
    }
  }
  else {
    /*    lc = uncompressn_pntr(&pn,(char *)b->keys+pntr_sc);*/
    lc = compressed_int_lc((char *)b->keys+pntr_sc);
    lc = lc + compressed_int_lc((char *)b->keys+pntr_sc+lc);
  }
  mvc(b->keys,pntr_sc,b1->keys,keyspace_lc-b1->chars_in_use-lc,lc);
  return(lc);
}

/* Key handling */

boolean eq_key(struct key *k1, struct key *k2)
{
  if ( k1->lc!=k2->lc ) return(false);
  else return( memcmp(&(k1->text),&(k2->text),(size_t)k1->lc)==0 );
}

static void print_lc_key(struct key *k, char caption[])
{level0_pntr p;

  unpack_lc_key(k->text,&p);
  printf("%s(free_lc)%u, %u/",caption,p.lc,p.segment);
  printf(UINT64_format,p.sc);
}

static void print_rec_key(struct key *k, char caption[])
{level0_pntr p;

  unpack_rec_key(k->text,&p);
  printf("%s(free_rec)%u/",caption,p.segment);
  printf(UINT64_format,p.sc);
}

static void print_user_key(struct key *k, char caption[])
{/*int i; boolean is_text;*/

  print_key_struct(stdout,k,caption);
  /*  is_text = true;
  for (i=0; i<k->lc; i++) if ( !isprint(k->text[i]) ) is_text = false;
  printf("%s",caption);
  if ( k->lc==0 ) printf("null");
  else if ( is_text ) for (i=0; i<k->lc; i++) printf("%c",k->text[i]);
  else {
    printf("0x");
    for (i=0; i<k->lc; i++) printf("%2x",k->text[i]);
    }*/
}

static void print_key(int index, struct key *k, char caption[])
{
  if ( index==user_ix ) print_user_key(k,caption);
  else if ( index==free_rec_ix ) print_rec_key(k,caption);
  else if ( index==free_lc_ix )  print_lc_key(k,caption);
}

void get_nth_key(struct ix_block *b, struct key *k, int n)
{
  if ( n<0 || n>=b->keys_in_block ) k->lc = 0;
  else {
    mvc(b->keys,keyspace_lc-b->prefix_lc,k->text,0,b->prefix_lc);
    k->lc = b->keys[n].lc + b->prefix_lc;
    mvc(b->keys,b->keys[n].sc,k->text,b->prefix_lc,b->keys[n].lc);
  }
}

/* get_nth_key_and_pntr extracts the nth key and pointer from    */
/*   block b and places them in k and p.  It returns the length  */
/*   of the compressed pointer.                                  */

int get_nth_key_and_pntr(struct fcb *f, struct ix_block *b, struct key *k, int n, levelx_pntr *p)
{int lc;

  get_nth_key(b,k,n);
  lc = k->lc - b->prefix_lc;
  if ( k->lc > 0 ) {
    if ( b->level==0 ) lc = unpack0_ptr(f,b,n,&(p->p0));
    else lc = unpackn_ptr(b,n,&(p->pn));
  }
  return(lc);
}

void get_max_key(struct ix_block *b, struct key *k)
{
  if ( b->keys_in_block<1 ) k->lc = 0;
  else {
    get_nth_key(b,k,b->keys_in_block-1);
  }
}

/**** I/O ***/


/* init_file_name separates the file name and any extension */
/*   and saves the two parts in the fcb                     */

static void init_file_name(struct fcb *f, char id[])
{int i; unsigned name_lc, f_lc, ext_lc = 0;

  name_lc = (unsigned) strlen(id);
  if (name_lc > max_filename_lc + max_extension_lc)
    fatal_error(f,bad_name_err); /* whole thing too long */
  i = name_lc - 1;
  /* scan  from right to left
     stop when we hit either a . or a path separator.
  */
  while ( i>=0 && id[i]!='.' && id[i]!=PATH_SEPARATOR) {
    i--;
    ext_lc++;
  }
  if (i >= 0 && id[i] == '.') {
    f_lc = i;
    ext_lc++;
  }
  else {
    f_lc = name_lc;
    ext_lc = 0;
  }
  if (f_lc>=max_filename_lc) fatal_error(f,bad_name_err);
  else {
    strncpy(f->file_name, id, (size_t)f_lc);
    f->file_name[f_lc] = '\0';
  }
  if ( ext_lc>=max_extension_lc ) fatal_error(f,bad_name_err);
  else {
    strncpy(f->file_extension, id + i, (size_t)ext_lc);
    f->file_extension[ext_lc] = '\0';
  }
}

/* build_segment_name builds a segment name by appending the segment */
/*   number to the file name and then appending any extension.       */

static void build_segment_name(struct fcb *f, unsigned segment, char name[])
{int suffix_lc; size_t name_lc;

  strcpy(name,f->file_name);
  if (segment>0) {
    name_lc = strlen(name);
    suffix_lc = sprintf(name+name_lc,"$%d",segment);
    name[name_lc+suffix_lc] = '\0';
  }
  strcat(name,f->file_extension);
}

static void byte_swap_UINT16s(unsigned char s[], int cnt)
{unsigned int i=0; unsigned char ch;

  while ( i<cnt*sizeof(UINT16) ) {
    ch = s[i];
    s[i] = s[i+1];
    s[i+1] = ch;
    i = i + sizeof(UINT16);
  }
}

static void byte_swap_UINT32(unsigned char n[])
{unsigned char ch;

/*  printf("int %d after swapping is",n); */
  ch = n[0];
  n[0] = n[3];
  n[3] = ch;
  ch = n[1];
  n[1] = n[2];
  n[2] = ch;
/*  printf(" %d\n",n); */
 /*  printf("int %d after swapping is %d\n",n.as_int,n1.as_int);*/
}

static void byte_swap_UINT64(unsigned char n[])
{unsigned char ch;

/*  printf("int %d after swapping is",n); */
  ch = n[0];
  n[0] = n[7];
  n[7] = ch;
  ch = n[1];
  n[1] = n[6];
  n[6] = ch;
  ch = n[2];
  n[2] = n[5];
  n[5] = ch;
  ch = n[3];
  n[3] = n[4];
  n[4] = ch;
/*  printf(" %d\n",n); */
 /*  printf("int %d after swapping is %d\n",n.as_int,n1.as_int);*/
}

static unsigned char read_byte(struct fcb *f, FILE *file)
{unsigned char ch=0;

  if ( fread(&ch,sizeof(char),(size_t)1,file)!=1 )
    set_error(f,read_err,"read_byte failed\n");
  return(ch);
}


static UINT16 read_UINT16(struct fcb *f, FILE *file)
{UINT16 n; unsigned char ch;
 unsigned char *p = (unsigned char *)&n;

  if ( fread(&n,sizeof(UINT16),(size_t)1,file)!=1 ) {
    set_error(f,read_err,"read_UINT16 failed\n");
    return(0);
  }
  if ( f->byte_swapping_required ) {
    ch = p[1];
    p[1] = p[0];
    p[0] = ch;
  }
  return(n);
}


static UINT32 read_UINT32(struct fcb *f, FILE *file)
{UINT32 n;

  if ( fread(&n,sizeof(UINT32),(size_t)1,file)!=1 ) {
    set_error(f,read_err,"read_UINT32 failed\n");
    return(0);
  }
  if ( f->byte_swapping_required ) byte_swap_UINT32((unsigned char *) &n);
  return(n);
}

static UINT64 read_UINT64(struct fcb *f, FILE *file)
{UINT64 n;

  if ( fread(&n,sizeof(UINT64),(size_t)1,file)!=1 ) {
    set_error(f,read_err,"read_UINT64 failed\n");
    return(0);
  }
  if ( f->byte_swapping_required ) byte_swap_UINT64((unsigned char *) &n);
  return(n);
}

static boolean read_fib(struct fcb *f,char id[], boolean byte_swapping_required,
  boolean read_only)
{int i,j; FILE_OFFSET position; FILE *file;

  file = fopen(id,"rb");
  if ( file==NULL ) f->error_code = badopen_err;
  else if ( fseeko(file,(FILE_OFFSET) 0,0)!=0 ) f->error_code = badopen_err;
  else {
    f->byte_swapping_required = byte_swapping_required;
    f->read_only = read_only;

    f->error_code = read_UINT32(f,file);
    f->version = read_UINT32(f,file);
    f->segment_cnt = read_UINT32(f,file);
    for ( i=0; i<max_index; i++) f->primary_level[i] = read_UINT32(f,file);
    f->marker = read_UINT32(f,file);
    f->file_ok = read_UINT32(f,file);
    for (i=0; i<max_level; i++)
      for (j=0; j<max_index; j++) {
        f->first_free_block[i][j].segment = read_UINT16(f,file);
        f->first_free_block[i][j].block = read_UINT64(f,file);
      }
    for (i=0; i<max_level; i++)
      for (j=0; j<max_index; j++) {
        f->first_at_level[i][j].segment = read_UINT16(f,file);
        f->first_at_level[i][j].block = read_UINT64(f,file);
      }
    for (i=0; i<max_level; i++)
      for (j=0; j<max_index; j++) {
        f->last_pntr[i][j].segment = read_UINT16(f,file);
        f->last_pntr[i][j].block = read_UINT64(f,file);
      }
    f->max_file_lc = read_UINT64(f,file);
    for (i=0; i<max_segment; i++) f->segment_length[i] = read_UINT64(f,file);
    if ( f->version<=6 ) {
      f->data_in_index_lc = min_disk_rec_lc - 1;
      position = ftello(file);
      if ( position!=v6_fib_lc ) set_error1(f,badopen_err,"**Unable to read fib, position=",(int)position);
    }
    else {
      f->data_in_index_lc = read_UINT32(f,file);
      position = ftello(file);
      if ( position!=fib_lc ) set_error1(f,badopen_err,"**Unable to read fib, position=",(int)position);
    }
    fclose(file);
  }
  return(f->error_code==no_err);
}

static void write_UINT16(struct fcb *f, FILE *file, UINT16 *i)
{UINT16 n; unsigned char ch;
 unsigned char *p = (unsigned char *)&n;

  n = *i;
  if ( f->byte_swapping_required ) {
    ch = p[0];
    p[0] = p[1];
    p[1] = ch;
  }
  if ( fwrite(&n,sizeof(UINT16),(size_t)1,file)!=1 )
    set_error(f,write_err,"write failed in write_UINT16\n");
}

static void write_UINT16s(struct fcb *f, FILE *file, unsigned char s[], unsigned int cnt)
{unsigned int i; unsigned char swapped[keyspace_lc];

  if ( f->byte_swapping_required ) {
    i = 0;
    while ( i<cnt*sizeof(UINT16) ) {
      swapped[i] = s[i+1];
      swapped[i+1] = s[i];
      i = i + sizeof(UINT16);
    }
    if ( fwrite(swapped,sizeof(UINT16),(size_t)cnt,file)!=cnt )
    set_error(f,write_err,"write_UINT16s failed\n");
  }
  else {
    if ( fwrite(s,sizeof(UINT16),(size_t)cnt,file)!=cnt )
      set_error(f,write_err,"write_UINT16s failed\n");
  }
}

static void write_UINT32(struct fcb *f, FILE *file, UINT32 i)
{UINT32 n;

  n = i;
  if ( f->byte_swapping_required ) byte_swap_UINT32((unsigned char *) &n);
  if ( fwrite(&n,sizeof(UINT32),(size_t)1,file)!= 1 )
    set_error(f,write_err,"write failed in write_UINT32\n");
}

static void write_UINT64(struct fcb *f, FILE *file, UINT64 i)
{UINT64 n;

  n = i;
  if ( f->byte_swapping_required ) byte_swap_UINT64((unsigned char *) &n);
  if ( fwrite(&n,sizeof(UINT64),(size_t)1,file)!= 1 )
    set_error(f,write_err,"write failed in write_UINT64\n");
}

static void write_fib(struct fcb *f)
{int i,j,fill_cnt,fib_blocks; FILE_OFFSET position; FILE *file;

  if ( f->error_code!=no_err ) return;
  else if ( f->read_only ) return;
  else {
    file = file_index(f,0);
    if ( file==NULL ) set_error(f,bad_close_err,"**Bad file in write_fib\n");
    else if ( fseeko(file,(FILE_OFFSET) 0,0)!=0 ) set_error(f,bad_close_err,"**Couldn't seek to fib\n");
    else {
      /*      f->byte_swapping_required = false;*/

      write_UINT32(f,file,f->error_code);
      write_UINT32(f,file,f->version);
      write_UINT32(f,file,f->segment_cnt);
      for ( i=0; i<max_index; i++) write_UINT32(f,file,f->primary_level[i]);
      write_UINT32(f,file,f->marker);
      write_UINT32(f,file,f->file_ok);
      for (i=0; i<max_level; i++)
        for (j=0; j<max_index; j++) {
          write_UINT16(f,file,&(f->first_free_block[i][j].segment));
          write_UINT64(f,file,f->first_free_block[i][j].block);
        }
      for (i=0; i<max_level; i++)
        for (j=0; j<max_index; j++) {
          write_UINT16(f,file,&(f->first_at_level[i][j].segment));
          write_UINT64(f,file,f->first_at_level[i][j].block);
        }
      for (i=0; i<max_level; i++)
        for (j=0; j<max_index; j++) {
          write_UINT16(f,file,&(f->last_pntr[i][j].segment));
          write_UINT64(f,file,f->last_pntr[i][j].block);
        }
      write_UINT64(f,file,f->max_file_lc);
      for (i=0; i<max_segment; i++) write_UINT64(f,file,f->segment_length[i]);

      if ( f->version<=6 ) {
        position = ftello(file);
        if ( position!=v6_fib_lc )
          set_error1(f,bad_close_err,"**Wrong fib length on close, position=%d",(int)position);
	fib_blocks = (v6_fib_lc-1) / v6_block_lc + 1;
        fill_cnt = ((fib_blocks * v6_block_lc) - v6_fib_lc) / sizeof(int);
        for (i=0; i<fill_cnt; i++) write_UINT32(f,file,0);
      }
      else {
        write_UINT32(f,file,f->data_in_index_lc);
        position = ftello(file);
        if ( position!=fib_lc )
          set_error1(f,bad_close_err,"**Wrong fib length on close, position=%d",(int)position);
	fib_blocks = (fib_lc-1) / block_lc + 1;
        fill_cnt = ((fib_blocks * block_lc) - fib_lc) / sizeof(int);
        for (i=0; i<fill_cnt; i++) write_UINT32(f,file,0);
      }
    }
  }
}

void read_page(struct fcb *f, struct leveln_pntr p, block_type_t *buf)
{FILE *file; FILE_OFFSET offset;

 read_cnt++;
  if ( f->trace ) {
    print_leveln_pntr("reading page ",&p);
    printf("\n");
  }
  file = file_index(f,p.segment);
  offset = (p.block) << f->block_shift;
  if ( file==NULL ) set_error(f,read_err,"**Bad file in read_page\n");
  else if ( fseeko(file,offset,0)!=0 )
    set_error(f,seek_err,"**Seek failed in read_page\n");
  else {
    buf->keys_in_block = read_UINT16(f,file);
    buf->chars_in_use = read_UINT16(f,file);
    buf->index_type = read_byte(f,file);
    buf->prefix_lc = read_byte(f,file);
    buf->unused = read_byte(f,file);
    buf->level = read_byte(f,file);
    buf->next.segment = read_UINT16(f,file);
    buf->next.block = read_UINT64(f,file);
    buf->prev.segment = read_UINT16(f,file);
    buf->prev.block = read_UINT64(f,file);
    fread(buf->keys,(size_t) 1, (size_t) keyspace_lc,file);
    if ( ftello(file)!=(FILE_OFFSET)(offset+block_lc) )
      set_error1(f,read_err,"**I/O failure in read_page, bytes read=",(int)(ftello(file)-offset));
    if ( f->byte_swapping_required )
      byte_swap_UINT16s((char *)buf->keys,buf->keys_in_block*2);
  }
}

static void write_page(struct fcb *f, struct leveln_pntr p, block_type_t *buf)
{int pntr_lc,remaining; FILE *file; FILE_OFFSET offset;

  write_cnt++;
  if ( f->read_only ) {
    f->error_code = read_only_err;
    return;
  }
  if ( f->trace ) {
    print_leveln_pntr("writing page ",&p);
    printf("\n");
  }
  file = file_index(f,p.segment);
  offset = (p.block) << f->block_shift;
  if ( file==NULL ) set_error(f,write_err,"**Bad file in write_page\n");
  else if ( fseeko(file,offset,0)!=0 )
    set_error(f,seek_err,"**Seek error in write_page\n");
  else {
    write_UINT16(f,file,&(buf->keys_in_block));
    write_UINT16(f,file,&(buf->chars_in_use));
    if ( fwrite(&(buf->index_type),sizeof(char),(size_t)1,file)!=1 ) set_error(f,write_err,"write byte failed\n");
    if ( fwrite(&(buf->prefix_lc), sizeof(char),(size_t)1,file)!=1 ) set_error(f,write_err,"write byte failed\n");
    if ( fwrite(&(buf->unused),sizeof(char),(size_t)1,file)!=1 ) set_error(f,write_err,"write byte failed\n");
    if ( fwrite(&(buf->level),     sizeof(char),(size_t)1,file)!=1 ) set_error(f,write_err,"write byte failed\n");
    write_UINT16(f,file,&(buf->next.segment));
    write_UINT64(f,file,buf->next.block);
    write_UINT16(f,file,&(buf->prev.segment));
    write_UINT64(f,file,buf->prev.block);
    write_UINT16s(f,file,(char *)buf->keys,(unsigned)buf->keys_in_block*2);
    pntr_lc = buf->keys_in_block * key_ptr_lc;
    remaining = keyspace_lc - pntr_lc;
    fwrite(buf->keys+buf->keys_in_block,(size_t) 1, (size_t) remaining,file);
    if ( ftello(file)!=(FILE_OFFSET)(offset+block_lc) )
      set_error1(f,read_err,"**I/O failure in write_page, bytes written=",(int)(ftello(file)-offset));
  }
}

/*void flush_kf_buffers(struct fcb *f)
{int i;

  for (i=0; i<f->buffers_in_use; i++)
    if (f->buffer[i].modified && (f->buffer[i].lock_cnt==0) ) {
      write_page(f,f->buffer[i].contents,&(f->buffer[i].b) );
      f->buffer[i].modified = false;
    }
}*/


#define flush_window 256

/*static int write_page_and_flush(struct fcb *f, struct leveln_pntr p, void *buf)
{int i,min_block,max_block,nbr_cnt=0,nbr_list[flush_window]; struct leveln_pntr pn;

  min_block = (p.block / flush_window) * flush_window;
  max_block = min_block + flush_window;
  for (i=0; i<f->buffers_in_use; i++)
    if (f->buffer[i].modified && (f->buffer[i].lock_cnt==0) ) {
      pn = f->buffer[i].contents;
      if ( pn.segment==p.segment && pn.block>=min_block && pn.block<=max_block ) {
        nbr_list[nbr_cnt] = i;
        nbr_cnt++;
      }
    }
  for (i=0; i<nbr_cnt; i++) {
    write_page(f,f->buffer[nbr_list[i]].contents,&(f->buffer[nbr_list[i]].b) );
    f->buffer[nbr_list[i]].modified = false;
  }
  return(nbr_cnt);
}*/

/* vacate_file_index finds the LRU file_index, closes the segment */
/*   currently in use, marks the segment as closed and returns an */
/*   index into open_file to be used for segment I/O              */

static int vacate_file_index(struct fcb *f)
{int i,oldest,age,max_age;

  oldest = 0; max_age = 0;
  for ( i=0; i<f->open_file_cnt; i++ ) {
    age = f->current_age - f->file_age[i];
    if ( age>max_age ) {
      oldest = i; max_age = age;
    }
  }
  f->segment_ix[f->open_segment[oldest]] = max_files;
  fclose(f->open_file[oldest]);
  return(oldest);
}

/* open_segment opens a file segment.  If it is new it is opened in */
/*   write+ mode otherwise it is opened in read+ mode.  If the open */
/*   fails then f->open_file[ix] is set to NULL and f->error_code is*/
/*   set.  In any case the directories segment_ix[] and             */
/*   open_segment[] are set.  */

static void open_segment(struct fcb *f, unsigned segment, int ix)
{char name[max_filename_lc+10]; char* mode;

  build_segment_name(f,segment,name);
  if ( segment >= (unsigned)f->segment_cnt ) {
    if ( f->read_only ) {
      f->error_code = read_only_err;
      return;
    }
    else f->open_file[ix] = fopen(name,"wb+");
  }
  else {
    mode = f->read_only ? "rb" : "rb+";
    f->open_file[ix] = fopen(name,mode);
  }
  if (f->open_file[ix]==NULL) {
    f->error_code = segment_open_err;
  }
  f->segment_ix[segment] = ix;
  f->open_segment[ix] = segment;
  if ( f->trace)  printf("Opening segment %s on file index %d\n",name,ix);
}

static int file_ix(struct fcb *f, unsigned segment)
{int ix;

  ix = f->segment_ix[segment];
  if (ix<max_files) /* have a file open */;
  else if (f->open_file_cnt<max_files) {
    ix = f->open_file_cnt;
    f->open_file_cnt++;
    open_segment(f,segment,ix);
  }
  else {
    ix = vacate_file_index(f);
    open_segment(f,segment,ix);
  }
  f->file_age[ix] = f->current_age;
  if ( f->trace ) printf("  segment %d open on file index %d\n",segment,ix);
  return(ix);
}


/* file_index returns a file index to be used for I/O to a given   */
/*   segment of the keyed file.  If there is a file index open for */
/*   the segment, it is returned.  If there is an unused file   */
/*   index it is opened for the segment and returned.  Otherwise   */
/*   the LRU index is closed and reopened for the segment.         */

static FILE *file_index(struct fcb *f, unsigned segment)
{int ix;

  if ( segment<max_segment ) {
    ix = file_ix(f,segment);
    return(f->open_file[ix]);
  }
  else return(NULL);
}

static void set_position(struct fcb *f, int index, struct leveln_pntr b, int ix)
{
  f->position[index] = b; f->position_ix[index] = ix;
}


/* Buffer handling */

/* reset_ages is called when f->current_age reaches INT_MAX.    */
/*   The age of all open files is set to 0 and                  */
/*   f->current_age is set to 0.                                */

static void reset_ages(struct fcb *f)
{int i;

/*  printf("**Resetting ages\n");*/
  for (i=0; i<f->open_file_cnt; i++) f->file_age[i] = 0;
  f->current_age = 0;
}


#define hash_value(b,limit)  ((((int)b.block) + b.segment) % limit)


static int search_hash_chain(struct fcb *f, struct leveln_pntr block)
{int k,next,bufix=-1/*,cnt=0*/;

  
  k =  hash_value(block,f->buf_hash_entries);
  next = f->buf_hash_table[k];
  while ( next>=0 ) {
    if ( eq_pntr(f->buffer[next].contents,block) ) {
      bufix = next; next = -1;
    }
    else next = f->buffer[next].hash_next;
  }
  /*  printf("  searched hash chain %d for %d/%d, ",k,block.segment,block.block);
  if ( bufix<0 ) printf("not found, cnt=%d\n",cnt);
  else  printf("found in bufix=%d, cnt=%d\n",bufix,cnt);*/
  return(bufix);
}

/* hash_chain_insrt inserts a buffer in a hash_chain     */
/*   The block pointer is hashed and the hash table is   */
/*   checked.  If the hash table entry<0 then the        */
/*   chain is empty and the buffer inserted as a         */
/*   chain of length one.  If the entry>=0 then the      */
/*   chain is searched and the buffer inserted.          */
/* It assumes that the the buffer contents and           */
/*   hash_next fields have been set prior to call.       */

static void hash_chain_insert(struct fcb *f, int bufix)
{int k,next,last=-1; struct leveln_pntr block;

  block = f->buffer[bufix].contents;
  k = hash_value(block,f->buf_hash_entries);
  next = f->buf_hash_table[k];
  if ( next<0 ) {
    f->buf_hash_table[k] = bufix;
    f->buffer[bufix].hash_next = -1;
  }
  else {
    while ( next>=0 && gt_n_pntr(block,f->buffer[next].contents) ) {
      last = next; next = f->buffer[next].hash_next;
    }
    if ( last<0 ) {
      f->buffer[bufix].hash_next = f->buf_hash_table[k];
      f->buf_hash_table[k] = bufix;
    }
    else {
      f->buffer[last].hash_next = bufix;
      f->buffer[bufix].hash_next = next;
    }
  }
  if ( f->trace ) {
    printf("  inserted buffer %d (",bufix);
    print_leveln_pntr("",&block);
    printf(") into hash chain %d\n",k);
    print_hash_chain(stdout,f,k);
  }
}

/* hash_chain_remove removes buffer[bufix] from its hash chain */
/* It assumes that the the buffer contents and           */
/*   hash_next fields have been set prior to call.       */

static void hash_chain_remove(struct fcb *f, int bufix)
{int k,next,last=0; struct leveln_pntr block;

  block = f->buffer[bufix].contents;
  k = hash_value(block,f->buf_hash_entries);
  if ( f->trace ) {
    printf("Removing buffer %d from hash chain %d\n",bufix,k);
    printf("  old "); print_hash_chain(stdout,f,k);
  }
  next = f->buf_hash_table[k];
  if ( next==bufix ) f->buf_hash_table[k] = f->buffer[bufix].hash_next;
  else {
    while ( (next>=0) && !eq_pntr(block,f->buffer[next].contents) ) {
      last = next; next = f->buffer[next].hash_next;
    }
    if ( next<0 ) {
      if ( show_errors )
        printf("**Tried to remove nonexistent buffer (%d) from hash chain %d\n",bufix,k);
    }
    else f->buffer[last].hash_next = f->buffer[next].hash_next;
  }
  if ( f->trace ) {
    printf("  new "); print_hash_chain(stdout,f,k);
  }
  f->buffer[bufix].hash_next = -1;
}

/* make_buffer_youngest removes the buffer in bufix from the */
/*   age chain and inserts it as the youngest buffer         */
/* It assumes that the buffer older and younger fields are   */
/*   set prior to call.                                      */

static void make_buffer_youngest(struct fcb *f,int bufix)
{int older,younger;

  older = f->buffer[bufix].older;
  younger = f->buffer[bufix].younger;
  if ( younger>=0 ) { /* not allready youngest */
    if ( older==-1 ) {
      f->oldest_buffer = younger;
      f->buffer[younger].older = -1;
    }
    else {
      f->buffer[older].younger = younger;
      f->buffer[younger].older = older;
    }
    f->buffer[f->youngest_buffer].younger = bufix;
    f->buffer[bufix].younger = -1;
    f->buffer[bufix].older = f->youngest_buffer;
    f->youngest_buffer = bufix;
  }
}


static void init_buffer_hash_fields(struct fcb *f, int i, struct leveln_pntr *b)
{
  f->buffer[i].older = -1;
  f->buffer[i].younger = -1;
  f->buffer[i].hash_next = -1;
}

/* initialize_buffer initializes all of the buffer header fields except the */
/*   hashing fields (older, younger, hash_next).  It may reinitialize some  */
/*   fields (e.g., contents) that were set by the hashing functions.        */

static void initialize_buffer(struct fcb *f, int bufix, struct leveln_pntr *contents)
{
  f->buffer[bufix].contents = *contents;
  f->buffer[bufix].modified = false;
  f->buffer[bufix].lock_cnt = 0;
  f->buffer[bufix].last_ix = -1;
  f->buffer[bufix].search_cnt = 0;
  f->buffer[bufix].compare_cnt = 0;
}

#define buddy_window 16

int write_block_and_buddies(struct fcb *f, int bufix)
{int i,younger_buddies=0,older_buddies=0,ix,buddy_list[buddy_window];
 struct leveln_pntr buddy; boolean done=false;

  buddy = f->buffer[bufix].contents;
  while ( buddy.block>0 && younger_buddies<buddy_window && !done) {
    buddy.block--;
    ix = search_hash_chain(f,buddy);
    if ( ix>=0 && f->buffer[ix].modified && f->buffer[ix].lock_cnt==0) {
      buddy_list[younger_buddies] = ix;
      younger_buddies++;
    }
    else done = true;
  }
  for (i=younger_buddies-1; i>=0; i--) {
    ix = buddy_list[i];
    write_page(f,f->buffer[ix].contents,&(f->buffer[ix].b));
    f->buffer[ix].modified = false;
  }

  write_page(f,f->buffer[bufix].contents,&(f->buffer[bufix].b));

  buddy = f->buffer[bufix].contents;
  done = false;
  while ( older_buddies<buddy_window && !done) {
    buddy.block++;
    ix = search_hash_chain(f,buddy);
    if ( ix>=0 && f->buffer[ix].modified && f->buffer[ix].lock_cnt==0) {
      buddy_list[older_buddies] = ix;
      older_buddies++;
    }
    else done = true;
  }
  for (i=0; i<older_buddies; i++) {
    ix = buddy_list[i];
    write_page(f,f->buffer[ix].contents,&(f->buffer[ix].b));
    f->buffer[ix].modified = false;
  }
#ifdef log_buffers
  if ( younger_buddies>0 || older_buddies>0 ) {
    fprintf(buffer_log,"Wrote block %d/%lu, %d younger buddies, %d older buddies\n",
      f->buffer[bufix].contents.segment,
      f->buffer[bufix].contents.block,younger_buddies,older_buddies);
  }
#endif


  return(younger_buddies+older_buddies+1);
}

/* vacate_oldest_buffer is called when a new buffer is needed          */
/*   if there are unallocated buffers then the next one is             */
/*   added to the buffer chain and returned.  If all buffers           */
/*   are in use then the oldest unlocked buffer is flushed             */
/*   (if necessary) and returned                                       */
/* If an unallocated buffer is returned then it is initialized (by     */
/*   init_buffer_hash_fields) but further initialization by the caller */
/*   will be     */
/*   required.  If a buffer is vacated then only the buffer management */
/*   fields (older, younger, hash_next) are set.  The caller should    */
/*   not modify those fields but should initialize everything else as  */
/*   necessary.                                                        */

static int vacate_oldest_buffer(struct fcb *f, struct leveln_pntr *b)
{int oldest,cnt=0,locked_cnt=0,i; boolean done; struct leveln_pntr oldest_block;

  if ( f->buffers_in_use < f->buffers_allocated ) {
    oldest = f->buffers_in_use;
    init_buffer_hash_fields(f,oldest,b);
    initialize_buffer(f,oldest,b);
    if ( f->buffers_in_use==0 ) {
      f->youngest_buffer = oldest; f->oldest_buffer = oldest;
    }
    else {
      f->buffer[f->youngest_buffer].younger = oldest;
      f->buffer[oldest].older = f->youngest_buffer;
      f->youngest_buffer = oldest;
    }
    f->buffers_in_use++;
#ifdef log_buffers
    fprintf(buffer_log,"Paging block %d/%lu into unused buffer %d, ",b->segment,b->block,oldest);
    fprintf(buffer_log,"MRU chain after insert:");
    print_buffer_MRU_chain(buffer_log,f);
#endif
  }
  else {
    do {
      oldest = f->oldest_buffer;
      make_buffer_youngest(f,oldest);
      cnt++;
      if ( cnt>f->buffers_allocated ) {
        done = true; 
        f->error_code = alloc_buf_err;
        if ( show_errors )
          printf("**couldn't allocate a buffer, allocated=%d, locked=%d\n",
            f->buffers_allocated,locked_cnt);
      }
      else if ( f->buffer[oldest].lock_cnt>0 ) {
        done = false; locked_cnt++;
      }
      else done = true;
    }
    while ( !done );
    oldest_block = f->buffer[oldest].contents;
    if ( f->buffer[oldest].modified ) {
      /*      done = false;
      buddy = oldest_block;
      while ( buddy.block>0 && buddy_cnt<buddy_window && !done) {
	buddy.block--;
        buddy_ix = search_hash_chain(f,buddy);
	if ( buddy_ix>=0 && f->buffer[buddy_ix].modified ) buddy_cnt++;
	else done = true;
      }
      if ( buddy_cnt>0 ) fprintf(buffer_log,"Could write %d buddies\n",buddy_cnt);
      write_page(f,oldest_block,&(f->buffer[oldest].b));*/
      write_block_and_buddies(f,oldest);

      /*      i = write_page_and_flush(f,f->buffer[oldest].contents,&(f->buffer[oldest].b));*/
      if ( trace_io ) {
        print_leveln_pntr("  wrote block ",&oldest_block);
        print_buffer_caption(stdout,f,oldest);
        printf(" from buffer %d, %d others in window\n",oldest,i);
      }
    }
    hash_chain_remove(f,oldest);
    initialize_buffer(f,oldest,b);
#ifdef log_buffers
    fprintf(buffer_log,"Paging block %d/%lu into buffer %d",b->segment,b->block,oldest);
    if ( f->buffer[oldest].modified ) fprintf(buffer_log,", flushing ");
    else fprintf(buffer_log,", replacing ");
    fprintf(buffer_log,"block %d/%lu, ",oldest_block.segment,oldest_block.block);
    fprintf(buffer_log,"MRU chain after insert:");
    print_buffer_MRU_chain(buffer_log,f);
#endif
  }
  if ( f->trace ) printf("  just vacated oldest buffer, bufix=%d\n",oldest);
  return(oldest);
}


static void set_empty_block_prefix(struct ix_block *b, struct key *prefix, unsigned prefix_lc)
{
  mvc(prefix->text,0,b->keys,keyspace_lc-prefix_lc,prefix_lc);
  b->chars_in_use = prefix_lc;
  b->prefix_lc = prefix_lc;
}

static void initialize_index_block(struct ix_block *b, int index, unsigned lvl,
  struct key *prefix, unsigned prefix_lc)
{
  set_empty_block_prefix(b,prefix,prefix_lc);
  b->keys_in_block = 0;
  b->index_type = index;
  b->level = lvl;
  b->next = nulln_ptr;
  b->prev = nulln_ptr;
}

/*static int get_buffer(struct fcb *f, struct leveln_pntr block, boolean *not_found)
{int bufix;


  f->current_age++;
  if ( f->current_age==INT_MAX ) reset_ages(f);
  bufix = search_hash_chain(f,block);
  *not_found = bufix<0 ;

  if ( *not_found ) {


    bufix = vacate_oldest_buffer(f);
    f->buffer[bufix].contents = block;
    f->buffer[bufix].modified = false;
    hash_chain_insert(f,bufix);
  }
  else make_buffer_youngest(f,bufix);
  return(bufix);
}*/

static int get_index(struct fcb *f, struct leveln_pntr b)
{int bufix,index_type; struct key dummy;

  f->current_age++;
  if ( f->current_age==INT_MAX ) reset_ages(f);
  bufix = search_hash_chain(f,b);

  if ( bufix>=0 ) {
    make_buffer_youngest(f,bufix);
#ifdef log_buffers
    fprintf(buffer_log,"Found lvl %d block %d/%ld in buffer %d\n",f->buffer[bufix].b.level,
      b.segment,b.block,bufix);
#endif
  }
  else {
    bufix = vacate_oldest_buffer(f,&b);
    hash_chain_insert(f,bufix);
    read_page(f,b,&(f->buffer[bufix].b));
    if ( trace_io ) {
      print_leveln_pntr("  read block ",&b);
      print_buffer_caption(stdout,f,bufix);
      printf(" into buffer %d\n",bufix);
    }
  }
  if ( f->error_code==no_err ) {
    index_type = f->buffer[bufix].b.index_type;
    f->mru_at_level[f->buffer[bufix].b.level][index_type] = b;
  }
  else initialize_index_block(&(f->buffer[bufix].b),user_ix,level_zero,&dummy,(unsigned)0);
  return(bufix);
}

/*static int get_index(struct fcb *f, struct leveln_pntr b)
{boolean not_found; int bufix,index_type; struct key dummy;

  bufix = get_buffer(f,b,&not_found);
  if ( not_found ) {
    read_page(f,b,&(f->buffer[bufix].b));
    initialize_buffer(f,bufix,&b,false);
    if ( trace_io ) {
      print_leveln_pntr("  read block ",&b);
      print_buffer_caption(stdout,f,bufix);
      printf(" into buffer %d\n",bufix);
    }
  }
  if ( f->error_code==no_err ) {
    index_type = f->buffer[bufix].b.index_type;
    f->mru_at_level[f->buffer[bufix].b.level][index_type] = b;
  }
  else initialize_index_block(&(f->buffer[bufix].b),user_ix,level_zero,&dummy,(unsigned)0);
  return(bufix);
}*/

/*int kf_wire_index(struct fcb *f, struct leveln_pntr b)
{int bufix;

  bufix = get_index(f,b);
  f->buffer[bufix].lock_cnt = 1;
  return(bufix);
}*/

/* Prefix compression.  Keys in a block may have a common prefix that */
/*   is omitted when stored.   */
/*                                                                    */
/*   On block splits the prefix can never get shorter since the max   */
/*   key for the split blocks is the same as for the original block.  */

static int find_prefix_lc(struct key *k1, struct key *k2)
{int i=0,max_lc;

  max_lc = min(k1->lc,k2->lc);
  max_lc = min(max_lc,max_prefix_lc);
  while ( i<max_lc && (k1->text[i]==k2->text[i]) ) i++;
  return(i);
}

static int block_prefix_lc(struct ix_block *b)
{struct key first,last;

  if ( b->keys_in_block>1 ) {
    get_nth_key(b,&first,0);
    get_max_key(b,&last);
    return( find_prefix_lc(&first,&last) );
  }
  else return(b->prefix_lc);
}

static unsigned ix_pool_lc(struct ix_block *b)
{
  return(b->chars_in_use + (b->keys_in_block * key_ptr_lc));
}

/* ix_pool_lc_after_insert returns the size that an ix_pool would */
/*   be if key k and pointer p were inserted in entry ix of block */
/*   b.  If the insertion is at either the beginning or end of    */
/*   block then any change in length due to a new prefix must be  */
/*   computed.  The returned size may exceed keyspace_lc.  The    */
/*   prefix_lc of the block after insertion is returned in        */
/*   new_prefix_lc.                                               */

static int ix_pool_lc_after_insert(struct fcb *f, struct ix_block *b, struct key *k,
  levelx_pntr *p, int ix, int *new_prefix_lc)
{int needed,pool_lc,prefix_difference=0,save_pool_lc; struct key min,max;

  needed = k->lc - b->prefix_lc + levelx_pntr_lc(f,p,b->level) + key_ptr_lc;

  pool_lc = ix_pool_lc(b) + needed;
  save_pool_lc = pool_lc;
  if ( ix==0 ) {
    get_max_key(b,&max);
    prefix_difference = b->prefix_lc - find_prefix_lc(k,&max);
    pool_lc = pool_lc + (prefix_difference * (b->keys_in_block+1)) - prefix_difference;
  }
  else if ( ix==b->keys_in_block ) {
    get_nth_key(b,&min,0);
    prefix_difference = b->prefix_lc - find_prefix_lc(&min,k);
    pool_lc = pool_lc + (prefix_difference * (b->keys_in_block+1)) - prefix_difference;
  }
  if ( f->trace ) {
    printf("in ix_pool_aft_insert, needed=%d, k->lc=%d, orig ix_pool(b)=%d, prefix_lc=%d/%d, ",
      needed,k->lc,ix_pool_lc(b),b->prefix_lc,b->prefix_lc-prefix_difference);
    printf("keys_in_block=%d, ix=%d\n  insert pool before prefix adjustment=%d, after=%d\n",
      b->keys_in_block,ix,save_pool_lc,pool_lc);
  }
  *new_prefix_lc = b->prefix_lc - prefix_difference;
  return( pool_lc );
}

/* ix_pool_lc_after_replace returns the size that an ix_pool would */
/*   be if key k and pointer p replace those currently in entry ix */
/*   of block b.  If the replace is at either the beginning or end */
/*   of block then any change in length due to a new prefix must be*/
/*   computed.  The returned size may exceed keyspace_lc.  The     */
/*   prefix_lc of the block after replacement is returned in       */
/*   new_prefix_lc.                                                */

static int ix_pool_lc_after_replace(struct fcb *f, struct ix_block *b, struct key *k, levelx_pntr *p,
  int ix, int *new_prefix_lc)
{int pool_lc,difference,pntr_difference,key_difference,prefix_difference=0,save_pool_lc;
struct key min,max; struct leveln_pntr pn;

  if ( b->level==0 ) pntr_difference = level0_pntr_lc(f,&(p->p0)) - unpack0_lc(f,b,ix);
  else pntr_difference = leveln_pntr_lc(&(p->pn)) - unpackn_ptr(b,ix,&pn);
  key_difference =  (k->lc - b->prefix_lc - b->keys[ix].lc);
  difference =  key_difference + pntr_difference;

  pool_lc = ix_pool_lc(b) + difference;
  save_pool_lc = pool_lc;
  if ( ix==0 ) {
    get_max_key(b,&max);
    prefix_difference = b->prefix_lc - find_prefix_lc(k,&max);
    pool_lc = pool_lc + (prefix_difference * b->keys_in_block) - prefix_difference;
  }
  else if ( ix==b->keys_in_block-1 ) {
    get_nth_key(b,&min,0);
    prefix_difference = b->prefix_lc - find_prefix_lc(&min,k);
    pool_lc = pool_lc + (prefix_difference * b->keys_in_block) - prefix_difference;
  }
  if ( f->trace ) {
    printf("in ix_pool_aft_repl, diff=%d, k->lc=%d, orig ix_pool(b)=%d, prefix_lc=%d/%d, keys_in_block=%d, ix=%d\n",
      difference,k->lc,ix_pool_lc(b),b->prefix_lc,b->prefix_lc-prefix_difference,b->keys_in_block,ix);
    printf("  insert pool before prefix adjustment=%d, after=%d\n",save_pool_lc,pool_lc);
  }
  *new_prefix_lc = b->prefix_lc - prefix_difference;
  return( pool_lc );
}

/* ix_pool_lc_after_change returns the size that an ix_pool would */
/*   be if block b is modified with key k and pointer p.  If      */
/*   insert=true then k and p are to be inserted in entry ix.  If */
/*   insert=false then k and p replace entry ix.  If the change   */
/*   is at either the beginning or end of block then any change   */
/*   in length due to a new prefix is computed.  The returned     */
/*   size may exceed keyspace_lc.  The prefix_lc of the block     */
/*   after change is returned in new_prefix_lc.                   */

static int ix_pool_lc_after_change(struct fcb *f, struct ix_block *b, struct key *k, levelx_pntr *p,
  int ix, int *new_prefix_lc, boolean insert)
{
  if ( insert ) return(ix_pool_lc_after_insert(f,b,k,p,ix,new_prefix_lc));
  else  return(ix_pool_lc_after_replace(f,b,k,p,ix,new_prefix_lc));
}

/* Index searching */

/* Interior index blocks contain entries that point to the subtree
 *   containing keys <= entry key (and greater than the previous
 *   entry key).  Each level in the index has an additional pointer
 *   that points to the last subtree -- the subtree containing keys
 *   <= the largest possible key.  This last pointer is not stored
 *   in the index block but is found in last_pntr[].
 */

/* compare_key compares a key k with the ix^th entry stored in an   */
/*   index block.                                                   */

static int compare_key(unsigned char k[], int klc, struct ix_block *b, int ix)
{int r;

  if ( klc<=b->keys[ix].lc ) {
    r = memcmp(k,(char *) b->keys+b->keys[ix].sc,(size_t)klc );
    if (r<0) return(cmp_less);
    else if (r>0) return(cmp_greater);
    else if ( klc==b->keys[ix].lc ) return(cmp_equal);
    else return(cmp_less);
  }
  else {
    r = memcmp(k,(char *) b->keys+b->keys[ix].sc,(size_t)b->keys[ix].lc );
    if (r<0) return(cmp_less);
    else if (r>0) return(cmp_greater);
    else if ( klc==b->keys[ix].lc ) return(cmp_equal);
    else return(cmp_greater);
  }
}

/* search_block searches the block for the first entry>=k             */
/*   if k = some entry then  found=true   and ix is entry             */
/*   if k < some entry then  found=false  and ix is entry             */
/*   if k > all entries then found=false ix = keys_in_block           */
/* If the block is empty (happens with freespace blocks where the     */
/*   only block is the primary) then the key is treated as greater    */
/*   than all entries.                                                */

static int search_block(struct fcb *f, int bufix, struct key *k, boolean *found)
{int mid,high,ix=0,r=0,prefix_lc,klc; /*int compare_cnt=0;*/
 unsigned char *t; struct ix_block *b;

  *found = false;
  b = &(f->buffer[bufix].b);
  if ( b->keys_in_block>0 ) {
    f->buffer[bufix].search_cnt++;
    prefix_lc = b->prefix_lc;
    if ( k->lc<prefix_lc ) {
      r = memcmp(k->text,(char *) b->keys+keyspace_lc-prefix_lc,(size_t)k->lc );
      if (r>0) ix = b->keys_in_block;
      else ix = 0;
    }
    else {
      if ( prefix_lc>0 ) r = memcmp(k->text,(char *) b->keys+keyspace_lc-prefix_lc,(size_t)prefix_lc );

      if ( r==0 ) {
        klc = k->lc - prefix_lc;
        t = k->text + prefix_lc;
        high = b->keys_in_block-1;

        mid = (ix + high) / 2; 
        while ( ix<=high ) {
	  /*          compare_cnt++;*/
          switch ( compare_key(t,klc,b,mid) ) {
            case cmp_greater: 
              ix = mid + 1;
              mid = (ix + high) / 2;
              break;
            case cmp_equal:
              ix = mid;
              *found = true;
              high = -1;
              break;
            case cmp_less:
              high = mid - 1;
              mid = (ix + high) / 2;
              break;
          }
        }
      }
      else if ( r<0 ) ix = 0;
      else /* r>0 */ ix = b->keys_in_block;
    }
    /*    f->buffer[bufix].compare_cnt += compare_cnt;*/
    /*    f->buffer[bufix].last_ix = ix;*/
  }
  /* now ix points to first entry>=k or keys_in_block */
  if ( f->trace ) {
    printf("(%s)searched level %d ",f->search_block_caller,f->buffer[bufix].b.level);
    print_leveln_pntr("block ",&(f->buffer[bufix].contents));
    print_key(f->buffer[bufix].b.index_type,k,"for k=");
    if (ix>=b->keys_in_block) printf(" larger than any in block\n");
    else if ( *found ) printf(" found it, ix=%d\n",ix);
    else printf(" not found, ix=%d\n",ix);
  }
  return(ix);
}

/*
 * search_index searches index blocks down to stop_lvl and returns
 *   a pointer to the block at stop_lvl-1 in which the key lies.
 *   By construction, the key must be smaller than some key in
 *   each block searched unless it is in the rightmost block at
 *   this level.  If a key is larger than any in this level, then
 *   the last_pntr pointer is the returned.
 */

static struct leveln_pntr search_index(struct fcb *f, int index, UINT32 stop_lvl,
  struct key *k)
{struct leveln_pntr child; int ix,bufix; boolean done=false,found; char *name="search_index";

  child = f->first_at_level[f->primary_level[index]][index];
  if ( f->trace ) f->search_block_caller = name;
  if ( stop_lvl<=f->primary_level[index] )
    do {
      bufix = get_index(f,child);
      ix = search_block(f,bufix,k,&found);
      done = f->buffer[bufix].b.level<=stop_lvl;
      if ( ix>=f->buffer[bufix].b.keys_in_block ) { /* larger than any key */
        if ( null_pntr(f->buffer[bufix].b.next) )
          child = f->last_pntr[f->buffer[bufix].b.level][index];
	else {
          done = true; f->error_code = max_key_err; child = nulln_ptr;
	  if ( show_errors ) {
            print_key(index,k,"Uh Oh.  k=");
            printf(" is larger than any key in block\n");
	  }
	}
      }
      else unpackn_ptr(&(f->buffer[bufix].b),ix,&child);
    }
    while ( !done );
  return(child);
}

/* static void get_parent_key(struct fcb *f, struct ix_block *mid, struct key *parent_key)
{int parent_level,parent_ix,index,ix; boolean found; struct leveln_pntr p; struct key k;

  parent_key->lc = 0;
  get_nth_key(mid,&k,mid->keys_in_block-1);
  parent_level = mid->level + 1;
  index = mid->index_type;
  p = nulln_ptr;
  if ( parent_level<=f->primary_level[index] ) {
    p = search_index(f,index,parent_level+1,&k);
    parent_ix = get_index(f,p);

    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,parent_ix,&k,&found);
    get_nth_key(&(f->buffer[parent_ix].b),parent_key,ix);
  }
  if ( f->trace ) {
    print_key(index,&k," parent key for k=");
    print_key(index,parent_key," is ");
    printf(", parent_block is %d/%lu",p.segment,p.block);
    printf("\n");
  }
}*/

static struct leveln_pntr parent_block(struct fcb *f, int bufix, struct key *k)
{int parent_level,index;

/* int parent_ix,ix; boolean found; struct leveln_pntr p;
  parent_level = f->buffer[bufix].b.level + 1;
  index = f->buffer[bufix].b.index_type;
  parent_key->lc = 0; prev_key->lc = 0;
  p = nulln_ptr;
  if ( parent_level<=f->primary_level[index] ) {
    p = search_index(f,index,parent_level+1,k);
    parent_ix = get_index(f,p);

    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,&(f->buffer[parent_ix].b),k,&found);
    get_nth_key(&(f->buffer[parent_ix].b),parent_key,ix);
    get_nth_key(&(f->buffer[parent_ix].b),prev_key,ix-1);
  }
  if ( f->trace ) {
    print_key(index,k," parent key for k=");
    print_key(index,parent_key," is ");
    print_key(index,prev_key,", prev is ");
    printf("\n");
  }
  if ( !eq_pntr(p,f->mru_at_level[parent_level][index]) )
    printf("**Uh Oh. parent doesn't match MRU pointer, prnt_level=%d,primary_level=%d\n",
      parent_level,f->primary_level[index]);
  return(p);*/

  parent_level = f->buffer[bufix].b.level + 1;
  index = f->buffer[bufix].b.index_type;
  return(f->mru_at_level[parent_level][index]);
}

/* file initialization */

static void set_block_shift(struct fcb *f)
{int i;

  i = block_lc;
  f->block_shift = 0;
  while (i>0) {
    i = i>>1;
    if ( i>0 ) f->block_shift++;
  }
}

static boolean machine_is_little_endian()
{int i=1; unsigned char *p=(char *)&i;

  if ( p[0]==1 ) return(true);
  else return(false);
}


/* init_key initializes the temporary part of the fcb and a few static  */
/*   variables.  It assumes that the fib has been initialized and that  */
/*   the endedness of the machine has been set.                         */

static void init_key(struct fcb *f, char id[], int lc)
{int i,j,hash_target,hash_blocks;;

  if ( !check_fcb(f) ) return;
  if ( f->version>=6 && f->version<=current_version ) { /* ok */ }
  else { f->error_code = version_err; return; }
  if ( lc<(int)min_fcb_lc ) { f->error_code = smallfcb_err; return; }
  set_block_shift(f);
  f->trace = false; f->trace_freespace = false;
  f->open_file_cnt = 0;
  init_file_name(f,id);
  for (i=0; i<max_segment; i++) f->segment_ix[i] = max_files;
  f->current_age = 0;
  if ( lc==min_fcb_lc ) f->buffers_allocated = min_buffer_cnt;
  else f->buffers_allocated = min_buffer_cnt + (lc-min_fcb_lc) / buffer_lc;
  f->buffers_allocated = min(f->buffers_allocated,max_buffer_cnt);
  hash_target = f->buffers_allocated * buf_hash_load_factor;
  hash_blocks = ((hash_target - 1) / hash_entries_per_buf) + 1;
  f->buf_hash_table = (int *) &(f->buffer[f->buffers_allocated-hash_blocks]);
  f->buf_hash_entries = hash_blocks * hash_entries_per_buf;
  f->buffers_allocated = f->buffers_allocated - hash_blocks;
  for (i=0; i<f->buf_hash_entries; i++) f->buf_hash_table[i] = -1;
  f->buffers_in_use = 0;
  f->oldest_buffer = -1;
  f->youngest_buffer = -1;
  for (j=0; j<max_index; j++) {
    f->seq_cnt[j] = 0;
    for (i=0; i<max_level; i++) f->mru_at_level[i][j] = nulln_ptr;
  }
#ifdef log_buffers
  buffer_log = fopen("buffer_log","w");
#endif
}

/* record moving */

/* extract_rec moves the record identified by p into rec */

/*int extract_rec(struct fcb *f,level0_pntr *p, unsigned char rec[], int *rec_lc ,
  unsigned max_rec_lc)
{size_t size; FILE *file;

  if ( check_fcb(f) ) {
    *rec_lc = p->lc;
    if ( (unsigned)*rec_lc>max_rec_lc ) {
      f->error_code = longrec_err; *rec_lc = max_rec_lc;
    }
    if ( (unsigned)p->lc<=f->data_in_index_lc ) memcpy(rec,&p->data_rec,(size_t) *rec_lc);
    else {
      file = file_index(f,p->segment);
      if ( f->error_code!=no_err ) return(f->error_code);
      if ( fseeko(file,(FILE_OFFSET)p->sc,0)!=0 ) {
        f->error_code = seek_err; return(f->error_code);
      }
      size = fread(rec,(size_t) 1,(size_t) *rec_lc,file);
      if ( size!=(size_t)*rec_lc ) f->error_code = read_err;
    }
  }
  return(f->error_code);
}*/


/* insert_rec moves the record in r into the file.  If the record is   */
/*   sufficiently short it is simply copied into the level0_pntr and   */
/*   will be compressed into the index block.  Otherwise is is written */
/*   to disk.                                                          */

static void insert_rec(struct fcb *f, char r[], level0_pntr *p)
{
  size_t size,lc;
  FILE *file;
  if ( f->read_only ) f->error_code = read_only_err;
  else if ( p->lc<=f->data_in_index_lc ) memcpy(&p->data_rec,r,(size_t)p->lc);
  else {
    file = file_index(f,p->segment);
    if ( file==NULL || f->error_code==segment_open_err ) {
      set_error(f,move_rec_err,"No file in insert_rec");
      return;
    }
    if ( fseeko(file,(FILE_OFFSET)p->sc,0)!=0 ) {
      f->error_code = seek_err;
      return;
    }
    lc = p->lc;
    size = fwrite(r,(size_t) 1,lc,file);
    if ( size!=lc ) {
      set_error(f,move_rec_err,"**insert_rec failed\n");
    }
  }
}



/*** buffer handling ***/

static void lock_buffer(f,bufix)
struct fcb *f; int bufix;
{
  f->buffer[bufix].lock_cnt++;
}

static void unlock_buffer(f,bufix)
struct fcb *f; int bufix;
{
  f->buffer[bufix].lock_cnt--;
}

#define mark_modified(f,bufix) f->buffer[bufix].modified = true

/* static void mark_modified(f,bufix)
struct fcb *f; int bufix;
{
  f->buffer[bufix].modified = true;
}*/

static int get_index_update(struct fcb *f, struct leveln_pntr b)
{int bufix;

  bufix = get_index(f,b); mark_modified(f,bufix); return(bufix);
}

/* Space allocation */

/* extend_file extends the current segment by lc characters or   */
/*   opens a new segment if the current segment is full.  A      */
/*   pointer to the newly allocated block is returned in p.      */
/*   allocate_rec and allocate_block manage                      */
/*   segment_cnt.  Note that nospace_err is set when we allocate */
/*   space in the last possible segment even though the          */
/*   allocation succeeds.  It simplifies error handling.         */
/*   Space is always allocated in units that are an integral     */
/*   number of blocks.  The pointer returned by extend file is a */
/*   block number rather than the offset within the segment.     */
/*   This pointer is used directly for block structured data     */
/*   (index and freespace blocks) but must be converted to an    */
/*   offset for data records.                                    */

static boolean extend_file(struct fcb *f, unsigned lc, struct leveln_pntr *p)
{int current,ix;

  current = f->segment_cnt - 1;
  if ( (f->max_file_lc - f->segment_length[current]) < lc ) { /* segment full */
    if ( f->segment_cnt>=(max_segment-1) ) {
      set_error1(f,nospace_err,"  extend_file failed, segment=",current);
    }
    current++;
    ix = file_ix(f,(unsigned)current); /* opens current segment */
    f->segment_cnt++;
  }
  p->segment =current;
  p->block = f->segment_length[current] >> f->block_shift;
  /*  p->block = f->segment_length[current];*/
  f->segment_length[current] = f->segment_length[current] + lc;
  if ( f->trace ) {
    print_leveln_pntr("  extended file,",p);
    printf("lc=%d\n",lc);
  }
  return(f->error_code!=nospace_err);
}

/* deallocate_block places the block in buffer bufix at the front of the free */
/*   block chain.  It assumes that the buffer is marked modified (it needs    */
/*   to be written to preserve the chain).  Any locking required is handled   */
/*   by the caller.   */

static void deallocate_block(struct fcb *f, int bufix)
{int index_type,level; struct leveln_pntr p;

  index_type = f->buffer[bufix].b.index_type;
  level = f->buffer[bufix].b.level;
  p = f->buffer[bufix].contents;

  if ( f->trace_freespace ) {
    print_leveln_pntr("deallocating block",&p);
    printf("\n");
  }
  f->buffer[bufix].b.next = f->first_free_block[level][index_type];
  f->first_free_block[level][index_type] = p;

}

static int allocate_index_block(struct fcb *f, int index, struct leveln_pntr *b, unsigned lvl,
  struct key *prefix, unsigned prefix_lc)
{int bufix;
/* printf("  allocating index block at level %d\n",lvl); */
  bufix = allocate_block(f,index,lvl);
  initialize_index_block(&(f->buffer[bufix].b),index,lvl,prefix,prefix_lc);

  f->mru_at_level[lvl][index] = f->buffer[bufix].contents;
  *b = f->buffer[bufix].contents;
  return(bufix);
}

/*** index deletion ***/

/* simple_delete deletes the ix^th key from block b and returns */
/*   the number of bytes deleted (compressed key+compressed     */
/*   pointer+key_ptr_lc                                         */

static int simple_delete(struct fcb *f, struct ix_block *b, int ix)
{int i,key_sc,deleted_lc,deleted_sc,plc; struct leveln_pntr pn;

  if (ix>=b->keys_in_block ) {
    if ( show_errors )
      printf("**attempted to simple delete nonexistent key, ix=%d, keys_in_block=%d\n",
        ix,b->keys_in_block);
    return(0);
  }
  else {
    key_sc = keyspace_lc - b->chars_in_use;
    if ( b->level>0 ) plc = unpackn_ptr(b,ix,&pn);
    else plc = unpack0_lc(f,b,ix);
    deleted_lc = b->keys[ix].lc + plc;
    deleted_sc = b->keys[ix].sc;
    mvc(b->keys,key_sc,b->keys,key_sc+deleted_lc,(unsigned)deleted_sc-key_sc);
    b->chars_in_use = b->chars_in_use - deleted_lc;
    b->keys_in_block--;
    for (i=ix; i<b->keys_in_block; i++) b->keys[i] = b->keys[i+1];
    for (i=0; i<b->keys_in_block; i++)
      if (b->keys[i].sc<deleted_sc )
        b->keys[i].sc = b->keys[i].sc + deleted_lc;
    return(deleted_lc+key_ptr_lc);
  }
}

/* remove_primary removes the primary index block and reduces primary_level */
/*   by one (if it is>0).  Note that we do not remove the free_block chain  */
/*   at the old primary level.  If the depth of the index increases later   */
/*   then these free blocks will be reused otherwise they will be unused.   */
/*   They could be removed and added to a chain at a lower level but there  */
/*   cases where the primary is repeatedly removed by a delete then         */
/*   immediately recreated by an insert (esp. in freespace management)      */
/*   which could result in very long freespace chains.                      */

static void remove_primary(struct fcb *f, int index_type)
{int old_primary_level,bufix; struct leveln_pntr b;

  if ( f->trace )
    printf("  removing primary block at level %u, index_type=%d\n",
      f->primary_level[index_type],index_type);
  old_primary_level = f->primary_level[index_type];
  if ( old_primary_level>0 ) {
    b = f->first_at_level[old_primary_level][index_type];
    bufix = get_index_update(f,b);
    f->primary_level[index_type]--;
    deallocate_block(f,bufix);
  }
}

/* remove index block is called when a delete results in an empty index */
/*   block.  Ordinarily the block is removed and the reference to the   */
/*   block in any parent level is removed.  If the block is the last in */
/*   the chain at any level then it is pointed to by last_pntr entry    */
/*   in the parent level and we must make last_pntr point to the        */
/*   previous block at this level and delete the parent reference to    */
/*   that block.  When removing a block from a chain at primary_level-1 */
/*   results in a chain containing a single block then primary_level is */
/*   reduced by one and that singleton becomes the primary.             */

static void remove_index_block(struct fcb *f, struct key *parent_key, int bufix)
{int i,index_type,level,primary_level; struct leveln_pntr next,prev;
struct key max; level0_pntr dummy;

  index_type = f->buffer[bufix].b.index_type;
  level = f->buffer[bufix].b.level;
  primary_level = f->primary_level[index_type];
  next = f->buffer[bufix].b.next;
  prev = f->buffer[bufix].b.prev;
  if ( null_pntr(next) && null_pntr(prev) ) { /* only block at this level */
    if ( level==0 && primary_level==0 ) ; /* OK */
    else {
      set_error1(f,bad_dlt_err,"**Hmmm, singleton block at level=",level); 
    }
  }
  else if ( null_pntr(prev) ) { /* first block in chain */
    f->first_at_level[level][index_type] = next;
    i = get_index_update(f,next);
    f->buffer[i].b.prev = nulln_ptr;
    if ( level==primary_level-1 && null_pntr(f->buffer[i].b.next) )
      remove_primary(f,index_type);
    else if ( level<primary_level ) index_delete(f,index_type,*parent_key,&dummy,level+1);
    deallocate_block(f,bufix);
  }
  else if ( null_pntr(next) ) { /* last block in chain */
    i= get_index_update(f,prev);
    f->buffer[i].b.next = nulln_ptr;
    if ( level==primary_level-1 && null_pntr(f->buffer[i].b.prev) )
      remove_primary(f,index_type);
    else if ( level<primary_level ) {
      f->last_pntr[level+1][index_type] = prev;
      get_max_key(&(f->buffer[i].b),&max);
      index_delete(f,index_type,max,&dummy,level+1);
    }
    deallocate_block(f,bufix);
  }
  else { /* somewhere in the middle of chain */
    i= get_index_update(f,prev);
    f->buffer[i].b.next = next;
    i = get_index_update(f,next);
    f->buffer[i].b.prev = prev;
    if ( level<primary_level ) index_delete(f,index_type,*parent_key,&dummy,level+1);
    deallocate_block(f,bufix);
  }
}



/* index_delete removes key k from index block b at level level and   */
/*   returns the pointer p associated with that key.  If key k is not */
/*   found at the given level then error_code is set to dltnokey_err  */
/*   and p is set to null.                                            */

static void index_delete(struct fcb *f, int index_type, struct key k,
  level0_pntr *p, UINT32 level)
{int bufix,ix; boolean found,at_end,update_parent; struct key old_max_key,new_separator;
 struct leveln_pntr b; char *name="ix_delete";

  *p = null0_ptr;
  b = search_index(f,index_type,level+1,&k);
  bufix = get_index(f,b);
  if ( f->buffer[bufix].b.level!=level ){
    set_error(f,bad_dlt_err,"**Uh oh, wrong level in index delete\n");
  }
  if ( f->trace ) f->search_block_caller = name;
  ix = search_block(f,bufix,&k,&found);
  if ( f->trace ) {
    print_key(index_type,&k,"deleting key=");
    print_leveln_pntr(" from block",&b);
    printf("keys_in_block(before)=%d, found=%d, ix=%d\n",
      f->buffer[bufix].b.keys_in_block,found,ix);
  }
  if ( !found ) f->error_code = dltnokey_err;
  else {
    lock_buffer(f,bufix);
    mark_modified(f,bufix);
    at_end = ix==(f->buffer[bufix].b.keys_in_block-1);
    update_parent = at_end && (ix>0) && (f->primary_level[index_type]>0);
    get_max_key(&(f->buffer[bufix].b),&old_max_key);
    unpack0_ptr(f,&(f->buffer[bufix].b),ix,p);
    simple_delete(f,&(f->buffer[bufix].b),ix);
    if ( f->buffer[bufix].b.keys_in_block==0 ) { /* empty block */
      remove_index_block(f,&old_max_key,bufix);
    }
    else if ( ix==0 ) { /* block not empty, deleted first key */
      check_ix_block_compression(f,&(f->buffer[bufix].b));
    }
    else if ( at_end ) { /* block not empty, deleted last key */
      get_max_key(&(f->buffer[bufix].b),&new_separator);
      check_ix_block_compression(f,&(f->buffer[bufix].b));
      if ( f->primary_level[index_type]>0 && !null_pntr(f->buffer[bufix].b.next) )
        replace_max_key(f,index_type,&old_max_key,&new_separator,
          f->buffer[bufix].contents,level+1);
    }
    unlock_buffer(f,bufix);
  }
}

/*** index insertion ***/

/* simple_insert inserts key k and pointer p in entry ix of block b.   */
/*   It can be used when a) it is known that the key and pointer will  */
/*   fit in the block and b) when it is known that the key has the     */
/*   same prefix as the block.  Otherwise, prefix_simple_insert should */
/*   be used.                                                          */

static boolean simple_insert(struct fcb *f, struct ix_block *b, int ix, struct key *k, levelx_pntr *p)
{int i,plc,entry_lc; boolean ok=true;

  if ( b->level==0 ) plc = pack0_ptr(f,b,&(p->p0));
  else plc = packn_ptr(b,p->pn);
  entry_lc = k->lc - b->prefix_lc + plc;
  if ( (b->chars_in_use+entry_lc+(b->keys_in_block+1)*key_ptr_lc)<=keyspace_lc ) {
    b->keys_in_block++;
    b->chars_in_use = b->chars_in_use + entry_lc;
    for (i=b->keys_in_block-1; i>ix; i--) b->keys[i] = b->keys[i-1];
    b->keys[ix].lc = k->lc - b->prefix_lc;
    b->keys[ix].sc = keyspace_lc - b->chars_in_use;
    mvc(k->text,b->prefix_lc,b->keys,b->keys[ix].sc,b->keys[ix].lc);
  }
  else {
    ok = false;
    if ( show_errors ) printf("**Uh oh, simple_insert overflow\n");
  }
  return(ok);
}

/* move_keys moves cnt keys and pointers from b to b1.  The first key */
/*   moved is the ix_th entry in b which is moved to the ix1_th entry */
/*   in b1.  Entries in b1 are moved if necessary and any prefix      */
/*   difference between the blocks is accounted for.                  */

static boolean move_keys(struct fcb *f, struct ix_block *b, int ix, struct ix_block *b1, int ix1,
  int cnt)
{int i,current_lc,old_cnt,move_cnt,entry_lc,plc; boolean ok=true; struct key k;

/*  printf("**in move_keys, ix=%d, ix1=%d, cnt=%d, b1 before ptr move is\n",ix,ix1,cnt);
    print_index_block(stdout,f,b1);*/
  current_lc = ix_pool_lc(b1);
  if ( current_lc+(cnt*key_ptr_lc)<=keyspace_lc ) {
    old_cnt = b1->keys_in_block;
    move_cnt = old_cnt - ix1;
    for (i=1; i<=move_cnt; i++) b1->keys[old_cnt+cnt-i] = b1->keys[old_cnt-i];
    b1->keys_in_block = b1->keys_in_block + cnt;
  }
  /*  printf("**in move_keys, b1 after ptr move is\n");
      print_index_block(stdout,f,b1);*/

  i = 0;
  while ( ok && i<cnt ) {
    plc = copy_ptr(f,b,ix+i,b1);
    get_nth_key(b,&k,ix+i);

    entry_lc = k.lc - b1->prefix_lc + plc;
    if ( (b1->chars_in_use+entry_lc+(b1->keys_in_block*key_ptr_lc))<=keyspace_lc ) {
      b1->chars_in_use = b1->chars_in_use + entry_lc;
      b1->keys[ix1+i].lc = k.lc - b1->prefix_lc;
      b1->keys[ix1+i].sc = keyspace_lc - b1->chars_in_use;
      mvc(k.text,b1->prefix_lc,b1->keys,b1->keys[ix1+i].sc,b1->keys[ix1+i].lc);
    }
    else {
      ok = false;
      if ( show_errors ) printf("**Uh oh, move overflow\n");
    }
    i++;
  }
  /*  printf("**in move_keys, b1 after key move is\n");
      print_index_block(stdout,f,b1);*/

  return(ok);
}

/* delete_keys deletes cnt keys and pointers from block b.  The keys are */
/*   deleted from positions ix..(ix+cnt-1).  It first copies ix entries  */
/*   to a temporary block buf.  It then copies any entries above ix+cnt-1*/
/*   to buf and then copies the temporary over the original.             */

static void delete_keys(struct fcb *f, struct ix_block *b, int ix, int cnt)
{int i,plc,entry_lc,sc; struct ix_block buf; struct key k;
#define delete_threshold 1

/*printf("**in delete_keys, ix=%d, cnt=%d, b before is\n",ix,cnt);
  print_index_block(stdout,f,b);*/

  if ( cnt<=delete_threshold ) {
    for (i=0; i<cnt; i++) simple_delete(f,b,ix);
  }
  else {
    k.lc = b->prefix_lc;
    mvc((char *)b->keys,keyspace_lc-b->prefix_lc,k.text,0,b->prefix_lc);
    initialize_index_block(&buf,b->index_type,b->level,&k,(unsigned)b->prefix_lc);
    for (i=0; i<ix; i++) {
      plc = copy_ptr(f,b,i,&buf);
      get_nth_key(b,&k,i);
      entry_lc = k.lc - b->prefix_lc + plc;
      buf.chars_in_use = buf.chars_in_use + entry_lc;
      b->keys[i].lc = k.lc - b->prefix_lc;
      b->keys[i].sc = keyspace_lc - buf.chars_in_use;
      mvc(k.text,b->prefix_lc,buf.keys,b->keys[i].sc,b->keys[i].lc);
    }

    /*buf.keys_in_block = ix;
printf("**in delete_keys, ix=%d, cnt=%d, buf after reloc is\n",ix,cnt);
print_index_block(stdout,f,&buf);*/

    for (i=ix+cnt; i<b->keys_in_block; i++) {
      plc = copy_ptr(f,b,i,&buf);
      get_nth_key(b,&k,i);
      entry_lc = k.lc - b->prefix_lc + plc;
      buf.chars_in_use = buf.chars_in_use + entry_lc;
      b->keys[i-cnt].lc = k.lc - b->prefix_lc;
      b->keys[i-cnt].sc = keyspace_lc - buf.chars_in_use;
      mvc(k.text,b->prefix_lc,buf.keys,b->keys[i-cnt].sc,b->keys[i-cnt].lc);
    }
    sc = keyspace_lc - buf.chars_in_use;
    mvc(buf.keys,sc,b->keys,sc,buf.chars_in_use);
    b->keys_in_block = b->keys_in_block - cnt;
    b->chars_in_use = buf.chars_in_use;
  }
  
  /*printf("**in delete_keys, ix=%d, cnt=%d, b after is\n",ix,cnt);
    print_index_block(stdout,f,b);*/

}

/* compress_ix_block adds or removes prefixes from keys in an index block  */
/*   to give the block a given prefix_lc.  We assume that all keys in the  */
/*   block have a common prefix of length>=prefix_lc.  Decreasing the      */
/*   prefix length (inserts at either end) is never a problem (all keys    */
/*   have the prefix).  Increasing the prefix length (delete or block      */
/*   split) can only occur if all keys have the new prefix length.         */

static int compress_ix_block(struct fcb *f,struct ix_block *b, unsigned prefix_lc)
{int i,prefix_difference,pool_sc,old_key_sc,chars_in_use; unsigned expected_pool_lc,pntr_lc,pntr_sc,plc;
/* UINT16 segment; UINT64 psc;*/
 struct ix_block copy; struct key prefix; char expansion[max_prefix_lc];/* struct leveln_pntr pn;*/

  if ( b->prefix_lc==prefix_lc ) { /* do nothing */ }
  else {
    prefix_difference = b->prefix_lc - prefix_lc;
    expected_pool_lc = ix_pool_lc(b) + (prefix_difference * b->keys_in_block) - prefix_difference;
    if ( f->trace ) {
      printf("Compressing ix block from prefix_lc=%d to %d, pool_lc before=%d, after=%d\n",
        b->prefix_lc,prefix_lc,ix_pool_lc(b),expected_pool_lc);
    }
    if ( expected_pool_lc>keyspace_lc ) {
      if ( show_errors ) printf("**overflow in compress_ix_block\n");
    }
    else if (b->keys_in_block==0 ) { /* nothing to compress */
      b->chars_in_use = 0;
      b->prefix_lc = 0;
    }
    else {
      pool_sc = keyspace_lc - b->chars_in_use;
      mvc(b->keys,pool_sc,copy.keys,pool_sc,b->chars_in_use);
      get_nth_key(b,&prefix,0);
      if ( prefix.lc<prefix_lc && show_errors )
        printf("**key used for prefix compression too short, lc=%d, prefix_lc=%d\n",prefix.lc,prefix_lc);
      b->prefix_lc = prefix_lc;
      b->chars_in_use = prefix_lc;
      chars_in_use = b->chars_in_use;
      mvc(prefix.text,0,b->keys,keyspace_lc-prefix_lc,prefix_lc);
      if ( prefix_difference>0 ) mvc(prefix.text,prefix_lc,expansion,0,(unsigned)prefix_difference);
      for (i=0; i<b->keys_in_block; i++) {


        old_key_sc = b->keys[i].sc;
        pntr_sc = old_key_sc + b->keys[i].lc;

        if ( b->level==0 ) {
          pntr_lc = uncompress_UINT32(&plc,(char *)copy.keys+pntr_sc);
          if ( plc<=f->data_in_index_lc ) pntr_lc = pntr_lc + plc;
          else {
	    /*	    pntr_lc = pntr_lc + uncompress_UINT16(&segment,(char *)copy.keys+pntr_sc+pntr_lc);
		    pntr_lc = pntr_lc + uncompress_UINT64(&psc,(char *)copy.keys+pntr_sc+pntr_lc);*/
	    pntr_lc = pntr_lc + compressed_int_lc((char *)copy.keys+pntr_sc+pntr_lc);
	    pntr_lc = pntr_lc + compressed_int_lc((char *)copy.keys+pntr_sc+pntr_lc);
          }
        }
        else {
	  /*	  pntr_lc = uncompressn_pntr(&pn,(char *)copy.keys+pntr_sc);*/
	  pntr_lc = compressed_int_lc((char *)copy.keys+pntr_sc);
	  pntr_lc = pntr_lc + compressed_int_lc((char *)copy.keys+pntr_sc+pntr_lc);
	}

        chars_in_use = chars_in_use + pntr_lc + b->keys[i].lc + prefix_difference;

        b->keys[i].sc = keyspace_lc - chars_in_use;
        b->keys[i].lc = b->keys[i].lc + prefix_difference;
        if ( prefix_difference>0 ) {
          mvc(expansion,0,b->keys,b->keys[i].sc,(unsigned)prefix_difference);;
          mvc(copy.keys,old_key_sc,b->keys,b->keys[i].sc+prefix_difference,b->keys[i].lc+pntr_lc-prefix_difference);
	}
        else mvc(copy.keys,old_key_sc-prefix_difference,b->keys,b->keys[i].sc,b->keys[i].lc+pntr_lc);
      }
      b->chars_in_use = chars_in_use;
      /*      if ( ix_pool_lc(b)!=expected_pool_lc && show_errors)
        printf("**compressed ix block lc=%d doesn't match expected=%d, original prefix_lc=%d, prefix_lc=%d, keys=%d, original pool=%d\n",
	ix_pool_lc(b),expected_pool_lc,original_prefix_lc,prefix_lc,b->keys_in_block,original_pool_lc);*/
    }
  }
  return(b->chars_in_use);
}

/* check_ix_block_compression checks that the prefix length recorded in */
/*   the block is consistent with the keys in the block.  If it is not  */
/*   then the block is recompressed.                                    */

static void check_ix_block_compression(struct fcb *f, struct ix_block *b)
{unsigned prefix_lc;

  if ( b->keys_in_block>1 ) {
    prefix_lc = block_prefix_lc(b);
    if ( prefix_lc!=b->prefix_lc ) {
      compress_ix_block(f,b,prefix_lc);
    }
  }
}


/* prefix_simple_insert inserts key k and pointer p in entry ix of block b.  */
/*   If the insert succeeds then true is returned otherwise false is         */
/*   is returned and block b is unchanged.  If the key prefix does not match */
/*   the prefix used in b then b is uncompressed as far as necessary.  The   */
/*   key is inserted and the block is recompressed.                          */


static boolean prefix_simple_insert(struct fcb *f, struct ix_block *b, int ix, struct key *k,
  levelx_pntr *p)
{int prefix_difference,actual_lc,expected_lc; unsigned prefix_lc; boolean fits,ok=true;

  fits = (unsigned)ix_pool_lc_after_insert(f,b,k,p,ix,&prefix_lc) <= keyspace_lc;
  if ( fits ) {
    if ( ix==0 || ix==b->keys_in_block ) { /* k inserted at beginning or end */
      prefix_difference = b->prefix_lc - prefix_lc;
      expected_lc = b->chars_in_use + (prefix_difference * b->keys_in_block) - prefix_difference;
      if ( prefix_lc!=b->prefix_lc ) actual_lc = compress_ix_block(f,b,prefix_lc);
      else actual_lc = b->chars_in_use;
      if ( expected_lc!=actual_lc ) {
        ok = false;
        if ( show_errors ) {
          printf("**prefix_simple_insert at ");
          if ( ix==0 ) printf("beginning got wrong compressed ix block length\n");
          else printf("end got wrong compressed ix block length\n");
	  printf("    orig keys_in_block=%d,",b->keys_in_block);
          print_key(b->index_type,k," key=");
          printf("\n    expected_lc=%d, actual=%d, difference=%d\n",
            expected_lc,actual_lc,prefix_difference);
	}
      }
    }
    ok = ok && simple_insert(f,b,ix,k,p);
    if ( !ok && show_errors ) printf("**insert failed in prefix_simple_insert\n");
  }
  return(fits);
}

/* replace_max_key is used to replace a max_key in a parent block at level. */
/*   The old max_key is used to search for the entry in the parent block.   */
/*   The pointer found must match the child.  The key value is replaced and */
/*   propagated upward if it replaces the max_key in this (parent) block.   */
/*   However, if the key replacement causes this block to split then it is  */
/*   not necessary to propagate further since split_block will update the   */
/*   parent max_key values.  replace_max_key is similar to         */ 
/*   replace_max_key_and_pntr except that it only replaces the key and can  */
/*   ignore the request if the old and new keys are identical.              */

static void replace_max_key(struct fcb *f, int index, struct key *old_key, struct key *new_key,
  struct leveln_pntr child, unsigned level)
{int ix,bufix,new_prefix_lc; unsigned new_lc; char *name="rep_max_key";
boolean found=false,propagate; struct leveln_pntr p; struct key k; levelx_pntr px,childx;

  if ( level>f->primary_level[index] ){
    set_error1(f,repl_max_key_err,"**trying to replace_max_key in level above primary=",(int) level);
  }
  else if ( !eq_key(old_key,new_key) ) {
    p = search_index(f,index,level+1,old_key);
    bufix = get_index_update(f,p);
    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,bufix,old_key,&found);
    propagate = (ix==f->buffer[bufix].b.keys_in_block-1) && (!null_pntr(f->buffer[bufix].b.next));
    get_nth_key_and_pntr(f,&(f->buffer[bufix].b),&k,ix,&px);

    if ( f->trace ) {
      printf("  replacing max_key\n");
      print_key(index,old_key,"    old="); printf("\n");
      print_key(index,new_key,"    new=");
      printf("\n    level=%u,",level);
      print_leveln_pntr("child=",&child);
      printf("propagate=%d\n",propagate);
    }
    if ( !found || !eq_pntr(child,px.pn) ) {
      if ( ix==f->buffer[bufix].b.keys_in_block && null_pntr(f->buffer[bufix].b.next) ) {/* ok */}
      else {
        f->error_code = ix_struct_err;
        if ( show_errors ) {  
          printf("**Uh oh. Couldn't find entry in replace_max_key, index=%d, found=%d, level=%u\n",
            index,found,level);
          print_key(index,old_key,"  old key=");
          print_key(index,new_key,"\n  new key=");
          print_leveln_pntr("\n  child=",&child);
          print_leveln_pntr("px=",&(px.pn));
          printf("\n ix=%d, keys_in_block=%d, ",ix,f->buffer[bufix].b.keys_in_block);
          print_leveln_pntr("next_ptr=",&(f->buffer[bufix].b.next));
          printf("\n");
	}
      }
    }
    else {
      childx.pn = child;
      new_lc = ix_pool_lc_after_replace(f,&(f->buffer[bufix].b),new_key,&childx,ix,&new_prefix_lc);
      if ( new_lc <= keyspace_lc) {
        simple_delete(f,&(f->buffer[bufix].b),ix);
        if ( !prefix_simple_insert(f,&(f->buffer[bufix].b),ix,new_key,&childx) ) {
          set_error(f,repl_max_key_err,"**prefix_simple_insert failed in replace_max_key\n");
	}
      }
      else {
        split_block(f,new_key,&childx,bufix,ix,false);
	propagate = false;
      }
      if ( propagate && level<f->primary_level[index] ) replace_max_key(f,index,old_key,new_key,p,level+1);
    }
  }
}

/* replace_max_key_and_pntr replaces old_key and old_child in an index block */
/*   at level level with new_key and new_child and propogates new_key upward */
/*   if necessary (new_key is a new max).  Note that the pointer replacement */
/*   occurs even if old_key==new_key.  If propagation is necessary then only */
/*   the key is propagated upward. */

static boolean replace_max_key_and_pntr(struct fcb *f, int index, struct key *old_key, struct key *new_key,
  struct leveln_pntr old_child, struct leveln_pntr new_child, unsigned level)
{int ix,bufix,new_prefix_lc; unsigned new_lc; char *name="rep_maxkey&pntr";
boolean found=false,propagate,split=false; struct leveln_pntr p; struct key k; levelx_pntr px,childx;

  if ( level>f->primary_level[index] ){
    set_error1(f,repl_max_key_err,"**trying to replace_max_key_and_pntr in level above primary=",(int)level);
  }
  else {
    p = search_index(f,index,level+1,old_key);
    bufix = get_index_update(f,p);
    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,bufix,old_key,&found);
    propagate = (ix==f->buffer[bufix].b.keys_in_block-1) && (!null_pntr(f->buffer[bufix].b.next));
    get_nth_key_and_pntr(f,&(f->buffer[bufix].b),&k,ix,&px);

    if ( f->trace ) {
      printf("  replacing max_key_and_pntr\n");
      print_key(index,old_key,"    old=");
      printf("\n");
      print_key(index,new_key,"    new=");
      printf("\n    level=%d, ",level);
      print_leveln_pntr("old_child=",&old_child);
      print_leveln_pntr("new_child=",&new_child);
      printf("propagate=%d\n",propagate);
    }
    if ( !found || !eq_pntr(old_child,px.pn) ) {
      if ( ix==f->buffer[bufix].b.keys_in_block && null_pntr(f->buffer[bufix].b.next) ) {
        if ( !eq_pntr(f->last_pntr[level][index],old_child) ) {
          f->error_code = ix_struct_err;
          if ( show_errors ) {
            printf("**last_pntr[%d][%d]",level,index);
            print_leveln_pntr("=",&(f->last_pntr[level][index]));
            print_leveln_pntr("doesn't match old_child=",&old_child);
            printf("\n");
	  }
	}
        f->last_pntr[level][index] = new_child;
      }
      else {
        f->error_code = ix_struct_err;
        if ( show_errors ) {
          printf("**Uh oh. Couldn't find entry in replace_max_key_and_pntr, found=%d, level=%d\n",found,level);
          print_key(index,old_key,"  old key=");
          print_key(index,new_key,"\n  new key=");
          print_leveln_pntr("\n  old_child=",&old_child);
          print_leveln_pntr("px=",&(px.pn));
          printf("\n");
	}
      }
    }
    else {
      childx.pn = new_child;
      new_lc = ix_pool_lc_after_replace(f,&(f->buffer[bufix].b),new_key,&childx,ix,&new_prefix_lc);
      if ( new_lc<=keyspace_lc) {
        simple_delete(f,&(f->buffer[bufix].b),ix);
        if ( !prefix_simple_insert(f,&(f->buffer[bufix].b),ix,new_key,&childx) ) {
          set_error(f,repl_max_key_err,"**prefix_simple_insert failed in replace_max_key_and_pntr\n");
	}
      }
      else {
        split = true;
        split_block(f,new_key,&childx,bufix,ix,false);
	propagate = false;
      }
      if ( propagate && level<f->primary_level[index] ) replace_max_key(f,index,old_key,new_key,p,level+1);
    }
  }
  return(split);
}

/* moving keys -- on input we are given two blocks; a mid block from */
/*   which keys will be moved and either a rt or lt block into which */
/*   they will be moved.  We are also given a new key and pointer that */
/*   will either be inserted (insert=true) in position ix or will    */
/*   replace (insert=false) the entry in position ix.  We always do  */
/*   cnt inserts to the new block; one of these inserts will be the  */
/*   new key if ix is in the move range.  When insert=true the total */
/*   number of keys to be distributed between blocks is one greater  */
/*   than the original content.  If ix is in the move range we will  */
/*   do cnt-1 deletes to mid otherwise we will do cnt deletes.  When */
/*   insert is false the end number of keys is the same as the       */
/*   original and we will always do cnt deletes to mid.  Note that   */
/*   if ix is not in the move range then the caller must decide what */
/*   to do with the new key.                                         */

static boolean move_keys_to_right(struct fcb *f, struct ix_block *mid, struct ix_block *rt, int cnt,
  struct key *new_key, levelx_pntr *new_p, int ix, boolean insert)
{int first,above_ix_cnt,below_ix_cnt; boolean ok=true,move_new_key=false;

  first = mid->keys_in_block - cnt;
  move_new_key = ix >= (first+insert);
  /*  printf("  move_keys_to_right, mid/rt_keys=%d/%d, insert=%d, ix=%d, cnt=%d, move_new=%d\n",mid->keys_in_block,
      rt->keys_in_block,insert,ix,cnt,move_new_key);*/
  if ( cnt==0 ) { /* do nothing */ }
  else if ( move_new_key ) {
    above_ix_cnt = mid->keys_in_block - (ix+1-insert);
    below_ix_cnt = cnt - above_ix_cnt - 1;
    ok = move_keys(f,mid,ix+1-insert,rt,0,above_ix_cnt);
    ok = ok && simple_insert(f,rt,0,new_key,new_p);
    ok = ok && move_keys(f,mid,ix-below_ix_cnt,rt,0,below_ix_cnt);
    /*    printf("    after move, mid_keys=%d, rt_keys=%d\n",mid->keys_in_block,rt->keys_in_block);*/
    delete_keys(f,mid,ix-below_ix_cnt,cnt-insert);
    /*    printf("    after delete, mid_keys=%d, rt_keys=%d\n",mid->keys_in_block,rt->keys_in_block);*/
  }
  else {
    ok = move_keys(f,mid,first,rt,0,cnt);
    /*    printf("    after move, mid_keys=%d, rt_keys=%d\n",mid->keys_in_block,rt->keys_in_block);*/
    delete_keys(f,mid,first,cnt);
    /*    printf("    after delete, mid_keys=%d, rt_keys=%d\n",mid->keys_in_block,rt->keys_in_block);*/
  }
  if ( !ok && show_errors ) printf("**move_keys failed in move_keys_to_right\n");
  return(move_new_key);
}

static boolean move_keys_to_left(struct fcb *f, struct ix_block *lt, struct ix_block *mid, int cnt,
  struct key *new_key, levelx_pntr *new_p, int ix, boolean insert)
{int i,next=0,delete_cnt=0; boolean ok=true,moved_new_key=false; struct key k; levelx_pntr p;

  for (i=0; i<cnt; i++)
    if ( next<ix ) {
      get_nth_key_and_pntr(f,mid,&k,next,&p);
      delete_cnt++;
      ok = ok && simple_insert(f,lt,lt->keys_in_block,&k,&p);
      next++;
    }
    else if ( next==ix ) {
      moved_new_key = true;
      if ( !insert ) delete_cnt++;
      ok = ok && simple_insert(f,lt,lt->keys_in_block,new_key,new_p);
      next++;
    }
    else { 
      get_nth_key_and_pntr(f,mid,&k,next-insert,&p);
      delete_cnt++;
      ok = ok && simple_insert(f,lt,lt->keys_in_block,&k,&p);
      next++;
    }
  delete_keys(f,mid,0,delete_cnt);
  /*  for (i=0; i<delete_cnt; i++) simple_delete(f,mid,0);*/
  if ( !ok && show_errors ) printf("**simple insert failed in move_keys_to_left\n");
  return(moved_new_key);
}

/* choose_key selects the nth key from block mid.  If insert==1 then   */
/*   new_key and new_p contain a key/ptr pair that are to be inserted  */
/*   in ix_th entry of mid and n can have values 0..mid->keys_in_block.*/
/*   if insert==0 then the new key/ptr pair will replace the ix^th     */
/*   entry and n can have values 0..mid->keys_in_block-1.  The length  */
/*   of the entire entry selected (key+ptr+key_ptr) is returned.       */

static int choose_key(struct fcb *f, struct ix_block *mid, struct key *new_key,
  levelx_pntr *new_p, int ix, int n, struct key *choice, boolean insert)
{int lc=0; levelx_pntr px;

  if ( n>=(mid->keys_in_block+insert) && show_errors )
    printf("**choosing key from rt block, n=%d, ix=%d, keys_in_block=%d\n",n,ix,mid->keys_in_block);
  if ( n<ix ) lc = get_nth_key_and_pntr(f,mid,choice,n,&px);
  else if ( n==ix ) {
    *choice = *new_key;
    lc = levelx_pntr_lc(f,new_p,mid->level);
  }
  else if ( n<(mid->keys_in_block+insert) ) lc = get_nth_key_and_pntr(f,mid,choice,n-insert,&px);
  else choice->lc = 0;
  if ( lc==0 ) return(0);
  else return(lc + choice->lc + key_ptr_lc);
}

/* choose_right_move_cnt figures out how many keys should be moved from */
/*  index block mid to rt in order to make rt have at least target      */
/*  bytes in its index pool.  If insert=true then k is a new key to     */
/*  be inserted in location ix.  Otherwise, k will replace the entry    */
/*  in ix. The number of keys to move is returned, the sizes of mid     */
/*  and rt after moving are returned in mid_lc_in_out and rt_lc_out.       */
/*  and the prefix_lc's after moving are returned in mid_prefix_lc_out  */
/*  and rt_prefix_lc_out.                                               */
/*  Note that new_mid_lc may still exceed max_keyspace.                 */

static int choose_right_move_cnt(struct fcb *f, struct ix_block *mid, struct ix_block *rt,
struct key *k, levelx_pntr *new_p, int ix, boolean insert, unsigned target,
int *mid_lc_in_out, int *mid_prefix_lc_in_out, int *rt_lc_out, int *rt_prefix_lc_out)
{int rt_cnt=0,lc,mid_prefix_lc,rt_prefix_lc,prefix_difference,mid_lc,
 mid_change,rt_change,new_mid_prefix_lc,new_rt_prefix_lc,move_ix;
 unsigned rt_lc;
 boolean done=false;
 struct key min_key,max_key,rt_mid_key,move_key;

 /*  if ( insert ) mid_lc = ix_pool_lc_after_insert(f,mid,k,new_p,ix,&mid_prefix_lc);
     else  mid_lc = ix_pool_lc_after_replace(f,mid,k,new_p,ix,&mid_prefix_lc);*/
  mid_lc = *mid_lc_in_out;
  mid_prefix_lc = *mid_prefix_lc_in_out;
  rt_lc = ix_pool_lc(rt);
  rt_prefix_lc = rt->prefix_lc;

  if ( f->trace ) {
    print_key(mid->index_type,k,"finding right_mov_cnt, insert key=");
    printf(", lc=%d, pntr_lc=%d, ix=%d, level=%d",k->lc,levelx_pntr_lc(f,new_p,mid->level),ix,mid->level);
    printf(" target=%d, mid_lc=%d, rt_lc=%d\n",target,mid_lc,rt_lc);
  }

  choose_key(f,mid,k,new_p,ix,0,&min_key,insert);

  if ( rt->keys_in_block>0 ) get_max_key(rt,&max_key);
  else choose_key(f,mid,k,new_p,ix,mid->keys_in_block+insert-1,&max_key,insert);

  done = rt_lc>target;
  while ( !done) {
    move_ix = mid->keys_in_block - rt_cnt + insert - 1;
    lc = choose_key(f,mid,k,new_p,ix,move_ix,&move_key,insert);/*key that will move*/
    choose_key(f,mid,k,new_p,ix,move_ix-1,&rt_mid_key,insert);
    new_mid_prefix_lc = find_prefix_lc(&min_key,&rt_mid_key);
    if ( mid_prefix_lc!=new_mid_prefix_lc ) {
      prefix_difference = mid_prefix_lc - new_mid_prefix_lc;
      mid_change = -(lc-new_mid_prefix_lc) + prefix_difference * (mid->keys_in_block - rt_cnt + insert) - prefix_difference;
    }
    else mid_change = -(lc-new_mid_prefix_lc);
    new_rt_prefix_lc = find_prefix_lc(&move_key,&max_key);
    if ( rt_prefix_lc!=new_rt_prefix_lc ) {
      prefix_difference = rt_prefix_lc - new_rt_prefix_lc;
      rt_change = (lc-new_rt_prefix_lc) + prefix_difference * (rt->keys_in_block + rt_cnt) - prefix_difference;
    }
    else rt_change = (lc-new_rt_prefix_lc);
    if ( f->trace ) {
      print_key(mid->index_type,&move_key,"  checked move key=");
      printf(", lc=%d, mid_lc=%d, mid_change=%d, rt_lc=%d, rt_change=%d\n",
        lc,mid_lc,mid_change,rt_lc,rt_change);
    }
    if ( (unsigned)rt_lc+rt_change>keyspace_lc ) done = true;
    else if ( rt_cnt>=(mid->keys_in_block+insert-1) ) done = true; /* leave at least one key in mid */
    else {
      mid_prefix_lc = new_mid_prefix_lc;
      rt_prefix_lc = new_rt_prefix_lc;
      rt_lc = rt_lc + rt_change;
      mid_lc = mid_lc + mid_change;
      rt_cnt++;
      if ( rt_lc>=target) done = true;
    }
  }
  if ( f->trace ) printf(" should move %d keys to rt, mid_lc=%d, rt_lc=%d, mid_prefix_lc=%d, rt_prefix_lc=%d\n",
    rt_cnt,mid_lc,rt_lc,mid_prefix_lc,rt_prefix_lc);

  *mid_lc_in_out = mid_lc;
  *mid_prefix_lc_in_out = mid_prefix_lc;
  *rt_lc_out = rt_lc;
  *rt_prefix_lc_out = rt_prefix_lc;
  return(rt_cnt);

}


/* choose_left_move_cnt figures out how many keys should be moved from index block mid to */
/*   lt in order to make lt have at least target bytes in its index pool.  If insert=true */
/*   then k is a new key to be inserted in location ix.  Otherwise, k will replace the    */
/*   entry in ix. The number of keys to move is returned.                                 */
/*                                                                                        */
/* choose_left_move_cnt is called by choose_split_points (in which case */
/*   keys have been provisionally moved from mid to the right) and by   */
/*   split_block (no provisional moves so mid_keys_in_block matches the */
/*   the value in mid, but mid_lc_in_out and mid_prefix_lc_in_out may   */
/*   not match since the insert/replace may have altered them.          */
/*                                                                      */
/* Note that mid_lc_in_out on output may still exceed max_keyspace.     */
/* Parameters:                                                          */
/*    mid_keys_in_block - provisional number of keys in mid, may differ */
/*      from value recorded in mid since keys may have been             */
/*      (provisionally) been moved to the right.                        */
/*    mid_lc_in_out     - on input, provisional size of the pool in mid */
/*      differs from value recorded in mid since keys may have been     */
/*      (provisionally) moved to the right.  On output, the size of     */
/*      mid pool after moving keys to the left.                         */
/*    mid_prefix_lc_in_out - on input, the prefix_lc of mid after any   */
/*      provisional moves.  On output, the prefix_lc after moving keys  */
/*      to the left.                                                    */
/*    lt_lc_out - size of lt pool after moving keys                     */
/*    lt_prefix_lc_out - prefix_lc of lt after moving keys              */

static int choose_left_move_cnt(struct fcb *f, struct ix_block *lt, struct ix_block *mid,
  struct key *k, levelx_pntr *new_p, int ix, boolean insert, int mid_keys_in_block,
  int *mid_lc_in_out, int *mid_prefix_lc_in_out, int *lt_lc_out, int *lt_prefix_lc_out)
{int lt_cnt=0,lc,lt_prefix_lc,mid_prefix_lc,prefix_difference,lt_lc,mid_lc,
 lt_change,mid_change,new_lt_prefix_lc,new_mid_prefix_lc,target;
 boolean done=false;
 struct key min_key,rt_mid_key,move_key,mid_min_key;

  get_nth_key(lt,&min_key,0);
  lt_lc = ix_pool_lc(lt);
  lt_prefix_lc = lt->prefix_lc;
  mid_lc = *mid_lc_in_out;
  mid_prefix_lc = *mid_prefix_lc_in_out;
  target = (lt_lc + mid_lc) / 2;

  if ( block_prefix_lc(lt)!=lt->prefix_lc && show_errors )
    printf("**lt prefix length has changed, expected=%d, actual=%d, index=%d\n",block_prefix_lc(lt),lt->prefix_lc,lt->index_type);
  if ( f->trace ) {
    print_key(mid->index_type,k,"finding left_mov_cnt, insert key=");
    printf(", lc=%d, pntr_lc=%d, ix=%d, level=%d",k->lc,levelx_pntr_lc(f,new_p,mid->level),ix,mid->level);
    printf(" target=%d, mid_lc=%d, mid_prefix_lc=%d, lt_lc=%d\n",target,mid_lc,mid_prefix_lc,lt_lc);
  }

  choose_key(f,mid,k,new_p,ix,mid_keys_in_block+insert-1,&rt_mid_key,insert);
  done = lt_lc>target;
  while ( !done ) {
    lc = choose_key(f,mid,k,new_p,ix,lt_cnt,&move_key,insert);
    choose_key(f,mid,k,new_p,ix,lt_cnt+1,&mid_min_key,insert);
    new_lt_prefix_lc = find_prefix_lc(&min_key,&move_key);
    if ( lt_prefix_lc!=new_lt_prefix_lc ) {
      prefix_difference = lt_prefix_lc - new_lt_prefix_lc;
      lt_change = (lc-new_lt_prefix_lc) + prefix_difference * (lt->keys_in_block + lt_cnt) - prefix_difference;
    }
    else lt_change = lc - new_lt_prefix_lc;
    new_mid_prefix_lc = find_prefix_lc(&mid_min_key,&rt_mid_key);
    if ( mid_prefix_lc!=new_mid_prefix_lc ) { 
      prefix_difference = mid_prefix_lc - new_mid_prefix_lc;
      /*      mid_change = -(lc-new_mid_prefix_lc) + prefix_difference * (mid_keys_in_block - lt_cnt + 1) - prefix_difference;*/
      mid_change = -(lc-new_mid_prefix_lc) + prefix_difference * (mid_keys_in_block - lt_cnt + insert) - prefix_difference;
    }
    else mid_change = -(lc-new_mid_prefix_lc);
    if ( f->trace ) {
      print_key(mid->index_type,&move_key,"  checked move key=");
      printf(", lc=%d, mid_lc=%d, mid_change=%d, lt_lc=%d, lt_change=%d\n",
        lc,mid_lc,mid_change,lt_lc,lt_change);
    }
    if ( (unsigned)lt_lc+lt_change>keyspace_lc ) done = true;
    else if ( lt_cnt>=(mid_keys_in_block+insert-1) ) done = true; /* leave at least one key in mid */
    else {
      lt_prefix_lc = new_lt_prefix_lc;
      mid_prefix_lc = new_mid_prefix_lc;
      lt_lc = lt_lc + lt_change;
      mid_lc = mid_lc + mid_change;
      /*      if ( lt_cnt==0 && lt->keys_in_block==0 ) min_key = move_key;*/
      lt_cnt++;
      if ( lt_lc+lt_change>=target) done = true;
    }
  }
  if ( f->trace ) {
    printf(" should move %d keys to lt, ",lt_cnt);
    printf("lt_lc=%d, lt_prefix=%d, mid_lc=%d, mid_prefix=%d\n",lt_lc,lt_prefix_lc,mid_lc,mid_prefix_lc);
  }

  *mid_lc_in_out = mid_lc;
  *mid_prefix_lc_in_out = mid_prefix_lc;
  *lt_lc_out = lt_lc;
  *lt_prefix_lc_out = lt_prefix_lc;
  return(lt_cnt);

}

static void shuffle_length_mismatch(struct ix_block *b, char caption[], unsigned expected_lc, struct key *k, boolean insert)
{
  if ( show_errors ) {
    printf("**shuffle length mismatch for %s, index_type=%d, level=%d\n",caption,b->index_type,b->level);
    print_key(b->index_type,k,"    key=");
    printf(", actual_lc=%d, expected_lc=%u, insert=%d\n",ix_pool_lc(b),expected_lc,insert);
  }
}

/* choose_split_points is called when an insert or replace of key k in   */
/*   block mid and will cause an overflow.  It attempts to move keys to  */
/*   neighboring blocks to make room for the new key. */

static boolean choose_split_points(struct fcb *f, struct ix_block *lt, struct ix_block *mid,
struct ix_block *rt, struct key *k, levelx_pntr *new_p, int ix, boolean insert)
{int lt_cnt=0,rt_cnt=0;
 unsigned lt_lc,mid_lc,rt_lc,target,lt_prefix_lc,mid_prefix_lc,rt_prefix_lc;
 boolean fits,moved_key_left,moved_key_right,key_not_moved;
 struct key mid_max_key,move_key,temp;
 boolean lt_full,rt_full; int needed,min_target;

  if ( f->trace ) {
    print_key(mid->index_type,k,"trying to move to nbrs, key=");
    printf(", lc=%d, pntr_lc=%d, ix=%d, level=%d, insert=%d\n",k->lc,levelx_pntr_lc(f,new_p,mid->level),ix,mid->level,insert);
  }

  lt_lc = ix_pool_lc(lt);
  lt_prefix_lc = lt->prefix_lc;
  mid_lc = ix_pool_lc_after_change(f,mid,k,new_p,ix,&mid_prefix_lc,insert);
  rt_lc = ix_pool_lc(rt);

  needed = mid_lc - keyspace_lc;
  if ( needed<0 ) printf("**Uh oh, needed=%d in choose_split_points\n",needed);
  min_target = keyspace_lc - needed - 1;
  lt_full = ( keyspace_lc - lt_lc ) < (unsigned) ( needed );
  rt_full = ( keyspace_lc - rt_lc ) < (unsigned) ( needed );
  /*  printf("lt_lc=%4d, mid_lc=%4d, rt_lc=%4d, min_target=%4d, ",
      lt_lc,mid_lc,rt_lc,min_target);*/
  if ( lt_full && rt_full ) {/*printf("giving up\n");*/ return(false);}
  else if ( lt_full ) target = min( (int) (mid_lc+rt_lc)/2, min_target); 
  else if ( rt_full ) target = min( (int) (mid_lc+lt_lc)/2, min_target);
  else target = min( (int) (lt_lc+mid_lc+rt_lc)/3, min_target); 
  /*  printf("new_target=%4d ",target);*/


  /*  target = ( lt_lc + mid_lc + rt_lc) / 3;
  if ( lt_lc>target ) target = (mid_lc + rt_lc) / 2;
  else if ( rt_lc>target ) target = (lt_lc + mid_lc) / 2;

  printf(" orig_target=%d\n",target);*/

  if ( f->trace ) printf("target=%d, lt_lc=%d, mid_lc=%d, rt_lc=%d\n",target,lt_lc,mid_lc,rt_lc);

  rt_cnt = choose_right_move_cnt(f,mid,rt,k,new_p,ix,insert,target,&mid_lc,&mid_prefix_lc,&rt_lc,&rt_prefix_lc);
  /*  choose_key(f,mid,k,new_p,ix,mid->keys_in_block-rt_cnt,&mid_max_key,true);*/
  choose_key(f,mid,k,new_p,ix,mid->keys_in_block-1+insert-rt_cnt,&mid_max_key,insert);
  lt_cnt = choose_left_move_cnt(f,lt,mid,k,new_p,ix,insert,mid->keys_in_block-rt_cnt,&mid_lc,&mid_prefix_lc,&lt_lc,&lt_prefix_lc);
  /*  lt_cnt = choose_left_move_cnt(f,lt,mid,k,new_p,ix,insert,mid->keys_in_block-rt_cnt+insert,&mid_lc,&mid_prefix_lc,&lt_lc,&lt_prefix_lc);*/

  if ( f->trace ) {
    printf(" should move %d keys to lt, %d to rt ",lt_cnt,rt_cnt);
    printf("lt_lc/prefix=%d/%d, mid_lc/prefix=%d/%d, rt_lc/prefix=%d/%d\n",
      lt_lc,lt_prefix_lc,mid_lc,mid_prefix_lc,rt_lc,rt_prefix_lc);
  }

  if ( lt_cnt==0 ) get_nth_key(lt,&move_key,lt->keys_in_block-1);
  else choose_key(f,mid,k,new_p,ix,lt_cnt-1,&move_key,insert);
  fits = (lt_lc<=keyspace_lc && mid_lc<=keyspace_lc && rt_lc<keyspace_lc);
  if ( fits ) {
    /*    key_not_moved = (ix>(lt_cnt-1)) && (ix<=mid->keys_in_block-rt_cnt);*/
    /*    key_not_moved = (ix>(lt_cnt-1)) && (ix<mid->keys_in_block-rt_cnt);*/
    compress_ix_block(f,rt,rt_prefix_lc);
    moved_key_right = move_keys_to_right(f,mid,rt,rt_cnt,k,new_p,ix,insert);
    if (block_prefix_lc(rt)!=rt->prefix_lc && show_errors)
      printf("   rt_prefix after move should be=%d, is=%d\n",block_prefix_lc(rt),rt->prefix_lc);
    compress_ix_block(f,lt,lt_prefix_lc);
    moved_key_left = move_keys_to_left(f,lt,mid,lt_cnt,k,new_p,ix,insert);
    if (block_prefix_lc(lt)!=lt->prefix_lc && show_errors)
      printf("   lt_prefix after move should be=%d, is=%d\n",block_prefix_lc(lt),lt->prefix_lc);
    compress_ix_block(f,mid,mid_prefix_lc);
    key_not_moved = !moved_key_right && !moved_key_left;
    if ( key_not_moved ) {
      if ( !insert && (ix-lt_cnt)<mid->keys_in_block ) simple_delete(f,mid,ix-lt_cnt);
      if ( simple_insert(f,mid,ix-lt_cnt,k,new_p) ) ;
      else if ( show_errors ) printf("**simple_insert failed in choose_split_points, ix=%d, lt_cnt=%d, rt_cnt=%d\n",ix,lt_cnt,rt_cnt);
    }
    if (block_prefix_lc(mid)!=mid->prefix_lc && show_errors )
      printf("   mid_prefix after moves and insert=%d, in_block=%d\n",block_prefix_lc(mid),mid->prefix_lc);
    get_max_key(lt,&temp);
    if ( !eq_key(&temp,&move_key) && show_errors ) {
      print_key(mid->index_type,&temp,"**actual lt max_key=");
      print_key(mid->index_type,&move_key," doesn't match expected=");
      printf("\n");
    }
    get_max_key(mid,&temp);
    if ( !eq_key(&temp,&mid_max_key) && show_errors ) {
      print_key(mid->index_type,&temp,"**actual mid max_key=");
      print_key(mid->index_type,&mid_max_key," doesn't match expected=");
      printf("\n");
    }
    if ( ix_pool_lc(lt)!=lt_lc )   shuffle_length_mismatch(lt ,"lt", lt_lc ,k,insert);
    if ( ix_pool_lc(mid)!=mid_lc ) shuffle_length_mismatch(mid,"mid",mid_lc,k,insert);
    if ( ix_pool_lc(rt)!=rt_lc )   shuffle_length_mismatch(rt, "rt", rt_lc, k,insert);
    }
  return(fits);
}

/* shuffle_keys is called when an insert to buffer[mid_ix] won't fit.  */
/*   It checks to see if it is possible to move keys to the right and  */
/*   left to make room.  If the block is at the beginning or end of    */
/*   the block chain at this level then the shuffle fails.  Otherwise, */
/*   choose_split_points moves the keys if the shuffle is possible     */
/*   otherwise it does nothing.  If the shuffle succeeded then the     */
/*   parent pointers are adjusted and the blocks are marked modified.  */

static boolean shuffle_keys(struct fcb *f, int mid_ix, struct key *k, levelx_pntr *p, int ix, boolean insert)
{int lt_ix,rt_ix,index_type; unsigned level;
 boolean shuffled=false; struct key lt_sep,old_lt_sep,mid_sep,old_mid_sep;

  lock_buffer(f,mid_ix);
  get_max_key(&(f->buffer[mid_ix].b),&old_mid_sep);
  level = f->buffer[mid_ix].b.level;
  index_type = f->buffer[mid_ix].b.index_type;
  if ( null_pntr(f->buffer[mid_ix].b.prev) ) shuffled = false;
  else if ( null_pntr(f->buffer[mid_ix].b.next) ) shuffled = false;
  else {
    lt_ix = get_index(f,f->buffer[mid_ix].b.prev);
    lock_buffer(f,lt_ix);
    get_max_key(&(f->buffer[lt_ix].b),&old_lt_sep);
    rt_ix = get_index(f,f->buffer[mid_ix].b.next);
    lock_buffer(f,rt_ix);

    if ( f->trace ) {
      print_index_block(stdout,f,&(f->buffer[lt_ix].b));
      print_index_block(stdout,f,&(f->buffer[mid_ix].b));
      print_index_block(stdout,f,&(f->buffer[rt_ix].b));
    }
    if ( choose_split_points(f,&(f->buffer[lt_ix].b),&(f->buffer[mid_ix].b),&(f->buffer[rt_ix].b),k,p,ix,insert) ) {
      shuffled = true;
      if ( level<f->primary_level[index_type] ) {
        get_max_key(&(f->buffer[lt_ix].b),&lt_sep);
        replace_max_key(f,index_type,&old_lt_sep,&lt_sep,f->buffer[lt_ix].contents,(unsigned)f->buffer[lt_ix].b.level+1);
        get_max_key(&(f->buffer[mid_ix].b),&mid_sep);
        replace_max_key(f,index_type,&old_mid_sep,&mid_sep,f->buffer[mid_ix].contents,(unsigned)f->buffer[mid_ix].b.level+1);
      }
      mark_modified(f,lt_ix);
      mark_modified(f,rt_ix);
    }
    if ( f->trace ) {
      if ( shuffled ) {
        printf("reshuffle succeeded, shuffled blocks are:\n");
        print_index_block(stdout,f,&(f->buffer[lt_ix].b));
        print_index_block(stdout,f,&(f->buffer[mid_ix].b));
        print_index_block(stdout,f,&(f->buffer[rt_ix].b));
      }
      else printf("reshuffle failed\n");
    }
    unlock_buffer(f,rt_ix);
    unlock_buffer(f,lt_ix);
  }
  unlock_buffer(f,mid_ix);

  return(shuffled);
}

/* create_new_primary creates a new primary block containing the key max  */
/*   and sets last_pntr */

static void create_new_primary(struct fcb *f, int index, struct leveln_pntr b, struct key *max,
  struct leveln_pntr newb)
{int ix; levelx_pntr bx; struct key dummy; /* block_type_t block;*/

  if ( f->primary_level[index]>=max_level-1 ) fatal_error(f,maxlevel_err);
  else {
    /*    printf("creating new %s primary, split blocks are\n",index_caption[index]);
    get_page(f,b,&block);
    print_index_block(stdout,f,&block);
    get_page(f,newb,&block);
    print_index_block(stdout,f,&block);*/



    f->primary_level[index]++;
    ix = allocate_index_block(f,index,&(f->first_at_level[f->primary_level[index]][index]),
      f->primary_level[index],&dummy,0);
    bx.pn = b;
    simple_insert(f,&(f->buffer[ix].b),0,max,&bx);
    f->last_pntr[f->primary_level[index]][index] = newb;

    if ( f->trace )
      printf("  creating new %s primary, levels are %s=%u,%s=%u,%s=%u\n",index_caption[index],
	   index_caption[user_ix],f->primary_level[user_ix],
	   index_caption[free_rec_ix],f->primary_level[free_rec_ix],
	   index_caption[free_lc_ix],f->primary_level[free_lc_ix]);
  }
}

/* split_block splits the block in buffer[bufix]. On entry key k    */
/*   belongs in entry ix and must be <= the */
/*   max_key in the parent block that points to this block.  It     */
/*   creates a new block, moves keys to it, and adjusts prev and    */
/*   next pointers. It then updates the parent block with a new     */
/*   pointer for the right block and a new max_key for the left or  */
/*   creates a new primary, as appropriate.                         */
/*   If we are inserting at eof (next=null, ix=keys_in_block)       */
/*   then sequential inserts are assumed, otherwise random.         */
/*   Note, however, that we do not use sequential mode for free     */
/*   space blocks.  This is because freespace management repeatedly */
/*   deletes the last key in the index then reinserts the last key  */
/*   (with a new lc) which leads to degenerate behavior when block  */
/*   splits result in a new primary.                                */

static void split_block(struct fcb *f, struct key *k, levelx_pntr *p, int bufix, int ix, boolean insert)
{int new_ix,i,index_type,old_block_lc,cnt,new_block_lc;
 unsigned parent_level,old_block_prefix_lc,new_block_prefix_lc,target;
 boolean split,seq,new_on_right,moved_new_key;
 struct leveln_pntr parent,oldb,newb,save_next,leftb,rightb;
 struct key left_max,right_max,original_max_key,old_max,new_max,temp; levelx_pntr leftbx;
 struct ix_block *old_block,*new_block;

  lock_buffer(f,bufix);
  index_type = f->buffer[bufix].b.index_type;
  oldb = f->buffer[bufix].contents;
  old_block = &(f->buffer[bufix].b);
  get_max_key(old_block,&original_max_key);
  old_block_lc = ix_pool_lc_after_change(f,old_block,k,p,ix,&old_block_prefix_lc,insert);
  target = old_block_lc / 2;

  seq = insert && null_pntr(f->buffer[bufix].b.next) && ix==f->buffer[bufix].b.keys_in_block;
  seq = seq && (index_type==user_ix);

  parent_level = f->buffer[bufix].b.level + 1;
  parent = parent_block(f,bufix,k);
  new_ix = allocate_index_block(f,index_type,&newb,f->buffer[bufix].b.level,k,0);
  lock_buffer(f,new_ix);
  new_block = &(f->buffer[new_ix].b);

  if ( f->trace ) {
    print_leveln_pntr("splitting block ",&oldb);
    print_leveln_pntr("new block is ",&newb);
    printf("insert=%d, target=%d, seq=%d, ix=%d\n",insert,target,seq,ix);
    print_key(index_type,k,"  ins/rep key=");
    printf("\n");
    print_key(index_type,&original_max_key,"  orig_max_key=");
    printf("\n");
    print_index_block(stdout,f,old_block);
  }
  if ( ix<(old_block->keys_in_block/2) ) { /* add block to left */
    new_on_right = false;
    new_block->next = oldb;
    new_block->prev = old_block->prev;
    if ( null_pntr(old_block->prev) ) {
      f->first_at_level[old_block->level][index_type] = newb;
    }
    else {
      i = get_index_update(f,old_block->prev);
      f->buffer[i].b.next = newb;
    }
    old_block->prev = newb;
    cnt = choose_left_move_cnt(f,new_block,old_block,k,p,ix,insert,
      old_block->keys_in_block,&old_block_lc,&old_block_prefix_lc,&new_block_lc,&new_block_prefix_lc);
    if ( ix==0 )set_empty_block_prefix(new_block,k,new_block_prefix_lc);
    else {
      get_nth_key(old_block,&temp,0);
      set_empty_block_prefix(new_block,&temp,new_block_prefix_lc);
    }
    moved_new_key = move_keys_to_left(f,new_block,old_block,cnt,k,p,ix,insert);
    if ( moved_new_key ) compress_ix_block(f,old_block,old_block_prefix_lc);
    else {
      if ( !insert ) simple_delete(f,old_block,ix-cnt);
      compress_ix_block(f,old_block,old_block_prefix_lc);
      if ( !simple_insert(f,old_block,ix-cnt,k,p) && show_errors )
        printf("**insert failed in split_block, new is on lt\n");
    }
  }
  else { /* add new block to right */
    new_on_right = true;
    new_block->next = old_block->next;
    new_block->prev = oldb;
    if ( !null_pntr(new_block->next) ) {
      i = get_index_update(f,new_block->next);
      f->buffer[i].b.prev = newb;
    }
    old_block->next = newb;
    if ( seq ) simple_insert(f,new_block,0,k,p);
    else {
      cnt = choose_right_move_cnt(f,old_block,new_block,k,p,ix,insert,target,
        &old_block_lc,&old_block_prefix_lc,&new_block_lc,&new_block_prefix_lc);
      if ( ix==old_block->keys_in_block-1 ) set_empty_block_prefix(new_block,k,new_block_prefix_lc);
      else set_empty_block_prefix(new_block,&original_max_key,new_block_prefix_lc);
      moved_new_key = move_keys_to_right(f,old_block,new_block,cnt,k,p,ix,insert);
      if ( moved_new_key ) compress_ix_block(f,old_block,old_block_prefix_lc);
      else {
        if ( !insert ) simple_delete(f,old_block,ix);
        compress_ix_block(f,old_block,old_block_prefix_lc);
        if ( !simple_insert(f,old_block,ix,k,p) && show_errors )
          printf("**insert failed in split_block, new is on rt\n");
      }
    }
  }

  check_ix_block_compression(f,old_block);
  check_ix_block_compression(f,new_block);
  
  if ( f->trace ) {
    printf("after split, new_on_right=%d, seq=%d, old block is\n",new_on_right,seq);
    print_index_block(stdout,f,old_block);
    printf("after split, new block is\n");
    print_index_block(stdout,f,new_block);
  }

  /*now propogate upward*/
  get_max_key(old_block,&old_max);
  get_max_key(new_block,&new_max);
  if ( new_on_right ) {
    leftb = oldb;
    rightb = newb;
    get_max_key(old_block,&left_max);
    get_max_key(new_block,&right_max);
    save_next = new_block->next;
  }
  else {
    leftb = newb;
    rightb = oldb;
    get_max_key(old_block,&right_max);
    get_max_key(new_block,&left_max);
    save_next = old_block->next;
  }
  unlock_buffer(f,bufix);
  unlock_buffer(f,new_ix);

  if ( parent_level>f->primary_level[index_type] )
    create_new_primary(f,index_type,leftb,&left_max,rightb);
  else {
    leftbx.pn = leftb;
    if ( null_pntr(save_next) ) {
      f->last_pntr[parent_level][index_type] = rightb;
      update_index(f,&left_max,parent,&leftbx);
    }
    else {
      split = replace_max_key_and_pntr(f,index_type,&original_max_key,&right_max,oldb,rightb,parent_level);
      if ( split ) {
        parent = search_index(f,index_type,parent_level+1,&left_max);
        if ( f->trace ) {
          print_leveln_pntr("  parent after replace_max split is ",&parent);
          printf("\n");
	}
      }
      update_index(f,&left_max,parent,&leftbx);
    }
  }
}

/* update_index1 inserts key k and pointer p into entry ix in the  */
/*   index block in buffer[bufix].  It assumes that the buffer has */
/*   been marked modified, locked and will be unlocked upon        */
/*   return              */  

static void update_index1(struct fcb *f, struct key *k, levelx_pntr *p, int bufix, int ix, boolean insert)
{int index_type,update_type,new_prefix_lc;
 unsigned level,new_lc;
 boolean at_end,update_parent=false;
 struct key old_max_key; struct leveln_pntr b;

  level = f->buffer[bufix].b.level;
  index_type = f->buffer[bufix].b.index_type;
  at_end = ix==f->buffer[bufix].b.keys_in_block;
  if ( at_end && f->primary_level[index_type]>level && !null_pntr(f->buffer[bufix].b.next) && insert) {
    update_parent = true;
    get_max_key(&(f->buffer[bufix].b),&old_max_key);
  }
  if ( f->trace ) {
    b = f->buffer[bufix].contents;
    print_leveln_pntr("  updating block ",&b);
    print_key(index_type,k,"with key="); printf(", ");
  }
  if ( insert )
    new_lc = ix_pool_lc_after_insert(f,&(f->buffer[bufix].b),k,p,ix,&new_prefix_lc);
  else new_lc = ix_pool_lc_after_replace(f,&(f->buffer[bufix].b),k,p,ix,&new_prefix_lc);
  if ( new_lc<=keyspace_lc ) {
    if ( !insert ) simple_delete(f,&(f->buffer[bufix].b),ix);
    prefix_simple_insert(f,&(f->buffer[bufix].b),ix,k,p);
    update_type = 0;
    if ( f->trace ) printf("simple insert\n");
    if ( update_parent )
      replace_max_key(f,index_type,&old_max_key,k,f->buffer[bufix].contents,(unsigned)level+1);
  }
  else if ( shuffle_keys(f,bufix,k,p,ix,insert) ) {
    update_type = 1;
    if ( f->trace ) printf("shuffled keys to insert\n");
  }
  else {
    update_type = 2;
    if ( f->trace ) {
      print_key(index_type,k,"block split, k=");
      printf(", keys_in_block=%d, chars_in_use=%d\n",
        f->buffer[bufix].b.keys_in_block,f->buffer[bufix].b.chars_in_use);
    }
    split_block(f,k,p,bufix,ix,insert);
  }
}

/* update_index inserts key k and pointer p into index block b. */

static void update_index(struct fcb *f, struct key *k, struct leveln_pntr b, levelx_pntr *p)
{int bufix,ix; boolean found; char *name="update_index";

  bufix = get_index_update(f,b);
  lock_buffer(f,bufix);
  if ( f->trace ) f->search_block_caller = name;
  ix = search_block(f,bufix,k,&found);
  update_index1(f,k,p,bufix,ix,!found);
  unlock_buffer(f,bufix);
}

/* intermediate calls */

/* extract_next extracts the key and pointer identified by the current file */
/*   position and advances the file position.  If max_key_lc==0 the key is  */
/*   not extracted.  Note that if the pointer contains the data_rec         */
/*   (lc<=f->max_data_in_index) then the data will extracted into rec.  The */
/*   caller should either point to the end destination for the data or      */
/*   point to the data_rec in p.  If max_rec_lc==0 no data will be          */
/*   extracted.                                                             */

static void extract_next(struct fcb *f, int index, int bufix, char t[], int *key_lc, int max_key_lc,
  level0_pntr *p, unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{struct key k;

  if ( f->position_ix[index]>=f->buffer[bufix].b.keys_in_block ) {
    t[0] = '\0'; *key_lc = 0; *p = null0_ptr;
    if ( null_pntr(f->buffer[bufix].b.next) ) f->error_code = ateof_err;
    else if ( show_errors ) {
      print_leveln_pntr("**Uh Oh.  Error in extract_next, block=",&(f->buffer[bufix].contents));
      printf("index=%d, position=%d, keys=%d, ",index,f->position_ix[index],
        f->buffer[bufix].b.keys_in_block);
      print_leveln_pntr("next",&(f->buffer[bufix].b.next));
      printf("\n");
    }
  }
  else {
    if ( max_key_lc>0 ) {
      get_nth_key(&(f->buffer[bufix].b),&k,f->position_ix[index]);
      if ( k.lc<=max_key_lc ) *key_lc = k.lc;
      else {
        f->error_code = longkey_err; *key_lc = max_key_lc;
      }
      mvc(k.text,0,t,0,(unsigned)*key_lc);
    }
    unpack0_ptr_and_rec(f,&(f->buffer[bufix]),f->position_ix[index],p,rec,rec_lc,(unsigned)max_rec_lc);
    if ( max_rec_lc==0 || max_rec_lc==f->data_in_index_lc ) /* not an error */;
    else if ( p->lc>(unsigned)*rec_lc ) f->error_code = longrec_err;
    f->position_ix[index]++;
    if ( f->position_ix[index]>=f->buffer[bufix].b.keys_in_block && !null_pntr(f->buffer[bufix].b.next) )
      set_position(f,index,f->buffer[bufix].b.next,0);
  }
}

void kf_set_bof(struct fcb *f, int index)
{
  f->position[index] = f->first_at_level[0][index];
  f->position_ix[index] = 0;
  f->seq_cnt[index] = 0;
}

int kf_next_rec(struct fcb *f, int index, unsigned char key[], int *key_lc, int max_key_lc,
  level0_pntr *p, unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{int ix; struct leveln_pntr next;

  if ( check_fcb(f) ) {
    ix = get_index(f,f->position[index]);
    next = f->buffer[ix].b.next;
    while ( f->position_ix[index]>=f->buffer[ix].b.keys_in_block && !null_pntr(next) ) {
      ix = get_index(f,next);
      set_position(f,index,next,0);
      next = f->buffer[ix].b.next;
    }
    extract_next(f,index,ix,key,key_lc,max_key_lc,p,rec,rec_lc,max_rec_lc);
  }
  return(f->error_code);
}

int kf_next_ptr(struct fcb *f, int index, unsigned char t[], int *key_lc, int max_key_lc,
   level0_pntr *p)
{int err,rec_lc;

  err = kf_next_rec(f,index,t,key_lc,max_key_lc,p,p->data_rec,&rec_lc,f->data_in_index_lc);
  return(err);
}

static int kf_prev_rec(struct fcb *f, int index, unsigned char t[], int *key_lc, int max_key_lc,
  level0_pntr *p, unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{int bufix; boolean done=false; struct leveln_pntr prev; struct key k;

  if ( check_fcb(f) ) {
    bufix = get_index(f,f->position[index]);
    if ( f->position_ix[index]==0 ) {
      prev = f->buffer[bufix].b.prev;
      do {
        if ( null_pntr(prev) ) {
          done = true; f->error_code = atbof_err;
          *key_lc = 0; t[0] = '\0'; *p = null0_ptr;
        }
        else {
          bufix = get_index(f,prev);
          set_position(f,index,prev,f->buffer[bufix].b.keys_in_block);
          prev = f->buffer[bufix].b.prev;
          done = f->buffer[bufix].b.keys_in_block > 0;
        }
      } while ( !done );
    }
    if ( f->error_code==no_err && f->position_ix[index]>0 ) {
      f->position_ix[index]--;
      get_nth_key(&(f->buffer[bufix].b),&k,f->position_ix[index]);
      *key_lc = f->buffer[bufix].b.keys[f->position_ix[index]].lc;
      if ( k.lc<=max_key_lc ) *key_lc = k.lc;
      else {
        *key_lc = max_key_lc; f->error_code = longkey_err;
      }
      mvc(k.text,0,t,0,(unsigned)*key_lc);
      unpack0_ptr_and_rec(f,&(f->buffer[bufix]),f->position_ix[index],p,rec,rec_lc,(unsigned)max_rec_lc);
      if ( max_rec_lc==0 || max_rec_lc==f->data_in_index_lc ) /* not an error */;
      else if ( p->lc>*rec_lc ) f->error_code = longrec_err;
    }
  }
  return(f->error_code);
}

static int kf_prev_ptr(struct fcb *f, int index, char t[], int *key_lc, int max_key_lc,
  level0_pntr *p)
{int err,rec_lc;

  err = kf_prev_rec(f,index,t,key_lc,max_key_lc,p,p->data_rec,&rec_lc,f->data_in_index_lc);
  return(err);
}

/* kf_get_ptr gets the pointer associated with key t. We've tried   */
/*   strategies for sequential access that look first in the        */
/*   current position block but they seem to work well only when    */
/*   we are accessing nearly every key.                             */

static int kf_get_rec(struct fcb *f, int index, unsigned char t[], int key_lc, level0_pntr *p,
  unsigned char rec[], unsigned *rec_lc, unsigned max_rec_lc)
{struct leveln_pntr b,last_position; int ix=0,lc,bufix=0,last_ix; struct key k;
boolean found=false,seq=false;
 char t1[maxkey_lc],*name="kf_get_ptr";

  set_up(f,t,key_lc,&k);
  if ( f->error_code==no_err ) {
    last_position = f->position[index];
    last_ix = f->position_ix[index];
    b = search_index(f,index,level_one,&k);
    bufix = get_index(f,b);
    if ( f->trace ) f->search_block_caller = name;
    ix = search_block(f,bufix,&k,&found);
    set_position(f,index,b,ix);
    /*  printf(" kf_get_ptr, last_posn/ix=" UINT64_format "/%d, current posn/ix=" UINT64_format "/%d\n",
	  last_position.block,last_ix,f->position[index].block,f->position_ix[index]);*/

    if ( (eq_pntr(b,last_position) && ix>=last_ix) ) seq = true;
    else if ( eq_pntr(f->buffer[bufix].b.prev,last_position) ) seq = true;
    if ( seq ) {
      if ( f->seq_cnt[index]<INT_MAX ) (f->seq_cnt[index])++;
    }
    else f->seq_cnt[index] = 0;

    if ( found ) {
      extract_next(f,index,bufix,t1,&lc,0,p,rec,rec_lc,max_rec_lc);
    }
    else if ( f->error_code==no_err ) {
      f->error_code = getnokey_err; *p = null0_ptr;
    }
  }
  return(f->error_code);
}

static int kf_get_ptr(struct fcb *f,int index, unsigned char t[], int key_lc,
  level0_pntr *p)
{int err,rec_lc;

  err = kf_get_rec(f,index,t,key_lc,p,p->data_rec,&rec_lc,f->data_in_index_lc);
  return(err);
}

static int kf_put_ptr(struct fcb *f, int index, char t[], int key_lc, level0_pntr p)
{struct leveln_pntr b; levelx_pntr px; struct key k;

  if ( f->read_only ) f->error_code = read_only_err;
  else {
    set_up(f,t,key_lc,&k);
    if ( f->error_code==no_err ) {
      b = search_index(f,index,level_one,&k);
      px.p0 = p;
      update_index(f,&k,b,&px);
      kf_set_bof(f,index);
    }
  }
  return(f->error_code);
}

static int kf_delete_ptr(struct fcb *f, int index, char t[], int key_lc)
{struct key k; level0_pntr p;

  if ( f->read_only ) f->error_code = read_only_err;
  else {
    set_up(f,t,key_lc,&k);
    if (f->error_code==no_err ) {
      index_delete(f,index,k,&p,level_zero);
      kf_set_bof(f,index);
    }
  }
  return(f->error_code);
}

static int kf_put_rec(struct fcb *f,int index, unsigned char t[], int key_lc, char r[], unsigned rlc)
{int ix,bufix,lc,rec_lc; boolean have_space=false,found; char t1[maxkey_lc],*name="kf_put_rec";
 struct key k; struct leveln_pntr b; level0_pntr p; levelx_pntr px; unsigned char dummy[2];

  if ( f->read_only ) f->error_code = read_only_err;
  else {
    set_up(f,t,key_lc,&k);
    if ( f->error_code==no_err ) {
      b = search_index(f,index,level_one,&k);
      bufix = get_index_update(f,b);
      lock_buffer(f,bufix);
      if ( f->trace ) f->search_block_caller = name;
      ix = search_block(f,bufix,&k,&found);
      set_position(f,index,b,ix);
      if ( found  ) {
        extract_next(f,index,bufix,t1,&lc,0,&p,dummy,&rec_lc,0);

        if ( rlc<=f->data_in_index_lc ) {
          if ( p.lc>f->data_in_index_lc ) deallocate_rec(f,p);
          have_space = true; p = dummy_ptr; p.lc = rlc;
        }
        else { /* new rec goes on disk */
          if ( p.lc>f->data_in_index_lc && (rec_allocation_lc(rlc)==rec_allocation_lc(p.lc)) ) {
            have_space = true; p.lc = rlc;
          }
          else {
            if ( p.lc>f->data_in_index_lc ) deallocate_rec(f,p);
            have_space =  allocate_rec(f,rlc,&p);
          }
        }
      }
      else {
        have_space = allocate_rec(f,rlc,&p);
      }
      if ( have_space ) {
        insert_rec(f,r,&p); 
        px.p0 = p;
	update_index1(f,&k,&px,bufix,ix,!found);
        kf_set_bof(f,index);
      }
      unlock_buffer(f,bufix);
    }
  }
  return(f->error_code);
}

static int kf_delete_rec(struct fcb *f, int index, unsigned char t[], int key_lc)
{level0_pntr p; struct key k;

  if ( f->read_only ) f->error_code = read_only_err;
  else {
    set_up(f,t,key_lc,&k);
    if (f->error_code==no_err ) {
      index_delete(f,index,k,&p,level_zero);
      if (f->error_code==no_err ) deallocate_rec(f,p);
      kf_set_bof(f,index);
    }
  }
  return(f->error_code);
}


/* Freespace management */

static boolean contiguous(level0_pntr p1, level0_pntr p2)
{
  if ( p1.segment!=p2.segment ) return(false);
  if ( (p1.sc+p1.lc)==p2.sc ) return(true);
  else return(false);
}

/* allocate_block allocates a new block and returns the buffer index */
/*   in which it lies.  If there are any blocks on the free block    */
/*   chain for this index level then the first block is returned.  If the */
/*   free block chain is empty then block_allocation_unit blocks are */
/*   allocated.  One is returned and the rest are initialized,       */
/*   and placed on the free block chain for this level.     */

static int allocate_block(struct fcb *f, int index_type, unsigned level)
{int i,bufix=0,temp; struct leveln_pntr p,p1; block_type_t b; struct key k;

  if ( null_pntr(f->first_free_block[level][index_type]) ) {
    if ( extend_file(f,(unsigned)(block_allocation_unit*block_lc),&p) ) {
      /* add blocks after the first to a free_block chain */
      p1.segment = p.segment;
      p1.block = p.block + block_allocation_unit - 1;
      initialize_index_block(&b,index_type,level,&k,(unsigned)0);
      for (i=0; i<block_allocation_unit-1; i++) {
        temp = vacate_oldest_buffer(f,&p1);
	mark_modified(f,temp);
        initialize_index_block(&(f->buffer[temp].b),index_type,level,&k,(unsigned)0);
        f->buffer[temp].b.next = f->first_free_block[level][index_type];
        f->first_free_block[level][index_type] = p1;
        hash_chain_insert(f,temp);
        p1.block--;
      }
      bufix = vacate_oldest_buffer(f,&p);
      mark_modified(f,bufix);
      hash_chain_insert(f,bufix);
    }
  }
  else { /* we have a previously deallocated block */
    p = f->first_free_block[level][index_type];
    /*    printf("  reallocating empty block %d/%d\n",p.segment,p.block);*/
    bufix = get_index_update(f,p);
    mark_modified(f,bufix);
    f->first_free_block[level][index_type] = f->buffer[bufix].b.next;
  }
  if ( f->trace_freespace ) {
    print_leveln_pntr("  just allocated block ",&p);
    printf("in buf=%d, seg_cnt=%u\n",bufix,f->segment_cnt);
  }
  return(bufix);
}

static int unpack_16bit(unsigned char key[], UINT16 *n)
{unsigned i;

  *n = 0;
  for (i=0; i<sizeof(UINT16); i++) {
    *n = (*n << 8) + key[i];
  }
  return(sizeof(UINT16));
}

static int unpack_32bit(unsigned char key[], UINT32 *n)
{unsigned i;

  *n = 0;
  for (i=0; i<sizeof(UINT32); i++) {
    *n = (*n << 8) + key[i];
  }
  return(sizeof(UINT32));
}

static int unpack_64bit(unsigned char key[], UINT64 *n)
{unsigned i;

  *n = 0;
  for (i=0; i<sizeof(UINT64); i++) {
    *n = (*n << 8) + key[i];
  }
  return(sizeof(UINT64));
}

static int pack_16bit(unsigned char key[], UINT32 n)
{int i;

  for (i=1; i>=0 ; i--) {
    key[i] = n & 255;
    n = n >> 8;
  }
  return((int)sizeof(UINT16));
}

static int pack_32bit(unsigned char key[], UINT32 n)
{int i;

  for (i=sizeof(UINT32)-1; i>=0 ; i--) {
    key[i] = n & 255;
    n = n >> 8;
  }
  return((int)sizeof(UINT32));
}

static int pack_64bit(unsigned char key[], UINT64 n)
{int i;

  for (i=sizeof(UINT64)-1; i>=0 ; i--) {
    key[i] = n & 255;
    n = n >> 8;
  }
  return((int)sizeof(UINT64));
}

int unpack_lc_key(unsigned char key[], level0_pntr *p)
{int lc; UINT16 segment; UINT32 plc; UINT64 sc;

  lc = unpack_32bit(key,&plc);
  p->lc = plc;
  lc = lc + unpack_16bit(key+lc,&segment);
  p->segment = segment;
  lc = lc + unpack_64bit(key+lc,&sc);
  p->sc = sc;
  /*  printf("unpacked lc key %d/%d/%d, lc=%d\n",p->lc,p->segment,p->sc,lc);*/
  return(lc);
}

static int pack_lc_key(unsigned char key[], level0_pntr p)
{int lc;

  lc = pack_32bit(key,p.lc);
  lc = lc + pack_16bit(key+lc,p.segment);
  lc = lc + pack_64bit(key+lc,p.sc);
  return(lc);
}

int unpack_rec_key(unsigned char key[], level0_pntr *p)
{int lc; UINT16 segment; UINT64 sc;

  lc = unpack_16bit(key,&segment);
  p->segment = segment;
  lc = lc + unpack_64bit(key+lc,&sc);
  p->sc = sc;
  /* printf("unpacked rec key %d/%d, lc=%d\n",p->segment,p->sc,lc);*/
  return(lc);
}

static int pack_rec_key(unsigned char key[], level0_pntr p)
{int lc;

  lc = pack_16bit(key,p.segment);
  lc = lc + pack_64bit(key+lc,p.sc);
  return(lc);
}

static void insert_freespace_entry(struct fcb *f, level0_pntr *p0)
{int err,key_lc; unsigned char key[maxkey_lc]; level0_pntr p;

  if ( f->trace_freespace ) {
    print_level0_pntr("inserting freespace entry ",p0);
    printf("\n");
  }
  p = *p0;
  p.lc = rec_allocation_lc(p0->lc);
  key_lc = pack_lc_key(key,p);
  err = kf_put_ptr(f,free_lc_ix,key,key_lc,dummy_ptr);
  if ( err!=no_err ) {
    set_error1(f,free_insrt_err,"**Couldn't insert free_lc entry, err=",err);
  }
  else {
    key_lc = pack_rec_key(key,p);
    err = kf_put_rec(f,free_rec_ix,key,key_lc,(char *) &(p.lc),(unsigned)sizeof(int));
    if ( err!=no_err ) {
      set_error1(f,free_insrt_err,"**Couldn't insert free_rec entry, err=",err);
    }
  }
}

static void delete_freespace_entry(struct fcb *f, level0_pntr *p0)
{int err,key_lc; unsigned char key[maxkey_lc]; level0_pntr p;

  if ( f->trace_freespace ) {
    print_level0_pntr("deleting freespace entry ",p0);
    printf("\n");
  }
  p = *p0;
  p.lc = rec_allocation_lc(p0->lc);
  key_lc = pack_lc_key(key,p);
  err = kf_delete_ptr(f,free_lc_ix,key,key_lc);
  if ( err!=no_err ) {
    f->error_code = free_dlt_err;
    if ( show_errors ) {
      print_level0_pntr("**Couldn't delete free_lc entry ",&p);
      printf("err=%d\n",err);
    }
  }
  else {
    key_lc = pack_rec_key(key,p);
    err = kf_delete_rec(f,free_rec_ix,key,key_lc);
    if ( err!=no_err ) {
      f->error_code = free_dlt_err;
      if ( show_errors ) {
        print_level0_pntr("**Couldn't delete free_rec entry ",&p);
        printf("err=%d, free_rec_ix=%d\n",err,free_rec_ix);
      }
    }
  }
}

static boolean allocate_rec(struct fcb *f, unsigned lc, level0_pntr *p)
{int err,key_lc,dlc; unsigned block_allocate_lc,rec_allocate_lc; boolean have_space=false;
 unsigned char key[maxkey_lc],dummy[2];
 level0_pntr p0,dummy_p0; struct leveln_pntr pn;

  if ( f->trace_freespace ) printf("allocating rec lc=%u\n",lc);
  p->segment = 0; p->sc = 0; p->lc = lc;
  if ( lc<=f->data_in_index_lc ) have_space = true;
  else {
    rec_allocate_lc = rec_allocation_lc(lc);
    p0.segment = 0; p0.sc = 0; p0.lc = rec_allocate_lc;
    key_lc = pack_lc_key(key,p0);
    err = kf_get_rec(f,free_lc_ix,key,key_lc,&dummy_p0,dummy,&dlc,0);
    if ( err==getnokey_err ) {
      err = kf_next_rec(f,free_lc_ix,key,&key_lc,maxkey_lc,&dummy_p0,dummy,&dlc,0);
    }
    if ( err==ateof_err ) {
      f->error_code = no_err;
      block_allocate_lc = allocation_lc(lc,(unsigned)(block_allocation_unit*block_lc));
      have_space = extend_file(f,block_allocate_lc,&pn);
      p->segment = pn.segment;
      p->sc = pn.block << f->block_shift;
      p0.segment = pn.segment;
      p0.sc = p->sc + rec_allocate_lc;
      p0.lc = block_allocate_lc - rec_allocate_lc;
      if ( f->trace_freespace ) printf("new space, block_lc=%d, residual=%d\n",
        block_allocate_lc,block_allocate_lc-rec_allocate_lc);
      if ( p0.lc>0 ) insert_freespace_entry(f,&p0);
    }
    else if ( err==no_err ) {
      if ( key_lc!=freespace_lc_key_lc )
        set_error1(f,alloc_rec_err,"**Uh Oh. Expected free_lc_key to be 14, is=",key_lc);
      unpack_lc_key(key,&p0);
      if ( p0.lc>=rec_allocate_lc ) {
        have_space = true;
        p->segment = p0.segment;
        p->sc = p0.sc;
        delete_freespace_entry(f,&p0);
        p0.sc = p0.sc + rec_allocate_lc;
        p0.lc = p0.lc - rec_allocate_lc;
        if ( p0.lc>0 ) insert_freespace_entry(f,&p0);
      }
    }
    else set_error1(f,alloc_rec_err,"**Couldn't get free_lc entry, err=",err);
  }
  return(have_space);
}

/* deallocate_rec deallocates any space allocated with pointer p. */
/*   If p.lc<data_in_index_lc then p doesn't point to a disk record, the  */
/*   data has been stored in p.sc.                                */


static void deallocate_rec(struct fcb *f, level0_pntr p)
{int err,key_lc,start_key_lc,lc,rec_lc,dlc; level0_pntr p0,p1,dummy_p0;
unsigned char start_key[maxkey_lc],key[maxkey_lc],dummy[2];

  if ( f->trace_freespace ) {
    print_level0_pntr("deallocating rec ",&p);
    printf("\n");
  }
  /*  if ( p.lc >= min_disk_rec_lc ) {*/ /* a real disk record */
  if ( p.lc > f->data_in_index_lc ) {
    p0 = p;
    p0.lc = rec_allocation_lc(p.lc);
    start_key_lc = pack_rec_key(start_key,p0);
    err = kf_get_rec(f,free_rec_ix,start_key,start_key_lc,&dummy_p0,(char *) &lc,&rec_lc,(unsigned)sizeof(int));
    if ( err==no_err ) {
      f->error_code = dealloc_rec_err;
      if ( show_errors ) {
        print_level0_pntr("**Uh Oh. Trying to deallocate entry allready in free list ",&p0);
        printf("\n");
      }
    }
    err = kf_prev_rec(f,free_rec_ix,key,&key_lc,maxkey_lc,&dummy_p0,(char *) &lc,&rec_lc,(unsigned)sizeof(int));
    if ( err==atbof_err ) /* nothing to merge */ { if ( f->trace_freespace ) printf("prev is bof\n"); }
    else if ( err!=no_err ) {
      set_error1(f,dealloc_rec_err,"**couldn't get prev rec in deallocate_rec, err=",err);
    }
    else {
      key_lc = unpack_rec_key(key,&p1);
      p1.lc = lc;
      if ( f->trace_freespace ) print_level0_pntr("prev rec is ",&p1);
      if ( contiguous(p1,p0) ) {
        delete_freespace_entry(f,&p1);
        p0.sc = p1.sc;
        p0.lc = p0.lc + p1.lc;
        if ( f->trace_freespace ) print_level0_pntr("contiguous, merged entry is ",&p0);
        err = kf_get_rec(f,free_rec_ix,start_key,start_key_lc,&dummy_p0,(char *) &lc,&rec_lc,(unsigned)sizeof(int));
      }
      else err = kf_next_rec(f,free_rec_ix,key,&key_lc,maxkey_lc,&dummy_p0,dummy,&dlc,0/*get past current entry*/);
      if ( f->trace_freespace ) printf("\n");
    }
    err = kf_next_rec(f,free_rec_ix,key,&key_lc,maxkey_lc,&dummy_p0,(char *)&lc,&rec_lc,(int)sizeof(int));
    if ( err==ateof_err ) /* nothing to merge */ { if ( f->trace_freespace ) printf("next is eof\n"); }
    else if ( err!=no_err ) {
      set_error1(f,dealloc_rec_err,"**couldn't get next rec in deallocate_rec, err=",err);
    }
    else {
      key_lc = unpack_rec_key(key,&p1);
      p1.lc = lc;
      if ( f->trace_freespace ) print_level0_pntr("next rec is ",&p1);
      if ( contiguous(p0,p1) ) {
        delete_freespace_entry(f,&p1);
        p0.lc = p0.lc + p1.lc;
        if ( f->trace_freespace ) print_level0_pntr(" contiguous, merged entry is ",&p0);
      }
      if ( f->trace_freespace ) printf("\n");
    }
    insert_freespace_entry(f,&p0);
  }
}

static void init_free_space(struct fcb *f)
{int bufix; struct leveln_pntr b; struct key dummy;

  bufix = allocate_index_block(f,free_rec_ix,&b,level_zero,&dummy,0);
  f->first_at_level[level_zero][free_rec_ix] = f->buffer[bufix].contents;
  kf_set_bof(f,free_rec_ix);
  bufix = allocate_index_block(f,free_lc_ix,&b,level_zero,&dummy,0);
  f->first_at_level[level_zero][free_lc_ix] = f->buffer[bufix].contents;
  kf_set_bof(f,free_lc_ix);
}


/* user callable entries */

int set_bof(struct fcb *f)
{
  if ( check_fcb(f) ) {
    kf_set_bof(f,user_ix);
  }
  return(f->error_code);
}

int set_eof(struct fcb *f)
{int ix;

  if ( check_fcb(f) ) {
    if ( f->primary_level[user_ix]==0 ) f->position[user_ix] = f->first_at_level[0][user_ix];
    else f->position[user_ix] = f->last_pntr[1][user_ix];
    ix = get_index(f,f->position[user_ix]);
    f->position_ix[user_ix] = f->buffer[ix].b.keys_in_block;
  }
  return(f->error_code);
}

int open_key(struct fcb *f, char id[], int lc, int read_only)
{  
  /*  read_fib(f,id,false,read_only);*/
  read_fib(f,id,machine_is_little_endian(),read_only);
  if ( f->error_code!=no_err ) fatal_error(f,badopen_err);
  else {
    init_key(f,id,lc);
    kf_set_bof(f,user_ix);
    kf_set_bof(f,free_rec_ix);
    kf_set_bof(f,free_lc_ix);
  }
  return(f->error_code);
}

int close_key(struct fcb *f)
{int i; FILE *temp;

#ifdef log_buffers
  fclose(buffer_log);
#endif
  if ( f->trace ) {
    printf("  read_cnt =%d\n",read_cnt);
    printf("  write_cnt=%d\n",write_cnt);
  }
  if ( f->marker!=keyf ) f->error_code = notkeyfil_err;
  else {
    f->error_code = no_err;
    for (i=0; i<f->buffers_in_use; i++) {
      if (f->buffer[i].modified){
        write_page(f,f->buffer[i].contents,&(f->buffer[i].b) );
        if ( trace_io ) {
          print_leveln_pntr("  wrote block ",&(f->buffer[i].contents));
          print_buffer_caption(stdout,f,i);
          printf(" from buffer %d\n",i);
        }
      }
      if (f->buffer[i].lock_cnt>0) {
        set_error(f,bad_close_err,"**Buffer locked at close\n");
      }
    }
    write_fib(f);
    for (i=0; i<f->open_file_cnt; i++) {
      if (f->trace) printf("  closing segment %d\n",f->open_segment[i]);
      temp = f->open_file[i];
      fclose(temp);
    }
    f->marker = 0;
  }
  return(f->error_code);
}

int create_key_ld(struct fcb *f, char id[], int fcb_lc, int data_in_index_lc)
{int i,j,bufix; struct leveln_pntr b; struct key dummy;

  f->error_code = no_err; f->version = current_version;
  f->segment_cnt = 0;
  f->marker = keyf; f->file_ok = true;
  f->read_only = false;
  for (i=0; i<max_index; i++) {
    f->primary_level[i] = level_zero;
    for (j=0; j<max_level; j++) {
      f->first_free_block[j][i] = nulln_ptr;
      f->first_at_level[j][i] = nulln_ptr;
      f->last_pntr[j][i] = nulln_ptr;
    }
  }
  f->max_file_lc = 1;
  for (i=0; i<file_lc_bits; i++) f->max_file_lc = f->max_file_lc * 2;
  f->max_file_lc = f->max_file_lc - 1;
  for (i=0; i<max_segment; i++) f->segment_length[i] = 0;
  f->data_in_index_lc = min_data_in_index_lc;
  if ( data_in_index_lc>max_data_in_index_lc ) set_error(f,data_lc_err,"New data_lc too big");
  /*  else if ( (unsigned)data_in_index_lc<min_data_in_index_lc );*/  /* leave at min */
  else f->data_in_index_lc = data_in_index_lc;

  if ( f->error_code==no_err ) {
    f->byte_swapping_required = machine_is_little_endian();
    /*    f->byte_swapping_required = false; */

    init_key(f,id,fcb_lc);
    if ( f->error_code==no_err ) {
      write_fib(f);
      f->segment_cnt = 1; f->segment_length[0] = ((fib_lc-1)/block_lc + 1) * block_lc;
      init_free_space(f);
      bufix = allocate_index_block(f,user_ix,&b,level_zero,&dummy,0);
      f->first_at_level[level_zero][user_ix] = f->buffer[bufix].contents;
      kf_set_bof(f,user_ix);
    }
  }
  return(f->error_code);
}

int create_key(struct fcb *f, char id[], int fcb_lc)
{

  create_key_ld(f,id,fcb_lc,(int) sizeof(UINT32));
  return(f->error_code);
}

int keyrec_lc(level0_pntr *p)
{
  if ( p->segment==max_segment && p->sc==0 ) return(p->lc);
  else if ( p->segment>=max_segment ) return(-1);
  else return(p->lc);
}

int next_ptr(struct fcb *f, char key[], int *key_lc, int max_key_lc, keyfile_pointer *pntr)
{int err;

  err = kf_next_ptr(f,user_ix,key,key_lc,max_key_lc,pntr);
  return(err);
}

int prev_ptr(struct fcb *f, char key[], int *key_lc, int max_key_lc, keyfile_pointer *pntr)
{int err;

  err = kf_prev_ptr(f,user_ix,key,key_lc,max_key_lc,pntr);
  return(err);
}


int get_ptr(struct fcb *f, char t[], int key_lc, keyfile_pointer *pntr)
{int err;

  err = kf_get_ptr(f,user_ix,t,key_lc,pntr);
  return(err);
}


int get_rec(struct fcb *f,char t[],int key_lc, char r[],int *rlc,int max_lc)
{level0_pntr dummy_p0;

  return( kf_get_rec(f,user_ix,t,key_lc,&dummy_p0,r,(unsigned *)rlc,(unsigned)max_lc) );
}

int next_rec(struct fcb *f, char t[], int *key_lc, int max_key_lc,
   char r[],int *rlc,int max_lc)
{level0_pntr dummy_p0;

  return( kf_next_rec(f,user_ix,t,key_lc,max_key_lc,&dummy_p0,r,(unsigned *)rlc,(unsigned)max_lc) );
}

int prev_rec(struct fcb *f, char t[], int *key_lc, int max_key_lc,
   char r[],int *rlc,int max_lc)
{level0_pntr dummy_p0;

  return( kf_prev_rec(f,user_ix,t,key_lc,max_key_lc,&dummy_p0,r,(unsigned *)rlc,(unsigned)max_lc) );
}

int put_ptr(struct fcb *f, char t[], int key_lc, level0_pntr *p)
{
  return( kf_put_ptr(f,user_ix,t,key_lc,*p) );
}

int delete_ptr(struct fcb *f, char t[], int key_lc)
{
  return( kf_delete_ptr(f,user_ix,t,key_lc) );
}

int put_rec(struct fcb *f,char t[], int key_lc, char r[], int rlc)
{
  return( kf_put_rec(f,user_ix,t,key_lc,r,(unsigned)rlc) );
}

int delete_rec(struct fcb *f, unsigned char key[], int key_lc)
{
  return( kf_delete_rec(f,user_ix,key,key_lc) );
}

/* Functions to support subrecords: */

/* Function: get_subrec
   Retrieve a part of a record.  This is provided for those cases where the 
   records are so large they must be read into memory from the disk in smaller
   pieces.  Use get_ptr to get the keyfile_pointer, then call get_subrec 
   repeatedly with bytes_to_read set to some manageable size and incrementing 
   offset by bytes_actually_read each time until p->lc bytes have been read.  
   (You must keep track of offset and bytes_to_read so that you don't ask for 
   too much.)

   Returns no_err if all went well, else a non-zero err code.
*/

int get_subrec(
   struct fcb *f, 
   level0_pntr *p,      // IN - the original pointer from get_ptr
   int offset,                 // IN - offset of data to get within the rec 
   int bytes_to_read,          // IN - how many bytes of the rec to get
   unsigned char *rec,         // IN-OUT - where to put the bytes from the rec
   int *bytes_actually_read,   // OUT - how many bytes were actually transferred
   int max_lc)                 // IN - max bytes to transfer
{size_t size; FILE *file;

  if ( check_fcb(f) ) {
    *bytes_actually_read = bytes_to_read;
    if ( *bytes_actually_read>max_lc ) {
      f->error_code = longrec_err; *bytes_actually_read = max_lc;
    }
    /*    if ( p->lc<min_disk_rec_lc ) {
      if ( (unsigned)offset+bytes_to_read>=min_disk_rec_lc ) {
        f->error_code = longrec_err; *bytes_actually_read = min_disk_rec_lc - offset - 1;
      }
      memcpy(rec,(unsigned char *)(&p->sc)+offset,(size_t) *bytes_actually_read);
      }*/
    if ( p->lc<=f->data_in_index_lc ) {
      if ( (unsigned)offset+bytes_to_read>=f->data_in_index_lc ) {
        f->error_code = longrec_err; *bytes_actually_read = f->data_in_index_lc - offset - 1;
      }
      memcpy(rec,(unsigned char *)(&p->data_rec)+offset,(size_t) *bytes_actually_read);
    }
    else {
      file = file_index(f,p->segment);
      if ( f->error_code!=no_err ) return(f->error_code);
      if ( fseeko(file,(FILE_OFFSET)p->sc+offset,0)!=0 ) {
        f->error_code = seek_err; return(f->error_code);
      }
      size = fread(rec,(size_t) 1,(size_t) *bytes_actually_read,file);
      if ( size!=(size_t)*bytes_actually_read ) f->error_code = read_err;
    }
  }
  return(f->error_code);
}


void kf_test(struct fcb *f)
{

}
