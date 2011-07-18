/*
 *
 * arch/arm/mach-meson/meson.c
 *
 *  Copyright (C) 2010 AMLOGIC, INC.
 *
 * License terms: GNU General Public License (GPL) version 2
 * Platform machine definition.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/device.h>
#include <linux/spi/flash.h>
#include <mach/hardware.h>
#include <mach/platform.h>
#include <mach/memory.h>
#include <mach/clock.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>
#include <mach/lm.h>
#include <asm/memory.h>
#include <asm/mach/map.h>
#include <mach/nand.h>
#include <linux/i2c.h>
#include <linux/i2c-aml.h>
#include <mach/power_gate.h>
#include <linux/aml_bl.h>
#include <linux/delay.h>
#include <mach/usbclock.h>

#ifdef CONFIG_AM_UART_WITH_S_CORE
#include <linux/uart-aml.h>
#endif
#include <mach/card_io.h>
#include <mach/pinmux.h>
#include <mach/gpio.h>
#include <linux/delay.h>
#include <mach/clk_set.h>
#include "board-m3-refe00.h"

#if defined(CONFIG_TOUCHSCREEN_ADS7846)
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/ads7846.h>
#endif

#ifdef CONFIG_ANDROID_PMEM
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/android_pmem.h>
#endif

#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android_composite.h>
#endif

#ifdef CONFIG_SENSORS_MXC622X
#include <linux/mxc622x.h>
#endif

#ifdef CONFIG_SENSORS_MMC31XX
#include <linux/mmc31xx.h>
#endif

#ifdef CONFIG_AMLOGIC_PM
#include <linux/power_supply.h>
#include <linux/aml_power.h>
#endif

#ifdef CONFIG_TCA6424
#include <linux/tca6424.h>
#endif

#ifdef CONFIG_SUSPEND
#include <mach/pm.h>
#endif

#if defined(CONFIG_JPEGLOGO)
static struct resource jpeglogo_resources[] = {
    [0] = {
        .start = CONFIG_JPEGLOGO_ADDR,
        .end   = CONFIG_JPEGLOGO_ADDR + CONFIG_JPEGLOGO_SIZE - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = CODEC_ADDR_START,
        .end   = CODEC_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device jpeglogo_device = {
    .name = "jpeglogo-dev",
    .id   = 0,
    .num_resources = ARRAY_SIZE(jpeglogo_resources),
    .resource      = jpeglogo_resources,
};
#endif

#ifdef CONFIG_SARADC_AM
#include <linux/saradc.h>
static struct platform_device saradc_device = {
    .name = "saradc",
    .id = 0,
    .dev = {
        .platform_data = NULL,
    },
};
#endif

#if defined(CONFIG_KEYPADS_AM)||defined(CONFIG_KEYPADS_AM_MODULE)
static struct resource intput_resources[] = {
    {
        .start = 0x0,
        .end = 0x0,
        .name = "8726",
        .flags = IORESOURCE_IO,
    },
};

static struct platform_device input_device = {
    .name = "m1-kp",
    .id = 0,
    .num_resources = ARRAY_SIZE(intput_resources),
    .resource = intput_resources,

};
#endif

#if defined(CONFIG_ADC_KEYPADS_AM)||defined(CONFIG_ADC_KEYPADS_AM_MODULE)
#include <linux/input.h>
#include <linux/adc_keypad.h>

static struct adc_key adc_kp_key[] = {// android\rootfs\device\amlogic\m1ref\aml-usbkbd.kl
    {KEY_MENU,  "menu", CHAN_4, 0, 60},//KEY_PAGEUP=15=BACK in aml-usbkbd.kl
    {KEY_UP,  "up", CHAN_4, 179, 60},//KEY_PAGEUP=104=VOLUME_UP in aml-usbkbd.kl
    {KEY_DOWN, "down", CHAN_4, 285, 60},//KEY_PAGEUP=217=VOLUME_DOWN in aml-usbkbd.kl
    {KEY_LEFT , "left", CHAN_4, 400, 60},
    {KEY_RIGHT, "right", CHAN_4, 507, 60},//KEY_PAGEUP=102=HOME in aml-usbkbd.kl
    {KEY_ESC, "exit", CHAN_4, 623, 60},//KEY_LEFTMETA=125=SEARCH in aml-usbkbd.kl
    {KEY_ENTER, "ok", CHAN_4, 851, 60},//KEY_LEFTMETA=125=SEARCH in aml-usbkbd.kl
};

static struct adc_kp_platform_data adc_kp_pdata = {
    .key = &adc_kp_key[0],
    .key_num = ARRAY_SIZE(adc_kp_key),
};

static struct platform_device input_device_adc = {
    .name = "m1-adckp",
    .id = 0,
    .num_resources = 0,
    .resource = NULL,
    .dev = {
        .platform_data = &adc_kp_pdata,
    }
};
#endif

#if defined(CONFIG_KEY_INPUT_CUSTOM_AM) || defined(CONFIG_KEY_INPUT_CUSTOM_AM_MODULE)
#include <linux/input.h>
#include <linux/input/key_input.h>

int _key_code_list[] = {KEY_POWER};

static inline int key_input_init_func(void)
{
    WRITE_CBUS_REG(0x21d0/*RTC_ADDR0*/, (READ_CBUS_REG(0x21d0/*RTC_ADDR0*/) &~(1 << 11)));
    WRITE_CBUS_REG(0x21d1/*RTC_ADDR0*/, (READ_CBUS_REG(0x21d1/*RTC_ADDR0*/) &~(1 << 3)));
}
static inline int key_scan(int *key_state_list)
{
    int ret = 0;
    key_state_list[0] = ((READ_CBUS_REG(0x21d1/*RTC_ADDR1*/) >> 2) & 1) ? 0 : 1;
    return ret;
}

static  struct key_input_platform_data  key_input_pdata = {
    .scan_period = 20,
    .fuzz_time = 60,
    .key_code_list = &_key_code_list[0],
    .key_num = ARRAY_SIZE(_key_code_list),
    .scan_func = key_scan,
    .init_func = key_input_init_func,
    .config = 0,
};

static struct platform_device input_device_key = {
    .name = "m1-keyinput",
    .id = 0,
    .num_resources = 0,
    .resource = NULL,
    .dev = {
        .platform_data = &key_input_pdata,
    }
};
#endif

