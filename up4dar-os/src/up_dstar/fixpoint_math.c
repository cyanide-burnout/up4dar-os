/*

Copyright (C) 2012   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (C) 2012   Denis Bederov, DL3OCK (denis.bederov@gmx.de)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
/*
 * trigono.c
 *
 * Created: 27.05.2012 02:56:00
 *  Author: mdirska
 */ 


#include "FreeRTOS.h"

#include "fixpoint_math.h"


static const short sin_tab[360] = {
     0,   174,   348,   523,   697,   871,  1045,  1218,  1391,  1564,
  1736,  1908,  2079,  2249,  2419,  2588,  2756,  2923,  3090,  3255,
  3420,  3583,  3746,  3907,  4067,  4226,  4383,  4539,  4694,  4848,
  4999,  5150,  5299,  5446,  5591,  5735,  5877,  6018,  6156,  6293,
  6427,  6560,  6691,  6819,  6946,  7071,  7193,  7313,  7431,  7547,
  7660,  7771,  7880,  7986,  8090,  8191,  8290,  8386,  8480,  8571,
  8660,  8746,  8829,  8910,  8987,  9063,  9135,  9205,  9271,  9335,
  9396,  9455,  9510,  9563,  9612,  9659,  9702,  9743,  9781,  9816,
  9848,  9876,  9902,  9925,  9945,  9961,  9975,  9986,  9993,  9998,
 10000,  9998,  9993,  9986,  9975,  9961,  9945,  9925,  9902,  9876,
  9848,  9816,  9781,  9743,  9702,  9659,  9612,  9563,  9510,  9455,
  9396,  9335,  9271,  9205,  9135,  9063,  8987,  8910,  8829,  8746,
  8660,  8571,  8480,  8386,  8290,  8191,  8090,  7986,  7880,  7771,
  7660,  7547,  7431,  7313,  7193,  7071,  6946,  6819,  6691,  6560,
  6427,  6293,  6156,  6018,  5877,  5735,  5591,  5446,  5299,  5150,
  4999,  4848,  4694,  4539,  4383,  4226,  4067,  3907,  3746,  3583,
  3420,  3255,  3090,  2923,  2756,  2588,  2419,  2249,  2079,  1908,
  1736,  1564,  1391,  1218,  1045,   871,   697,   523,   348,   174,
     0,  -174,  -348,  -523,  -697,  -871, -1045, -1218, -1391, -1564,
 -1736, -1908, -2079, -2249, -2419, -2588, -2756, -2923, -3090, -3255,
 -3420, -3583, -3746, -3907, -4067, -4226, -4383, -4539, -4694, -4848,
 -5000, -5150, -5299, -5446, -5591, -5735, -5877, -6018, -6156, -6293,
 -6427, -6560, -6691, -6819, -6946, -7071, -7193, -7313, -7431, -7547,
 -7660, -7771, -7880, -7986, -8090, -8191, -8290, -8386, -8480, -8571,
 -8660, -8746, -8829, -8910, -8987, -9063, -9135, -9205, -9271, -9335,
 -9396, -9455, -9510, -9563, -9612, -9659, -9702, -9743, -9781, -9816,
 -9848, -9876, -9902, -9925, -9945, -9961, -9975, -9986, -9993, -9998,
-10000, -9998, -9993, -9986, -9975, -9961, -9945, -9925, -9902, -9876,
 -9848, -9816, -9781, -9743, -9702, -9659, -9612, -9563, -9510, -9455,
 -9396, -9335, -9271, -9205, -9135, -9063, -8987, -8910, -8829, -8746,
 -8660, -8571, -8480, -8386, -8290, -8191, -8090, -7986, -7880, -7771,
 -7660, -7547, -7431, -7313, -7193, -7071, -6946, -6819, -6691, -6560,
 -6427, -6293, -6156, -6018, -5877, -5735, -5591, -5446, -5299, -5150,
 -5000, -4848, -4694, -4539, -4383, -4226, -4067, -3907, -3746, -3583,
 -3420, -3255, -3090, -2923, -2756, -2588, -2419, -2249, -2079, -1908,
 -1736, -1564, -1391, -1218, -1045,  -871,  -697,  -523,  -348,  -174
};



int fixpoint_sin (int degree)
{
	int d = degree;
	
	while (d >= 360)
	{
		d -= 360;
	}
	
	while (d < 0)
	{
		d += 360;
	}
	
	return sin_tab[d];
}


int fixpoint_cos (int degree)
{
	return fixpoint_sin(degree + 90);	
}


