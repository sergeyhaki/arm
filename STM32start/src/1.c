#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "1.h"

struct filt8_dcb_struct  filt8_dcb;
extern inline s8 filt8_dcb_update( s8 in, struct filt8_dcb_struct *dcb );

struct filt16_dcb_struct filt16_dcb;
extern inline s16 filt16_dcb_update( s16 in, struct filt16_dcb_struct *dcb );

#define PROGMEM


#define SAMPLES 256
struct fft_sample_8 { s8 r, i; };
struct fft_sample_8 samples[SAMPLES];


//sine table
const s8 fft_256_sine_256_8[192] PROGMEM = {
  0, 3, 6, 9, 12, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 51, 54, 57, 60, 63, 65, 68, 71, 73, 76, 78, 81, 83, 85, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
  117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127, 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118, 117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100, 98, 96, 94, 92,
  90, 88, 85, 83, 81, 78, 76, 73, 71, 68, 65, 63, 60, 57, 54, 51, 49, 46, 43, 40, 37, 34, 31, 28, 25, 22, 19, 16, 12, 9, 6, 3, 0, -3, -6, -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
  -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88, -90, -92, -94, -96, -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116, -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126,
  -126, -127, -127, -127
};

// Fixed-point multiplication & scaling.Scaling ensures that result remains 16-bit
inline static s8 fft_mul_8(s8 a, s8 b) {
    //shift right one less bit (i.e. 15-1)
    s16 c = ((s16)a * (s16)b) >> 6;

    //last shift + rounding bit
    return (c >> 1) + (c & 0x01);
}

u8 sqrt16_div( u16 x ){
	u16 a, b=x;
	a = x = 0x3f;     x = b/x;
	a = x = (x+a)>>1; x = b/x;
	a = x = (x+a)>>1; x = b/x;
	x = (x+a)>>1;
	return(x);
}

u8 sqrt16_shift( u16 x ){
	u8 b = ((u8)(x >> 8)) ? 0x80 : 8, result=0;
	while (b){
		u8 g = result | b;
		if ((u16)g * g <= x) result = g;
		b >>= 1;
	}
	return result;
}

u8 sqrt16_shift2( u16 n ){
    u8 c,g;
    if ((u8)(n >> 8)) c = g = 0x80; else c = g = 8;
    for (;;) {
        if ((u16)g*g > n) g ^= c;
        c >>= 1;
        if (c == 0) return g;
        g |= c;
    }
}

#define sqrt_dijkstra_iter(N) \
    try = root + (1 << (N));  \
    if (n >= try << (N)){     \
        n -= try << (N);      \
        root |= 2 << (N);     \
    }
u8 sqrt16_dijkstra( u16 n ){
    u16 root=0, try;
    sqrt_dijkstra_iter(7); sqrt_dijkstra_iter(6); sqrt_dijkstra_iter(5); sqrt_dijkstra_iter(4);
    sqrt_dijkstra_iter(3); sqrt_dijkstra_iter(2); sqrt_dijkstra_iter(1); sqrt_dijkstra_iter(0);
    return root >> 1;
}

const u16 sqrt16_tab11[33] PROGMEM = {0,1,1,2,2,4,5,8,11,16,22,32,45,64,90,128,181,256,362,512,724,1024,1448,2048,2896,4096,5792,8192,11585,16384,23170,32768,46340};
const u16 sqrt16_tab12[32] PROGMEM = {32768,33276,33776,34269,34755,35235,35708,36174,36635,37090,37540,37984,38423,38858,39287,39712,40132,40548,40960,41367,41771,42170,42566,42959,43347,43733,44115,44493,44869,45241,45611,45977};
u8 sqrt16_tab1(u16 x){
    u8 cnt=0; u16 t=x;
    while (t){ cnt++; t >>= 1; }
    if (6 >= cnt) t = (x << (6-cnt));
    else          t = (x >> (cnt-6));
    return ((u32)sqrt16_tab11[cnt] * sqrt16_tab12[t & 31])>>15;
}

