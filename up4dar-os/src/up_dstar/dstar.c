/*

Copyright (C) 2011,2012   Michael Dirska, DL1BFF (dl1bff@mdx.de)

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
 * dstar.c
 *
 * Created: 03.04.2011 11:22:04
 *  Author: mdirska
 */ 

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "dstar.h"

#include <string.h>

#include "rx_dstar_crc_header.h"

#include "vdisp.h"
#include "rtclock.h"
#include "phycomm.h"






// --------------------------------

/*
static int berechne_hub (unsigned char Mittelwert, unsigned char FrameSync[])
{
	register short Summe = 0;

Summe += FrameSync[ 5] - Mittelwert;
Summe += FrameSync[ 6] - Mittelwert;
Summe += FrameSync[ 7] - Mittelwert;
Summe += FrameSync[ 8] - Mittelwert;
Summe += FrameSync[ 9] - Mittelwert;
Summe += FrameSync[10] - Mittelwert;
Summe += FrameSync[11] - Mittelwert;
Summe += FrameSync[12] - Mittelwert;
Summe += FrameSync[13] - Mittelwert;
Summe += FrameSync[14] - Mittelwert;

Summe += FrameSync[29] - Mittelwert;
Summe += FrameSync[30] - Mittelwert;
Summe += FrameSync[31] - Mittelwert;
Summe += FrameSync[32] - Mittelwert;


Summe += Mittelwert - FrameSync[41];
Summe += Mittelwert - FrameSync[42];
Summe += Mittelwert - FrameSync[43];
Summe += Mittelwert - FrameSync[44];

Summe += Mittelwert - FrameSync[71];
Summe += Mittelwert - FrameSync[72];
Summe += Mittelwert - FrameSync[73];
Summe += Mittelwert - FrameSync[74];
Summe += Mittelwert - FrameSync[75];
Summe += Mittelwert - FrameSync[76];
Summe += Mittelwert - FrameSync[77];
Summe += Mittelwert - FrameSync[78];
Summe += Mittelwert - FrameSync[79];
Summe += Mittelwert - FrameSync[80];
Summe += Mittelwert - FrameSync[81];
Summe += Mittelwert - FrameSync[82];
Summe += Mittelwert - FrameSync[83];
Summe += Mittelwert - FrameSync[84];
Summe += Mittelwert - FrameSync[85];
Summe += Mittelwert - FrameSync[86];

return  Summe/4;


}

*/

static xQueueHandle dstarQueue;


static U32 qTimeout = 0;

static struct dstarPacket dp;

static int diagram_displayed;
static int repeater_msg;

static int hub_min = 3000;
static int hub_max = 0;

#define PPM_BUFSIZE 10
int ppm_buf[PPM_BUFSIZE];
int ppm_ptr;
int ppm_display_active;

static void mkPrintableString (char * data, int len)
{
	int i;
	for (i=0; i < len; i++)
	{
		if ((data[i] < 32) || (data[i] > 126))
		{
			data[i] = ' ';
		}
	}
	data[len] = 0;
}


static char buf[40];


static void printHeader( int ypos, unsigned char crc_result, unsigned char * header_data,
	char * desc)
{
	
	memcpy(buf, header_data + 19, 8);
	mkPrintableString(buf,8);
	
	
	if ((crc_result == 0) && (ypos == 5))
	{
		if ((header_data[0] & 0x07) == 0)
		{
			// vdisp_clear_rect (0, 48, 128, 16);
			vdisp_prints_xy(0, 48, VDISP_FONT_6x8, 0, "UR:");
			vdisp_prints_xy(18, 48, VDISP_FONT_6x8, 0, buf);
			
			rtclock_disp_xy(70, 48, 2, 0);
		}
		else
		{
			// vdisp_load_buf();
			repeater_msg = 1;
			vdisp_prints_xy(80, 16, VDISP_FONT_6x8, 0, buf);
		}
		
	}
	else if ((crc_result == 0) && (ypos == 9))
	{
		if (diagram_displayed != 0)
		{
			vdisp_clear_rect (0, 0, 128, 36);
			diagram_displayed = 0;
		}
		vdisp_prints_xy(0, 24, VDISP_FONT_6x8, 0, "UR:");
		vdisp_prints_xy(18, 24, VDISP_FONT_6x8, 0, buf);
	}
	
	
	memcpy(buf, header_data + 27, 8);
	mkPrintableString(buf,8);
	
	
	if ((crc_result == 0) && (ypos == 5))
	{
		if ((header_data[0] & 0x07) == 0)
		{
			vdisp_prints_xy(0, 36, VDISP_FONT_8x12, 0, "RX:");
			vdisp_prints_xy(24, 36, VDISP_FONT_8x12, 0, buf);
		}
		else
		{
			vdisp_prints_xy(80, 8, VDISP_FONT_6x8, 0, buf);
		}
	}
	else if ((crc_result == 0) && (ypos == 9))
	{
		vdisp_prints_xy(0, 16, VDISP_FONT_6x8, 0, "RX:");
		vdisp_prints_xy(18, 16, VDISP_FONT_6x8, 0, buf);
	}
	
	
	memcpy(buf, header_data + 35, 4);
	mkPrintableString(buf,4);
	
	
	if ((crc_result == 0) && (ypos == 5))
	{
		if ((header_data[0] & 0x07) == 0)
		{
			vdisp_prints_xy(88, 36, VDISP_FONT_8x12, 0, "/");
			vdisp_prints_xy(96, 36, VDISP_FONT_8x12, 0, buf);
		}
		
	}
	else if ((crc_result == 0) && (ypos == 9))
	{
		vdisp_prints_xy(66, 16, VDISP_FONT_6x8, 0, "/");
		vdisp_prints_xy(74, 16, VDISP_FONT_6x8, 0, buf);
	}
}


