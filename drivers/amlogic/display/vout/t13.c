/*
 * AMLOGIC T13 LCD panel driver.
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
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/vout/tcon.h>

#include <mach/gpio.h>
#include <mach/am_regs.h>
#include <mach/pinmux.h>
#include <mach/power_gate.h>

/*
For Ramos 6236M, Innolux AT070TN93 V.2 */ 
#define LCD_WIDTH       800 
#define LCD_HEIGHT      480
#define MAX_WIDTH       1056
#define MAX_HEIGHT      525
#define VIDEO_ON_LINE   17

static void t13_power_on(void);
static void t13_power_off(void);

static tcon_conf_t tcon_config =
{
    .width      = LCD_WIDTH,
    .height     = LCD_HEIGHT,
    .max_width  = MAX_WIDTH,
    .max_height = MAX_HEIGHT,
    .video_on_line = VIDEO_ON_LINE,
    .pll_ctrl = 0x0632,
    .clk_ctrl = 0x1fc1,
    .gamma_cntl_port = (1 << LCD_GAMMA_EN) | (0 << LCD_GAMMA_RVS_OUT) | (1 << LCD_GAMMA_VCOM_POL),
    .gamma_vcom_hswitch_addr = 0,
    .rgb_base_addr = 0xf0,
    .rgb_coeff_addr = 0x74a,
    .pol_cntl_addr = (0x0 << LCD_CPH1_POL) |(0x1 << LCD_HS_POL) | (0x1 << LCD_VS_POL),
    .dith_cntl_addr = 0x600,
    .sth1_hs_addr = 64,
    .sth1_he_addr = 73,
    .sth1_vs_addr = 0,
    .sth1_ve_addr = MAX_HEIGHT - 1,
    .sth2_hs_addr = 0,
    .sth2_he_addr = 0,
    .sth2_vs_addr = 0,
    .sth2_ve_addr = 0,
    .oeh_hs_addr = 67,
    .oeh_he_addr = 67+LCD_WIDTH-1,
    .oeh_vs_addr = VIDEO_ON_LINE,
    .oeh_ve_addr = VIDEO_ON_LINE+LCD_HEIGHT-1,
    .vcom_hswitch_addr = 0,
    .vcom_vs_addr = 0,
    .vcom_ve_addr = 0,
    .cpv1_hs_addr = 40,
    .cpv1_he_addr = 560,
    .cpv1_vs_addr = 0,
    .cpv1_ve_addr = MAX_HEIGHT - 1,
    .cpv2_hs_addr = 0,
    .cpv2_he_addr = 0,
    .cpv2_vs_addr = 0,
    .cpv2_ve_addr = 0,
    .stv1_hs_addr = 0,
    .stv1_he_addr = MAX_WIDTH - 1,
    .stv1_vs_addr = VIDEO_ON_LINE,
    .stv1_ve_addr = VIDEO_ON_LINE,
    .stv2_hs_addr = 0,
    .stv2_he_addr = 0,
    .stv2_vs_addr = 0,
    .stv2_ve_addr = 0,
    .oev1_hs_addr = 20,
    .oev1_he_addr = 66,
    .oev1_vs_addr = 0,
    .oev1_ve_addr = MAX_HEIGHT - 1,
    .oev2_hs_addr = 0,
    .oev2_he_addr = 0,
    .oev2_vs_addr = 0,
    .oev2_ve_addr = 0,
    .oev3_hs_addr = 0,
    .oev3_he_addr = 0,
    .oev3_vs_addr = 0,
    .oev3_ve_addr = 0,
    .inv_cnt_addr = (0<<LCD_INV_EN) | (0<<LCD_INV_CNT),
    .tcon_misc_sel_addr = (1<<LCD_STV1_SEL) | (1<<LCD_STV2_SEL),
    .dual_port_cntl_addr = (1<<LCD_TTL_SEL) | (1<<LCD_ANALOG_SEL_CPH3) | (1<<LCD_ANALOG_3PHI_CLK_SEL),
    .flags = 0,
    .screen_width = 5,
    .screen_height = 3,
    .sync_duration_num = 481,
    .sync_duration_den = 8,
    .power_on=t13_power_on,
    .power_off=t13_power_off,
};
static struct resource tcon_resources[] = {
    [0] = {
        .start = (ulong)&tcon_config,
        .end   = (ulong)&tcon_config + sizeof(tcon_conf_t) - 1,
        .flags = IORESOURCE_MEM,
    },
};