const u16 sqrt16_tab21[] PROGMEM = {
    0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529, 576, 625, 676, 729, 784, 841, 900, 961,
    1024, 1089, 1156, 1225, 1296, 1369, 1444, 1521, 1600, 1681, 1764, 1849, 1936, 2025, 2116, 2209, 2304, 2401, 2500, 2601, 2704, 2809, 2916, 3025,
    3136, 3249, 3364, 3481, 3600, 3721, 3844, 3969, 4096, 4225, 4356, 4489, 4624, 4761, 4900, 5041, 5184, 5329, 5476, 5625, 5776, 5929, 6084, 6241,
    6400, 6561, 6724, 6889, 7056, 7225, 7396, 7569, 7744, 7921, 8100, 8281, 8464, 8649, 8836, 9025, 9216, 9409, 9604, 9801, 10000, 10201, 10404, 10609,
    10816, 11025, 11236, 11449, 11664, 11881, 12100, 12321, 12544, 12769, 12996, 13225, 13456, 13689, 13924, 14161, 14400, 14641, 14884, 15129,
    15376, 15625, 15876, 16129, 16384, 16641, 16900, 17161, 17424, 17689, 17956, 18225, 18496, 18769, 19044, 19321, 19600, 19881, 20164, 20449,
    20736, 21025, 21316, 21609, 21904, 22201, 22500, 22801, 23104, 23409, 23716, 24025, 24336, 24649, 24964, 25281, 25600, 25921, 26244, 26569,
    26896, 27225, 27556, 27889, 28224, 28561, 28900, 29241, 29584, 29929, 30276, 30625, 30976, 31329, 31684, 32041, 32400, 32761, 33124, 33489,
    33856, 34225, 34596, 34969, 35344, 35721, 36100, 36481, 36864, 37249, 37636, 38025, 38416, 38809, 39204, 39601, 40000, 40401, 40804, 41209,
    41616, 42025, 42436, 42849, 43264, 43681, 44100, 44521, 44944, 45369, 45796, 46225, 46656, 47089, 47524, 47961, 48400, 48841, 49284, 49729,
    50176, 50625, 51076, 51529, 51984, 52441, 52900, 53361, 53824, 54289, 54756, 55225, 55696, 56169, 56644, 57121, 57600, 58081, 58564, 59049,
    59536, 60025, 60516, 61009, 61504, 62001, 62500, 63001, 63504, 64009, 64516, 65025
};
u8 sqrt16_tab2(u16 x) {
    const u16 *p = sqrt16_tab21;

    if (p[128] <= x) p += 128;
    if (p[ 64] <= x) p +=  64;
    if (p[ 32] <= x) p +=  32;
    if (p[ 16] <= x) p +=  16;
    if (p[  8] <= x) p +=   8;
    if (p[  4] <= x) p +=   4;
    if (p[  2] <= x) p +=   2;
    if (p[  1] <= x) p +=   1;

    return p - sqrt16_tab21;
}


// Forward/inverse fast Fourier transform. n_log - log2 of input samples, inverse==1 for inverse FFT
u16 fft256_8_calc(u8 inverse, struct fft_sample_8 *samples) {
    u16 l = 1, scale = 0;
    u8  nn = 255, shift = 1, k=7;

    // decimation in time - re-order data
    u16 mr = 0;
    for (u8 m=0; m<nn; m++) {
    	u16 l = 256;
    	do { l >>= 1; } while (mr+l > nn);
    	mr = (mr & (l-1)) + l;
    	if (mr <= m+1) continue;
    	s8 x = samples[m+1].r;
    	samples[m+1].r  = samples[mr].r;
    	samples[mr].r = x;
    	x = samples[m+1].i;
    	samples[m+1].i  = samples[mr].i;
    	samples[mr].i = x;
    }

    while (l < 256) {

/*    	if (inverse) {
        // variable scaling, depending upon data
    		shift = 0;
    		for (u16 i=0; i<256; ++i) {
    			s8 j = samples[i].r, m = samples[i].i;
    			if (j < 0) j = -j;
    			if (m < 0) m = -m;
//    			if (j > 16383 || m > 16383) { shift = 1; break; }
    		}
    		if (shift) ++scale;
    	}
*/
    	u16 istep = l << 1;
    	for (u16 m=0; m<l; ++m) {
    		u16 j = m << k;

    		s8 wr = fft_256_sine_256_8[j + (256 / 4)];
    		s8 wi = -fft_256_sine_256_8[j];

//    		if (inverse) wi = -wi;
    		if (shift) { wr >>= 1; wi >>= 1; }

    		for (u16 i=m; i<256; i+=istep) {
    			u16 j  = i + l;
    			s8  qr = samples[j].r, qi = samples[j].i;
    			s8  tr = fft_mul_8(wr, qr) - fft_mul_8(wi, qi);
    			s8  ti = fft_mul_8(wr, qi) + fft_mul_8(wi, qr);
    			qr = samples[i].r; qi = samples[i].i;
    			if (shift) { qr >>= 1; qi >>= 1; }
    			samples[j].r = qr - tr;
    			samples[j].i = qi - ti;
    			samples[i].r = qr + tr;
    			samples[i].i = qi + ti;
    		}
    	}
    	k--;
    	l = istep;
	}

    return scale;
}