static unsigned char sdHeaderBuf[41];
static unsigned char sdHeaderPos = 0;

static unsigned char sdTypeFlag = 0;
static unsigned char sdData[6];  // +1 because of mkPrintableString


static unsigned char checkSDHeaderCRC(void) 
{
	unsigned short sum = rx_dstar_crc_header(sdHeaderBuf);
	
	if (sdHeaderBuf[39] != (sum & 0xFF))
	{
		return 1;		
	}
	
	if (sdHeaderBuf[40] != ((sum >> 8) & 0xFF))
	{
		return 1;		
	}
	
	return 0;
}

static void processSDHeader( unsigned char len )
{
	int i;
	
	if ((len < 1) || (len > 5))
	{
		sdHeaderPos = 0;  // reset
		return;
	}
	
	if ((sdHeaderPos + len) > (sizeof sdHeaderBuf))
	{
		sdHeaderPos = 0;  // reset
		return;
	}
	
	for (i=0; i < len; i++)
	{
		sdHeaderBuf[sdHeaderPos + i] = sdData[i];
	}
	
	sdHeaderPos += len;
	
	if (sdHeaderPos == 41)
	{
		unsigned char res = checkSDHeaderCRC();
		
		printHeader(9, res, sdHeaderBuf, "Header from slow data channel:");
		sdHeaderPos = 0;
	}
	
	if (len < 5)  // last frame always shorter than 5
	{
		sdHeaderPos = 0;
	}
}



static void processSlowData( unsigned char sdPos, unsigned char * sd )
{	
	if (sdPos & 1)
	{	
		sdTypeFlag = sd[0];
		sdData[0] = sd[1];
		sdData[1] = sd[2];
	}
	else
	{	
		sdData[2] = sd[0];
		sdData[3] = sd[1];
		sdData[4] = sd[2];
		
		unsigned char len;
		
		len = sdTypeFlag & 0x07;  // ignore Bit 3
		
		if (len > 5) // invalid length
		{
			len = 0;  
		}
		
		switch (sdTypeFlag & 0xF0)
		{
			case 0x50:
				processSDHeader(len);
				break;
				
			case 0x40:
				if ((sdTypeFlag & 0x0C) == 0)
				{
					mkPrintableString((char *) sdData, 5);
					
					vdisp_prints_xy( ((sdTypeFlag & 0x03) * 30), 56, VDISP_FONT_6x8, 0, (char *) sdData );
				}
				break;
		}
	}
}

static U32 voicePackets = 0;
static U32 syncPackets = 0;

static unsigned char dState = 0;

#define FRAMESYNC_LEN	90

static unsigned char frameSync[FRAMESYNC_LEN];