static void t13_setup_gama_table(tcon_conf_t *pConf)
{
    int i;
    const unsigned short gamma_adjust[256] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
        64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
        96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
        128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
        160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
        192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
        224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
    };

    for (i=0; i<256; i++) {
        pConf->GammaTableR[i] = gamma_adjust[i] << 2;
        pConf->GammaTableG[i] = gamma_adjust[i] << 2;
        pConf->GammaTableB[i] = gamma_adjust[i] << 2;
    }
}

void power_on_backlight(void)
{
    #ifdef CONFIG_MACH_MESON_8726M
    /* PIN31, GPIOA_7, Pull high, BL_PWM Enable*/
    //set_gpio_val(GPIOA_bank_bit(7), GPIOA_bit_bit0_14(7), 1);
    //set_gpio_mode(GPIOA_bank_bit(7), GPIOA_bit_bit0_14(7), GPIO_OUTPUT_MODE);
    SET_CBUS_REG_MASK(PERIPHS_PIN_MUX_2, (1<<31)); 
    SET_CBUS_REG_MASK(PWM_MISC_REG_AB, (1 << 0));         
    WRITE_CBUS_REG_BITS(PWM_PWM_A,40000,0,16);  //low
    WRITE_CBUS_REG_BITS(PWM_PWM_A,20000,16,16);  //hi 
    #else
    /* PIN31, GPIOA_8, Pull high, BL_PWM Enable*/
    set_gpio_val(GPIOA_bank_bit(8), GPIOA_bit_bit0_14(8), 1);
    set_gpio_mode(GPIOA_bank_bit(8), GPIOA_bit_bit0_14(8), GPIO_OUTPUT_MODE);
    #endif
    /* PIN28, GPIOA_6, Pull high, For En_5V */
    set_gpio_val(GPIOA_bank_bit(6), GPIOA_bit_bit0_14(6), 1);
    set_gpio_mode(GPIOA_bank_bit(6), GPIOA_bit_bit0_14(6), GPIO_OUTPUT_MODE);
}

void power_off_backlight(void)
{
    #ifdef CONFIG_MACH_MESON_8726M
    /* PIN31, GPIOA_7, Pull high, BL_PWM Enable*/
    set_gpio_val(GPIOA_bank_bit(7), GPIOA_bit_bit0_14(7), 0);
    set_gpio_mode(GPIOA_bank_bit(7), GPIOA_bit_bit0_14(7), GPIO_OUTPUT_MODE);
    #else
    /* PIN31, GPIOA_8, Pull low, BL_PWM Disable*/ 
    set_gpio_val(GPIOA_bank_bit(8), GPIOA_bit_bit0_14(8), 0);
    set_gpio_mode(GPIOA_bank_bit(8), GPIOA_bit_bit0_14(8), GPIO_OUTPUT_MODE);
    #endif

    /* PIN28, GPIOA_6, Pull low, For En_5v */
    //set_gpio_val(GPIOA_bank_bit(6), GPIOA_bit_bit0_14(6), 0);
    //set_gpio_mode(GPIOA_bank_bit(6), GPIOA_bit_bit0_14(6), GPIO_OUTPUT_MODE);
}

static void power_on_lcd(void)
{
    /* PIN165, GPIOC_4, Pull low, For LCD_3.3V */
    #ifdef CONFIG_MACH_MESON_8726M
    #else
    set_gpio_val(GPIOC_bank_bit0_26(4), GPIOC_bit_bit0_26(4), 0);
    set_gpio_mode(GPIOC_bank_bit0_26(4), GPIOC_bit_bit0_26(4), GPIO_OUTPUT_MODE);
    #endif
    #ifdef CONFIG_MACH_MESON_8726M
    /* PIN172, GPIOC_3, Pull high, For AVDD */
    set_gpio_val(GPIOC_bank_bit0_26(3), GPIOC_bit_bit0_26(3), 1);
    set_gpio_mode(GPIOC_bank_bit0_26(3), GPIOC_bit_bit0_26(3), GPIO_OUTPUT_MODE);
    #else
    /* PIN172, GPIOC_11, Pull high, For AVDD */
    set_gpio_val(GPIOC_bank_bit0_26(11), GPIOC_bit_bit0_26(11), 1);
    set_gpio_mode(GPIOC_bank_bit0_26(11), GPIOC_bit_bit0_26(11), GPIO_OUTPUT_MODE);
    #endif
}

