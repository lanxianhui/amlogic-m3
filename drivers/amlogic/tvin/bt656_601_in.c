/*
 * Amlogic M1 & M2
 * frame buffer driver  -------bt656 & 601 input
 * Copyright (C) 2010 Amlogic, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/amports/amstream.h>
#include <linux/amports/ptsserv.h>
#include <linux/amports/canvas.h>
#include <linux/workqueue.h>
#include <linux/dma-mapping.h>
#include <asm/atomic.h>
#include <mach/am_regs.h>
#include "bt656_601_in.h"


#define DEVICE_NAME "amvdec_656in"
#define MODULE_NAME "amvdec_656in"
#define BT656IN_IRQ_NAME "amvdec_656in-irq"

//#define HANDLE_BT656IN_IRQ



#define BT656IN_COUNT 			32
#define BT656IN_ANCI_DATA_SIZE     	0x4000
#define BR656IN_NTSC_FLAG		0x10

#ifdef DEBUG
#define pr_dbg(fmt, args...) printk(KERN_DEBUG "amvdec656in: " fmt, ## args)
#else
#define pr_dbg(fmt, args...)
#endif
#define pr_error(fmt, args...) printk(KERN_ERR "amvdec656in: " fmt, ## args)



/* Per-device (per-bank) structure */
//typedef struct bt656_in_dev_s {
    /* ... */
//    struct cdev         cdev;             /* The cdev structure */
    //wait_queue_head_t   wait_queue;            /* wait queues */
//}am656in_dev_t;

//static struct am656in_dev_t am656in_dev_;
typedef struct {
    unsigned long   pbufAddr;
    unsigned long   pbufSize;

    unsigned        pin_mux_reg1;   //for bt656 input
    unsigned        pin_mux_mask1;

    unsigned        pin_mux_reg2;   //for bt601 input
    unsigned        pin_mux_mask2;

    unsigned        pin_mux_reg3;   //for camera input
    unsigned        pin_mux_mask3;

    unsigned        active_pixel;
    unsigned        active_line;

    unsigned char   input_mode;  //0:656--PAL ; 1:656--NTSC   ccir656 input
                                //2:601--PAL ; 3:601--NTSC   ccir656 input
                                //4:640x480 camera inout(progressive)
                                //5:800x600 camera inout(progressive)
                                //6:1024x768 camera inout(progressive)
                                //.....
                                //0xff: disable 656in/601/camera decode;
}am656in_t;

am656in_t am656in_dec_info = {
        .pbufAddr = 0x81000000,
        .pbufSize = 0x1000000,
        .pin_mux_reg1 = 0,      //for bt656 input
        .pin_mux_mask1 = 0,
        .pin_mux_reg2 = 0,      //for bt656 input
        .pin_mux_mask2 = 0,
        .pin_mux_reg3 = 0,      //for camera input
        .pin_mux_mask3 = 0,
        .active_pixel = 720,
        .active_line = 288,
        .input_mode = 0xff,     //disable 656in/601/camera decode;



    };


#ifdef HANDLE_BT656IN_IRQ
static const char bt656in_dec_id[] = "bt656in-dev";
#endif

//static dev_t am656in_id;
//static struct class *am656in_class;
//static struct device *am656in_dev;


static void init_656in_dec_parameter(unsigned char input_mode)
{
    am656in_dec_info.input_mode = input_mode;
    switch(input_mode)
    {
        case 0:     //656--PAL(interlace)
            am656in_dec_info.active_pixel = 720;
            am656in_dec_info.active_line = 288;
            pr_dbg("PAL input mode is selected for bt656, \n");
            break;

        case 1:     //656--NTSC(interlace)
            am656in_dec_info.active_pixel = 720;
            am656in_dec_info.active_line = 240;
            pr_dbg("NTSC input mode is selected for bt656, \n");
            break;

        case 2:     //601--PAL(interlace)
            am656in_dec_info.active_pixel = 720;
            am656in_dec_info.active_line = 288;
            pr_dbg("PAL input mode is selected for bt601, \n");
            break;

        case 3:     //601--NTSC(interlace)
            am656in_dec_info.active_pixel = 720;
            am656in_dec_info.active_line = 240;
            pr_dbg("NTSC input mode is selected for bt601, \n");
            break;

        case 4:     //640x480 camera inout(progressive)
            am656in_dec_info.active_pixel = 640;
            am656in_dec_info.active_line = 480;
            pr_dbg("640x480 input mode is selected for camera, \n");
            break;

        case 5:     //800x600 camera inout(progressive)
            am656in_dec_info.active_pixel = 800;
            am656in_dec_info.active_line = 600;
            pr_dbg("800x600 input mode is selected for camera, \n");
            break;

        case 6:     //1024x768 camera inout(progressive)
            am656in_dec_info.active_pixel = 1024;
            am656in_dec_info.active_line = 768;
            pr_dbg("1024x768 input mode is selected for camera, \n");
            break;

        case 0xff:
            pr_dbg("bt656_601 input decode is not start, do nothing \n");
            break;

        default:
            pr_dbg("bt656_601 input mode is not defined, do nothing \n");
            break;

    }

    return;
}

//NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
static void reset_bt656in_dec(void)
{
		unsigned temp_data;
		// reset BT656in module.
		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data |= ( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data &= ~( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

        WRITE_CBUS_REG(BT_FIELDSADR, (4 << 16) | 4);	// field 0/1 start lcnt: default value
// configuration the BT PORT control
// For standaREAD_CBUS_REG bt656 in stream, there's no HSYNC VSYNC pins.
// So we don't need to configure the port.
		WRITE_CBUS_REG(BT_PORT_CTRL, 1 << BT_D8B);	// data itself is 8 bits.

		WRITE_CBUS_REG(BT_SWAP_CTRL,	( 4 << 0 ) |        //POS_Y1_IN
						( 5 << 4 ) |        //POS_Cr0_IN
 						( 6 << 8 ) |        //POS_Y0_IN
						( 7 << 12 ));       //POS_CB0_IN

        WRITE_CBUS_REG(BT_LINECTRL , 0)  ;
// ANCI is the field blanking data, like close caption. If it connected to digital camara interface, the jpeg bitstream also use this ANCI FIFO.
		WRITE_CBUS_REG(BT_ANCISADR, am656in_dec_info.pbufAddr);
		WRITE_CBUS_REG(BT_ANCIEADR, am656in_dec_info.pbufAddr + BT656IN_ANCI_DATA_SIZE);

		WRITE_CBUS_REG(BT_AFIFO_CTRL,	(1 <<31) |     // load start and end address to afifo.
						(1 << 6) |     // fill _en;
						(1 << 3)) ;     // urgent


    		WRITE_CBUS_REG(BT_INT_CTRL ,   // (1 << 5) |    //ancififo done int.
//						(1 << 4) |    //SOF interrupt enable.
//						(1 << 3) |      //EOF interrupt enable.
						(1 << 1)); // |      //input overflow interrupt enable.
//						(1 << 0));      //bt656 controller error interrupt enable.

		WRITE_CBUS_REG(BT_ERR_CNT, (626 << 16) | (1760));

		if(am656in_dec_info.input_mode  == 0) //input is PAL
		{
				WRITE_CBUS_REG(BT_VBIEND, 	22 | (22 << 16));		//field 0/1 VBI last line number
				WRITE_CBUS_REG(BT_VIDEOSTART, 	23 | (23 << 16));	//Line number of the first video start line in field 0/1.
				WRITE_CBUS_REG(BT_VIDEOEND , 	312 |          //  Line number of the last video line in field 1. added video end for avoid overflow.
								(312 <<16));					// Line number of the last video line in field 0
				WRITE_CBUS_REG(BT_CTRL ,	//(1 << BT_UPDATE_ST_SEL) |  //Update bt656 status register when start of frame.
								(1 << BT_COLOR_REPEAT) | //Repeated the color data when do 4:2:2 -> 4:4:4 data transfer.
								(1 << BT_AUTO_FMT ) |			//use haREAD_CBUS_REGware to check the PAL/NTSC format input format if it's standaREAD_CBUS_REG BT656 input format.
								(1 << BT_MODE_BIT     ) | // BT656 standaREAD_CBUS_REG interface.
								(1 << BT_EN_BIT       ) |    // enable BT moduale.
								(1 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
								(1 << BT_CLK27_SEL_BIT) |    // use external xclk27.
								(1 << BT_XCLK27_EN_BIT)) ;    // xclk27 is input.
		}
		else //if(am656in_dec_info.input_mode  == 1) //input is PAL	//input is NTSC
		{
				WRITE_CBUS_REG(BT_VBIEND, 	21 | (21 << 16));		//field 0/1 VBI last line number
				WRITE_CBUS_REG(BT_VIDEOSTART, 	18 | (18 << 16));	//Line number of the first video start line in field 0/1.
				WRITE_CBUS_REG(BT_VIDEOEND , 	257 |          //  Line number of the last video line in field 1. added video end for avoid overflow.
								(257 <<16));					// Line number of the last video line in field 0
				WRITE_CBUS_REG(BT_CTRL ,	//(1 << BT_UPDATE_ST_SEL) |  //Update bt656 status register when start of frame.
								(1 << BT_COLOR_REPEAT) | //Repeated the color data when do 4:2:2 -> 4:4:4 data transfer.
								(1 << BT_AUTO_FMT ) |		//use haREAD_CBUS_REGware to check the PAL/NTSC format input format if it's standaREAD_CBUS_REG BT656 input format.
								(1 << BT_MODE_BIT     ) | // BT656 standaREAD_CBUS_REG interface.
								(1 << BT_EN_BIT       ) |    // enable BT moduale.
								(1 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
								(1 << BT_CLK27_SEL_BIT) |    // use external xclk27.
								(1 << BT_XCLK27_EN_BIT) |		// xclk27 is input.
								(1 << BT_FMT_MODE_BIT));   //input format is NTSC
		}

		return;
}

//NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
static void reset_bt601in_dec(void)
{
		unsigned temp_data;
		// reset BT656in module.
		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data |= ( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data &= ~( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

        WRITE_CBUS_REG(BT_PORT_CTRL,    (0 << BT_IDQ_EN )   |     // use external idq pin.
                                                (1 << BT_IDQ_PHASE )   |
                                                ( 1 << BT_FID_HSVS ) |         // FID came from HS VS.
                                                ( 1 << BT_HSYNC_PHASE ) |
                                                (1 << BT_D8B )     |
                                                (4 << BT_FID_DELAY ) |
                                                (5 << BT_VSYNC_DELAY) |
                                                (5 << BT_HSYNC_DELAY));

        WRITE_CBUS_REG(BT_601_CTRL2 , ( 10 << 16));     // FID field check done point.

		WRITE_CBUS_REG(BT_SWAP_CTRL,	( 4 << 0 ) | // suppose the input bitstream format is Cb0 Y0 Cr0 Y1.
							( 5 << 4 ) |
							( 6 << 8 ) |
							( 7 << 13 ) );

        WRITE_CBUS_REG(BT_LINECTRL , ( 1 << 31 ) |   //software line ctrl enable.
                                    (1644 << 16 ) |    //1440 + 204
                                    220 )  ;

        // ANCI is the field blanking data, like close caption. If it connected to digital camara interface, the jpeg bitstream also use this ANCI FIFO.
        WRITE_CBUS_REG(BT_ANCISADR, am656in_dec_info.pbufAddr);
        WRITE_CBUS_REG(BT_ANCIEADR, am656in_dec_info.pbufAddr + BT656IN_ANCI_DATA_SIZE);

		WRITE_CBUS_REG(BT_AFIFO_CTRL,	(1 <<31) |     // load start and end address to afifo.
                        				(1 << 6) |     // fill _en;
                        				(1 << 3)) ;     // urgent

    	WRITE_CBUS_REG(BT_INT_CTRL ,   // (1 << 5) |    //ancififo done int.
//						(1 << 4) |    //SOF interrupt enable.
//						(1 << 3) |      //EOF interrupt enable.
						(1 << 1)); // |      //input overflow interrupt enable.
//						(1 << 0));      //bt656 controller error interrupt enable.
        WRITE_CBUS_REG(BT_ERR_CNT, (626 << 16) | (2000));
                                                                    //otherwise there is always error flag,
                                                                    //because the camera input use HREF ont HSYNC,
                                                                    //there are some lines without HREF sometime
		WRITE_CBUS_REG(BT_FIELDSADR, (1 << 16) | 1);	// field 0/1 start lcnt

		if(am656in_dec_info.input_mode == 2) //input is PAL
		{
				WRITE_CBUS_REG(BT_VBIEND, 22 | (22 << 16));		//field 0/1 VBI last line number
				WRITE_CBUS_REG(BT_VIDEOSTART, 23 | (23 << 16));	//Line number of the first video start line in field 0/1.
				WRITE_CBUS_REG(BT_VIDEOEND , 312 |          //  Line number of the last video line in field 1. added video end for avoid overflow.
                     							(312 <<16));					// Line number of the last video line in field 0
				WRITE_CBUS_REG(BT_CTRL ,	(0 << BT_MODE_BIT     ) |    // BT656 standaREAD_CBUS_REG interface.
                                            (1 << BT_AUTO_FMT )     |
                                            (1 << BT_EN_BIT       ) |    // enable BT moduale.
                                            (0 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
                                            (0 << BT_FMT_MODE_BIT ) |     //PAL
                                            (1 << BT_SLICE_MODE_BIT )|    // no ancillay flag.
                                            (0 << BT_FID_EN_BIT )   |     // use external fid pin.
                                            (1 << BT_CLK27_SEL_BIT) |  // use external xclk27.
                                            (1 << BT_XCLK27_EN_BIT) );   // xclk27 is input.
     }

		else //if(am656in_dec_info.input_mode == 3) 	//input is NTSC
		{
				WRITE_CBUS_REG(BT_VBIEND, 21 | (21 << 16));		//field 0/1 VBI last line number
				WRITE_CBUS_REG(BT_VIDEOSTART, 18 | (18 << 16));	//Line number of the first video start line in field 0/1.
				WRITE_CBUS_REG(BT_VIDEOEND , 257 |          //  Line number of the last video line in field 1. added video end for avoid overflow.
                     					(257 <<16));		// Line number of the last video line in field 0
				WRITE_CBUS_REG(BT_CTRL ,(0 << BT_MODE_BIT     ) |    // BT656 standaREAD_CBUS_REG interface.
                                        (1 << BT_AUTO_FMT )     |
                                        (1 << BT_EN_BIT       ) |    // enable BT moduale.
                                        (0 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
                                        (1 << BT_FMT_MODE_BIT ) |     // NTSC
                                        (1 << BT_SLICE_MODE_BIT )|    // no ancillay flag.
                                        (0 << BT_FID_EN_BIT )   |     // use external fid pin.
                                        (1 << BT_CLK27_SEL_BIT) |  // use external xclk27.
                                        (1 << BT_XCLK27_EN_BIT) );   // xclk27 is input.
      }
		return;
}

//CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
static void reset_camera_dec(void)
{
		unsigned temp_data;
		// reset BT656in module.
		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data |= ( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);

		temp_data = READ_CBUS_REG(BT_CTRL);
		temp_data &= ~( 1 << BT_SOFT_RESET );
		WRITE_CBUS_REG(BT_CTRL, temp_data);


        WRITE_CBUS_REG(BT_PORT_CTRL,    (0 << BT_IDQ_EN )   |     // use external idq pin.
                                        (0 << BT_IDQ_PHASE )   |
                                        ( 0 << BT_FID_HSVS ) |         // FID came from HS VS.
                                        ( 1 << BT_VSYNC_PHASE ) |
                                        (0 << BT_D8B )     |
                                        (4 << BT_FID_DELAY ) |
                                        (0 << BT_VSYNC_DELAY) |
                                        (0 << BT_HSYNC_DELAY));

        WRITE_CBUS_REG(BT_601_CTRL2 , ( 10 << 16));     // FID field check done point.

		WRITE_CBUS_REG(BT_SWAP_CTRL,	( 7 << 0 ) | // suppose the input bitstream format is Cb0 Y0 Cr0 Y1.
							( 4 << 4 ) |
							( 6 << 8 ) |
							( 5 << 13 ) );

        WRITE_CBUS_REG(BT_LINECTRL , ( 1 << 31 ) |   //software line ctrl enable.
                                    ((am656in_dec_info.active_pixel << 1)<< 16 ) |    //the number of active data per line
                                    0 )  ;

        // ANCI is the field blanking data, like close caption. If it connected to digital camara interface, the jpeg bitstream also use this ANCI FIFO.
        WRITE_CBUS_REG(BT_ANCISADR, am656in_dec_info.pbufAddr);
        WRITE_CBUS_REG(BT_ANCIEADR, am656in_dec_info.pbufAddr + BT656IN_ANCI_DATA_SIZE);

		WRITE_CBUS_REG(BT_AFIFO_CTRL,	(1 <<31) |     // load start and end address to afifo.
                        				(1 << 6) |     // fill _en;
                        				(1 << 3)) ;     // urgent

    	WRITE_CBUS_REG(BT_INT_CTRL ,   // (1 << 5) |    //ancififo done int.
//						(1 << 4) |    //SOF interrupt enable.
//						(1 << 3) |      //EOF interrupt enable.
						(1 << 1)); // |      //input overflow interrupt enable.
//						(1 << 0));      //bt656 controller error interrupt enable.
        WRITE_CBUS_REG(BT_ERR_CNT, ((2000) << 16) | (2000 * 10));   //total lines per frame and total pixel per line
                                                                    //otherwise there is always error flag,
                                                                    //because the camera input use HREF ont HSYNC,
                                                                    //there are some lines without HREF sometime

        WRITE_CBUS_REG(BT_FIELDSADR, (1 << 16) | 1);	// field 0/1 start lcnt
        WRITE_CBUS_REG(BT_VBIEND, 1 | (1 << 16));		//field 0/1 VBI last line number
        WRITE_CBUS_REG(BT_VIDEOSTART, 1 | (1 << 16));	//Line number of the first video start line in field 0/1.
        WRITE_CBUS_REG(BT_VIDEOEND , am656in_dec_info.active_line|          //  Line number of the last video line in field 1. added video end for avoid overflow.
                                    (am656in_dec_info.active_line <<16));					// Line number of the last video line in field 0
        WRITE_CBUS_REG(BT_CTRL ,	(1 << BT_EN_BIT       ) |    // enable BT moduale.
				                (0 << BT_REF_MODE_BIT ) |    // timing reference is from bit stream.
				                (0 << BT_FMT_MODE_BIT ) |     //PAL
				                (1 << BT_SLICE_MODE_BIT )|    // no ancillay flag.
                                (0 << BT_MODE_BIT     ) |    // BT656 standard interface.
                                (1 << BT_CLK27_SEL_BIT) |  // use external xclk27.
                                (0 << BT_FID_EN_BIT )   |     // use external fid pin.
                                (1 << BT_XCLK27_EN_BIT) |   // xclk27 is input.
                                (1 << BT_PROG_MODE  )   |
                                (0 << BT_AUTO_FMT    ) );

		return;
}


void set_next_field_656_601_camera_in_anci_address(unsigned char index)
{
		unsigned pbufAddr;
		pbufAddr = am656in_dec_info.pbufAddr + index * 0x200;
//		//set next field ANCI.
		WRITE_CBUS_REG(BT_ANCISADR, pbufAddr);
}

//input_mode is 0 or 1,         NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
//                              0:656--PAL ; 1:656--NTSC   ccir656 input
//input_mode is 2 or 3,         NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
//                              2:601--PAL ; 3:601--NTSC   ccir656 input
//input_mode is more than 3,    CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
//                              4:640x480 camera inout(progressive)
//                              5:800x600 camera inout(progressive)
//                              6:1024x768 camera inout(progressive)
//                              .....
//                              0xff: disable 656in/601/camera decode;
void start_amvdec_656_601_camera_in(unsigned char input_mode)
{
    if((am656in_dec_info.input_mode == 0xff) && (input_mode != 0xff))
    {
        init_656in_dec_parameter(input_mode);

        if(input_mode < 2)  //NTSC or PAL input(interlace mode): D0~D7(with SAV + EAV )
        {
            reset_bt656in_dec();
        }
        else if(input_mode < 4)
        {
            reset_bt601in_dec();
            if(am656in_dec_info.pin_mux_reg2 != 0)
                SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);  //set the related pin mux
        }
        else 
        {
            reset_camera_dec();
             if(am656in_dec_info.pin_mux_reg3 != 0)
                SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg3, am656in_dec_info.pin_mux_mask3);  //set the related pin mux
        }
        if(am656in_dec_info.pin_mux_reg1 != 0)
            SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg1, am656in_dec_info.pin_mux_mask1);  //set the related pin mux

    }
    return;
}


//input_mode is 0 or 1,         NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
//                              0:656--PAL ; 1:656--NTSC   ccir656 input
//input_mode is 2 or 3,         NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
//                              2:601--PAL ; 3:601--NTSC   ccir656 input
//input_mode is more than 3,    CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
//                              4:640x480 camera inout(progressive)
//                              5:800x600 camera inout(progressive)
//                              6:1024x768 camera inout(progressive)
//                              .....
//                              0xff: disable 656in/601/camera decode;
void stop_amvdec_656_601_camera_in(unsigned char input_mode)
{
    unsigned temp_data;
		// reset BT656in module.
    if(am656in_dec_info.input_mode != 0xff)
    {
        if(am656in_dec_info.pin_mux_reg1 != 0)
            CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg1, am656in_dec_info.pin_mux_mask1);

        if(am656in_dec_info.pin_mux_reg2 != 0)
            CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);

        if(am656in_dec_info.pin_mux_reg3 != 0)
            CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg3, am656in_dec_info.pin_mux_mask3);  //set the related pin mux

        temp_data = READ_CBUS_REG(BT_CTRL);
        temp_data &= ~( 1 << BT_EN_BIT );
        WRITE_CBUS_REG(BT_CTRL, temp_data);	//disable BT656 input

        //reset 656 input module
        temp_data = READ_CBUS_REG(BT_CTRL);
        temp_data |= ( 1 << BT_SOFT_RESET );
        WRITE_CBUS_REG(BT_CTRL, temp_data);
        temp_data = READ_CBUS_REG(BT_CTRL);
        temp_data &= ~( 1 << BT_SOFT_RESET );
        WRITE_CBUS_REG(BT_CTRL, temp_data);

        am656in_dec_info.input_mode = 0xff;
    }

    return;
}



#ifdef HANDLE_BT656IN_IRQ
static irqreturn_t bt656in_dec_irq(int irq, void *dev_id)
{

}
#endif

//bt656 flag error = (pixel counter error) | (line counter error) | (input FIFO over flow)
static void bt656_in_dec_run(vframe_t * info)
{
    unsigned ccir656_status, field_total_line;

    ccir656_status = READ_CBUS_REG(BT_STATUS);

    if(ccir656_status & 0x80)
    {
        pr_dbg("bt656 input FIFO over flow, reset \n");
        reset_bt656in_dec();
        return ;
    }

    field_total_line = READ_CBUS_REG(BT_VLINE_STATUS);
    field_total_line &= 0xfff;
    if((field_total_line < 200) || (field_total_line > 320))  //skip current field
    {
        pr_dbg("656 in total line is less than 200 \n");
        return;
    }

    else if((field_total_line < 264) && (ccir656_status & 0x4000) && (am656in_dec_info.input_mode != 1)) //current field total line number is 240
    {
        init_656in_dec_parameter(1);
        reset_bt656in_dec();
        return;
    }

    else if( (field_total_line >= 264) && (!(ccir656_status & 0x4000)) && (am656in_dec_info.input_mode != 0) )  //current field total line number is 288(0x100 + 0x20)
    {
        init_656in_dec_parameter(0);
        reset_bt656in_dec();
        return;
    }

    else
    {
        if(ccir656_status & 0x10)  // current field is field 1.
        {
            info->type = VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_BOTTOM;
        }
        else
        {
            info->type = VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_TOP;
        }

        if(am656in_dec_info.input_mode == 0)	//the data in the buffer is PAL
        {
            info->width= 720;
            info->height = 576;
            info->duration = 1920;
        }
        else
        {
            info->width= 720;
            info->height = 480;
            info->duration = 1600;
        }

    }
    return;
}

static void bt601_in_dec_run(vframe_t * info)
{
    unsigned ccir656_status, field_total_line;

    ccir656_status = READ_CBUS_REG(BT_STATUS);

    if(ccir656_status & 0x80)
    {
        pr_dbg("bt601 input FIFO over flow, reset \n");
        reset_bt601in_dec();
        return ;
    }

    field_total_line = READ_CBUS_REG(BT_VLINE_STATUS);
    field_total_line &= 0xfff;
    if((field_total_line < 200) || (field_total_line > 320))  //skip current field
    {
        pr_dbg("601 in total line is less than 200 \n");
        return;
    }

    else if((field_total_line < 264) && (ccir656_status & 0x4000) && (am656in_dec_info.input_mode != 1)) //current field total line number is 240
    {
        init_656in_dec_parameter(1);
        reset_bt601in_dec();
        return;
    }

    else if( (field_total_line >= 264) && (!(ccir656_status & 0x4000)) && (am656in_dec_info.input_mode != 0) )  //current field total line number is 288(0x100 + 0x20)
    {
        init_656in_dec_parameter(0);
        reset_bt601in_dec();
        return;
    }

    else
    {
        if(ccir656_status & 0x10)  // current field is field 1.
        {
            info->type = VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_BOTTOM;
        }
        else
        {
            info->type = VIDTYPE_VIU_422 | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_TOP;
        }

        if(am656in_dec_info.input_mode == 0)	//the data in the buffer is PAL
        {
            info->width= 720;
            info->height = 576;
            info->duration = 1920;
        }
        else
        {
            info->width= 720;
            info->height = 480;
            info->duration = 1600;
        }
    }

    return;

}

static void camera_in_dec_run(vframe_t * info)
{
    unsigned ccir656_status, field_total_line;

    ccir656_status = READ_CBUS_REG(BT_STATUS);

    if(ccir656_status & 0x80)
    {
        pr_dbg("camera input FIFO over flow, reset \n");
        reset_camera_dec();
        return ;
    }

    field_total_line = READ_CBUS_REG(BT_VLINE_STATUS);
    field_total_line &= 0xfff;
    if((field_total_line < 200) || (field_total_line > 320))  //skip current field
    {
        pr_dbg("601 in total line is less than 200 \n");
        return;
    }

    else
    {
        info->type = VIDTYPE_PROGRESSIVE | VIDTYPE_VIU_FIELD | VIDTYPE_INTERLACE_BOTTOM;
        info->width= am656in_dec_info.active_pixel;
        info->height = am656in_dec_info.active_line;
        info->duration = 96000/30;   //30 frame per second

    }

    return;
}

/*
    If info.type ( --reture value )is 0xffffffff, the current field is error

input_mode is 0 or 1,         NTSC or PAL input(interlace mode): CLOCK + D0~D7(with SAV + EAV )
                              0:656--PAL ; 1:656--NTSC   ccir656 input
input_mode is 2 or 3,         NTSC or PAL input(interlace mode): CLOCK + D0~D7 + HSYNC + VSYNC + FID
                              2:601--PAL ; 3:601--NTSC   ccir656 input
input_mode is more than 3,    CAMERA input(progressive mode): CLOCK + D0~D7 + HREF + VSYNC
                              4:640x480 camera inout(progressive)
                              5:800x600 camera inout(progressive)
                              6:1024x768 camera inout(progressive)
                              .....
                              0xff: disable 656in/601/camera decode;
*/
vframe_t * amvdec_656_601_camera_in_run(void)
{
    unsigned ccir656_status;
    vframe_t info = {
            0xffffffff,         //type
            0xffffffff,         //type_backup
            0,                  //blend_mode
            0,                  //recycle_by_di_pre
            1600,               //duration
            0,                  //duration_pulldown
            0,                  //pts
            0xff,               //canvas0Addr
            0xff,               //canvas1Addr
            1440,               //bufWidth
            720,                //width
            480,                //height
            0,                  //ratio_control
    };

    if(am656in_dec_info.input_mode == 0xff){
        printk("bt656in decoder is not started\n");
        return &info;
    }

    ccir656_status = READ_CBUS_REG(BT_STATUS);
    WRITE_CBUS_REG(BT_STATUS, ccir656_status | (1 << 9));	//WRITE_CBUS_REGite 1 to clean the SOF interrupt bit

    if(am656in_dec_info.input_mode < 2)  //NTSC or PAL input(interlace mode): D0~D7(with SAV + EAV )
    {
        bt656_in_dec_run(&info);
    }
    else if(am656in_dec_info.input_mode < 4)
    {
        bt601_in_dec_run(&info);
    }
    else
    {
        camera_in_dec_run(&info);
    }

    return &info;
}


//static int am656in_open(struct inode *node, struct file *file)
//{
//	 am656in_dev_t *bt656_in_devp;

    /* Get the per-device structure that contains this cdev */
//    bt656_in_devp = container_of(node->i_cdev, am656in_dev_t, cdev);
//    file->private_data = bt656_in_devp;

//	return 0;

//}


//static int am656in_release(struct inode *node, struct file *file)
//{
//    am656in_dev_t *bt656_in_devp = file->private_data;

    /* Reset file pointer */
//    bt656_in_devp->current_pointer = 0;

    /* Release some other fields */
    /* ... */
//    return 0;
//}



//static int am656in_ioctl(struct inode *node, struct file *file, unsigned int cmd,   unsigned long args)
//{
//	int   r = 0;
//	switch (cmd) {
//		case BT656_DECODERIO_START:
//                    start_bt656in_dec();
//			break;

//		case BT656_DECODERIO_STOP:
//                    stop_bt656in_dec();
//			break;

//            default:

//                    break;
//	}
//	return r;
//}

//const static struct file_operations am656in_fops = {
//    .owner    = THIS_MODULE,
//    .open     = am656in_open,
//    .release  = am656in_release,
//    .ioctl    = am656in_ioctl,
//};

static int amvdec_656in_probe(struct platform_device *pdev)
{
    int r, i;
//    unsigned pbufSize;
    struct resource *s;

    printk("amvdec_656in probe start.\n");



//    r = alloc_chREAD_CBUS_REGev_region(&am656in_id, 0, BT656IN_COUNT, DEVICE_NAME);
//    if (r < 0) {
//        pr_error("Can't register major for am656indec device\n");
//        return r;
//    }

//    am656in_class = class_create(THIS_MODULE, DEVICE_NAME);
//    if (IS_ERR(am656in_class))
//    {
//        unregister_chREAD_CBUS_REGev_region(am656in_id, BT656IN_COUNT);
//        return PTR_ERR(aoe_class);
//    }

//    cdev_init(&am656in_dev_->cdev, &am656in_fops);
//    &am656in_dev_->cdev.owner = THIS_MODULE;
//    cdev_add(&am656in_dev_->cdev, am656in_id, BT656IN_COUNT);

//    am656in_dev = device_create(am656in_class, NULL, am656in_id, "bt656indec%d", 0);
//		if (am656in_dev == NULL) {
//				pr_error("device_create create error\n");
//				class_destroy(am656in_class);
//				r = -EEXIST;
//				return r;
//		}

//    s = platform_get_resource(pdev, IORESOURCE_MEM, 0);
//    if(s != NULL)
//    {
//	        am656in_dec_info.pbufAddr = (unsigned )(s->start);
//	        pbufSize = (unsigned )(s->end) - am656in_dec_info.pbufAddr + 1;
//	        if(pbufSize < am656in_dec_info.pbufSize)
//	        	pr_error("there is not enough memory for 656 decode \n");
//     }
//     else
//             pr_error("error in getting resource parameters0 \n");

    s = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if(s != NULL)
    {
	        am656in_dec_info.pin_mux_mask1 = (unsigned )(s->start);
	        am656in_dec_info.pin_mux_reg1 = ((unsigned )(s->end) - am656in_dec_info.pin_mux_mask1 ) & 0xffff;
	        if(am656in_dec_info.pin_mux_reg1 != 0)
	        	SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg1, am656in_dec_info.pin_mux_mask1);
     }
     else
             pr_error("error in getting bt656 resource parameters \n");

    s = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    if(s != NULL)
    {
	        am656in_dec_info.pin_mux_mask2 = (unsigned )(s->start);
	        am656in_dec_info.pin_mux_reg2 = ((unsigned )(s->end) - am656in_dec_info.pin_mux_mask2 ) & 0xffff;
	        if(am656in_dec_info.pin_mux_reg2 != 0)
	        	SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);
     }
     else
             pr_error("error in getting bt601 resource parameters \n");

    s = platform_get_resource(pdev, IORESOURCE_MEM, 3);
    if(s != NULL)
    {
            am656in_dec_info.pin_mux_mask3 = (unsigned )(s->start);
            am656in_dec_info.pin_mux_reg3 = ((unsigned )(s->end) - am656in_dec_info.pin_mux_mask3 ) & 0xffff;
            if(am656in_dec_info.pin_mux_reg2 != 0)
                SET_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg3, am656in_dec_info.pin_mux_mask3);
     }
     else
             pr_error("error in getting camera resource parameters \n");