static void dstarStateChange(unsigned char n)
{
	int i;
	int lastValue = 128;
	
	switch(n)
	{

		case 2:
			voicePackets = 0;
			syncPackets = 0;
			sdHeaderPos = 0;
			
			vdisp_save_buf();
			vdisp_clear_rect (0, 0, 128, 64);
			vdisp_prints_xy( 104, 48, VDISP_FONT_6x8, 1, "  0s" );
			repeater_msg = 0;
			ppm_ptr = 0;
			
			for (i=0; i < PPM_BUFSIZE ; i++)
			{
				ppm_buf[i] = 4;
			}
			
			ppm_display_active = 0;
			break;
			
		case 1:
		
			if (repeater_msg == 0)
			{				
				int secs = voicePackets / 50;
				
				if (secs > 0)
				{
					char buf[4];
					vdisp_i2s(buf, 3, 10, 0, secs);
					vdisp_prints_xy( 104, 48, VDISP_FONT_6x8, 0, buf );
					vdisp_prints_xy( 122, 48, VDISP_FONT_6x8, 0, "s" );
				}
				else
				{
					vdisp_prints_xy( 104, 48, VDISP_FONT_6x8, 0, "    " );
				}
				
				vdisp_prints_xy( 0,0, VDISP_FONT_6x8, 0, "       " );
			}
			else
			{
				vdisp_load_buf();
			}

			
			
			
			
			
			for (i=0; i < FRAMESYNC_LEN; i++)
			{
				int k = frameSync[i];
				
				if (k < 40)
				{
					k = 40;
				}
				
				if (k >= 216)
				{
					k = 215;
				}

				
					
				
				lastValue = k;
			}
			break;
			
		case 10:
		    vdisp_prints_xy( 0,0, VDISP_FONT_6x8, 1, " WAIT " );
			vdisp_prints_xy( 36,0, VDISP_FONT_6x8, 0, " " );
			break;
		
		case 4:
		    vdisp_prints_xy( 0,0, VDISP_FONT_6x8, 1, "  TX  " );
			vdisp_prints_xy( 36,0, VDISP_FONT_6x8, 0, " " );
			break;
	}
	
	dState = n;
}


static void print_diagram(int mean_value)
{
	int i;
	
	for (i=0; i < FRAMESYNC_LEN; i++)
	{
		int d = mean_value - frameSync[i];
					
		if ((d > -20) && (d < 20))
		{
			vdisp_set_pixel(38 + i, d + 20, 0, 1, 1);
			if ((i & 0x01) == 1)
			{
				vdisp_set_pixel(38 + i, 20, 0, 1, 1);
			}
		}
	}			
				
	char buf[5];
			
	vdisp_i2s(buf, 3, 10, 0, mean_value);
	vdisp_prints_xy( 116, 14, VDISP_FONT_4x6, 0, buf );
	
	/*			
	vdisp_set_pixel( 34, 20, 0, 0x0f, 3 );
	vdisp_set_pixel( 34, 10, 0, 0x0f, 3 );
	vdisp_set_pixel( 34, 30, 0, 0x0f, 3 );
	*/
}


void dstarResetCounters(void)
{
	voicePackets = 0;
	syncPackets = 0;
}

void dstarChangeMode(int m)
{
	char buf[7] = { 0x00, 0x10, 0x02, 0xD3, m, 0x10, 0x03 };
		
	phyCommSend( buf, sizeof buf );
	
	dstarResetCounters();
}