#if defined(CONFIG_FB_AM)
static struct resource fb_device_resources[] = {
    [0] = {
        .start = OSD1_ADDR_START,
        .end   = OSD1_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
#if defined(CONFIG_FB_OSD2_ENABLE)
    [1] = {
        .start = OSD2_ADDR_START,
        .end   = OSD2_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
#endif
};

static struct platform_device fb_device = {
    .name       = "mesonfb",
    .id         = 0,
    .num_resources = ARRAY_SIZE(fb_device_resources),
    .resource      = fb_device_resources,
};
#endif

#if defined(CONFIG_AMLOGIC_SPI_NOR)
static struct mtd_partition spi_partition_info[] = {
    /* Hide uboot partition
            {
                    .name = "uboot",
                    .offset = 0,
                    .size = 0x3e000,
            },
    //*/
    {
        .name = "ubootenv",
        .offset = 0x3e000,
        .size = 0x2000,
    },
    /* Hide recovery partition
            {
                    .name = "recovery",
                    .offset = 0x40000,
                    .size = 0x1c0000,
            },
    //*/
};

static struct flash_platform_data amlogic_spi_platform = {
    .parts = spi_partition_info,
    .nr_parts = ARRAY_SIZE(spi_partition_info),
};

static struct resource amlogic_spi_nor_resources[] = {
    {
        .start = 0xc1800000,
        .end = 0xc1ffffff,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device amlogic_spi_nor_device = {
    .name = "AMLOGIC_SPI_NOR",
    .id = -1,
    .num_resources = ARRAY_SIZE(amlogic_spi_nor_resources),
    .resource = amlogic_spi_nor_resources,
    .dev = {
        .platform_data = &amlogic_spi_platform,
    },
};
#endif

#ifdef CONFIG_USB_DWC_OTG_HCD
static void set_usb_a_vbus_power(char is_power_on)
{
	/* USB a power is controled by GPIOD_09*/
#define USB_A_POW_GPIO			       GPIOD_bank_bit0_9(9)
#define USB_A_POW_GPIO_BIT		GPIOD_bit_bit0_9(9)
#define USB_A_POW_GPIO_BIT_ON   1
#define USB_A_POW_GPIO_BIT_OFF  0
    if (is_power_on) {
        printk(KERN_INFO "set usb port power on (board gpio %d)!\n", USB_A_POW_GPIO_BIT);
        set_gpio_mode(USB_A_POW_GPIO, USB_A_POW_GPIO_BIT, GPIO_OUTPUT_MODE);
        set_gpio_val(USB_A_POW_GPIO, USB_A_POW_GPIO_BIT, USB_A_POW_GPIO_BIT_ON);
    } else    {
        printk(KERN_INFO "set usb port power off (board gpio %d)!\n", USB_A_POW_GPIO_BIT);
        set_gpio_mode(USB_A_POW_GPIO, USB_A_POW_GPIO_BIT, GPIO_OUTPUT_MODE);
        set_gpio_val(USB_A_POW_GPIO, USB_A_POW_GPIO_BIT, USB_A_POW_GPIO_BIT_OFF);
    }
}
//usb_a is OTG port
static struct lm_device usb_ld_a = {
    .type = LM_DEVICE_TYPE_USB,
    .id = 0,
    .irq = INT_USB_A,
    .resource.start = IO_USB_A_BASE,
    .resource.end = -1,
    .dma_mask_room = DMA_BIT_MASK(32),
    .port_type = USB_PORT_TYPE_OTG,
    .port_speed = USB_PORT_SPEED_DEFAULT,
    .dma_config = USB_DMA_BURST_SINGLE,
    .set_vbus_power = set_usb_a_vbus_power,
};
//usb_b is host port
static struct lm_device usb_ld_b = {
    .type = LM_DEVICE_TYPE_USB,
    .id = 1,
    .irq = INT_USB_B,
    .resource.start = IO_USB_B_BASE,
    .resource.end = -1,
    .dma_mask_room = DMA_BIT_MASK(32),
    .port_type = USB_PORT_TYPE_HOST,
    .port_speed = USB_PORT_SPEED_DEFAULT,
    .dma_config = USB_DMA_BURST_SINGLE,
    .set_vbus_power = 0,
};
#endif
#ifdef CONFIG_SATA_DWC_AHCI
static struct lm_device sata_ld = {
    .type = LM_DEVICE_TYPE_SATA,
    .id = 2,
    .irq = INT_SATA,
    .dma_mask_room = DMA_BIT_MASK(32),
    .resource.start = IO_SATA_BASE,
    .resource.end = -1,
};
#endif

#if defined(CONFIG_AM_STREAMING)
static struct resource codec_resources[] = {
    [0] = {
        .start =  CODEC_ADDR_START,
        .end   = CODEC_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = STREAMBUF_ADDR_START,
        .end = STREAMBUF_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device codec_device = {
    .name       = "amstream",
    .id         = 0,
    .num_resources = ARRAY_SIZE(codec_resources),
    .resource      = codec_resources,
};
#endif

#if defined(CONFIG_AM_VIDEO)
static struct resource deinterlace_resources[] = {
    [0] = {
        .start =  DI_ADDR_START,
        .end   = DI_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device deinterlace_device = {
    .name       = "deinterlace",
    .id         = 0,
    .num_resources = ARRAY_SIZE(deinterlace_resources),
    .resource      = deinterlace_resources,
};
#endif

#if defined(CONFIG_TVIN_VDIN)
static struct resource vdin_resources[] = {
    [0] = {
        .start =  VDIN_ADDR_START,      //pbufAddr
        .end   = VDIN_ADDR_END,             //pbufAddr + size
        .flags = IORESOURCE_MEM,
    },


};

static struct platform_device vdin_device = {
    .name       = "vdin",
    .id         = -1,
    .num_resources = ARRAY_SIZE(vdin_resources),
    .resource      = vdin_resources,
};

//add pin mux info for bt656 input
static struct resource bt656in_resources[] = {
    [0] = {
        .start =  VDIN_ADDR_START,      //pbufAddr
        .end   = VDIN_ADDR_END,             //pbufAddr + size
        .flags = IORESOURCE_MEM,
    },
    [1] = {     //bt656/camera/bt601 input resource pin mux setting
        .start =  0x3000,       //mask--mux gpioD 15 to bt656 clk;  mux gpioD 16:23 to be bt656 dt_in
        .end   = PERIPHS_PIN_MUX_5 + 0x3000,
        .flags = IORESOURCE_MEM,
    },

    [2] = {         //camera/bt601 input resource pin mux setting
        .start =  0x1c000,      //mask--mux gpioD 12 to bt601 FIQ; mux gpioD 13 to bt601HS; mux gpioD 14 to bt601 VS;
        .end   = PERIPHS_PIN_MUX_5 + 0x1c000,
        .flags = IORESOURCE_MEM,
    },

    [3] = {         //bt601 input resource pin mux setting
        .start =  0x800,        //mask--mux gpioD 24 to bt601 IDQ;;
        .end   = PERIPHS_PIN_MUX_5 + 0x800,
        .flags = IORESOURCE_MEM,
    },

};

static struct platform_device bt656in_device = {
    .name       = "amvdec_656in",
    .id         = -1,
    .num_resources = ARRAY_SIZE(bt656in_resources),
    .resource      = bt656in_resources,
};
#endif

#if defined(CONFIG_CARDREADER)
static struct resource amlogic_card_resource[]  = {
    [0] = {
        .start = 0x1200230,   //physical address
        .end   = 0x120024c,
        .flags = 0x200,
    }
};
//need check later
void extern_wifi_power(int is_power)
{
    if (0 == is_power) {
#ifdef CONFIG_TCA6424
        unsigned char level;
        configIO(1, 0);
        level = getIO_level(1);
        level &= ~(1 << 2);
        setIO_level(1, level);
#else
        //CLEAR_CBUS_REG_MASK(CARD_PIN_MUX_4, (1 << 4));
        //CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 4));
        //SET_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 4));
#endif
    } else {
#ifdef CONFIG_TCA6424
        unsigned char level;
        configIO(1, 0);
        level = getIO_level(1);
        /*P12 WIFI/BT EN*/
        level |= (1 << 2);
        setIO_level(1, level);
        /*P10 WIFI RST */
        level = getIO_level(1);
        level &= ~(1 << 0);
        setIO_level(1, level);
        mdelay(2000);
        level |= (1 << 0);
        setIO_level(1, level);

#else
        /*GPIOD_6 WIFI/BT EN*/
        //CLEAR_CBUS_REG_MASK(CARD_PIN_MUX_4, (1 << 4));
        //CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 4));
        //SET_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 4));
        /*GPIOD_5 WIFI/BT SEL*/
        //CLEAR_CBUS_REG_MASK(CARD_PIN_MUX_4, (1 << 5));
        //CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 3));
        //CLEAR_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 3));
#endif
    }
    return;
}

void sdio_extern_init(void)
{
    extern_wifi_power(1);
}

static struct aml_card_info  amlogic_card_info[] = {
    [0] = {
        .name = "sd_card",
        .work_mode = CARD_HW_MODE,
        .io_pad_type = SDIO_B_CARD_0_5,
        .card_ins_en_reg = CARD_GPIO_ENABLE,
        .card_ins_en_mask = PREG_IO_29_MASK,
        .card_ins_input_reg = CARD_GPIO_INPUT,
        .card_ins_input_mask = PREG_IO_29_MASK,
        .card_power_en_reg = CARD_GPIO_ENABLE,
        .card_power_en_mask = PREG_IO_31_MASK,
        .card_power_output_reg = CARD_GPIO_OUTPUT,
        .card_power_output_mask = PREG_IO_31_MASK,
        .card_power_en_lev = 0,
        .card_wp_en_reg = 0,
        .card_wp_en_mask = 0,
        .card_wp_input_reg = 0,
        .card_wp_input_mask = 0,
        .card_extern_init = 0,
    },
#if 1
    [1] = {
        .name = "sdio_card",
        .work_mode = CARD_HW_MODE,
        .io_pad_type = SDIO_A_GPIOX_0_3,
        .card_ins_en_reg = 0,
        .card_ins_en_mask = 0,
        .card_ins_input_reg = 0,
        .card_ins_input_mask = 0,
#ifdef CONFIG_TCA6424
        .card_power_en_reg = 0,//EGPIO_GPIOD_ENABLE,
        .card_power_en_mask = 0,//PREG_IO_10_MASK,
        .card_power_output_reg = 0,//EGPIO_GPIOD_OUTPUT,
        .card_power_output_mask = 0,//PREG_IO_10_MASK,
        .card_power_en_lev = 0,//1,
#else
        /*GPIOD_8 WIFI RST*/
        .card_power_en_reg = 0,//EGPIO_GPIOD_ENABLE,
        .card_power_en_mask = 0,//PREG_IO_10_MASK,
        .card_power_output_reg = 0,//EGPIO_GPIOD_OUTPUT,
        .card_power_output_mask = 0,//PREG_IO_10_MASK,
        .card_power_en_lev = 0,//1,
#endif
        .card_wp_en_reg = 0,
        .card_wp_en_mask = 0,
        .card_wp_input_reg = 0,
        .card_wp_input_mask = 0,
        .card_extern_init = sdio_extern_init,
    },
#endif
};

static struct aml_card_platform amlogic_card_platform = {
    .card_num = ARRAY_SIZE(amlogic_card_info),
    .card_info = amlogic_card_info,
};

static struct platform_device amlogic_card_device = {
    .name = "AMLOGIC_CARD",
    .id    = -1,
    .num_resources = ARRAY_SIZE(amlogic_card_resource),
    .resource = amlogic_card_resource,
    .dev = {
        .platform_data = &amlogic_card_platform,
    },
};
#endif

#if defined(CONFIG_AML_AUDIO_DSP)
static struct resource audiodsp_resources[] = {
    [0] = {
        .start = AUDIODSP_ADDR_START,
        .end   = AUDIODSP_ADDR_END,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device audiodsp_device = {
    .name       = "audiodsp",
    .id         = 0,
    .num_resources = ARRAY_SIZE(audiodsp_resources),
    .resource      = audiodsp_resources,
};
#endif

static struct resource aml_m1_audio_resource[] = {
    [0] =   {
        .start  =   0,
        .end        =   0,
        .flags  =   IORESOURCE_MEM,
    },
};
static struct platform_device aml_audio = {
    .name               = "aml_m1_audio_wm8900",
    .id                     = -1,
    .resource       =   aml_m1_audio_resource,
    .num_resources  =   ARRAY_SIZE(aml_m1_audio_resource),
};
#if defined(CONFIG_TOUCHSCREEN_ADS7846)
#define SPI_0       0
#define SPI_1       1
#define SPI_2       2

// GPIOC_8(G20, XPT_CLK)
#define GPIO_SPI_SCK        ((GPIOC_bank_bit0_26(8)<<16) |GPIOC_bit_bit0_26(8))
// GPIOC_7(G21, XPT_IN)
#define GPIO_SPI_MOSI       ((GPIOC_bank_bit0_26(7)<<16) |GPIOC_bit_bit0_26(7))
// GPIOC_6(G22, XPT_OUT)
#define GPIO_SPI_MISO       ((GPIOC_bank_bit0_26(6)<<16) |GPIOC_bit_bit0_26(6))
// GPIOC_0(J20, XPT_NCS)
#define GPIO_TSC2046_CS ((GPIOC_bank_bit0_26(0)<<16) |GPIOC_bit_bit0_26(0))
// GPIOC_4(H20, NPEN_IRQ)
#define GPIO_TSC2046_PENDOWN    ((GPIOC_bank_bit0_26(4)<<16) |GPIOC_bit_bit0_26(4))

static const struct spi_gpio_platform_data spi_gpio_pdata = {
    .sck = GPIO_SPI_SCK,
    .mosi = GPIO_SPI_MOSI,
    .miso = GPIO_SPI_MISO,
    .num_chipselect = 1,
};

static struct platform_device spi_gpio = {
    .name       = "spi_gpio",
    .id         = SPI_2,
    .dev = {
        .platform_data = (void *)&spi_gpio_pdata,
    },
};

static const struct ads7846_platform_data ads7846_pdata = {
    .model = 7846,
    .vref_delay_usecs = 100,
    .vref_mv = 2500,
    .keep_vref_on = false,
    .swap_xy = 0,
    .settle_delay_usecs = 10,
    .penirq_recheck_delay_usecs = 0,
    .x_plate_ohms  = 500,
    .y_plate_ohms = 500,

    .x_min = 0,
    .x_max = 0xfff,
    .y_min = 0,
    .y_max = 0xfff,
    .pressure_min = 0,
    .pressure_max = 0xfff,

    .debounce_max = 0,
    .debounce_tol = 0,
    .debounce_rep = 0,

    .gpio_pendown = GPIO_TSC2046_PENDOWN,
    .get_pendown_state = NULL,

    .filter_init = NULL,
    .filter = NULL,
    .filter_cleanup = NULL,
    .wait_for_sync = NULL,
    .wakeup = false,
};

static struct spi_board_info spi_board_info_list[] = {
    [0] = {
        .modalias = "ads7846",
        .platform_data = (void *)&ads7846_pdata,
        .controller_data = (void *)GPIO_TSC2046_CS,
        .irq = INT_GPIO_0,
        .max_speed_hz = 500000,
        .bus_num = SPI_2,
        .chip_select = 0,
        .mode = SPI_MODE_0,
    },
};

static int ads7846_init_gpio(void)
{
    /* memson
        Bit(s)  Description
        256-105 Unused
        104     JTAG_TDO
        103     JTAG_TDI
        102     JTAG_TMS
        101     JTAG_TCK
        100     gpioA_23
        99      gpioA_24
        98      gpioA_25
        97      gpioA_26
        98-75   gpioE[21:0]
        75-50   gpioD[24:0]
        49-23   gpioC[26:0]
        22-15   gpioB[22;15]
        14-0        gpioA[14:0]
     */

    /* set input mode */
    gpio_direction_input(GPIO_TSC2046_PENDOWN);
    /* set gpio interrupt #0 source=GPIOC_4, and triggered by falling edge(=1) */
    gpio_enable_edge_int(27, 1, 0);

    //  // reg2 bit24~26
    //  CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_2,
    //  (1<<24) | (1<<25) | (1<<26));
    //  // reg3 bit5~7,12,16~18,22
    //  CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_3,
    //  (1<<5) | (1<<6) | (1<<7) | (1<<9) | (1<<12) | (1<<16) | (1<<17) | (1<<18) | (1<<22));
    //  // reg4 bit26~27
    //  CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_4,
    //  (1<<26) | (1<<27));
    //  // reg9 bit0,4,6~8
    //  CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_9,
    //  (1<<0) | (1<<4) | (1<<6) | (1<<7) | (1<<8));
    //  // reg10 bit0,4,6~8
    //  CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_10,
    //  (1<<0) | (1<<4) | (1<<6) | (1<<7) | (1<<8));

    return 0;
}
#endif

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_data = {
    .name = "pmem",
    .start = PMEM_START,
    .size = PMEM_SIZE,
    .no_allocator = 1,
    .cached = 0,
};

static struct platform_device android_pmem_device = {
    .name = "android_pmem",
    .id = 0,
    .dev = {
        .platform_data = &pmem_data,
    },
};
#endif

#if defined(CONFIG_AML_RTC)
static  struct platform_device aml_rtc_device = {
    .name            = "aml_rtc",
    .id               = -1,
};
#endif


#if defined(CONFIG_I2C_SW_AML)

static struct aml_sw_i2c_platform aml_sw_i2c_plat = {
    .sw_pins = {
        .scl_reg_out        = MESON_I2C_PREG_GPIOB_OUTLVL,
        .scl_reg_in     = MESON_I2C_PREG_GPIOB_INLVL,
        .scl_bit            = 2,    /*MESON_I2C_MASTER_A_GPIOB_2_REG*/
        .scl_oe         = MESON_I2C_PREG_GPIOB_OE,
        .sda_reg_out        = MESON_I2C_PREG_GPIOB_OUTLVL,
        .sda_reg_in     = MESON_I2C_PREG_GPIOB_INLVL,
        .sda_bit            = 3,    /*MESON_I2C_MASTER_A_GPIOB_3_BIT*/
        .sda_oe         = MESON_I2C_PREG_GPIOB_OE,
    },
    .udelay         = 2,
    .timeout            = 100,
};

static struct platform_device aml_sw_i2c_device = {
    .name         = "aml-sw-i2c",
    .id       = -1,
    .dev = {
        .platform_data = &aml_sw_i2c_plat,
    },
};

#endif

#if defined(CONFIG_I2C_AML)
static struct aml_i2c_platform aml_i2c_plat = {
    .wait_count     = 1000000,
    .wait_ack_interval  = 5,
    .wait_read_interval = 5,
    .wait_xfer_interval = 5,
    .master_no      = AML_I2C_MASTER_B,
    .use_pio            = 0,
    .master_i2c_speed   = AML_I2C_SPPED_400K,

    .master_b_pinmux = {
        .scl_reg    = MESON_I2C_MASTER_B_GPIOB_0_REG,
        .scl_bit    = MESON_I2C_MASTER_B_GPIOB_0_BIT,
        .sda_reg    = MESON_I2C_MASTER_B_GPIOB_1_REG,
        .sda_bit    = MESON_I2C_MASTER_B_GPIOB_1_BIT,
    }
};

static struct resource aml_i2c_resource[] = {
    [0] = {/*master a*/
        .start =    MESON_I2C_MASTER_A_START,
        .end   =    MESON_I2C_MASTER_A_END,
        .flags =    IORESOURCE_MEM,
    },
    [1] = {/*master b*/
        .start =    MESON_I2C_MASTER_B_START,
        .end   =    MESON_I2C_MASTER_B_END,
        .flags =    IORESOURCE_MEM,
    },
    [2] = {/*slave*/
        .start =    MESON_I2C_SLAVE_START,
        .end   =    MESON_I2C_SLAVE_END,
        .flags =    IORESOURCE_MEM,
    },
};

static struct platform_device aml_i2c_device = {
    .name         = "aml-i2c",
    .id       = -1,
    .num_resources    = ARRAY_SIZE(aml_i2c_resource),
    .resource     = aml_i2c_resource,
    .dev = {
        .platform_data = &aml_i2c_plat,
    },
};
#endif

#ifdef CONFIG_AMLOGIC_PM

static int is_ac_connected(void)
{
    return (READ_CBUS_REG(ASSIST_HW_REV) & (1 << 9)) ? 1 : 0;
}

//static int is_usb_connected(void)
//{
//  return 0;
//}

static void set_charge(int flags)
{
    //GPIOD_22 low: fast charge high: slow charge
//    set_gpio_val(GPIOD_bank_bit2_24(22), GPIOD_bit_bit2_24(22), 1);
//    set_gpio_mode(GPIOD_bank_bit2_24(22), GPIOD_bit_bit2_24(22), GPIO_OUTPUT_MODE);
}

extern int get_adc_sample(int chan);
static int get_bat_vol(void)
{
    return 1000;//get_adc_sample(5);
}

static int get_charge_status(void)
{
    return (READ_CBUS_REG(ASSIST_HW_REV) & (1 << 8)) ? 1 : 0;
}

static struct aml_power_pdata power_pdata = {
    .is_ac_online   = is_ac_connected,
    //.is_usb_online    = is_usb_connected,
    .set_charge = set_charge,
    .get_bat_vol = get_bat_vol,
    .get_charge_status = get_charge_status,
    .is_support_usb_charging = 0,
    //.supplied_to = supplicants,
    //.num_supplicants = ARRAY_SIZE(supplicants),
};

static struct platform_device power_dev = {
    .name       = "aml-power",
    .id     = -1,
    .dev = {
        .platform_data  = &power_pdata,
    },
};
#endif

#if defined(CONFIG_AM_UART_WITH_S_CORE)
static struct aml_uart_platform aml_uart_plat = {
    .uart_line[0]       =   UART_AO,
    .uart_line[1]       =   UART_A,
    .uart_line[2]       =   UART_B,
    .uart_line[3]       =   UART_C
};

static struct platform_device aml_uart_device = {
    .name         = "am_uart",
    .id       = -1,
    .num_resources    = 0,
    .resource     = NULL,
    .dev = {
        .platform_data = &aml_uart_plat,
    },
};
#endif

#ifdef CONFIG_AM_NAND
static struct mtd_partition normal_partition_info[] = {
#ifndef CONFIG_AMLOGIC_SPI_NOR
    {
        .name = "ubootenv",
        .offset = 4 * 1024 * 1024,
        .size = 1 * 1024 * 1024,
    },
    {
        .name = "recovery",
        .offset = 5 * 1024 * 1024,
        .size = 3 * 1024 * 1024,
    },
#endif
    {
        .name = "boot",
        .offset = 8 * 1024 * 1024,
        .size = 4 * 1024 * 1024,
    },
    {
        .name = "system",
        .offset = 12 * 1024 * 1024,
        .size = 148 * 1024 * 1024,
    },
    {
        .name = "cache",
        .offset = 160 * 1024 * 1024,
        .size = 16 * 1024 * 1024,
    },
    {
        .name = "userdata",
        .offset = MTDPART_OFS_APPEND,
        .size = MTDPART_SIZ_FULL,
    }
};


static struct aml_nand_platform aml_nand_mid_platform[] = {
#ifndef CONFIG_AMLOGIC_SPI_NOR
    {
        .name = NAND_BOOT_NAME,
        .chip_enable_pad = AML_NAND_CE0,
        .ready_busy_pad = AML_NAND_CE0,
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH16_MODE),
            },
        },
        .T_REA = 20,
        .T_RHOH = 15,
    },
#endif
    {
        .name = NAND_NORMAL_NAME,
        .chip_enable_pad = (AML_NAND_CE0 | (AML_NAND_CE1 << 4) | (AML_NAND_CE2 << 8) | (AML_NAND_CE3 << 12)),
        .ready_busy_pad = (AML_NAND_CE0 | (AML_NAND_CE0 << 4) | (AML_NAND_CE1 << 8) | (AML_NAND_CE1 << 12)),
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 4,
                .nr_partitions = ARRAY_SIZE(normal_partition_info),
                .partitions = normal_partition_info,
                .options = (NAND_TIMING_MODE5 | NAND_ECC_BCH16_MODE | NAND_TWO_PLANE_MODE),
            },
        },
        .T_REA = 20,
        .T_RHOH = 15,
    }
};

struct aml_nand_device aml_nand_mid_device = {
    .aml_nand_platform = aml_nand_mid_platform,
    .dev_num = ARRAY_SIZE(aml_nand_mid_platform),
};

static struct resource aml_nand_resources[] = {
    {
        .start = 0xc1108600,
        .end = 0xc1108624,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device aml_nand_device = {
    .name = "aml_m1_nand",
    .id = 0,
    .num_resources = ARRAY_SIZE(aml_nand_resources),
    .resource = aml_nand_resources,
    .dev = {
        .platform_data = &aml_nand_mid_device,
    },
};

#endif

#if defined(CONFIG_AMLOGIC_BACKLIGHT)

#define PWM_TCNT        (600-1)
#define PWM_MAX_VAL    (420)

static void aml_8726m_bl_init(void)
{
    unsigned val;

    WRITE_CBUS_REG_BITS(PERIPHS_PIN_MUX_0, 0, 22, 1);
    WRITE_CBUS_REG_BITS(PREG_AM_ANALOG_ADDR, 1, 0, 1);
    WRITE_CBUS_REG(VGHL_PWM_REG0, 0);
    WRITE_CBUS_REG(VGHL_PWM_REG1, 0);
    WRITE_CBUS_REG(VGHL_PWM_REG2, 0);
    WRITE_CBUS_REG(VGHL_PWM_REG3, 0);
    WRITE_CBUS_REG(VGHL_PWM_REG4, 0);
    val = (0 << 31)           |       // disable the overall circuit
          (0 << 30)           |       // 1:Closed Loop  0:Open Loop
          (PWM_TCNT << 16)    |       // PWM total count
          (0 << 13)           |       // Enable
          (1 << 12)           |       // enable
          (0 << 10)           |       // test
          (3 << 7)            |       // CS0 REF, Voltage FeedBack: about 0.27V
          (7 << 4)            |       // CS1 REF, Current FeedBack: about 0.54V
          (0 << 0);                   // DIMCTL Analog dimmer
    WRITE_CBUS_REG(VGHL_PWM_REG0, val);
    val = (1 << 30)           |       // enable high frequency clock
          (PWM_MAX_VAL << 16) |       // MAX PWM value
          (0 << 0);                  // MIN PWM value
    WRITE_CBUS_REG(VGHL_PWM_REG1, val);
    val = (0 << 31)       |       // disable timeout test mode
          (0 << 30)       |       // timeout based on the comparator output
          (0 << 16)       |       // timeout = 10uS
          (0 << 13)       |       // Select oscillator as the clock (just for grins)
          (1 << 11)       |       // 1:Enable OverCurrent Portection  0:Disable
          (3 << 8)        |       // Filter: shift every 3 ticks
          (0 << 6)        |       // Filter: count 1uS ticks
          (0 << 5)        |       // PWM polarity : negative
          (0 << 4)        |       // comparator: negative, Different with NikeD3
          (1 << 0);               // +/- 1
    WRITE_CBUS_REG(VGHL_PWM_REG2, val);
    val = (1 << 16) |       // Feedback down-sampling = PWM_freq/1 = PWM_freq
          (1 << 14) |       // enable to re-write MATCH_VAL
          (210 <<  0) ;   // preset PWM_duty = 50%
    WRITE_CBUS_REG(VGHL_PWM_REG3, val);
    val = (0 << 30) |       // 1:Digital Dimmer  0:Analog Dimmer
          (2 << 28) |       // dimmer_timebase = 1uS
          (1000 << 14) |    // Digital dimmer_duty = 0%, the most darkness
          (1000 <<  0) ;    // dimmer_freq = 1KHz
    WRITE_CBUS_REG(VGHL_PWM_REG4, val);
}

static unsigned aml_8726m_get_bl_level(void)
{
    unsigned level = 0;

    WRITE_CBUS_REG_BITS(VGHL_PWM_REG0, 1, 31, 1);
    WRITE_CBUS_REG_BITS(VGHL_PWM_REG4, 0, 30, 1);
    level = READ_CBUS_REG_BITS(VGHL_PWM_REG0, 0, 4);
    return level;
}

static void aml_8726m_set_bl_level(unsigned level)
{
    if (15 == level) {
        WRITE_CBUS_REG_BITS(VGHL_PWM_REG0, 0, 31, 1);
    } else {
        WRITE_CBUS_REG_BITS(VGHL_PWM_REG0, level, 0, 4);
    }
}

static void aml_8726m_power_on_bl(void)
{
    WRITE_CBUS_REG_BITS(PREG_EGPIO_EN_N, 0, 12, 1);
    WRITE_CBUS_REG_BITS(PREG_EGPIO_O, 1, 12, 1);

    WRITE_CBUS_REG_BITS(PREG_EGPIO_EN_N, 0, 7, 1);
    WRITE_CBUS_REG_BITS(PREG_EGPIO_O, 1, 7, 1);

    aml_8726m_set_bl_level(0);
}

static void aml_8726m_power_off_bl(void)
{
    WRITE_CBUS_REG_BITS(PREG_EGPIO_EN_N, 0, 12, 1);
    WRITE_CBUS_REG_BITS(PREG_EGPIO_O, 0, 12, 1);

    WRITE_CBUS_REG_BITS(PREG_EGPIO_EN_N, 0, 7, 1);
    WRITE_CBUS_REG_BITS(PREG_EGPIO_O, 0, 7, 1);
}

struct aml_bl_platform_data aml_bl_platform = {
    .bl_init = aml_8726m_bl_init,
    .power_on_bl = aml_8726m_power_on_bl,
    .power_off_bl = aml_8726m_power_off_bl,
    .get_bl_level = aml_8726m_get_bl_level,
    .set_bl_level = aml_8726m_set_bl_level,
};

static struct platform_device aml_bl_device = {
    .name = "aml-bl",
    .id = -1,
    .num_resources = 0,
    .resource = NULL,
    .dev = {
        .platform_data = &aml_bl_platform,
    },
};
#endif
#if  defined(CONFIG_AM_TV_OUTPUT)||defined(CONFIG_AM_TCON_OUTPUT)
static struct resource vout_device_resources[] = {
    [0] = {
        .start = 0,
        .end   = 0,
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device vout_device = {
    .name       = "mesonvout",
    .id         = 0,
    .num_resources = ARRAY_SIZE(vout_device_resources),
    .resource      = vout_device_resources,
};
#endif

#ifdef CONFIG_USB_ANDROID
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
static struct usb_mass_storage_platform_data mass_storage_pdata = {
    .nluns = 2,
    .vendor = "AMLOGIC",
    .product = "Android MID",
    .release = 0x0100,
};
static struct platform_device usb_mass_storage_device = {
    .name = "usb_mass_storage",
    .id = -1,
    .dev = {
        .platform_data = &mass_storage_pdata,
    },
};
#endif
static char *usb_functions[] = { "usb_mass_storage" };
static char *usb_functions_adb[] = {
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
    "usb_mass_storage",
#endif

#ifdef CONFIG_USB_ANDROID_ADB
    "adb"
#endif
};

static struct android_usb_product usb_products[] = {
    {
        .product_id     = 0x0c01,
        .num_functions  = ARRAY_SIZE(usb_functions),
        .functions      = usb_functions,
    },
    {
        .product_id     = 0x0c02,
        .num_functions  = ARRAY_SIZE(usb_functions_adb),
        .functions      = usb_functions_adb,
    },
};

static struct android_usb_platform_data android_usb_pdata = {
    .vendor_id      = 0x0bb4,
    .product_id     = 0x0c01,
    .version        = 0x0100,
    .product_name   = "Android MID",
    .manufacturer_name = "AMLOGIC",
    .num_products = ARRAY_SIZE(usb_products),
    .products = usb_products,
    .num_functions = ARRAY_SIZE(usb_functions_adb),
    .functions = usb_functions_adb,
};

static struct platform_device android_usb_device = {
    .name   = "android_usb",
    .id             = -1,
    .dev            = {
        .platform_data = &android_usb_pdata,
    },
};
#endif

#if defined(CONFIG_SUSPEND)
typedef struct {
    char name[32];
    unsigned bank;
    unsigned bit;
    gpio_mode_t mode;
    unsigned value;
    unsigned enable;
} gpio_data_t;

#define MAX_GPIO 0
static gpio_data_t gpio_data[MAX_GPIO] = {
    // ----------------------------------- power ctrl ---------------------------------
    {"GPIOC_3 -- AVDD_EN",      GPIOC_bank_bit0_26(3),      GPIOC_bit_bit0_26(3),   GPIO_OUTPUT_MODE, 1, 1},
    {"GPIOA_7 -- BL_PWM",       GPIOA_bank_bit0_14(7),      GPIOA_bit_bit0_14(7),   GPIO_OUTPUT_MODE, 1, 1},
    {"GPIOA_6 -- VCCx2_EN",     GPIOA_bank_bit0_14(6),      GPIOA_bit_bit0_14(6),   GPIO_OUTPUT_MODE, 1, 1},
    // ----------------------------------- i2s ---------------------------------
    {"TEST_N -- I2S_DOUT",      GPIOJTAG_bank_bit(16),      GPIOJTAG_bit_bit16(16), GPIO_OUTPUT_MODE, 1, 1},
    // ----------------------------------- wifi&bt ---------------------------------
    {"GPIOD_12 -- WL_RST_N",    GPIOD_bank_bit2_24(12),     GPIOD_bit_bit2_24(12),  GPIO_OUTPUT_MODE, 1, 1},
    {"GPIOD_14 -- BT/GPS_RST_N", GPIOD_bank_bit2_24(14),     GPIOD_bit_bit2_24(14),  GPIO_OUTPUT_MODE, 1, 1},
    {"GPIOD_18 -- UART_CTS_N",  GPIOD_bank_bit2_24(18),     GPIOD_bit_bit2_24(18),  GPIO_OUTPUT_MODE, 1, 1},
    {"GPIOD_21 -- BT/GPS",      GPIOD_bank_bit2_24(21),     GPIOD_bit_bit2_24(21),  GPIO_OUTPUT_MODE, 1, 1},
    // ----------------------------------- lcd ---------------------------------
    {"GPIOC_12 -- LCD_U/D",     GPIOC_bank_bit0_26(12),     GPIOC_bit_bit0_26(12),  GPIO_OUTPUT_MODE, 1, 1},
    {"GPIOA_3 -- LCD_PWR_EN",   GPIOA_bank_bit0_14(3),      GPIOA_bit_bit0_14(3),   GPIO_OUTPUT_MODE, 1, 1},
};

static void save_gpio(int port)
{
    gpio_data[port].mode = get_gpio_mode(gpio_data[port].bank, gpio_data[port].bit);
    if (gpio_data[port].mode == GPIO_OUTPUT_MODE) {
        if (gpio_data[port].enable) {
            printk("change %s output %d to input\n", gpio_data[port].name, gpio_data[port].value);
            gpio_data[port].value = get_gpio_val(gpio_data[port].bank, gpio_data[port].bit);
            set_gpio_mode(gpio_data[port].bank, gpio_data[port].bit, GPIO_INPUT_MODE);
        } else {
            printk("no change %s output %d\n", gpio_data[port].name, gpio_data[port].value);
        }
    }
}

static void restore_gpio(int port)
{
    if ((gpio_data[port].mode == GPIO_OUTPUT_MODE) && (gpio_data[port].enable)) {
        set_gpio_val(gpio_data[port].bank, gpio_data[port].bit, gpio_data[port].value);
        set_gpio_mode(gpio_data[port].bank, gpio_data[port].bit, GPIO_OUTPUT_MODE);
        // printk("%s output %d\n", gpio_data[port].name, gpio_data[port].value);
    }
}

typedef struct {
    char name[32];
    unsigned reg;
    unsigned bits;
    unsigned enable;
} pinmux_data_t;


#define MAX_PINMUX  12

pinmux_data_t pinmux_data[MAX_PINMUX] = {
    {"HDMI",    0, (1 << 2) | (1 << 1) | (1 << 0),                        1},
    {"TCON",    0, (1 << 14) | (1 << 11),                             1},
    {"I2S_OUT", 0, (1 << 18),                                     1},
    {"I2S_CLK", 1, (1 << 19) | (1 << 15) | (1 << 11),                     1},
    {"SPI",     1, (1 << 29) | (1 << 27) | (1 << 25) | (1 << 23),             1},
    {"I2C",     2, (1 << 5) | (1 << 2),                               1},
    {"SD",      2, (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | (1 << 8),      1},
    {"PWM",     2, (1 << 31),                                     1},
    {"UART_A",  3, (1 << 24) | (1 < 23),                              0},
    {"RGB",     4, (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0),   1},
    {"UART_B",  5, (1 << 24) | (1 < 23),                              0},
    {"REMOTE",  5, (1 << 31),                                     1},
};

static unsigned pinmux_backup[6];

static void save_pinmux(void)
{
    int i;
    for (i = 0; i < 6; i++) {
        pinmux_backup[i] = READ_CBUS_REG(PERIPHS_PIN_MUX_0 + i);
    }
    for (i = 0; i < MAX_PINMUX; i++) {
        if (pinmux_data[i].enable) {
            printk("%s %x\n", pinmux_data[i].name, pinmux_data[i].bits);
            clear_mio_mux(pinmux_data[i].reg, pinmux_data[i].bits);
        }
    }
}

static void restore_pinmux(void)
{
    int i;
    for (i = 0; i < 6; i++) {
        WRITE_CBUS_REG(PERIPHS_PIN_MUX_0 + i, pinmux_backup[i]);
    }
}

static void set_vccx2(int power_on)
{
    int i;
    if (power_on) {
        restore_pinmux();
        for (i = 0; i < MAX_GPIO; i++) {
            restore_gpio(i);
        }

        printk(KERN_INFO "set_vccx2 power up\n");
        set_gpio_val(GPIOA_bank_bit0_14(6), GPIOA_bit_bit0_14(6), 1);
        set_gpio_mode(GPIOA_bank_bit0_14(6), GPIOA_bit_bit0_14(6), GPIO_OUTPUT_MODE);
        //set clk for wifi
    } else {
        printk(KERN_INFO "set_vccx2 power down\n");
        set_gpio_val(GPIOA_bank_bit0_14(6), GPIOA_bit_bit0_14(6), 0);
        set_gpio_mode(GPIOA_bank_bit0_14(6), GPIOA_bit_bit0_14(6), GPIO_OUTPUT_MODE);

        save_pinmux();
        for (i = 0; i < MAX_GPIO; i++) {
            save_gpio(i);
        }
    }
}
static struct meson_pm_config aml_pm_pdata = {
    .pctl_reg_base = IO_APB_BUS_BASE,
    .mmc_reg_base = APB_REG_ADDR(0x1000),
    .hiu_reg_base = CBUS_REG_ADDR(0x1000),
    .power_key = 1 << 15,
    .ddr_clk = 0x00110820,
    .sleepcount = 128,
    .set_vccx2 = set_vccx2,
    .core_voltage_adjust = 5,
};

static struct platform_device aml_pm_device = {
    .name           = "pm-meson",
    .dev = {
        .platform_data  = &aml_pm_pdata,
    },
    .id             = -1,
};
#endif

#ifdef CONFIG_BT_DEVICE
#include <linux/bt-device.h>

static struct platform_device bt_device = {
    .name             = "bt-dev",
    .id               = -1,
};

static void bt_device_init(void)
{
    /* BT/GPS_RST_N */
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_12, (1 << 29));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 20));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 14));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, (1 << 13));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, (1 << 12));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_8, (1 << 21));

    /* BT/GPS */
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_12, (1 << 22));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, (1 << 19));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 17));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_0, (1 << 12));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 4));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_8, (1 << 28));

    /* WLBT_REGON */
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_12, (1 << 23));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, (1 << 14));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 17));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_0, (1 << 12));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 5));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_8, (1 << 27));

    /* BT_WAKE */
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_12, (1 << 27));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 18));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_0, (1 << 12));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 9));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_8, (1 << 23));

    /* UART_RTS_N(BT) */
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_12, (1 << 26));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, (1 << 17));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 17));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_0, (1 << 12));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 8));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_8, (1 << 24));

    /* UART_CTS_N(BT) */
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_12, (1 << 25));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_7, (1 << 16));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 17));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_0, (1 << 12));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_5, (1 << 7));
    CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_8, (1 << 25));

    /* WLBT_REGON */
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 18));
    SET_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 18));

    /* BT/GPS_RST_N */
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 12));
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 12));

    /* BT/GPS */
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 19));
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 19));

    /* UART_RTS_N(BT) */
    SET_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 15));

    /* UART_CTS_N(BT) */
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 16));
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 16));

    /* BT_WAKE */
    //CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1<<14));
    //SET_CBUS_REG_MASK(PREG_GGPIO_O, (1<<14));
}