#ifdef HANDLE_BT656IN_IRQ
        if (request_irq(INT_BT656, bt656in_dec_irq,	IRQF_SHARED, BT656IN_IRQ_NAME, (void *)bt656in_dec_id)
        {
                printk("bt656in irq register error.\n");
                return -ENOENT;
        }
#endif


		printk("amvdec_656in probe end.\n");
		return r;
}

static int amvdec_656in_remove(struct platform_device *pdev)
{
    /* Remove the cdev */
#ifdef HANDLE_BT656IN_IRQ
        free_irq(INT_BT656,(void *)bt656in_dec_id);
#endif

//    cdev_del(&am656in_dev_->cdev);

//    device_destroy(am656in_class, am656in_id);

//    class_destroy(am656in_class);

//    unregister_chREAD_CBUS_REGev_region(am656in_id, BT656IN_COUNT);

	  if(am656in_dec_info.pin_mux_reg1 != 0)
	     CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg1, am656in_dec_info.pin_mux_mask1);

	  if(am656in_dec_info.pin_mux_reg2 != 0)
	     CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg2, am656in_dec_info.pin_mux_mask2);

	  if(am656in_dec_info.pin_mux_reg3 != 0)
	     CLEAR_CBUS_REG_MASK(am656in_dec_info.pin_mux_reg3, am656in_dec_info.pin_mux_mask3);
    return 0;
}



static struct platform_driver amvdec_656in_driver = {
    .probe      = amvdec_656in_probe,
    .remove     = amvdec_656in_remove,
    .driver     = {
        .name   = DEVICE_NAME,
    }
};

static int __init amvdec_656in_init_module(void)
{
    printk("amvdec_656in module init\n");
    if (platform_driver_register(&amvdec_656in_driver)) {
        printk("failed to register amvdec_656in driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit amvdec_656in_exit_module(void)
{
    printk("amvdec_656in module remove.\n");
    platform_driver_unregister(&amvdec_656in_driver);
    return ;
}



module_init(amvdec_656in_init_module);
module_exit(amvdec_656in_exit_module);
