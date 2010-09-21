/*
 * Amlogic osd
 * frame buffer driver
 *
 * Copyright (C) 2009 Amlogic, Inc.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:  Tim Yao <timyao@amlogic.com>
 *		   
 *		    jianfeng_wang : add ge2d support 09/05/21	
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/spinlock.h>
#include <asm/cacheflush.h>
#include <mach/am_regs.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/osd/osd_main.h>
#include <linux/osd/osd_dev.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "osd_log.h"
#include <linux/amlog.h>

MODULE_AMLOG(AMLOG_DEFAULT_LEVEL, 0, LOG_LEVEL_DESC, LOG_MASK_DESC);

static myfb_dev_t  *gp_fbdev_list[OSD_COUNT]={NULL,NULL};

static DEFINE_MUTEX(dbg_mutex);


static void __init
_fbdev_set_default(struct myfb_dev *fbdev,int index )
{
    	/* setup default value */
	fbdev->fb_info->var = mydef_var[index];
	fbdev->fb_info->fix = mydef_fix;
	fbdev->bpp_type=fbdev->fb_info->var.bits_per_pixel ;
	osddev_set(fbdev);
}

bpp_color_bit_define_t*	
_find_color_format(int  bpp)
{
	return (bpp_color_bit_define_t*)&default_color_format_array[bpp];
}
static int
osd_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct fb_fix_screeninfo *fix;
    	struct myfb_dev *fbdev=( struct myfb_dev*)info->par;
	bpp_color_bit_define_t   *color_format_pt;

    	fix = &info->fix;
	color_format_pt=_find_color_format(var->bits_per_pixel);	
	if (color_format_pt->type_index==0)
	{
		return -EFAULT ;
	}
	amlog_mask_level(LOG_MASK_PARA,LOG_LEVEL_LOW,"select color format :index%d,bpp %d\r\n",color_format_pt->type_index, \
												color_format_pt->bpp) ;
	fbdev->bpp_type=var->bits_per_pixel ;
	var->red.offset = color_format_pt->red_offset;
	var->red.length = color_format_pt->red_length;
	var->red.msb_right= color_format_pt->red_msb_right ;
	var->green.offset  = color_format_pt->green_offset;
	var->green.length  = color_format_pt->green_length;
	var->green.msb_right = color_format_pt->green_msb_right;
	var->blue.offset   = color_format_pt->blue_offset;
	var->blue.length   = color_format_pt->blue_length;
	var->blue.msb_right = color_format_pt->blue_msb_right;
	var->transp.offset= color_format_pt->transp_offset ;
	var->transp.length = color_format_pt->transp_length ;
	var->transp.msb_right = color_format_pt->transp_msb_right ;
	var->bits_per_pixel=color_format_pt->bpp ;
	fix->visual=color_format_pt->color_type ;
	//adjust memory length.	
 	fix->line_length = var->xres_virtual*var->bits_per_pixel/8;
	if(var->xres_virtual*var->yres_virtual*var->bits_per_pixel/8> fbdev->fb_len )
	{
		amlog_mask_level(LOG_MASK_PARA,LOG_LEVEL_HIGH,"no enough memory for %d*%d*%d\r\n",var->xres,var->yres,var->bits_per_pixel);
		return  -ENOMEM;
	}
    	if (var->xres_virtual < var->xres)
        var->xres_virtual = var->xres;

    	if (var->yres_virtual < var->yres)
        var->yres_virtual = var->yres;

    	var->left_margin = var->right_margin = var->upper_margin = var->lower_margin = 0;
    

	if (var->xres + var->xoffset > var->xres_virtual)
	var->xoffset = var->xres_virtual - var->xres;
	if (var->yres + var->yoffset > var->yres_virtual)
	var->yoffset = var->yres_virtual - var->yres;
    
    	return 0;
}

static int
osd_set_par(struct fb_info *info)
{
	osddev_set((struct myfb_dev *)info->par);
       return  0;
}

static int
osd_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue,
        unsigned transp, struct fb_info *info)
{
    return osddev_setcolreg(regno, red, green, blue,
                        transp, (struct myfb_dev *)info->par);
}