static void power_off_lcd(void)
{
    #ifdef CONFIG_MACH_MESON_8726M
    /* PIN172, GPIOC_3, Pull high, For AVDD */
    set_gpio_val(GPIOC_bank_bit0_26(3), GPIOC_bit_bit0_26(3), 0);
    set_gpio_mode(GPIOC_bank_bit0_26(3), GPIOC_bit_bit0_26(3), GPIO_OUTPUT_MODE);
    #else
    /* PIN172, GPIOC_11, Pull low, For AVDD */
    set_gpio_val(GPIOC_bank_bit0_26(11), GPIOC_bit_bit0_26(11), 0);
    set_gpio_mode(GPIOC_bank_bit0_26(11), GPIOC_bit_bit0_26(11), GPIO_OUTPUT_MODE);
    #endif
    /* PIN165, GPIOC_4, Pull high, For LCD_3.3V */
    #ifdef CONFIG_MACH_MESON_8726M
    #else
    set_gpio_val(GPIOC_bank_bit0_26(4), GPIOC_bit_bit0_26(4), 1);
    set_gpio_mode(GPIOC_bank_bit0_26(4), GPIOC_bit_bit0_26(4), GPIO_OUTPUT_MODE); 
    #endif
}

static void set_tcon_pinmux(void)
{
    /* TCON control pins pinmux */
    
    /* GPIOD_12 --> U/D */
    #ifdef CONFIG_MACH_MESON_8726M
    set_gpio_val(GPIOC_bank_bit0_26(12), GPIOC_bit_bit0_26(12), 0);
    set_gpio_mode(GPIOC_bank_bit0_26(12), GPIOC_bit_bit0_26(12), GPIO_OUTPUT_MODE);
    #else
    set_gpio_val(GPIOD_bank_bit2_24(12), GPIOD_bit_bit2_24(12), 0);
    set_gpio_mode(GPIOD_bank_bit2_24(12), GPIOD_bit_bit2_24(12), GPIO_OUTPUT_MODE);
    #endif
    #ifdef CONFIG_MACH_MESON_8726M
    /* GPIOA_5 --> LCD_Clk, GPIOA_2 --> TCON_OEH, */
    set_mio_mux(0, 1<<11);
    set_mio_mux(0, 1<<14);

    /* RGB data pins */
    set_mio_mux(4,(0x3f<<0));   //For 8bits
//    set_mio_mux(4,(0x1<<0)|(1<<2)|(1<<4));   //For 6bits
    #else
    /* GPIOA_7 --> LCD_Clk, GPIOA_2 --> TCON_OEH, */
    set_mio_mux(0, 1<<9);
    set_mio_mux(0, 1<<14);

    /* RGB data pins */
    set_mio_mux(4,(1<<0)|(1<<2)|(1<<4));
    #endif
}
static void t13_power_on(void)
{
    video_dac_disable();
	set_tcon_pinmux();
	power_on_lcd();
	power_on_backlight();
      
}
static void t13_power_off(void)
{
	power_off_backlight();
    	power_off_lcd();
}

static void t13_io_init(void)
{
    printk("\n\nT13 LCD Init.\n\n");

    set_tcon_pinmux();

    power_on_lcd();
    power_on_backlight();
}

static struct platform_device tcon_dev = {
    .name = "tcon-dev",
    .id   = 0,
    .num_resources = ARRAY_SIZE(tcon_resources),
    .resource      = tcon_resources,
};

static int __init t13_init(void)
{
    t13_setup_gama_table(&tcon_config);
    t13_io_init();

    platform_device_register(&tcon_dev);

    return 0;
}

static void __exit t13_exit(void)
{
    power_off_backlight();
    power_off_lcd();

    platform_device_unregister(&tcon_dev);
}

subsys_initcall(t13_init);
//module_init(t13_init);
module_exit(t13_exit);

MODULE_DESCRIPTION("AMLOGIC T13 LCD panel driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tim Yao <timyao@amlogic.com>");



