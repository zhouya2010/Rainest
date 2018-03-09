/*
 * LCD.c
 *
 * Created: 2014/6/27 11:31:52
 *  Author: Administrator
 */ 

#include "LCD.h"
#include <string.h>
#include <ctype.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#ifndef F_CPU
#define F_CPU              12000000UL
#endif
#include <util/delay.h>

PROGMEM const  char char_a[5] = {0x20,0x54,0x54,0x54,0x78};
PROGMEM const  char char_b[5] = {0x7F,0x48,0x44,0x44,0x38};
PROGMEM const  char char_c[5] = {0x38,0x44,0x44,0x44,0x20};
PROGMEM const  char char_d[5] = {0x38,0x44,0x44,0x48,0x7E};
PROGMEM const  char char_e[5] = {0x38,0x54,0x54,0x54,0x18};
PROGMEM const  char char_f[5] = {0x10,0x7C,0x12,0x02,0x04};
PROGMEM const  char char_g[5] = {0x18,0xA4,0xA4,0xFC,0x00};
PROGMEM const  char char_h[5] = {0x7E,0x10,0x08,0x08,0x70};
PROGMEM const  char char_i[5] = {0x00,0x44,0x7D,0x40,0x00};
PROGMEM const  char char_j[5] = {0x00,0x20,0x40,0x3D,0x00};
PROGMEM const  char char_k[5] = {0x7E,0x10,0x28,0x44,0x00};
PROGMEM const  char char_l[5] = {0x00,0x42,0x7E,0x40,0x00};
PROGMEM const  char char_m[5] = {0x7C,0x04,0x18,0x04,0x7C};
PROGMEM const  char char_n[5] = {0x7C,0x08,0x04,0x04,0x78};
PROGMEM const  char char_o[5] = {0x38,0x44,0x44,0x44,0x38};
PROGMEM const  char char_p[5] = {0x7C,0x14,0x14,0x14,0x08};
PROGMEM const  char char_q[5] = {0x08,0x14,0x14,0x14,0x7C};
PROGMEM const  char char_r[5] = {0x7C,0x08,0x04,0x04,0x08};
PROGMEM const  char char_s[5] = {0x08,0x54,0x54,0x54,0x20};
PROGMEM const  char char_t[5] = {0x08,0x3E,0x48,0x40,0x20};
PROGMEM const  char char_u[5] = {0x3C,0x40,0x40,0x20,0x7C};
PROGMEM const  char char_v[5] = {0x1C,0x20,0x40,0x20,0x1C};
PROGMEM const  char char_w[5] = {0x3C,0x40,0x30,0x40,0x3C};
PROGMEM const  char char_x[5] = {0x44,0x28,0x10,0x28,0x44};
PROGMEM const  char char_y[5] = {0x0C,0x10,0x50,0x50,0x3C};
PROGMEM const  char char_z[5] = {0x44,0x64,0x54,0x4C,0x44};

PROGMEM const  char char_A[5] = {0x7C,0x12,0x12,0x12,0x7C};
PROGMEM const  char char_B[5] = {0x7E,0x4A,0x4A,0x4A,0x34};
PROGMEM const  char char_C[5] = {0x3C,0x42,0x42,0x42,0x24};
PROGMEM const  char char_D[5] = {0x7E,0x42,0x42,0x42,0x3C};
PROGMEM const  char char_E[5] = {0x7E,0x4A,0x4A,0x4A,0x42};
PROGMEM const  char char_F[5] = {0x7E,0x0A,0x0A,0x0A,0x02};
PROGMEM const  char char_G[5] = {0x3C,0x42,0x52,0x52,0x70};
PROGMEM const  char char_H[5] = {0x7E,0x08,0x08,0x08,0x7E};
PROGMEM const  char char_I[5] = {0x00,0x42,0x7E,0x42,0x00};
PROGMEM const  char char_J[5] = {0x20,0x40,0x42,0x3E,0x02};
PROGMEM const  char char_K[5] = {0x7E,0x08,0x14,0x22,0x40};
PROGMEM const  char char_L[5] = {0x7E,0x40,0x40,0x40,0x40};
PROGMEM const  char char_M[5] = {0x7E,0x04,0x18,0x04,0x7E};
PROGMEM const  char char_N[5] = {0x7E,0x04,0x08,0x10,0x7E};
PROGMEM const  char char_O[5] = {0x3C,0x42,0x42,0x42,0x3C};
PROGMEM const  char char_P[5] = {0x7E,0x12,0x12,0x12,0x0C};
PROGMEM const  char char_Q[5] = {0x3C,0x42,0x52,0x22,0x5C};
PROGMEM const  char char_R[5] = {0x7E,0x12,0x12,0x32,0x4C};
PROGMEM const  char char_S[5] = {0x24,0x4A,0x4A,0x4A,0x32};
PROGMEM const  char char_T[5] = {0x02,0x02,0x7E,0x02,0x02};
PROGMEM const  char char_U[5] = {0x3E,0x40,0x40,0x40,0x3E};
PROGMEM const  char char_V[5] = {0x1E,0x20,0x40,0x20,0x1E};
PROGMEM const  char char_W[5] = {0x3E,0x40,0x30,0x40,0x3E};
PROGMEM const  char char_X[5] = {0x62,0x14,0x08,0x14,0x62};
PROGMEM const  char char_Y[5] = {0x06,0x08,0x70,0x08,0x06};
PROGMEM const  char char_Z[5] = {0x42,0x62,0x52,0x4A,0x46};	