static void processPacket(void)
{
	
	
	
	switch(dp.cmdByte)
	{
		case 0x01:
			
			vdisp_clear_rect (0, 0, 128, 64);
			// dstarChangeMode(4);
			break;
			
		case 0x30:
			if (dp.dataLen == 40)
			{
				
				printHeader (5, dp.data[0], dp.data + 1, "Header from command 0x30:");
				
			}				
			break;
			
		case 0x31:
			if (dp.dataLen >= 10)
			{
				voicePackets ++;
				
				{
					char buf[4];
					int secs = voicePackets / 50;
					
					if (secs > 0)
					{
						vdisp_i2s(buf, 3, 10, 0, secs);
						vdisp_prints_xy( 104, 48, VDISP_FONT_6x8, 1, buf );
						vdisp_prints_xy( 122, 48, VDISP_FONT_6x8, 1, "s" );
					}
				}					
			}				
			break;
		case 0x32:
			syncPackets ++;
			
			if (dp.dataLen == 1)
			{
				ppm_buf[ppm_ptr] = dp.data[0];
				
				ppm_ptr++;
				
				if (ppm_ptr >= PPM_BUFSIZE)
				{
					ppm_ptr = 0;
					ppm_display_active = 1;
				}
				
				if (ppm_display_active != 0)
				{
					int i;
					char buf[5];
					int v;
					int minus = 0;
					
					for (i=0, v=0; i < PPM_BUFSIZE ; i++)
					{
						v += ppm_buf[i];
					}
					
					v = (v * 84) / PPM_BUFSIZE;
					v -= 7 * 84;
					// v -= 28;
					if (v < 0)
					{
						minus = 1;
						v = -v;
					}
					vdisp_i2s(buf, 3, 10, 0, v);
					vdisp_prints_xy( 6, 0, VDISP_FONT_6x8, 0, buf );
					vdisp_prints_xy( 0, 0, VDISP_FONT_6x8, 0, minus ? "-" : "+" );
					vdisp_prints_xy( 24, 0, VDISP_FONT_6x8, 0, "ppm" );
				}
			}				
			break;
		case 0x33:
			if (dp.dataLen == 4)
			{
				// dispPrintDecimalXY(0,5, dp.data[0]);
				
				processSlowData(dp.data[0], dp.data + 1);
			}
			break;
		case 0x34:
			break;
			
		case 0x35:
			if (dp.dataLen == (FRAMESYNC_LEN + 2))
			{
				
				if (dState != 2)
				{
					dstarStateChange(2);
				}
				
				int i;
				
				for (i=0; i < FRAMESYNC_LEN; i++)
				{
					frameSync[i] = dp.data[i+2];
				}			
				
				print_diagram(dp.data[0]);	
				
				char buf[5];
				int hub = dp.data[1] * 71 - 52;
				
				vdisp_i2s(buf, 4, 10, 0, hub);
				vdisp_prints_xy( 0, 10, VDISP_FONT_6x8, 0, buf );
				vdisp_prints_xy( 24, 10, VDISP_FONT_6x8, 0, "Hz" );
				vdisp_prints_xy( 0, 18, VDISP_FONT_6x8, 0, "Hub" );
				
				if (hub > hub_max)
				{
					hub_max = hub;
				}
				
				if (hub < hub_min)
				{
					hub_min = hub;
				}
				
				/*
				vdisp_i2s(buf, 4, 10, 0, hub_max);
				vdisp_prints_xy( 0, 20, VDISP_FONT_4x6, 0, buf );
				vdisp_i2s(buf, 4, 10, 0, hub_min);
				vdisp_prints_xy( 0, 26, VDISP_FONT_4x6, 0, buf );
				*/
				diagram_displayed = 1;
				
			} 
			
			/* else	if (dp.dataLen == FRAMESYNC_LEN)
			{
				int i;
				int sum = 0;
				
				for (i=0; i < FRAMESYNC_LEN; i++)
				{
					frameSync[i] = dp.data[i];
					sum += dp.data[i];
				}					
				
				// vdisp_clear_rect (0, 0, 128, 48);
				
				sum /= FRAMESYNC_LEN;
				
				print_diagram(sum);
				
				char buf[5];
				int hub = berechne_hub( sum & 0xFF, frameSync);
				vdisp_i2s(buf, 4, 10, 0, hub);
				vdisp_prints_xy( 0, 17, VDISP_FONT_4x6, 0, buf );
				
				if (hub > hub_max)
				{
					hub_max = hub;
				}
				
				if (hub < hub_min)
				{
					hub_min = hub;
				}
				
				vdisp_i2s(buf, 4, 10, 0, hub_max);
				vdisp_prints_xy( 0, 10, VDISP_FONT_4x6, 0, buf );
				vdisp_i2s(buf, 4, 10, 0, hub_min);
				vdisp_prints_xy( 0, 24, VDISP_FONT_4x6, 0, buf );
				
				diagram_displayed = 1;
			}  */
			
			break;
			
		case 0xD1:
			if (dp.dataLen >= 2)
			{
				
				if (dState != dp.data[1])
				{
					dstarStateChange(dp.data[1]);
				}
			}
			break;
			
		
	}	
}


static portTASK_FUNCTION( dstarRXTask, pvParameters )
{
	for( ;; )
	{
		if( xQueueReceive( dstarQueue, &dp, 500 ) )
		{
			processPacket();
		}
		else
		{
			// timeout
			qTimeout ++;
			// dispPrintDecimalXY(1, 8, qTimeout);
			
			dstarStateChange(0);
		}
		
	}
} 






void dstarInit( xQueueHandle dq )
{
	dstarQueue = dq;
	
	xTaskCreate( dstarRXTask, ( signed char * ) "DstarRx", configMINIMAL_STACK_SIZE, NULL,
		 tskIDLE_PRIORITY + 1 , ( xTaskHandle * ) NULL );

}