/*  Code von DL1BFF:

static const uint8_t cB_tab[100] = {

   0,   0,  30,  47,  60,  69,  77,  84,  90,  95,
 100, 104, 107, 111, 114, 117, 120, 123, 125, 127,
 130, 132, 134, 136, 138, 139, 141, 143, 144, 146,
 147, 149, 150, 151, 153, 154, 155, 156, 157, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169,
 169, 170, 171, 172, 173, 174, 174, 175, 176, 177,
 177, 178, 179, 179, 180, 181, 181, 182, 183, 183,
 184, 185, 185, 186, 186, 187, 188, 188, 189, 189,
 190, 190, 191, 191, 192, 192, 193, 193, 194, 194,
 195, 195, 196, 196, 197, 197, 198, 198, 199, 199,
};


int fixpoint_centiBel (uint32_t value)
{
    uint32_t v = value;
    int centiBel = 0;

    while (v >= 100)
    {
        centiBel += 100;
        v /= 10;
    }

    return centiBel + cB_tab[v];
}

*/


// Code von DL3OCK:

static const int log_Tabelle[9] = {50504453, 6944540, 725006, 72826, 7286, 729, 73, 7, 1};


	
int fixpoint_milliBel (int x)
{
	// Das ist die maximal moegliche Quadratsumme eines
	// 160 Abtaswerte langen Blocks
	// int Quadratsumme = (((1<<15)*(1<<15))>>7)*160;
	// printf("Quadratsumme=%d\r\n", Quadratsumme);
	
	// Eingangswert der Berechnung
	// int x = Quadratsumme>>3;		//<<== Hier kann man ein wenig mit dem Algorithmus spielen.
	
	int x0 = 1;
	// =======================================================================================
	// 1st Iteration order
	unsigned short log_Tabelle_0 = 0;
	int            x0_next       = x0*2;
	while (x0_next<=x && x0_next>x0){
		x0             = x0_next;
		x0_next        = x0*2;
		log_Tabelle_0 += 1;
	}
	// Debug-Ausgabe des Zwischenergebnisses
	//printf("log_Tabelle_0=%d\r\n", log_Tabelle_0);
	
	// =======================================================================================
	// 2nd Iteration order
	unsigned short log_Tabelle_1 = 0;
	if (x0>(1<<16))	x0_next = (x0>>14)*18022; else x0_next = (x0*18022)>>14;
	while (x0_next<=x && x0_next>x0){
		x0 = x0_next;
		if (x0>(1<<16))	x0_next = (x0>>14)*18022; else x0_next = (x0*18022)>>14;
		log_Tabelle_1 += 1;
		//printf("%d  ", x0_next);
	}
	// Debug-Ausgabe des Zwischenergebnisses
	//printf("log_Tabelle_1=%d\r\n", log_Tabelle_1);
	
	// =======================================================================================
	// 3rd Iteration order
	unsigned short log_Tabelle_2 = 0;
	if (x0>(1<<16))	x0_next = (x0>>14)*16548; else x0_next = (x0*16548)>>14;
	while (x0_next<=x && x0_next>x0){
		x0 = x0_next;
		if (x0>(1<<16))	x0_next = (x0>>14)*16548; else x0_next = (x0*16548)>>14;
		log_Tabelle_2 += 1;
	}
	// Debug-Ausgabe des Zwischenergebnisses
	//printf("log_Tabelle_2=%d\r\n", log_Tabelle_2);
	
	// =======================================================================================
	// 4th Iteration order
	unsigned short log_Tabelle_3 = 0;
	
	if (x0>(1<<16))	x0_next = (x0>>14)*16400; else x0_next = (x0*16400)>>14;
	while (x0_next<=x && x0_next>x0){
		x0 = x0_next;
		if (x0>(1<<16))	x0_next = (x0>>14)*16400; else x0_next = (x0*16400)>>14;
		log_Tabelle_3 += 1;
	}
	// Debug-Ausgabe des Zwischenergebnisses
	//printf("log_Tabelle_3=%d\r\n", log_Tabelle_3);
	
	// =======================================================================================

	int Pegel_mB = -1531392380 +
					log_Tabelle_0*log_Tabelle[0] +
					log_Tabelle_1*log_Tabelle[1] +
					log_Tabelle_2*log_Tabelle[2] +
					log_Tabelle_3*log_Tabelle[3];
	
	Pegel_mB >>= 17;
	Pegel_mB  *= 100;
	Pegel_mB >>= 7;

	//printf("\r\nPegel_dB    =%.2f\r\n", 1.0*Pegel_mB/100);
	//printf("Pegel exakt =%.2f\r\n", 10*log10(1.0*x/(160*8388608)));
	
	
	return Pegel_mB;
}