const u8 fft256_8_win_hanning[] PROGMEM = {
	0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9, 10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
	37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76, 79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124,
	127, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173, 176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
	218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244, 245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
	255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246, 245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220,
	218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131,
	128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82, 79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40,
	37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11, 10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0
};
const u8 fft256_8_win_hamming[] PROGMEM = {
	20, 20, 21, 21, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 28, 29, 30, 32, 33, 34, 36, 37, 39, 40, 42, 43, 45, 47, 49, 51, 53,
	55, 57, 59, 61, 63, 66, 68, 70, 73, 75, 77, 80, 82, 85, 88, 90, 93, 95, 98, 101, 104, 106, 109, 112, 115, 118, 120, 123, 126, 129, 132, 135,
	138, 141, 143, 146, 149, 152, 155, 158, 161, 163, 166, 169, 172, 174, 177, 180, 183, 185, 188, 190, 193, 196, 198, 200, 203, 205, 208, 210, 212, 214, 216, 219,
	221, 223, 225, 227, 228, 230, 232, 234, 235, 237, 238, 240, 241, 242, 244, 245, 246, 247, 248, 249, 250, 251, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
	255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 246, 245, 244, 242, 241, 240, 238, 237, 235, 234, 232, 230, 228, 227, 225, 223,
	221, 219, 216, 214, 212, 210, 208, 205, 203, 200, 198, 196, 193, 190, 188, 185, 183, 180, 177, 174, 172, 169, 166, 163, 161, 158, 155, 152, 149, 146, 143, 141,
	138, 135, 132, 129, 126, 123, 120, 118, 115, 112, 109, 106, 104, 101, 98, 95, 93, 90, 88, 85, 82, 80, 77, 75, 73, 70, 68, 66, 63, 61, 59, 57,
	55, 53, 51, 49, 47, 45, 43, 42, 40, 39, 37, 36, 34, 33, 32, 30, 29, 28, 27, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 21, 21, 20
};
const u8 fft256_8_win_blackman[] PROGMEM = {
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
	19, 21, 22, 23, 25, 26, 28, 29, 31, 33, 34, 36, 38, 40, 42, 44, 46, 49, 51, 53, 56, 58, 61, 63, 66, 69, 71, 74, 77, 80, 83, 86,
	89, 92, 95, 99, 102, 105, 109, 112, 115, 119, 122, 126, 129, 133, 136, 140, 143, 147, 150, 154, 158, 161, 165, 168, 172, 175, 179, 182, 185, 189, 192, 195,
	198, 201, 205, 208, 210, 213, 216, 219, 222, 224, 227, 229, 231, 234, 236, 238, 240, 241, 243, 245, 246, 248, 249, 250, 251, 252, 253, 253, 254, 254, 255, 255,
	255, 255, 255, 254, 254, 253, 253, 252, 251, 250, 249, 248, 246, 245, 243, 241, 240, 238, 236, 234, 231, 229, 227, 224, 222, 219, 216, 213, 210, 208, 205, 201,
	198, 195, 192, 189, 185, 182, 179, 175, 172, 168, 165, 161, 158, 154, 150, 147, 143, 140, 136, 133, 129, 126, 122, 119, 115, 112, 109, 105, 102, 99, 95, 92,
	89, 86, 83, 80, 77, 74, 71, 69, 66, 63, 61, 58, 56, 53, 51, 49, 46, 44, 42, 40, 38, 36, 34, 33, 31, 29, 28, 26, 25, 23, 22, 21,
	19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 9, 8, 7, 7, 6, 6, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2
};
const u8 fft256_8_win_nuttall[] PROGMEM = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5,
	5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20, 22, 23, 25, 27, 28, 30, 32, 34, 36, 39, 41, 43, 46, 48, 51,
	54, 57, 60, 63, 66, 69, 72, 76, 79, 83, 86, 90, 94, 98, 101, 105, 109, 113, 117, 122, 126, 130, 134, 138, 143, 147, 151, 155, 160, 164, 168, 172,
	176, 180, 185, 189, 192, 196, 200, 204, 208, 211, 215, 218, 221, 224, 227, 230, 233, 235, 238, 240, 242, 244, 246, 248, 249, 251, 252, 253, 254, 254, 255, 255,
	255, 255, 255, 254, 254, 253, 252, 251, 249, 248, 246, 244, 242, 240, 238, 235, 233, 230, 227, 224, 221, 218, 215, 211, 208, 204, 200, 196, 192, 189, 185, 180,
	176, 172, 168, 164, 160, 155, 151, 147, 143, 138, 134, 130, 126, 122, 117, 113, 109, 105, 101, 98, 94, 90, 86, 83, 79, 76, 72, 69, 66, 63, 60, 57,
	54, 51, 48, 46, 43, 41, 39, 36, 34, 32, 30, 28, 27, 25, 23, 22, 20, 19, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 8, 7, 6, 6,
	5, 5, 4, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void fft256_8_win(struct fft_sample_8 *samples, u8 *win){
	u8 i=0;
	do {
		samples->r = ((s16)samples->r * (s16)(*win)) >> 8;
		samples++; win++; i++;
	} while (i);
}