static void bt_device_on(void)
{
    /* BT/GPS_RST_N */
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 12));
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 12));
    msleep(200);
    SET_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 12));
}

static void bt_device_off(void)
{
    /* BT/GPS_RST_N */
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_EN_N, (1 << 12));
    CLEAR_CBUS_REG_MASK(PREG_GGPIO_O, (1 << 12));
    msleep(200);
}

struct bt_dev_data bt_dev = {
    .bt_dev_init    = bt_device_init,
    .bt_dev_on      = bt_device_on,
    .bt_dev_off     = bt_device_off,
};
#endif

static struct platform_device __initdata *platform_devs[] = {
#if defined(CONFIG_JPEGLOGO)
    &jpeglogo_device,
#endif
#if defined (CONFIG_AMLOGIC_PM)
    &power_dev,
#endif
#if defined(CONFIG_FB_AM)
    &fb_device,
#endif
#if defined(CONFIG_AM_STREAMING)
    &codec_device,
#endif
#if defined(CONFIG_AM_VIDEO)
    &deinterlace_device,
#endif
#if defined(CONFIG_TVIN_VDIN)
    &vdin_device,
    &bt656in_device,
#endif
#if defined(CONFIG_AML_AUDIO_DSP)
    &audiodsp_device,
#endif
    &aml_audio,
#if defined(CONFIG_CARDREADER)
    &amlogic_card_device,
#endif
#if defined(CONFIG_KEYPADS_AM)||defined(CONFIG_VIRTUAL_REMOTE)||defined(CONFIG_KEYPADS_AM_MODULE)
    &input_device,
#endif
#if defined(CONFIG_AMLOGIC_SPI_NOR)
    &amlogic_spi_nor_device,
#endif
#ifdef CONFIG_SARADC_AM
    &saradc_device,
#endif
#if defined(CONFIG_ADC_KEYPADS_AM)||defined(CONFIG_ADC_KEYPADS_AM_MODULE)
    &input_device_adc,
#endif
#if defined(CONFIG_KEY_INPUT_CUSTOM_AM) || defined(CONFIG_KEY_INPUT_CUSTOM_AM_MODULE)
    &input_device_key,  //changed by Elvis
#endif
#if defined(CONFIG_TOUCHSCREEN_ADS7846)
    &spi_gpio,
#endif
#ifdef CONFIG_AM_NAND
    &aml_nand_device,
#endif
#if defined(CONFIG_AML_RTC)
    &aml_rtc_device,
#endif
#if defined(CONFIG_ANDROID_PMEM)
    &android_pmem_device,
#endif
#if defined(CONFIG_I2C_SW_AML)
    &aml_sw_i2c_device,
#endif
#if defined(CONFIG_I2C_AML)
    &aml_i2c_device,
#endif
#if defined(CONFIG_AM_UART_WITH_S_CORE)
    &aml_uart_device,
#endif
#if defined(CONFIG_AMLOGIC_BACKLIGHT)
    &aml_bl_device,
#endif
#if  defined(CONFIG_AM_TV_OUTPUT)||defined(CONFIG_AM_TCON_OUTPUT)
    &vout_device,
#endif
#ifdef CONFIG_USB_ANDROID
    &android_usb_device,
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
    &usb_mass_storage_device,
#endif
#endif
#if defined(CONFIG_SUSPEND)
    &aml_pm_device,
#endif
#ifdef CONFIG_BT_DEVICE
    &bt_device,
#endif
};
static struct i2c_board_info __initdata aml_i2c_bus_info[] = {

#ifdef CONFIG_SENSORS_MMC31XX
    {
        I2C_BOARD_INFO(MMC31XX_I2C_NAME,  MMC31XX_I2C_ADDR),
    },
#endif

#ifdef CONFIG_SENSORS_MXC622X
    {
        I2C_BOARD_INFO(MXC622X_I2C_NAME,  MXC622X_I2C_ADDR),
    },
#endif
    {
        I2C_BOARD_INFO("wm8900", 0x1A),
    },
    {
        I2C_BOARD_INFO("tca6424", 0x22),
    },
};


