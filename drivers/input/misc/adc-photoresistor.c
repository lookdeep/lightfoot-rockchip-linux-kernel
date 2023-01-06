#include <linux/err.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>

#define CCI0_M_RD            1

static struct iio_channel *channel;

static long photoresistor_misc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int rc;
	int value;

	switch(cmd) {
	case CCI0_M_RD:
		rc = iio_read_channel_raw(channel, &value);
		if (rc < 0) {
			pr_err("%s: read iio raw failed\n", __func__);
			return -EFAULT;
		}
		rc = copy_to_user((int *)arg, &value, sizeof(int));
		if (rc) {
			pr_err("%s: copy_to_user failed\n", __func__);
			return -EFAULT;
		}
		break;
	default:
		return -EINVAL; 
	}

	return rc;
}

static int photoresistor_misc_open(struct inode* node, struct file* file)
{
	return 0;
}

static int photoresistor_misc_release(struct inode* node, struct file* file)
{
	return 0;
}

static const struct file_operations photoresistor_misc_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = photoresistor_misc_ioctl,
	.open           = photoresistor_misc_open,
	.release        = photoresistor_misc_release,
};

static struct miscdevice photoresistor_misc_device =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "photoresistor",
	.fops = &photoresistor_misc_fops,
};

static int adc_photoresistor_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	dev_info(dev, "Entry %s\n", __func__);
	channel = iio_channel_get(dev, NULL);
	if (IS_ERR(channel)) {
		channel = NULL;
		dev_err( dev, "%s() have not set adc chan\n", __func__);
		return -1;
	}
	
	misc_register(&photoresistor_misc_device);
	dev_info(dev, "Exit %s\n", __func__);
	return 0;
}

static int adc_photoresistor_remove(struct platform_device *pdev)
{
	misc_deregister(&photoresistor_misc_device);
	iio_channel_release(channel);
	return 0;
}


#ifdef CONFIG_OF
static const struct of_device_id adc_photoresistor_of_match[] = {
	{ .compatible = "adc-photoresistor", },
	{ }
};
MODULE_DEVICE_TABLE(of, adc_photoresistor_of_match);
#endif

static struct platform_driver __refdata adc_photoresistor_driver = {
	.driver = {
		.name    = "adc_photoresistor",
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(adc_photoresistor_of_match),
	},
	.probe  = adc_photoresistor_probe,
	.remove = adc_photoresistor_remove,
};
module_platform_driver(adc_photoresistor_driver);
MODULE_AUTHOR("Minrray <yangfuqi@minrrayav.com>");
MODULE_DESCRIPTION("Input driver for resistor ladder connected on ADC");
MODULE_LICENSE("GPL v2");