static int
osd_setcmap(struct fb_cmap *cmap, struct fb_info *info)
{
	int count, index, r;
	u16 *red, *green, *blue, *transp;
	u16 trans = 0xffff;

	red     = cmap->red;
	green   = cmap->green;
	blue    = cmap->blue;
	transp  = cmap->transp;
	index   = cmap->start;

	for (count = 0; count < cmap->len; count++) {
		if (transp)
			trans = *transp++;
		r = osddev_setcolreg(index++, *red++, *green++, *blue++, trans,
				(struct myfb_dev *)info->par);
		if (r != 0)
			return r;
	}

	return 0;
}

static  bool   check_cmd_support(unsigned int cmd)
{
   	 return   	(cmd == FBIOPUT_OSD_SRCCOLORKEY) ||
           		(cmd == FBIOPUT_OSD_SRCKEY_ENABLE) ||
           		(cmd == FBIOPUT_OSD_SET_GBL_ALPHA)||
           		(cmd == FBIOGET_OSD_GET_GBL_ALPHA);
       
}
static int
osd_ioctl(struct fb_info *info, unsigned int cmd,
               unsigned long arg)
{
	 struct myfb_dev *fbdev = (struct myfb_dev *)info->par;
	 void __user *argp = (void __user *)arg;
   	 u32  src_colorkey;//16 bit or 24 bit 
   	 u32  srckey_enable;
	 u32  gbl_alpha;

	if (! check_cmd_support(cmd))
    	{
     	   amlog_mask_level(LOG_MASK_IOCTL,LOG_LEVEL_HIGH,"command not supported\r\n ");
     	   return -1;
    	}
    	switch (cmd)
  	{
   		case  FBIOPUT_OSD_SRCKEY_ENABLE:
			copy_from_user(&srckey_enable,argp,sizeof(u32));
			break;
   		case  FBIOPUT_OSD_SRCCOLORKEY:
			copy_from_user(&src_colorkey,argp,sizeof(u32));
			break ;
		case FBIOPUT_OSD_SET_GBL_ALPHA:
			copy_from_user(&gbl_alpha,argp,sizeof(u32));
			break;
		case FBIOGET_OSD_GET_GBL_ALPHA:
			break;
		default :
			return -1;
	}
	mutex_lock(&fbdev->lock);

  	switch (cmd)
    	{
    		case FBIOPUT_OSD_SRCCOLORKEY:
	    	switch(fbdev->bpp_type)
	  	{
	 		case BPP_TYPE_16_655:
			case BPP_TYPE_16_844:
			case BPP_TYPE_16_565:
			case BPP_TYPE_24_888_B:
			case BPP_TYPE_24_RGB:
			case BPP_TYPE_YUV_422:
	  	   	amlog_mask_level(LOG_MASK_IOCTL,LOG_LEVEL_LOW,"set osd color key 0x%x\r\n",src_colorkey);
	  	 	osddev_set_colorkey(info->node,fbdev->bpp_type,src_colorkey);
			break;
			default: break;
	  	}
	   	break ;
	  	case FBIOPUT_OSD_SRCKEY_ENABLE:
	   	switch(fbdev->bpp_type)
	  	{
	 		case BPP_TYPE_16_655:
			case BPP_TYPE_16_844:
			case BPP_TYPE_16_565:
			case BPP_TYPE_24_888_B:
			case BPP_TYPE_24_RGB:
			case BPP_TYPE_YUV_422:	
			amlog_mask_level(LOG_MASK_IOCTL,LOG_LEVEL_LOW,"set osd color key %s\r\n",srckey_enable?"enable":"disable");
		   	osddev_srckey_enable(info->node,srckey_enable!=0?1:0);	
			break;
			default:break;
	 	}
	   	break;
	 	case FBIOPUT_OSD_SET_GBL_ALPHA:
	 	osddev_set_gbl_alpha(info->node,gbl_alpha);
	 	break;
	 	case  FBIOGET_OSD_GET_GBL_ALPHA:
	 	gbl_alpha=osddev_get_gbl_alpha(info->node);
	 	copy_to_user(argp, &gbl_alpha, sizeof(u32));
	  	break;
    	}

   	mutex_unlock(&fbdev->lock);
	
	return  0;
}
static int osd_open(struct fb_info *info, int arg)
{
	return 0;
	
}