PROGMEM const  char num_0[5] = {0x3C,0x52,0x4A,0x46,0x3C};
PROGMEM const  char num_1[5] = {0x00,0x44,0x7E,0x40,0x00};
PROGMEM const  char num_2[5] = {0x44,0x62,0x52,0x4A,0x44};
PROGMEM const  char num_3[5] = {0x22,0x42,0x4A,0x4A,0x36};
PROGMEM const  char num_4[5] = {0x30,0x28,0x24,0x7E,0x20};
PROGMEM const  char num_5[5] = {0x2E,0x4A,0x4A,0x4A,0x32};
PROGMEM const  char num_6[5] = {0x3C,0x4A,0x4A,0x4A,0x30};
PROGMEM const  char num_7[5] = {0x02,0x02,0x72,0x0A,0x06};
PROGMEM const  char num_8[5] = {0x34,0x4A,0x4A,0x4A,0x34};
PROGMEM const  char num_9[5] = {0x0C,0x52,0x52,0x52,0x3C};

PROGMEM const  char sign_line[5] = {0x18,0x18,0x18,0x18,0x18};	
PROGMEM const  char sign_colon[5] = {0x00,0x00,0x28,0x00,0x00};
PROGMEM const  char sign_colon2[5] = {0xFF,0x00,0x00,0x28,0x00};//  |:
PROGMEM const  char sign_below[5] = {0x40,0x40,0x40,0x40,0x40};
PROGMEM const  char sign_point[5] = {0x00,0x60,0x60,0x00,0x00};
PROGMEM const  char sign_degree[5] = {0x00,0x00,0x07,0x05,0x07};
PROGMEM const  char sign_well[5] = {0x14,0x3E,0x14,0x3E,0x14};
PROGMEM const  char sign_sprit[5] = {0x40,0x20,0x10,0x08,0x04};
PROGMEM const  char sign_excl[5] = {0x00,0x00,0x5F,0x00,0x00};
// PROGMEM const  char sign_and[5] = {0x6C,0x92,0xB2,0x6C,0x90};
PROGMEM const  char sign_and[5] = {0x70,0x8E,0x9A,0xE6,0xB0};
PROGMEM const  char sign_ques[5] = {0x04,0x02,0xA2,0x12,0x0C};
PROGMEM const  char sign_equal[5] = {0x00,0x28,0x28,0x28,0x28};
PROGMEM const  char sign_vertical[5] = {0xFF,0x00,0x00,0x00,0x00};
PROGMEM const  char sign_brackets_left[5] = {0x00,0x00,0x3C,0x42,0x00};
PROGMEM const  char sign_brackets_right[5] = {0x00,0x42,0x3C,0x00,0x00};
		
PROGMEM const  char sign_percent[6]  = {0x47,0x25,0x17,0xE8,0xA4,0xE2};
PROGMEM const  char sign_arrow[6] =	{0x18,0x18,0xFF,0x7E,0x3C,0x18};
PROGMEM const  char sign_plus[6] = {0x18,0x18,0x7E,0x7E,0x18,0x18};
PROGMEM const  char sign_sub[6] = {0x18,0x18,0x18,0x18,0x18,0x18};
PROGMEM const  char sign_net[5] = {0x60,0x00,0x70,0x00,0x7C};
PROGMEM const  char sign_disnet[5] = {0xBA,0xD6,0xEE,0xD6,0xBA};//x
PROGMEM const  char sign_uper_arrow[6] = {0x08,0x0C,0x3E,0x3E,0x0C,0x08};//^
PROGMEM const  char sign_temp_C[] = {0x03,0x3B,0x44,0x44,0x44,0x20};// 'c
PROGMEM const  char sign_temp_F[] = {0x01,0x7E,0x0A,0x0A,0x02};// 'F
PROGMEM const  char sign_comma[] = {0x80,0xE0,0x60,0x00,0x00}; //,
PROGMEM const  char sign_small_arrow[] = {0x10,0x10,0x54,0x28,0x10};//->
PROGMEM const  char sign_slash[] = {0x40,0x20,0x10,0x08,0x04};//  /
			