static int __init aml_i2c_init(void)
{

    i2c_register_board_info(0, aml_i2c_bus_info,
                            ARRAY_SIZE(aml_i2c_bus_info));
    return 0;
}

static void __init eth_pinmux_init(void)
{
    eth_set_pinmux(ETH_BANK0_GPIOY1_Y9, ETH_CLK_OUT_GPIOY0_REG6_17, 0);
    //power hold
    //setbits_le32(P_PREG_AGPIO_O,(1<<8));
    //clrbits_le32(P_PREG_AGPIO_EN_N,(1<<8));
    //set_gpio_mode(GPIOA_bank_bit(4),GPIOA_bit_bit0_14(4),GPIO_OUTPUT_MODE);
    //set_gpio_val(GPIOA_bank_bit(4),GPIOA_bit_bit0_14(4),1);

    CLEAR_CBUS_REG_MASK(PREG_ETHERNET_ADDR0, 1);           // Disable the Ethernet clocks
	// ---------------------------------------------
	// Test 50Mhz Input Divide by 2
	// ---------------------------------------------
	// Select divide by 2
    CLEAR_CBUS_REG_MASK(PREG_ETHERNET_ADDR0, (1<<3));     // desc endianess "same order" 
    CLEAR_CBUS_REG_MASK(PREG_ETHERNET_ADDR0, (1<<2));     // ata endianess "little"
    SET_CBUS_REG_MASK(PREG_ETHERNET_ADDR0, (1 << 1));     // divide by 2 for 100M
    SET_CBUS_REG_MASK(PREG_ETHERNET_ADDR0, 1);            // enable Ethernet clocks
    udelay(100);
    /* reset phy with GPIOA_23*/
    set_gpio_mode(GPIOA_bank_bit0_27(23), GPIOA_bit_bit0_27(23), GPIO_OUTPUT_MODE);
    set_gpio_val(GPIOA_bank_bit0_27(23), GPIOA_bit_bit0_27(23), 0);
    udelay(100);    //GPIOA_bank_bit0_27(23) reset end;
    set_gpio_val(GPIOA_bank_bit0_27(23), GPIOA_bit_bit0_27(23), 1);
    udelay(100);    //waiting reset end;
    aml_i2c_init();
}