static int
osd_blank(int blank_mode, struct fb_info *info)
{
 	amlog_mask_level(LOG_MASK_PARA,LOG_LEVEL_LOW,"osd%d\r\n"	,info->node);
    	osddev_enable((blank_mode != 0) ? 0 : 1,info->node);

    	return 0;
}

static int osd_pan_display(struct fb_var_screeninfo *var,
                        struct fb_info *fbi)
{
	
    	osddev_pan_display((struct myfb_dev *)fbi->par);
	amlog_mask_level(LOG_MASK_PARA,LOG_LEVEL_LOW,"osd_pan_display:=>osd%d\r\n",fbi->node);
	return 0;
}

#if defined(CONFIG_FB_OSD2_CURSOR)
static int osd_cursor(struct fb_info *fbi, struct fb_cursor *var)
{
	osddev_cursor((struct myfb_dev *)fbi->par, var->hot.x, var->hot.y);
	return 0;
}
#endif

static int osd_sync(struct fb_info *info)
{
	return 0;
}


/* fb_ops structures */
static struct fb_ops osd_ops = {
	.owner          = THIS_MODULE,
	.fb_check_var   = osd_check_var,
	.fb_set_par     = osd_set_par,
	.fb_setcolreg   = osd_setcolreg,
	.fb_setcmap     = osd_setcmap,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
#ifdef CONFIG_FB_SOFT_CURSOR
	.fb_cursor      = soft_cursor,
#elif defined(CONFIG_FB_OSD2_CURSOR)
	.fb_cursor      = osd_cursor,
#endif
	.fb_ioctl       = osd_ioctl,
	.fb_open        = osd_open,
	.fb_blank       = osd_blank,
	.fb_pan_display = osd_pan_display,
	.fb_sync        = osd_sync,
};

int osd_notify_callback(struct notifier_block *block, unsigned long cmd , void *para)
{
	const vinfo_t *vinfo;
	myfb_dev_t *fb_dev;
	int  i,blank;

	vinfo = get_current_vinfo();
	amlog_mask_level(LOG_MASK_PARA,LOG_LEVEL_LOW,"tv_server:vmode=%s\r\n", vinfo->name);

	switch(cmd)
	{
		case  VOUT_EVENT_MODE_CHANGE:
		amlog_mask_level(LOG_MASK_PARA,LOG_LEVEL_LOW,"recevie change mode  message \r\n");
		for(i=0;i<OSD_COUNT;i++)
		{
			if(NULL==(fb_dev=gp_fbdev_list[i])) continue;
			osddev_set(fb_dev);
		}
		break;

		case VOUT_EVENT_OSD_BLANK:
		blank=*(int*)para ;	
		for(i=0;i<OSD_COUNT;i++)
		{
			if(NULL==(fb_dev=gp_fbdev_list[i])) continue;
			osd_blank(blank,fb_dev->fb_info);
		}
		break;
	}
	return 0;
}

static struct notifier_block osd_notifier_nb = {
	.notifier_call	= osd_notify_callback,
};