PROGMEM const  char char_null[] = {0x00,0x00,0x00,0x00,0x00,0x00};

/***宋体 加粗 小四****/
PROGMEM const  char  num916_1[] = {0x00,0x10,0x10,0xF8,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x3F,0x20,0x20,0x00,0x00};
PROGMEM const  char  num916_2[] = {0x00,0x70,0x78,0x08,0x08,0x88,0xF8,0x70,0x00,0x00,0x30,0x38,0x2C,0x26,0x23,0x31,0x30,0x00};
PROGMEM const  char  num916_3[] = {0x00,0x30,0x38,0x88,0x88,0xC8,0x78,0x30,0x00,0x00,0x18,0x38,0x20,0x20,0x31,0x1F,0x0E,0x00};
PROGMEM const  char  num916_4[] = {0x00,0x00,0xC0,0xE0,0x30,0xF8,0xF8,0x00,0x00,0x00,0x07,0x07,0x24,0x24,0x3F,0x3F,0x24,0x00};
PROGMEM const  char  num916_5[] = {0x00,0xF8,0xF8,0x88,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x39,0x21,0x20,0x31,0x1F,0x0E,0x00};
PROGMEM const  char  num916_6[] = {0x00,0xE0,0xF0,0x98,0x88,0x98,0x18,0x00,0x00,0x00,0x0F,0x1F,0x31,0x20,0x31,0x1F,0x0E,0x00};
PROGMEM const  char  num916_7[] = {0x00,0x38,0x38,0x08,0xC8,0xF8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x3F,0x00,0x00,0x00,0x00};
PROGMEM const  char  num916_8[] = {0x00,0x70,0xF8,0x88,0x08,0x88,0xF8,0x70,0x00,0x00,0x1C,0x3E,0x23,0x21,0x23,0x3E,0x1C,0x00};
PROGMEM const  char  num916_9[] = {0x00,0xE0,0xF0,0x18,0x08,0x18,0xF0,0xE0,0x00,0x00,0x00,0x31,0x33,0x22,0x33,0x1F,0x0F,0x00};
PROGMEM const  char  num916_0[] = {0x00,0xE0,0xF0,0x18,0x08,0x18,0xF0,0xE0,0x00,0x00,0x0F,0x1F,0x30,0x20,0x30,0x1F,0x0F,0x00};

PROGMEM const  char char916_a[] = {0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x19,0x3D,0x26,0x22,0x22,0x3F,0x3F,0x20};
PROGMEM const  char char916_c[] = {0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x0E,0x1F,0x31,0x20,0x20,0x31,0x11,0x00};
PROGMEM const  char char916_e[] = {0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x3F,0x22,0x22,0x22,0x33,0x13,0x00};
PROGMEM const  char char916_l[] = {0x00,0x08,0x08,0xF8,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x3F,0x20,0x20,0x00,0x00};
PROGMEM const  char char916_m[] = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x20,0x3F,0x3F,0x20,0x3F,0x3F,0x20,0x3F,0x3F};
PROGMEM const  char char916_n[] = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x3F,0x21,0x00,0x20,0x3F,0x3F,0x20};	
PROGMEM const  char char916_p[] = {0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x80,0xFF,0xFF,0xA1,0x20,0x31,0x1F,0x0E,0x00};
PROGMEM const  char char916_r[] = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x20,0x20,0x3F,0x3F,0x21,0x20,0x01,0x01,0x00};
PROGMEM const  char char916_s[] = {0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x33,0x37,0x24,0x24,0x24,0x3D,0x19,0x00};
PROGMEM const  char char916_t[] = {0x00,0x80,0x80,0xE0,0xE0,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x1F,0x3F,0x20,0x20,0x00,0x00};
PROGMEM const  char char916_u[] = {0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x1F,0x3F,0x20,0x20,0x30,0x3F,0x3F,0x20};