static void __init device_pinmux_init(void)
{
    clearall_pinmux();
    /*other deivce power on*/
    uart_set_pinmux(UART_PORT_AO, UART_AO_GPIO_AO0_AO1_STD);
    /*pinmux of eth*/
    eth_pinmux_init();
    set_audio_pinmux(AUDIO_IN_JTAG); // for MIC input
    set_audio_pinmux(AUDIO_OUT_TEST_N); //External AUDIO DAC
    set_audio_pinmux(SPDIF_OUT_GPIOA); //SPDIF GPIOA_6
}

static void __init  device_clk_setting(void)
{
    /*Configurate the ethernet clock*/
    eth_clk_set(ETH_CLKSRC_MISC_CLK, get_misc_pll_clk(), (50 * CLK_1M), 0);
}

static void disable_unused_model(void)
{
    CLK_GATE_OFF(VIDEO_IN);
    CLK_GATE_OFF(BT656_IN);
    //CLK_GATE_OFF(ETHERNET);
    //CLK_GATE_OFF(SATA);
    CLK_GATE_OFF(WIFI);
    //video_dac_disable();
    //audio_internal_dac_disable();
}

static __init void m1_init_machine(void)
{
    meson_cache_init();

    device_clk_setting();
    device_pinmux_init();
    platform_add_devices(platform_devs, ARRAY_SIZE(platform_devs));

#ifdef CONFIG_USB_DWC_OTG_HCD
    set_usb_phy_clk(USB_PHY_CLOCK_SEL_XTAL_DIV2);
    set_usb_phy_id_mode(USB_PHY_PORT_B,USB_PHY_MODE_SW_HOST);
    lm_device_register(&usb_ld_a);
    lm_device_register(&usb_ld_b);
#endif
#ifdef CONFIG_SATA_DWC_AHCI
    set_sata_phy_clk(SATA_PHY_CLOCK_SEL_DEMOD_PLL);
    lm_device_register(&sata_ld);
#endif
#if defined(CONFIG_TOUCHSCREEN_ADS7846)
    ads7846_init_gpio();
    spi_register_board_info(spi_board_info_list, ARRAY_SIZE(spi_board_info_list));
#endif
    disable_unused_model();
}