static int __init
osd_probe(struct platform_device *pdev)
{
	int r;
       	struct fb_info *fbi=NULL;
    	struct fb_var_screeninfo *var;
  	struct fb_fix_screeninfo *fix;
	struct resource *mem;
	int  index,Bpp;
	logo_object_t  *init_logo_obj=NULL;
	int  logo_osd_index=0;
	myfb_dev_t 	*fbdev = NULL;
	
	

	
	vout_register_client(&osd_notifier_nb);

#ifdef CONFIG_AM_LOGO
	init_logo_obj = get_current_logo_obj();
	if( init_logo_obj )
	{
		if(init_logo_obj->para.output_dev_type<LOGO_DEV_VID) //osd0 or osd1
		{
			logo_osd_index= init_logo_obj->para.output_dev_type;
		}else{
			init_logo_obj=NULL; //if logo device on video layer ,
		}					 //we cant use it .
		
	}
#endif

   	if (NULL==init_logo_obj )
    	{
#ifdef CONFIG_AM_TCON_OUTPUT
		set_current_vmode(VMODE_LCD);
#else
    		set_current_vmode(VMODE_720P);	
#endif
    	}
    	for (index=0;index<OSD_COUNT;index++)
    	{
    		//platform resource 
		if (!(mem = platform_get_resource(pdev, IORESOURCE_MEM, index)))
		{
			amlog_level(LOG_LEVEL_HIGH,"No frame buffer memory define.\n");
			r = -EFAULT;
			goto failed2;
		}
		//if we have no resource then no need to create this device.
		amlog_level(LOG_LEVEL_HIGH,"[osd%d] 0x%x-0x%x\n",index,mem->start,mem->end);
		if (!mem || mem->start== 0 || mem->end==0 || mem->start==mem->end)
		{
			continue ;
		}
		
    		fbi = framebuffer_alloc(sizeof(struct myfb_dev), &pdev->dev);
    		if(!fbi)
    		{
        		r = -ENOMEM;
        		goto failed1;
    		}
	
		fbdev = (struct myfb_dev *)fbi->par;
		fbdev->fb_info = fbi;
		fbdev->dev = pdev;
 	
		mutex_init(&fbdev->lock);

    		var = &fbi->var;
    		fix = &fbi->fix;

		
		gp_fbdev_list[index]=fbdev;
		fbdev->fb_mem_paddr = mem->start;
		fbdev->fb_len = mem->end - mem->start + 1;
		fbdev->fb_mem_vaddr = ioremap_wc(fbdev->fb_mem_paddr, fbdev->fb_len);

		if (!fbdev->fb_mem_vaddr)
		{
			amlog_level(LOG_LEVEL_HIGH,"failed to ioremap framebuffer\n");
        		r = -ENOMEM;
        		goto failed3;
		}
	
		//clear framebuffer memory
		 amlog_level(LOG_LEVEL_HIGH,"Frame buffer memory assigned at phy:0x%08x, vir:0x%p, size=%dK\n",
	    	fbdev->fb_mem_paddr, fbdev->fb_mem_vaddr, fbdev->fb_len >> 10);
		 

		if(init_logo_obj && index==logo_osd_index ) //adjust default var info
		{
			int  bpp=init_logo_obj->dev->output_dev.osd.color_depth;//bytes per pixel
		
			mydef_var[index].xres=init_logo_obj->dev->vinfo->width;
			mydef_var[index].yres=init_logo_obj->dev->vinfo->height;	
			mydef_var[index].xres_virtual=init_logo_obj->dev->vinfo->width;
			mydef_var[index].yres_virtual=init_logo_obj->dev->vinfo->height<<1;//logo always use double buffer
			mydef_var[index].bits_per_pixel=bpp ;
			amlog_level(LOG_LEVEL_HIGH,"init fbdev bpp is :%d\r\n",mydef_var[index].bits_per_pixel);
			
			if(mydef_var[index].bits_per_pixel>32) 
			{
				mydef_var[index].bits_per_pixel=32;
			}
		}else{
			mydef_var[index].xres=1280;
			mydef_var[index].yres=720;
			mydef_var[index].xres_virtual=1280;
			mydef_var[index].yres_virtual=1440;
			mydef_var[index].bits_per_pixel=16;
			memset((char*)fbdev->fb_mem_vaddr, 0, fbdev->fb_len);	
		}
	
		_fbdev_set_default(fbdev,index);
		Bpp=(fbdev->bpp_type >8?(fbdev->bpp_type>16?(fbdev->bpp_type>24?4:3):2):1);
		fix->line_length=var->xres_virtual*Bpp;
		fix->smem_start = fbdev->fb_mem_paddr;
		fix->smem_len = fbdev->fb_len;

		if (fb_alloc_cmap(&fbi->cmap, 16, 0) != 0) {
			amlog_level(LOG_LEVEL_HIGH,"unable to allocate color map memory\n");
      		r = -ENOMEM;
        	goto failed3;
    		}

		if (!(fbi->pseudo_palette = kmalloc(sizeof(u32) * 16, GFP_KERNEL))) {
			amlog_level(LOG_LEVEL_HIGH,"unable to allocate pseudo palette memory\n");
        	r = -ENOMEM;
        	goto failed4;
		}

		memset(fbi->pseudo_palette, 0, sizeof(u32) * 16);

	   	fbi->fbops = &osd_ops;
    		fbi->screen_base = (char __iomem *)fbdev->fb_mem_vaddr ;
		fbi->screen_size = fix->smem_len;
	
		osd_check_var(var, fbi);
    		register_framebuffer(fbi);
		if(NULL==init_logo_obj )//if we have init a logo object ,then no need to setup hardware . 
		{
			osddev_set(fbdev);
		}
		
   	}	

	index=0;

	amlog_level(LOG_LEVEL_HIGH,"osd probe ok  \r\n");
	return 0;

failed4:
	fb_dealloc_cmap(&fbi->cmap);
failed3:
	dev_set_drvdata(&pdev->dev, NULL);
failed2:
    	unregister_framebuffer(fbi);
failed1:
    	amlog_level(LOG_LEVEL_HIGH,"Driver module insert failed.\n");
   	return r;
}