void fft_amp_8(struct fft_sample_8 *samples, u16 n, u8 (*sqrt16)(u16)){
	while (n--){
		samples->r = sqrt16( (s16)samples->r * samples->r + (s16)samples->i * samples->i );
		samples++;
	}
}


struct fft_sample_16 { s16 r, i; };
const s16 fft_256_sine_16[192] PROGMEM = {
	0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739, 9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
	23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956, 30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
	32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285, 32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571, 30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
	23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279, 12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179, 6393, 5602, 4808, 4011, 3212, 2410, 1608, 804,
	0, -804, -1608, -2410, -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793, -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
	-23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790, -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757
};
s16 fft_mul_16(s16 a, s16 b) {
	if ((a == -32768)&&(b == -32768)) a = -32767;
	s32 c = (s32)a * (s32)b;
	return c >> 15;
}
void fft256_forw_16(struct fft_sample_16 *samples) {
    u16 l = 1;
    u8  nn = 255, k=7;

    // decimation in time - re-order data
    u16 mr = 0;
    for (u8 m=0; m<nn; m++) {
    	u16 l = 256;
    	do { l >>= 1; } while (mr+l > nn);
    	mr = (mr & (l-1)) + l;
    	if (mr <= m+1) continue;
    	s16 x = samples[m+1].r;
    	samples[m+1].r = samples[mr].r;
    	samples[mr].r  = x;
    	x = samples[m+1].i;
    	samples[m+1].i = samples[mr].i;
    	samples[mr].i  = x;
    }

    while (l < 256) {
    	u16 istep = l << 1;
    	for (u16 m=0; m<l; ++m) {
    		u16 j = m << k;

    		s16 wr =  fft_256_sine_16[j + (256 / 4)];
    		s16 wi = -fft_256_sine_16[j];

    		wr /= 2; wi /= 2;

    		for (u16 i=m; i<256; i+=istep) {
    			u16 j  = i + l;
    			s16 qr = samples[j].r, qi = samples[j].i;
    			s16 tr = fft_mul_16(wr, qr) - fft_mul_16(wi, qi);
    			s16 ti = fft_mul_16(wr, qi) + fft_mul_16(wi, qr);
    			qr = samples[i].r; qi = samples[i].i;
    			qr /= 2; qi /= 2;
    			samples[j].r = qr - tr; samples[j].i = qi - ti;
    			samples[i].r = qr + tr; samples[i].i = qi + ti;
    		}
    	}
    	k--;
    	l = istep;
	}
}

u16 sqrt32_shift( u32 x ){
	u16 b = (u8)(x >> 24) ? 0x8000 : (u8)(x >> 16) ? 0x0800 : (u8)(x >> 8)  ? 0x80 : 8, result=0;
	while (b){
		u16 g = result | b;
		if ((u32)g * g <= x) result = g;
		b >>= 1;
	}
	return result;
}
void fft_amp_16(struct fft_sample_16 *samples, u16 n, u16 (*sqrt32)(u32)){
	while (n--){
		samples->r = sqrt32( (u32)((s32)samples->r * (s32)samples->r) + (u32)((s32)samples->i * (s32)samples->i));
		samples++;
	}
}

