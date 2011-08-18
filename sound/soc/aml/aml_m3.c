#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>

#include "aml_dai.h"
#include "aml_pcm.h"
#include "aml_m3_codec.h"


static int aml_m3_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
		struct snd_soc_pcm_runtime *rtd = substream->private_data;
		struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
		struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
		int ret;
		// TODO
printk("***Entered %s:%s\n", __FILE__,__func__);
		
		ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS);
		if(ret<0)
			return ret;
			
		ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_NB_NF|SND_SOC_DAIFMT_CBS_CFS);
		if(ret<0)
			return ret;
		
	return 0;
}
	
static struct snd_soc_ops aml_m3_ops = {
	.hw_params = aml_m3_hw_params,
};

static int aml_m3_set_bias_level(struct snd_soc_card *card,
					enum snd_soc_bias_level level)
{
	int ret = 0;
	// TODO
printk("***Entered %s:%s\n", __FILE__,__func__);
	return ret;
}

static int aml_m3_codec_init(struct snd_soc_codec *codec)
{
printk("***Entered %s:%s\n", __FILE__,__func__);
		return 0;
}


static struct snd_soc_dai_link aml_m3_dai = {
	.name = "AML-M3",
	.stream_name = "AML M3 PCM",
	.cpu_dai = &aml_dai[0],  //////
	.codec_dai = &aml_m3_codec_dai,
	.init = aml_m3_codec_init,
	.ops = &aml_m3_ops,
	.rate = 44100,
};

static struct snd_soc_card snd_soc_aml_m3 = {
	.name = "AML-M3",
	.platform = &aml_soc_platform,
	.dai_link = &aml_m3_dai,
	.num_links = 1,
	.set_bias_level = aml_m3_set_bias_level,
};

static struct snd_soc_device aml_m3_snd_devdata = {
	.card = &snd_soc_aml_m3,
	.codec_dev = &soc_codec_dev_aml_m3,
};

static struct platform_device *aml_m3_snd_device;
static struct platform_device *aml_m3_platform_device;

static int aml_m3_audio_probe(struct platform_device *pdev)
{
		int ret;
		//pdev->dev.platform_data;
		// TODO
printk("***Entered %s:%s\n", __FILE__,__func__);
		aml_m3_snd_device = platform_device_alloc("soc-audio", -1);
		if (!aml_m3_snd_device) {
			printk(KERN_ERR "ASoC: Platform device allocation failed\n");
			ret = -ENOMEM;
		}
	
		platform_set_drvdata(aml_m3_snd_device,&aml_m3_snd_devdata);
		aml_m3_snd_devdata.dev = &aml_m3_snd_device->dev;
	
		ret = platform_device_add(aml_m3_snd_device);
		if (ret) {
			printk(KERN_ERR "ASoC: Platform device allocation failed\n");
			goto error;
		}
		
		aml_m3_platform_device = platform_device_register_simple("aml_m3_codec",
								-1, NULL, 0);
		return 0;							
error:								
		platform_device_put(aml_m3_snd_device);								
		return ret;
}

static int aml_m3_audio_remove(struct platform_device *pdev)
{
printk("***Entered %s:%s\n", __FILE__,__func__);
		platform_device_unregister(aml_m3_snd_device);
		return 0;
}

static struct platform_driver aml_m3_audio_driver = {
	.probe  = aml_m3_audio_probe,
	.remove = aml_m3_audio_remove,
	.driver = {
		.name = "aml_m3_audio",
		.owner = THIS_MODULE,
	},
};

static int __init aml_m3_init(void)
{
		return platform_driver_register(&aml_m3_audio_driver);
}

static void __exit aml_m3_exit(void)
{
		platform_driver_unregister(&aml_m3_audio_driver);
}

module_init(aml_m3_init);
module_exit(aml_m3_exit);

/* Module information */
MODULE_AUTHOR("AMLogic, Inc.");
MODULE_DESCRIPTION("ALSA SoC AML M3 AUDIO");
MODULE_LICENSE("GPL");
