/*
 *  memory.h - Memory Definition
 *  Copyright (C) 2001-2003 Foster (Original Code)
 *  Copyright (C) 2004-2005 Olivier "Evilo" Biot (PS2 Port)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>

#include "neocd.h"
#include "static/startup.h"
#include "../video/video.h"
#include "../misc/misc.h"
//#include "../sound/timer.h"
#include "../save/mc.h"


extern unsigned short  *video_paletteram_ng __attribute__((aligned(64)));

char		*neogeo_prg_memory __attribute__((aligned(64))) = NULL;
char		*neogeo_spr_memory __attribute__((aligned(64))) = NULL;
char		*neogeo_pcm_memory __attribute__((aligned(64))) = NULL;
char		*neogeo_fix_memory __attribute__((aligned(64))) = NULL;

// vector table
char 		neogeo_game_vectors[0x80]  __attribute__((aligned(64))); 


/*** Globals ***************************************************************************************/
int watchdog_counter=-1;
int memcard_write=0;

static int prot_data; //used for the protection thingers

int   initialize_memmap(int machine_type);
unsigned int m68k_read_memory_8( unsigned int address);
unsigned int m68k_read_memory_16( unsigned int address);
unsigned int m68k_read_memory_32( unsigned int address);
int read16_200000_2fffff(int offset);
void m68k_write_memory_8( unsigned int address, unsigned int value);
void m68k_write_memory_16( unsigned int address, unsigned int value);
void m68k_write_memory_32(const unsigned int address, unsigned int value);
void write16_200000_2fffff(int offset,int data);





/***Helper Functions********************************************************************************/
static int    read_upload(int);
static void   write_upload(int, int);
static void   write_upload_word(int, int);
static void   write_upload_dword(int, int);

static int    cpu_readvidreg(int);
static void   cpu_writevidreg(int, int);
static void   cpu_writeswitch(int, int);
static int    cpu_readcoin(int);

static void   write_memcard(int,int);

void cdda_control(void);

static void   watchdog_reset_w(void);


/***************************************************************************************************/
int initialize_memmap(int machine_type) 
{
    // Allocate needed memory buffers	
    printf("NEOGEO: Allocating memory :\n");
	
    printf("PRG (2MB) ... \n");
    if (neogeo_prg_memory==NULL)
      neogeo_prg_memory = (char*)memalign(64, 0x200000);
    if (neogeo_prg_memory==NULL)
	return -1;

    printf("SPR (8MB) ... \n");
    if (neogeo_spr_memory==NULL)
      neogeo_spr_memory = (char*)memalign(64, 0x900000);
    if (neogeo_spr_memory==NULL)
	return -1;
	
    printf("PCM (1Mb) ... \n");
    if (neogeo_pcm_memory==NULL)
      neogeo_pcm_memory = (char*)memalign(64, 0x600000);
    if (neogeo_pcm_memory==NULL)
	return -1;

    
    printf("FIX ROM (128kb) ... \n");
    if (neogeo_fix_memory == NULL)
   	  neogeo_fix_memory = (char*)memalign(64, 0x20000);
    if (neogeo_fix_memory==NULL)
   	return -1;

    printf("DONE!\n");
    

    // set up NEOCD Startup RAM	  
    if (machine_type == NCD_MACHINE)
    {
      printf("Set startup ram...");
      memcpy(neogeo_prg_memory + 0x10F300, startup_bin, STARTUP_BIN_LEN); 
      swab(neogeo_prg_memory + 0x10F300, neogeo_prg_memory + 0x10F300, STARTUP_BIN_LEN);
      printf("DONE!\n");
    }
    
    
    /* took from mame  : info from elsemi, this is how nebula works, is there a better way in mame? */
    memcpy( neogeo_game_vectors, neogeo_prg_memory,  0x80 );
    
    return 0;
    
}

/***************************************************************************************************/