const u16 fft256_16_win_hanning[] PROGMEM = {
	0, 10, 39, 89, 158, 246, 355, 482, 630, 796, 982, 1187, 1411, 1654, 1915, 2196,
	2494, 2811, 3146, 3499, 3869, 4257, 4662, 5084, 5522, 5977, 6448, 6935, 7438, 7956, 8488, 9036,
	9597, 10173, 10762, 11365, 11980, 12608, 13248, 13900, 14563, 15237, 15922, 16616, 17321, 18035, 18758, 19489,
	20228, 20975, 21728, 22489, 23256, 24028, 24806, 25588, 26375, 27166, 27960, 28756, 29556, 30357, 31160, 31963,
	32767, 33572, 34375, 35178, 35979, 36779, 37575, 38369, 39160, 39947, 40729, 41507, 42279, 43046, 43807, 44560,
	45307, 46046, 46777, 47500, 48214, 48919, 49613, 50298, 50972, 51635, 52287, 52927, 53555, 54170, 54773, 55362,
	55938, 56499, 57047, 57579, 58097, 58600, 59087, 59558, 60013, 60451, 60873, 61278, 61666, 62036, 62389, 62724,
	63041, 63339, 63620, 63881, 64124, 64348, 64553, 64739, 64905, 65053, 65180, 65289, 65377, 65446, 65496, 65525,
	65535, 65525, 65496, 65446, 65377, 65289, 65180, 65053, 64905, 64739, 64553, 64348, 64124, 63881, 63620, 63339,
	63041, 62724, 62389, 62036, 61666, 61278, 60873, 60451, 60013, 59558, 59087, 58600, 58097, 57579, 57047, 56499,
	55938, 55362, 54773, 54170, 53555, 52927, 52287, 51635, 50972, 50298, 49613, 48919, 48214, 47500, 46777, 46046,
	45307, 44560, 43807, 43046, 42279, 41507, 40729, 39947, 39160, 38369, 37575, 36779, 35979, 35178, 34375, 33572,
	32768, 31963, 31160, 30357, 29556, 28756, 27960, 27166, 26375, 25588, 24806, 24028, 23256, 22489, 21728, 20975,
	20228, 19489, 18758, 18035, 17321, 16616, 15922, 15237, 14563, 13900, 13248, 12608, 11980, 11365, 10762, 10173,
	9597, 9036, 8488, 7956, 7438, 6935, 6448, 5977, 5522, 5084, 4662, 4257, 3869, 3499, 3146, 2811,
	2494, 2196, 1915, 1654, 1411, 1187, 982, 796, 630, 482, 355, 246, 158, 89, 39, 10
};
const u16 fft256_16_win_hamming[] PROGMEM = {
	5243, 5252, 5279, 5324, 5388, 5470, 5569, 5687, 5822, 5975, 6146, 6335, 6541, 6764, 7005, 7263,
	7538, 7829, 8137, 8462, 8802, 9159, 9532, 9920, 10323, 10742, 11175, 11623, 12086, 12562, 13052, 13556,
	14072, 14602, 15144, 15698, 16264, 16842, 17431, 18031, 18641, 19261, 19891, 20530, 21178, 21835, 22500, 23172,
	23852, 24539, 25233, 25933, 26638, 27349, 28064, 28784, 29508, 30235, 30966, 31699, 32434, 33171, 33910, 34649,
	35389, 36129, 36868, 37607, 38344, 39079, 39812, 40543, 41270, 41994, 42714, 43429, 44140, 44845, 45545, 46238,
	46925, 47605, 48278, 48943, 49600, 50248, 50887, 51517, 52137, 52747, 53347, 53936, 54513, 55080, 55634, 56176,
	56705, 57222, 57726, 58216, 58692, 59154, 59602, 60036, 60454, 60858, 61246, 61619, 61975, 62316, 62641, 62949,
	63240, 63515, 63773, 64013, 64237, 64443, 64632, 64803, 64956, 65091, 65209, 65308, 65390, 65453, 65499, 65526,
	65535, 65526, 65499, 65453, 65390, 65308, 65209, 65091, 64956, 64803, 64632, 64443, 64237, 64013, 63773, 63515,
	63240, 62949, 62641, 62316, 61975, 61619, 61246, 60858, 60454, 60036, 59602, 59154, 58692, 58216, 57726, 57222,
	56705, 56176, 55634, 55080, 54513, 53936, 53347, 52747, 52137, 51517, 50887, 50248, 49600, 48943, 48278, 47605,
	46925, 46238, 45545, 44845, 44140, 43429, 42714, 41994, 41270, 40543, 39812, 39079, 38344, 37607, 36868, 36129,
	35389, 34649, 33910, 33171, 32434, 31699, 30966, 30235, 29508, 28784, 28064, 27349, 26638, 25933, 25233, 24539,
	23852, 23172, 22500, 21835, 21178, 20530, 19891, 19261, 18641, 18031, 17431, 16842, 16264, 15698, 15144, 14602,
	14072, 13556, 13052, 12562, 12086, 11623, 11175, 10742, 10323, 9920, 9532, 9159, 8802, 8462, 8137, 7829,
	7538, 7263, 7005, 6764, 6541, 6335, 6146, 5975, 5822, 5687, 5569, 5470, 5388, 5324, 5279, 5252,
};
const u16 fft256_16_win_blackman[] PROGMEM = {
	451, 455, 466, 484, 511, 545, 586, 636, 693, 758, 831, 913, 1003, 1102, 1210, 1327,
	1453, 1588, 1734, 1889, 2055, 2231, 2418, 2617, 2826, 3047, 3281, 3526, 3784, 4054, 4338, 4635,
	4946, 5270, 5609, 5962, 6330, 6712, 7109, 7522, 7950, 8393, 8853, 9327, 9818, 10325, 10848, 11387,
	11942, 12513, 13100, 13704, 14323, 14957, 15608, 16274, 16955, 17651, 18362, 19088, 19827, 20581, 21348, 22128,
	22920, 23725, 24541, 25369, 26207, 27055, 27912, 28778, 29652, 30534, 31422, 32316, 33216, 34119, 35027, 35937,
	36849, 37762, 38675, 39588, 40499, 41407, 42312, 43213, 44109, 44998, 45880, 46754, 47619, 48473, 49317, 50149,
	50967, 51772, 52562, 53337, 54094, 54835, 55557, 56259, 56942, 57603, 58243, 58860, 59454, 60024, 60569, 61089,
	61583, 62050, 62489, 62901, 63285, 63640, 63965, 64261, 64526, 64761, 64966, 65139, 65281, 65392, 65471, 65519,
	65535, 65519, 65471, 65392, 65281, 65139, 64966, 64761, 64526, 64261, 63965, 63640, 63285, 62901, 62489, 62050,
	61583, 61089, 60569, 60024, 59454, 58860, 58243, 57603, 56942, 56259, 55557, 54835, 54094, 53337, 52562, 51772,
	50967, 50149, 49317, 48473, 47619, 46754, 45880, 44998, 44109, 43213, 42312, 41407, 40499, 39588, 38675, 37762,
	36849, 35937, 35027, 34119, 33216, 32316, 31422, 30534, 29652, 28778, 27912, 27055, 26207, 25369, 24541, 23725,
	22920, 22128, 21348, 20581, 19827, 19088, 18362, 17651, 16955, 16274, 15608, 14957, 14323, 13704, 13100, 12513,
	11942, 11387, 10848, 10325, 9818, 9327, 8853, 8393, 7950, 7522, 7109, 6712, 6330, 5962, 5609, 5270,
	4946, 4635, 4338, 4054, 3784, 3526, 3281, 3047, 2826, 2617, 2418, 2231, 2055, 1889, 1734, 1588,
	1453, 1327, 1210, 1102, 1003, 913, 831, 758, 693, 636, 586, 545, 511, 484, 466, 455,
};
void fft256_16_win(struct fft_sample_16 *samples, const u16 *win){
	u8 i=0;
	do { samples->r = ((s32)samples->r * (s32)(*win++)) >> 16; samples++;
	} while (++i);
}

struct fft_sample_16 samples16[SAMPLES];

int main(){
/*	s16 x = -32768;
	u16 y = 65535;
	printf("%d \r\n", (s32)x * (s32)y);
	exit(0);
*/

	for (u16 i=0; i<SAMPLES; i++) {
		double a = i * 2.0 * 3.14159265358979323846 / 256.0;
		//samples16[i].r = filt16_dcb_update( ((sin(a*20 + 3.14 / 10) + sin(a*50 + 3.14 / 8) + sin(a*90 + 3.14 / 5) + sin(a*100 + 3.14 / 3)) / 4.0) * 32767, &filt16_dcb );
	}

	fft256_16_win( &samples16[0], &fft256_16_win_blackman[0] );
	fft256_forw_16( &samples16[0] );

	fft_amp_16( &samples16[0], 256, &sqrt32_shift);

	for (u16 i=0; i<SAMPLES/2; i++) { printf("%d , %d \r\n", i, samples16[i].r); }

	return 0;
}