static int
osd_remove(struct platform_device *pdev)
{
   	struct fb_info *fbi;
	int i=0;

    	amlog_level(LOG_LEVEL_HIGH,"osd_remove.\n");
	if (!pdev)
		return -ENODEV;
	
	vout_unregister_client(&osd_notifier_nb);
	
	
	for(i=0;i<OSD_COUNT;i++)
	{
		if(gp_fbdev_list[i])
		{
			myfb_dev_t * fbdev=gp_fbdev_list[i];

			fbi = fbdev->fb_info;
			iounmap(fbdev->fb_mem_vaddr);
      			kfree(fbi->pseudo_palette);
     			fb_dealloc_cmap(&fbi->cmap);
         		unregister_framebuffer(fbi);
			framebuffer_release(fbi);
		}
	}
	return 0;
}

#ifndef MODULE

/* Process kernel command line parameters */
static int __init
osd_setup_attribute(char *options)
{
    char *this_opt = NULL;
    int r = 0;

    if (!options || !*options)
        goto exit;

    while (!r && (this_opt = strsep(&options, ",")) != NULL) {
        if (!strncmp(this_opt, "xres:", 5))
            mydef_var[0].xres = mydef_var[0].xres_virtual = simple_strtoul(this_opt + 5, NULL, 0);
        else if (!strncmp(this_opt, "yres:", 5))
            mydef_var[0].yres = mydef_var[0].yres_virtual = simple_strtoul(this_opt + 5, NULL, 0);
        else {
            amlog_level(LOG_LEVEL_HIGH,"invalid option\n");
            r = -1;
        }
    }
exit:
    return r;
}

#endif

#ifdef  CONFIG_PM
static int osd_suspend(struct platform_device *pdev, pm_message_t state)
{
	osddev_suspend();
	return 0;
}

static int osd_resume(struct platform_device *pdev)
{
	osddev_resume();
	return 0;
}


#endif 


/****************************************/

static struct platform_driver
osd_driver = {
    .probe      = osd_probe,
    .remove     = osd_remove,
#ifdef  CONFIG_PM      
    .suspend  =osd_suspend,
    .resume    =osd_resume,
#endif    
    .driver     = {
        .name   = "mesonfb",
    }
};

static int __init
osd_init_module(void)
{
	amlog_level(LOG_LEVEL_HIGH,"osd_init\n");

#ifndef MODULE
    {
      	char *option;
      if (fb_get_options("mesonfb", &option)) {
           return -ENODEV;
   	}
	  
   	osd_setup_attribute(option);
    }
#endif

    if (platform_driver_register(&osd_driver)) {
        amlog_level(LOG_LEVEL_HIGH,"failed to register osd driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit
osd_remove_module(void)
{
    amlog_level(LOG_LEVEL_HIGH,"osd_remove_module.\n");

    platform_driver_unregister(&osd_driver);
}

/****************************************/
module_init(osd_init_module);
module_exit(osd_remove_module);

MODULE_DESCRIPTION("AMLOGIC framebuffer driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tim Yao <timyao@amlogic.com>");