/*VIDEO MEMORY MAPING*/
static __initdata struct map_desc meson_video_mem_desc[] = {
    {
        .virtual    = PAGE_ALIGN(__phys_to_virt(RESERVED_MEM_START)),
        .pfn        = __phys_to_pfn(RESERVED_MEM_START),
        .length     = RESERVED_MEM_END - RESERVED_MEM_START + 1,
        .type       = MT_DEVICE,
    },
};

static __init void m1_map_io(void)
{
    meson_map_io();
    iotable_init(meson_video_mem_desc, ARRAY_SIZE(meson_video_mem_desc));
}

static __init void m1_irq_init(void)
{
    meson_init_irq();
}

static __init void m1_fixup(struct machine_desc *mach, struct tag *tag, char **cmdline, struct meminfo *m)
{
    struct membank *pbank;
    m->nr_banks = 0;
    pbank = &m->bank[m->nr_banks];
    pbank->start = PAGE_ALIGN(PHYS_MEM_START);
    pbank->size  = SZ_64M & PAGE_MASK;
    pbank->node  = PHYS_TO_NID(PHYS_MEM_START);
    m->nr_banks++;
    pbank = &m->bank[m->nr_banks];
    pbank->start = PAGE_ALIGN(RESERVED_MEM_END + 1);
    pbank->size  = (PHYS_MEM_END - RESERVED_MEM_END) & PAGE_MASK;
    pbank->node  = PHYS_TO_NID(RESERVED_MEM_END + 1);
    m->nr_banks++;
}

MACHINE_START(MESON3_8726M_SKT, "AMLOGIC MESON3 8726M SKT SH")
    .phys_io        = MESON_PERIPHS1_PHYS_BASE,
    .io_pg_offst    = (MESON_PERIPHS1_PHYS_BASE >> 18) & 0xfffc,
    .boot_params    = BOOT_PARAMS_OFFSET,
    .map_io         = m1_map_io,
    .init_irq       = m1_irq_init,
    .timer          = &meson_sys_timer,
    .init_machine   = m1_init_machine,
    .fixup          = m1_fixup,
    .video_start    = RESERVED_MEM_START,
    .video_end      = RESERVED_MEM_END,
MACHINE_END