PROGMEM const  char char916_A[] = {0x00,0x00,0xC0,0xF8,0xF8,0xE0,0x00,0x00,0x00,0x20,0x3C,0x3F,0x23,0x02,0x27,0x3F,0x38,0x20};
PROGMEM const  char char916_P[] = {0x08,0xF8,0xF8,0x08,0x08,0x08,0xF8,0xF0,0x00,0x20,0x3F,0x3F,0x21,0x01,0x01,0x01,0x00,0x00};
//PROGMEM const  char char916_M[] = {0xF8,0xF8,0x70,0xC0,0x00,0xC0,0x70,0xF8,0xF8,0x3F,0x3F,0x00,0x01,0x07,0x01,0x00,0x3F,0x3F}; //16.9.21
PROGMEM const  char char916_F[] = {0x08,0xF8,0xF8,0x88,0xE8,0xE8,0x18,0x10,0x00,0x20,0x3F,0x3F,0x20,0x03,0x03,0x00,0x00,0x00};
PROGMEM const  char  char916_O[] = {0xE0,0xF0,0x18,0x08,0x08,0x18,0xF0,0xE0,0x00,0x0F,0x1F,0x30,0x20,0x20,0x30,0x1F,0x0F,0x00};
PROGMEM const  char char916_S[] = {0x00,0x70,0xF8,0x88,0x08,0x08,0x38,0x38,0x00,0x00,0x38,0x38,0x21,0x21,0x23,0x3E,0x1C,0x00};
PROGMEM const  char sign916_colon[] = {0x00,0x00,0x00,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x30,0x00,0x00,0x00};

