
typedef uint8_t  u8;
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;

struct filt8_dcb_struct { s8 prev, cur, out; };
extern struct filt8_dcb_struct filt8_dcb;
inline s8 filt8_dcb_update( s8 in, struct filt8_dcb_struct *dcb ){
	dcb->prev = dcb->cur;
	dcb->cur  = in;
	dcb->out  = (s16)in - (s16)dcb->prev + (((s16)127 * dcb->out) / 128);
	return dcb->out;
}

struct filt16_dcb_struct { s16 prev, cur, out; };
extern struct filt16_dcb_struct filt16_dcb;
inline s16 filt16_dcb_update( s16 in, struct filt16_dcb_struct *dcb ){
	dcb->prev = dcb->cur;
	dcb->cur  = in;
	dcb->out  = (s32)in - (s32)dcb->prev + (((s32)32735 * dcb->out) / 32768);
	return dcb->out;
}