/***************************************************************************************************/
unsigned int m68k_read_memory_8(unsigned int offset)
{
    offset&=0xffffff;
    if(offset<0x200000)
        return *((u8*)&neogeo_prg_memory[offset^1]);


    switch(offset>>16)
    {
        case    0x30:    return read_player1();
        case    0x32:    return cpu_readcoin(offset);
        case    0x34:    return read_player2();
        case    0x38:    return read_pl12_startsel();
        case    0x80:    if(offset&0x01) return neogeo_memorycard[(offset&0x3fff)>>1];
                         else return -1;

        /* BIOS ROM */
        case    0xc0:
        case    0xc1:
        case    0xc2:
        case    0xc3:
        case    0xc4:
        case    0xc5:
        case    0xc6:
        case    0xc7:    return neogeo_rom_memory[(offset^1)&0x0fffff];

        /* upload region */
        case    0xe0:
        case    0xe1:
        case    0xe2:
        case    0xe3:    return read_upload(offset&0xfffff);

        default:
            //logaccess("m68k_read_memory_8(0x%x) PC=%x\n",offset,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
    return 0;
}








/***************************************************************************************************/
unsigned int  m68k_read_memory_16(unsigned int offset) {
    offset&=0xffffff;
    if(offset<0x200000)
        return *((uint16*)&neogeo_prg_memory[offset]);

    switch(offset>>16)
    {
    	case	0x20:	
    	case	0x21:	
    	case	0x22:	
    	case	0x23:	
    	case	0x24:	
    	case	0x25:	
    	case	0x26:
	case	0x27:	
	case	0x28:	
	case	0x29:	
	case	0x2a:	
	case	0x2b:	
	case	0x2c:	
	case	0x2d:
	case	0x2e:	
	case	0x2f:	return read16_200000_2fffff(offset); 
			break;

        case    0x3c:    return cpu_readvidreg(offset);
        
        case    0x40:    return video_paletteram_ng[offset&0x1fff];
        case    0x80:    return 0xff00|neogeo_memorycard[(offset&0x3fff)>>1];

        /* BIOS ROM */
        case    0xc0:
        case    0xc1:
        case    0xc2:
        case    0xc3:
        case    0xc4:
        case    0xc5:
        case    0xc6:
        case    0xc7:    return *(uint16*)&neogeo_rom_memory[offset&0x0fffff];
		
	case	0xff:	 break;


        default:
            //logaccess("m68k_read_memory_16(0x%x) PC=%x\n",offset,m68k_get_reg(NULL,M68K_REG_PPC));
            break;
    }
    return 0;
}



/***************************************************************************************************/
unsigned int  m68k_read_memory_32(unsigned int offset) {
    unsigned int data;
    offset&=0xffffff;
    data=m68k_read_memory_16(offset)<<16;
    data|=m68k_read_memory_16(offset+2);
    return data;
}

/***************************************************************************************************/
int read16_200000_2fffff(int offset)
{
	u16 res = (prot_data >> 24) & 0xff;

	switch (offset)
	{
		case 0x55550 >> 1:
		case 0xffff0 >> 1:
		case 0x00000 >> 1:
		case 0xff000 >> 1:
		case 0x36000 >> 1:
		case 0x36008 >> 1:
			return res;

		case 0x36004 >> 1:
		case 0x3600c >> 1:
			return ((res & 0xf0) >> 4) | ((res & 0x0f) << 4);

		default:
			return 0;
	}
}



/***************************************************************************************************/
void m68k_write_memory_8(unsigned int offset, unsigned int data) {
    int temp;
    data&=0xff;
    offset&=0xffffff;

    if(offset<0x200000) {
        neogeo_prg_memory[offset^1]=(char)data;
        return;
    }

    switch(offset>>16)
    {
        case    0x30:   watchdog_reset_w(); break;
        case    0x32:	//if(!(offset&0xffff)) 
	    		//{
                	   
                	   if (neocdSettings.soundOn || neocdSettings.CDDAOn)
                	   {
                	      sound_code=data&0xff;
                	      pending_command=1;
                	      
			      _z80nmi();
			      /* spin for a while to let the Z80 read the command  -> 20 USEC */
			      _z80spinuntil_time (machine_def.z80_cycles * TIME_IN_USEC(20));
			      // timer sync
                	      my_timer();
                	   }
            		//}
	    		break;
	    
	case	0x38:	break;
        case    0x3a:   cpu_writeswitch(offset, data); break;
        case    0x3c:   
        		temp=cpu_readvidreg(offset);
            		if(offset&0x01) 
            		   cpu_writevidreg(offset, (temp&0xff)|(data<<8));
            		else 
            		   cpu_writevidreg(offset, (temp&0xff00)|data);
            		break;
        
        case    0x80:   if(offset&0x01) write_memcard(offset,data); 
        		break;

        /* upload */
        case    0xe0:
        case    0xe1:
        case    0xe2:
        case    0xe3:   write_upload(offset&0xfffff,data); break;
		
	/* cdrom */
	case	0xff:	break;

        default:       //logaccess("m68k_write_memory_8(0x%x,0x%x) PC=%x\n",offset,data,m68k_get_reg(NULL,M68K_REG_PC));
            	       break;
    }
}



/***************************************************************************************************/


void m68k_write_memory_16(unsigned int offset, unsigned int data)
{
    data&=0xffff;
    offset&=0xffffff;

    if(offset<0x200000) {
        *(uint16*)&neogeo_prg_memory[offset]=(uint16)data;
        return;
    }

    switch(offset>>16)
    {
    	case	0x20:	
    	case	0x21:	
    	case	0x22:	
    	case	0x23:	
    	case	0x24:	
    	case	0x25:	
    	case	0x26:
	case	0x27:	
	case	0x28:	
	case	0x29:	
	case	0x2a:	
	case	0x2b:	
	case	0x2c:	
	case	0x2d:
	case	0x2e:	
	case	0x2f:	write16_200000_2fffff(offset, data); 
			break;
        
        case    0x3a:   cpu_writeswitch(offset, data); 
        		break;
        case    0x3c:   cpu_writevidreg(offset, data); 
        		break;
        case    0x40:   offset =(offset&0x1fff)>>1;
                        data  &=0x7fff;
                        video_paletteram_ng[offset]=(uint16)data;
                        video_paletteram_pc[offset]=video_color_lut[data]; 
                        break;
        case    0x80:   write_memcard(offset,data); 
        		break;

        /* upload */
        case    0xe0:
        case    0xe1:
        case    0xe2:
        case    0xe3:    write_upload_word(offset&0xfffff,data); break;
		
		case	0xff: break;

        default:
            //logaccess("m68k_write_memory_16(0x%x,0x%x) PC=%x\n",offset,data,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
}



/***************************************************************************************************/


#if 1
void m68k_write_memory_32(const unsigned int offset, unsigned int data) {
	unsigned int off = offset & 0xffffff;
	//unsigned int word1=(data>>16)&0xffff;
	//unsigned int word2=data&0xffff;
	m68k_write_memory_16(off, (data>>16)&0xffff);// word1
	m68k_write_memory_16(off+2,data&0xffff);//word2
}

#else
void m68k_write_memory_32(const unsigned int offset, unsigned int data) {
    int temp=((data&0xffff)<<16)|((data>>16)&0xffff);
    offset&=0xffffff;


    if(offset<0x200000) {
        *(uint32*)&neogeo_prg_memory[offset]=temp;
        return;
    }

    switch(offset>>16)
    {
        case    0x3a:    cpu_writeswitch(offset, temp); break;
        case    0x3c:    cpu_writevidreg(offset, temp);
                         cpu_writevidreg(offset+2, data); break;
        case    0x40:    offset =(offset&0x1fff)>>1;
                         data  &=0x7fff;
                         temp  &=0x7fff;
                         video_paletteram_ng[offset]   = (uint16)temp;
                         video_paletteram_pc[offset]   = video_color_lut[temp];
                         video_paletteram_ng[offset+1] = (uint16)data;
                         video_paletteram_pc[offset+1] = video_color_lut[data]; break;


        case    0x80:    offset =(offset&0x3fff)>>1;
                         neogeo_memorycard[offset]  = (char)temp;
                         neogeo_memorycard[offset+1]= (char)data; break;

        case    0xe0:
        case    0xe1:
        case    0xe2:
        case    0xe3:    write_upload_dword(offset&0xfffff,temp); break;
		
		case	0xff:	break;

        default:
            //logaccess("m68k_write_memory_32(0x%x,0x%x) PC=%x\n",offset,data,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
}
#endif

/***************************************************************************************************/
void write16_200000_2fffff(int offset,int data)
{
	//supposedly for fatal fury 2
	switch (offset)
	{
		case 0x255552 :	 /* data == 0x5555; read back from 55550, ffff0, 00000, ff000 */
			prot_data = 0xff00ff00;
			break;

		case 0x256782 :	 /* data == 0x1234; read back from 36000 *or* 36004 */
			prot_data = 0xf05a3601;
			break;

		case 0x242812 :	 /* data == 0x1824; read back from 36008 *or* 3600c */
			prot_data = 0x81422418;
			break;

		case 0x255550 :
		case 0x2ffff0 :
		case 0x2ff000 :
		case 0x236000 :
		case 0x236004 :
		case 0x236008 :
		case 0x23600c :
			prot_data <<= 8;
			break;

		default:
			break;
	}
}


/***************************************************************************************************/
static int    cpu_readvidreg(int offset)
{
    switch(offset)
    {

        case    0x3c0000:    return *(uint16*)&video_vidram[video_pointer<<1]; break;
        case    0x3c0002:    return *(uint16*)&video_vidram[video_pointer<<1]; break;
        case    0x3c0004:    return video_modulo; break;
        case    0x3c0006:    return ((((neogeo_frame_counter_speed-1)&0xffff)<<8)|
                               (neogeo_frame_counter&7)|128); break;
        case    0x3c0008:    return *(uint16*)&video_vidram[video_pointer<<1]; break;
        case    0x3c000a:    return *(uint16*)&video_vidram[video_pointer<<1]; break;


        default:
            //logaccess("cpu_readvidreg(0x%x) PC=%x\n",offset,m68k_get_reg(NULL,M68K_REG_PC));
            return 0;
            break;
    }
}


/***************************************************************************************************/
void    cpu_writevidreg(int offset, int data)
{
    //printf ("offset %x\n", offset);

    //printf ("video_vidram : %p\n",video_vidram);

    switch(offset)
    {
        case    0x3c0000:    video_pointer=(uint16)data; break;
        case    0x3c0002:    *(uint16*)&video_vidram[(video_pointer<<1)]=(uint16)data;
        		     //*((uint16 *)video_vidram + (video_pointer<<1)) = (uint16)data; 
        		     //((uint16*)&video_vidram)[(video_pointer<<1)] = (uint16)data;
                             video_pointer+=video_modulo; 
                             break; 
                             
        case    0x3c0004:    video_modulo=(int16)data; break;

        case    0x3c0006:    neogeo_frame_counter_speed=((data>>8)&0xff)+1; break;
        //case    0x3c0006:    neogeo_frame_counter_speed=(data>>8)&0xff; break;

        case    0x3c0008:    /* IRQ position */    break;
        case    0x3c000a:    /* IRQ position */    break;
        case    0x3c000c:    /* IRQ acknowledge */ break;

        default: break;
    }
    //printf ("done\n");
}

/***************************************************************************************************/

static void neogeo_setpalbank0 (void) {
    video_paletteram_ng=video_palette_bank0_ng;
    video_paletteram_pc=video_palette_bank0_pc;
}


static void neogeo_setpalbank1 (void) {
    video_paletteram_ng=video_palette_bank1_ng;
    video_paletteram_pc=video_palette_bank1_pc;
}


static void neogeo_select_bios_vectors (void) 
{
    memcpy(neogeo_prg_memory, neogeo_rom_memory, 0x80);
}


static void neogeo_select_game_vectors (void) 
{
    memcpy( neogeo_prg_memory, neogeo_game_vectors, 0x80 );
    //printf("write game vectors stub\n");
}


static void cpu_writeswitch(int offset, int data)
{
    switch(offset)
    {
        case 0x3a0000: /* NOP */ break;
        case 0x3a0001: /* NOP */ break;

        case 0x3a0002: neogeo_select_bios_vectors(); break;
        case 0x3a0003: neogeo_select_bios_vectors(); break;

        case 0x3a000e: neogeo_setpalbank1(); break;
        case 0x3a000f: neogeo_setpalbank1(); break;

        case 0x3a0010: /* NOP */ break;
        case 0x3a0011: /* NOP */ break;

        case 0x3a0012: neogeo_select_game_vectors(); break;
        case 0x3a0013: neogeo_select_game_vectors(); break;

        case 0x3a001e: neogeo_setpalbank0(); break;
        case 0x3a001f: neogeo_setpalbank0(); break;

        default:
            //logaccess("cpu_writeswitch(0x%x,0x%x) PC=%x\n",offset,data,m68k_get_reg(NULL,M68K_REG_PC));
            break;
    }
}

/***************************************************************************************************/


void neogeo_sound_irq(int irq)
{
    if (irq)
 	_z80raise(0);
    else 
	_z80lower();

}


// TO REVIEW !!
static int cpu_readcoin(int addr)
{
    addr &= 0xFFFF;
    int coinflip = pd4990a_testbit_r();
    int databit = pd4990a_databit_r();
    int res =0;
    
    res = 0xff ^ (coinflip << 6) ^ (databit << 7);
    
            
    if (neocdSettings.soundOn || neocdSettings.CDDAOn)
    {
        res |= result_code;
        if (pending_command) 
           res &= 0x7f;
    } 
    else 
    {
        res |= 0x01;
    }
    return res;
}


static void watchdog_reset_w (void)
{
    //if (watchdog_counter == -1) logaccess("Watchdog Armed!\n");
    watchdog_counter = 3 * machine_def.fps_rate;  /* 3s * 50/60 fps */
}


static int read_upload(int offset) 
{
    int zone = m68k_read_memory_8(0x10FEDA);
    /* int bank = m68k_read_memory_8(0x10FEDB); */

    /* read_upload is disabled for now.*/
	/* fixes Metal Slug */
    return -1;

    switch (zone) {
        case 0x00: /* 68000 */
            return neogeo_prg_memory[offset^1];
        case 0x01: /* Z80 */
            return subcpu_memspace[(offset>>1)&0xFFFF];
        default:
            //logaccess ("read_upload unimplemented zone %x\n",zone);
            return -1;
    }
}


static void write_upload(int offset, int data) 
{
    int zone = m68k_read_memory_8(0x10FEDA);
    /* int bank = m68k_read_memory_8(0x10FEDB); */

    switch (zone) {
        case 0x00: /* 68000 */
            neogeo_prg_memory[offset^1]=(char)data;
            break;
        case 0x01: /* Z80 */
            subcpu_memspace[(offset>>1)&0xFFFF]=(char)data;
            break;
        case 0x11: /* FIX */
            neogeo_fix_memory[offset>>1]=(char)data;
            break;
        default:
            //logaccess ("write_upload unimplemented zone %x\n",zone); 
            break;
    }
}


static void write_upload_word(int offset, int data) 
{
    int zone = m68k_read_memory_8(0x10FEDA);
    int bank = m68k_read_memory_8(0x10FEDB);
    int offset2;
    char *dest;
    char sprbuffer[4];
	
  	data&=0xffff;

    switch (zone) {
        case 0x12: /* SPR */

            offset2=offset & ~0x02;

            offset2+=(bank<<20);

            if((offset2&0x7f)<64)
               offset2=(offset2&0xfff80)+((offset2&0x7f)<<1)+4;
            else
               offset2=(offset2&0xfff80)+((offset2&0x7f)<<1)-128;

            dest=&neogeo_spr_memory[offset2];

            if (offset & 0x02) 
            {
                /* second word */
		*(uint16*)(&dest[2])=(uint16)data;
		/* reformat sprite data */
               	swab(dest, sprbuffer, sizeof(sprbuffer));
               	extract8(sprbuffer, dest);
	    } 
	    else 
	    {
		/* first word */
		*(uint16*)(&dest[0])=(uint16)data;
            }
            break;

        case 0x13: /* Z80 */
            subcpu_memspace[(offset>>1)&0xFFFF]=(char)data;
            break;
        case 0x14: /* PCM */
            neogeo_pcm_memory[(offset>>1)+(bank<<19)]=(char)data;
            break;
        default:
            //logaccess ("write_upload_word unimplemented zone %x\n",zone); 
            break;
    }
}


static void write_upload_dword(int offset, int data) 
{
    int zone = m68k_read_memory_8(0x10FEDA);
    int bank = m68k_read_memory_8(0x10FEDB);
    char *dest;
    char sprbuffer[4];

    switch (zone) {
        case 0x12: /* SPR */
            offset+=(bank<<20);

            if((offset&0x7f)<64)
               offset=(offset&0xfff80)+((offset&0x7f)<<1)+4;
            else
               offset=(offset&0xfff80)+((offset&0x7f)<<1)-128;

            dest=&neogeo_spr_memory[offset];
            swab((char*)&data, sprbuffer, sizeof(sprbuffer));
            extract8(sprbuffer, dest);

            break;
        default:
            //logaccess ("write_upload_dword unimplemented zone %x\n",zone); 
            break;
    }
}


static void write_memcard(int offset, int data) 
{
    data&=0xff;
    //printf("write memory card\n");
    neogeo_memorycard[(offset&0x3fff)>>1]=(char)data; 
    /* signal that memory card has been written */
    memcard_write=3; 
}