PROGMEM const  char char916_null[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

/************************************************************************/
/* 函数名：char_swich*/
/* 功能：把ASCII码转换为对应的点阵数组*/
/*参数：p：ASCII码值*/
/*返回值：对应的数组指针*/
/************************************************************************/	
PGM_P char_swich(const char *p)
{
	PGM_P s;
	if (islower(*p))
	{
		switch(*p)
		{
			case 'a':		s = char_a; break;
			case 'b':		s = char_b; break;
			case 'c':		s = char_c; break;
			case 'd':		s = char_d; break;
			case 'e':		s = char_e; break;
			case 'f':		s = char_f; break;
			case 'g':		s = char_g; break;
			case 'h':		s = char_h; break;
			case 'i':		s = char_i; break;
			case 'j':		s = char_j; break;
			case 'k':		s = char_k; break;
			case 'l':		s = char_l; break;
			case 'm':		s = char_m; break;
			case 'n':		s = char_n; break;
			case 'o':		s = char_o; break;
			case 'p':		s = char_p; break;
			case 'q':		s = char_q; break;
			case 'r':		s = char_r; break;
			case 's':		s = char_s; break;
			case 't':		s = char_t; break;
			case 'u':		s = char_u; break;
			case 'v':		s = char_v; break;
			case 'w':		s = char_w; break;
			case 'x':		s = char_x; break;
			case 'y':		s = char_y; break;
			case 'z':		s = char_z; break;
			default:  s =	char_null; break;
		}
		return s;
	}
	if (isupper(*p))
	{
		switch(*p)
		{
			case 'A':		s = char_A; break;
			case 'B':		s = char_B; break;
			case 'C':		s = char_C; break;
			case 'D':		s = char_D; break;
			case 'E':		s = char_E; break;
			case 'F':		s = char_F; break;
			case 'G':		s = char_G; break;
			case 'H':		s = char_H; break;
			case 'I':		s = char_I; break;
			case 'J':		s = char_J; break;
			case 'K':		s = char_K; break;
			case 'L':		s = char_L; break;
			case 'M':		s = char_M; break;
			case 'N':		s = char_N; break;
			case 'O':		s = char_O; break;
			case 'P':		s = char_P; break;
			case 'Q':		s = char_Q; break;
			case 'R':		s = char_R; break;
			case 'S':		s = char_S; break;
			case 'T':		s = char_T; break;
			case 'U':		s = char_U; break;
			case 'V':		s = char_V; break;
			case 'W':		s = char_W; break;
			case 'X':		s = char_X; break;
			case 'Y':		s = char_Y; break;
			case 'Z':		s = char_Z; break;
			default:		s =	char_null; break;
		}
		return s;
	}
	if (isdigit(*p))
	{
		switch(*p)
		{
			case '1':		s = num_1; break;
			case '2':		s = num_2; break;
			case '3':		s = num_3; break;
			case '4':		s = num_4; break;
			case '5':		s = num_5; break;
			case '6':		s = num_6; break;
			case '7':		s = num_7; break;
			case '8':		s = num_8; break;
			case '9':		s = num_9; break;
			case '0':		s = num_0; break;
			default:		s =	char_null; break;
		}
		return s;
	}
	else
	{
		if('!' == *p) { s = sign_excl; return s;}
		if('%' == *p) { s = sign_percent; return s;}
		if('&' == *p) { s = sign_and; return s;}
		if('*' == *p) { s = sign_disnet; return s;}	
		if('+' == *p) {s = sign_plus; return s;}
		if('.' == *p) {s = sign_point; return s;}
		if('-' == *p) {s = sign_line; return s;}
		if(':' == *p) {s = sign_colon; return s;}
		if(';' == *p) {s = sign_colon2; return s;}
		if('>' == *p) {s = sign_arrow; return s;}
		if('@' == *p) {s = sign_net; return s;}
		if('|' == *p) {s = sign_vertical; return s;}
		if('^' == *p) {s = sign_uper_arrow; return s;}
		if ('`' == *p) {s = sign_temp_F; return s;}
		if('#' == *p) {s = sign_temp_C; return s;}
		if('?' == *p) {s = sign_ques; return s;}	
		if('(' == *p) {s = sign_brackets_left; return s;}
		if(')' == *p) {s = sign_brackets_right; return s;}
		if(',' == *p) {s = sign_comma; return s;}
		if ('~' == *p) {s = sign_small_arrow; return s;}
		if('/' == *p) {s = sign_slash; return s;}
		else {s = char_null; return s;}
	}
}


PGM_P char_swich916(const char *p)
{
	PGM_P s;
	if (isdigit(*p))
	{
		switch(*p)
		{
			case '1':		s = num916_1; break;
			case '2':		s = num916_2; break;
			case '3':		s = num916_3; break;
			case '4':		s = num916_4; break;
			case '5':		s = num916_5; break;
			case '6':		s = num916_6; break;
			case '7':		s = num916_7; break;
			case '8':		s = num916_8; break;
			case '9':		s = num916_9; break;
			case '0':		s = num916_0; break;
			default:		s =	char916_null; break;
		}
		return s;
	}
	
	if (islower(*p))
	{
		switch(*p)
		{
			case 'a':		s = char916_a; break;
			case 'c':		s =  char916_c; break;
			case 'e':		s = char916_e; break;
			case 'l':		s = char916_l; break;
			case 'm':		s = char916_m; break;
			case 'n':		s = char916_n; break;
			case 'p':		s = char916_p; break;
			case 'r':		s = char916_r; break;
			case 's':		s = char916_s; break;
			case 't':		s = char916_t; break;
			case 'u':		s = char916_u; break;
			default:  s =	char916_null; break;
		}
		return s;
	}
	
	if (isupper(*p))
	{
		switch(*p)
		{
			case 'A':		s = char916_A; break;
			case 'P':		s = char916_P; break;
//			case 'M':		s = char916_M; break;
			case 'F':		s = char916_F; break;
			case 'O':		s = char916_O;	break;
			case 'S':		s = char916_S;	break;
			default:		s =	char916_null; break;
		}
		return s;
	}
	else
	{
		if(':' == *p) {s = sign916_colon; return s;}
		else{ s = char916_null; return s;}
	}
}

/************************************************************************/
/* 函数名：LCDIoinit*/
/* 功能：LCD IO引脚初始化*/
/*参数：无*/
/*返回值：无*/
/************************************************************************/
void LCDIoinit(void)
{
	DDRB |= (1 << CS);
	PORTB &= ~(1 << CS);
	DDRA |= (1 << SID)|(1 << SCK)|(1 << RS)|(1 << RES);
	PORTA &= ~((1 << RS)|(1 << SID)|(1<<RES));
	PORTA |= (1 << SCK);
}

/************************************************************************/
/* 函数名：DataSend*/
/* 功能：发送数据给LCD*/
/*参数：data：发送的数据*/
/*返回值：无*/
/************************************************************************/
 void DataSend(unsigned char data)
 {
	 unsigned char i;
	 SCK_H();
	 _delay_us(2);
	 for(i=0;i<8;i++)
	 {
		 SCK_L();
		 _delay_us(2);
		 if(data & 0x80)
		 {
			 SID_H();
		 }
		 else
		 {
			 SID_L();
		 }
		 SCK_H();
		 _delay_us(2);
		 data=data<<1;
	 }
 }
 
 /************************************************************************/
 /* 函数名：WriteCommand*/
 /* 功能：LCD写命令*/
 /*参数：data：命令值*/
 /*返回值：无*/
 /************************************************************************/
 void WriteCommand( uchar data )
 {
	 CS_L();
	 RS_L();	//Command
	 DataSend(data);
	 _delay_us(5);
	 RS_H();	//Command
 }

/************************************************************************/
/* 函数名：WriteData*/
/* 功能：LCD写数据*/
/*参数：data：数据*/
/*返回值：无*/
/************************************************************************/
 void WriteData( uchar data )
 {
	 CS_L();
	 RS_H();	//data
	 DataSend(data);
	 _delay_us(5);
 }
 
 /************************************************************************/
 /* 函数名：LCD_clear*/
 /* 功能：清屏*/
 /*参数：无*/
 /*返回值：无*/
 /************************************************************************/
 void LCD_clear(void)
 {
	 uchar i,j;
	 for(i=0;i<8;i++)
	 {
		 WriteCommand(0xB0 | i);  //Set Page Address
		 WriteCommand(0x10); //Set Column Address = 0
		 WriteCommand(0x00); //Colum from S1 -> S128 auto add
		 for(j=0;j<132;j++)
		 {
			 WriteData( 0 );
		 }
	 }
 }
 
 /************************************************************************/
 /* 函数名：LCD_init*/
 /* 功能：LCD初始化*/
 /*参数：无*/
 /*返回值：无*/
 /************************************************************************/
 void LCD_init(void)
 {
	 LCDIoinit();
	 _delay_us(5);
	 RES_L();
	 _delay_us(50);
	 RES_H();
	 _delay_us(50);
	 WriteCommand(0xAE);//Display OFF
	 WriteCommand(0xA2); /*1/9 偏压比（bias）*/
	 
	 #if DISPALY_MOD
	 	WriteCommand(0xA1); //SEG Direction
	 	WriteCommand(0xC0); //COM Direction
	#else
	 	WriteCommand(0xA0); /*列扫描顺序：从左到右*/
	 	WriteCommand(0xC8); /*行扫描顺序：从上到下*/
	#endif
	 WriteCommand(0x20);
	 WriteCommand(0x24); /*粗调对比度，可设置范围 0x20～0x27*/
	 WriteCommand(0x81); /*微调对比度*/
	 WriteCommand(0x24); /*0x1a,微调对比度的值，可设置范围 0x00～0x3f 1f*/
	 
	 WriteCommand(0x2C); /*升压步聚 1*/
	 WriteCommand(0x2E); /*升压步聚 2*/
	 WriteCommand(0x2F); /*升压步聚 3*/
	 
	 WriteCommand(0xA6);//Normal Display (not reverse dispplay)
	 WriteCommand(0xA4);//All Pixel ON
	 
	 _delay_us(5);
	 WriteCommand(0xB0);	//Set Page Address = 0
	 WriteCommand(0x10);  //Set Column Address 4 higher bits = 0
	 WriteCommand(0x01);//Set Column Address 4 lower bits = 1 , from IC SEG1 -> SEG128
	 LCD_clear();
	 WriteCommand(0xAF); /*开显示*/
 }
 
 /************************************************************************/
 /* 函数名：LCD_show_string*/
 /* 功能：显示字符串*/
 /*参数：page：行数(0~7);	 col：列数(0~21) Data：显示的字符串*/
 /*返回值：无*/
 /************************************************************************/
 void LCD_show_string(uchar page, uchar col, const char *Data)
 {
	 const char *p1 = Data;
	 
	 while('\0' != *p1 )
	 {
		 LCD_show_char(page,col,*p1);
		 p1++;
		 col++;
		 if (col > 20)
		 {
			 page++;
			 col = 0;
		 }
	 }
 }
 
 /************************************************************************/
 /* 函数名：LCD_show_stringR*/
 /* 功能：显示字符串（反显）*/
 /*参数：page：行数(0~7);	 col：列数(0~21) Data：显示的字符串*/
 /*返回值：无*/
 /************************************************************************/
 void LCD_show_stringR(uchar page, uchar col, const char *Data)
 {
	 PGM_P p1 = NULL;
	 const char *p = Data;
	 unsigned char i = 0;
	 
	 i =col * 6;	//字符宽度为6，一行最多显示21个字符
	 #if DISPALY_MOD
	 i +=4;
	 #endif
	 
	 WriteCommand(0xB0 | page);  //Set Page Address
	 WriteCommand(0x10 | ((i>>4) & 0x0F)); //Set Column Address = 0
	 WriteCommand(0x0F & i); //Colum from S1 -> S128 auto add

	 while('\0' != *p)
	 {
		 p1 = char_swich(p);
		 WriteData(0xff);
		 for(i=0; i<5; i++)
		 {
			 WriteData(~pgm_read_byte(&p1[i]));//读取数据写入LCD
		 }
		
		 p++;
		 col++;
		 if ((col > 20) && ('\0' != *p))
		 {
			 WriteData(0xff);
			 
			 col = 0;
			 #if DISPALY_MOD
				col +=4;
			 #endif
			 
			 page++;
			 if(page > 7) break;
			 WriteCommand(0xB0 | page);  //Set Page Address
			 WriteCommand(0x10 | ((col>>4) & 0x0F)); //Set Column Address = 0
			 WriteCommand(0x0F & col); //Colum from S1 -> S128 auto add
		 }
	 }
	 WriteData(0xff);
 }
 
 /************************************************************************/
 /* 函数名：LCD_show_number*/
 /* 功能：显示数字*/
 /*参数：page：行数(0~7);	 col：列数(0~21) Data：显示的数字*/
 /*返回值：无*/
 /************************************************************************/
 void LCD_show_number( uchar page, uchar col, const uint Data )
 {
 	char temp[6] = {0};
 	utoa(Data,temp,10);
 	LCD_show_string(page,col,temp);
 }
  
  /************************************************************************/
  /* 函数名：LCD_show_numberR*/
  /* 功能：显示数字（反显）*/
  /*参数：page：行数(0~7);	 col：列数(0~21) Data：显示的数字*/
  /*返回值：无*/
  /************************************************************************/
  void LCD_show_numberR( uchar page, uchar col, const uint Data )
  {
	  char temp[6] = {0};
	  utoa(Data,temp,10);
	  LCD_show_stringR(page,col,temp);
// 	  WriteData(0xff);
  }
  
  /************************************************************************/
  /* 函数名：LCD_show_strP*/
  /* 功能：显示字符串，该字符串保存于代码段中*/
  /*参数：page：行数(0~7);	 col：列数(0~21) Data：显示的字符串*/
  /*返回值：无*/
  /************************************************************************/
 void LCD_show_strP(uchar page, uchar col, const char *Data)
 {
	 PGM_P p1 = NULL;
	 char *p = NULL;
	 unsigned char i = 0;
	 unsigned char j = 0;
	 
	p = (char *) malloc(sizeof(char)*strlen_P(Data)+1);
	if(NULL == p) return;
	
	strcpy_P(p,Data);

	 i =col * 6;	//字符宽度为6，一行最多显示21个字符
	#if DISPALY_MOD
	i +=4;
	#endif

	WriteCommand(0xB0 | page);  //Set Page Address
	WriteCommand(0x10 | ((i>>4) & 0x0F)); //Set Column Address = 0
	WriteCommand(0x0F & i); //Colum from S1 -> S128 auto add

	 while('\0' != p[j])
	 {
		 p1 = char_swich(&p[j]);
		 for(i=0; i<5; i++)
		 {
			 WriteData(pgm_read_byte(&p1[i]));//读取数据写入LCD
		 }
		 WriteData(0);
		 j++;
		col++;
		if (col > 20)
		{
			col = 0;
			#if DISPALY_MOD
			col +=4;
			#endif

			page++;
			if(page > 7) break;
			WriteCommand(0xB0 | page);  //Set Page Address
			WriteCommand(0x10 | ((col>>4) & 0x0F)); //Set Column Address = 0
			WriteCommand(0x0F & col); //Colum from S1 -> S128 auto add
		}
	 }
	 free(p);
	 p = NULL;
 }
 
 /************************************************************************/
 /* 函数名：LCD_show_char*/
 /* 功能：显示一个字符*/
 /*参数：page：行数(0~7);	 col：列数(0~21) Data：显示的字符*/
 /*返回值：无*/
 /************************************************************************/
 void LCD_show_char( uchar page, uchar col, const char Data )
 {
 	unsigned char i = 0;
 	PGM_P p1 = NULL;
	 
 	i =col * 6;	//字符宽度为6，一行最多显示21个字符
 	#if DISPALY_MOD
 	i +=4;
 	#endif
 	
 	WriteCommand(0xB0 | page);  //Set Page Address
 	WriteCommand(0x10 | ((i>>4) & 0x0F)); //Set Column Address = 0
 	WriteCommand(0x0F & i); //Colum from S1 -> S128 auto add
	 
	p1 = char_swich(&Data);
	for(i=0; i<5; i++)
	{
		WriteData(pgm_read_byte(&p1[i]));//读取数据写入LCD
	}
	WriteData(0);	
 }

/************************************************************************/
/* 函数名：LCD_show_charR*/
/* 功能：反显一个字符*/
/*参数：page：行数(0~7);	 col：列数(0~21) Data：显示的字符*/
/*返回值：无*/
/************************************************************************/
void LCD_show_charR( uchar page, uchar col, const char Data )
{
	 unsigned char i = 0;
	 PGM_P p1 = NULL;
	 
	 i =col * 6;	//字符宽度为6，一行最多显示21个字符
	 #if DISPALY_MOD
	 i +=4;
	 #endif
	 
	 WriteCommand(0xB0 | page);  //Set Page Address
	 WriteCommand(0x10 | ((i>>4) & 0x0F)); //Set Column Address = 0
	 WriteCommand(0x0F & i); //Colum from S1 -> S128 auto add
	 
	 p1 = char_swich(&Data);
	 for(i=0; i<5; i++)
	 {
		 WriteData(~pgm_read_byte(&p1[i]));//读取数据写入LCD
	 }
// 	 WriteData(0xff);
}

/************************************************************************/
/* 函数名：LCD_show_icon*/
/* 功能：反显一个字符*/
/*参数：page：行数(0~7);	 col：列数(0~21) Data：显示的字符*/
/*返回值：无*/
/************************************************************************/
void LCD_show_icon( uchar page, uchar col, const char Data )
{
	unsigned char i = 0;
	PGM_P p1 = NULL;
	
	i =col * 6;	//字符宽度为6，一行最多显示21个字符
	#if DISPALY_MOD
	i +=4;
	#endif

	WriteCommand(0xB0 | page);  //Set Page Address
	WriteCommand(0x10 | ((i>>4) & 0x0F)); //Set Column Address = 0
	WriteCommand(0x0F & i); //Colum from S1 -> S128 auto add
	
	p1 = char_swich(&Data);
	for(i=0; i<6; i++)
	{
		WriteData(pgm_read_byte(&p1[i]));//读取数据写入LCD
	}
}
 /************************************************************************/
 /* 函数名：LCD_show_number2*/
 /* 功能：显示数字*/
 /*参数：page：行数(0~7)	  col：列数(0~21) 
				Data：显示的数字   len：显示位数*/
 /*返回值：无*/
 /************************************************************************/

void LCD_show_number2( uchar page, uchar col, const uint Data, uchar len )
{
	char temp[6] = {0};
	utoa(Data,temp,10);
	while(len > strlen(temp))
	{
		strcat(temp," ");
	}
	LCD_show_string(page,col,temp);

}

void LCD_show_char916( uchar page, uchar col, const char Data )
{
	unsigned char i = 0;
	unsigned char j = 0;
	PGM_P p1 = NULL;
	
	i =col * 9;	//字符宽度为6，一行最多显示21个字符
	#if DISPALY_MOD
	i +=4;
	#endif
	
	WriteCommand(0xB0 | page);  //Set Page Address
	WriteCommand(0x10 | ((i>>4) & 0x0F)); //Set Column Address = 0
	WriteCommand(0x0F & i); //Colum from S1 -> S128 auto add
	
	p1 = char_swich916(&Data);
	for(j=0,i=0; j<18; j++)
	{
		i++;
		WriteData(pgm_read_byte(&p1[j]));//读取数据写入LCD
	
		if (i >= 9)
		{
			i =col * 9;
			#if DISPALY_MOD
			i +=4;
			#endif
			page++;				
			WriteCommand(0xB0 | page);  //Set Page Address
			WriteCommand(0x10 | ((i>>4) & 0x0F)); //Set Column Address = 0
			WriteCommand(0x0F & i); //Colum from S1 -> S128 auto add
			i = 0;
		} 
	}
}

void LCD_show_str916( uchar page, uchar col, const char *Data )
{
	const char *p1 = Data;
	
	while('\0' != *p1 )
	{
		LCD_show_char916(page,col,*p1);
		p1++;
		col++;
	}
}


void LCD_show_num916( uchar page, uchar col, unsigned int num )
{
	char temp[6] = {0};
	utoa(num,temp,10);
	LCD_show_str916(page,col,temp);
}
