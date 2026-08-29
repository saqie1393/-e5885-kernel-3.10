/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include "bsp_leds.h"
#include "../leds_balong.h"

static void led_breath_set(struct led_classdev *led_cdev, unsigned long *full_on, unsigned long *full_off, unsigned long *fade_on, unsigned long *fade_off)
{
    struct balong_led_device *led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);

    del_timer_sync(&led_cdev->blink_timer);

    LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "%s full_on %d, full_off %d, fade_on %d, fade_off %d\n",
              led_cdev->name, *full_on, *full_off, *fade_on, *fade_off);

    if (led_dev && led_dev->pdata && led_dev->pdata->led_breath_set &&
            !led_dev->pdata->led_breath_set(led_cdev, full_on, full_off, fade_on, fade_off))
        return;

    return;
}

static ssize_t led_fullon_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct led_classdev *led_cdev = NULL;
    struct balong_led_device *led_dev = NULL;

    led_cdev = dev_get_drvdata(dev);
    if (NULL == led_cdev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_cdev\n");
        return LED_ERROR;
    }

    led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    if (NULL == led_dev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev\n");
        return LED_ERROR;
    }

    if(NULL == led_dev->pdata)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev->pdata\n");
        return LED_ERROR;
    }

    return snprintf(buf, LED_SYSFS_MAX_STR_LENTH, "%lu\n", led_dev->pdata->full_on);
}

static ssize_t led_fullon_store(struct device *dev,	struct device_attribute *attr, const char *buf, size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct balong_led_device *led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    int ret = -EINVAL;
    char *after;
    unsigned long state = simple_strtoul(buf, &after, 10);
    size_t count = after - buf;

    if (isspace(*after))
        count++;

    if ((count == size) && led_cdev && led_dev && led_dev->pdata)
    {

        LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "%s full_on %d, full_off %d, fade_on %d, fade_off %d\n",
                  led_cdev->name, state, led_dev->pdata->full_off, led_dev->pdata->fade_on, led_dev->pdata->fade_off);

        led_breath_set(led_cdev, &state, &led_dev->pdata->full_off,
                       &led_dev->pdata->fade_on, &led_dev->pdata->fade_off);
        led_dev->pdata->full_on = state;
        ret = count;
    }

    return ret;
}

static ssize_t led_fulloff_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
    struct led_classdev *led_cdev = NULL;
    struct balong_led_device *led_dev = NULL;

    if(NULL == dev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == dev\n");
        return LED_ERROR;
    }

    led_cdev = dev_get_drvdata(dev);
    if(NULL == led_cdev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_cdev\n");
        return LED_ERROR;
    }

    led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    if(NULL == led_dev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev\n");
        return LED_ERROR;
    }


    if(NULL == led_dev->pdata)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev\n");
        return LED_ERROR;
    }

    return snprintf(buf, LED_SYSFS_MAX_STR_LENTH, "%lu\n", led_dev->pdata->full_off);
}

static ssize_t led_fulloff_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct balong_led_device *led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    int ret = -EINVAL;
    char *after;
    unsigned long state = simple_strtoul(buf, &after, 10);
    size_t count = after - buf;

    if (isspace(*after))
        count++;

    if ((count == size) && led_cdev && led_dev && led_dev->pdata)
    {

        LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "%s full_on %d, full_off %d, fade_on %d, fade_off %d\n", led_cdev->name, led_dev->pdata->full_on, state,
                  led_dev->pdata->fade_on, led_dev->pdata->fade_off);
        led_breath_set(led_cdev, &led_dev->pdata->full_on, &state,
                       &led_dev->pdata->fade_on, &led_dev->pdata->fade_off);

        led_dev->pdata->full_off = state;
        ret = count;
    }

    return ret;
}

static ssize_t led_fadeon_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct led_classdev *led_cdev = NULL;
    struct balong_led_device *led_dev = NULL;

    led_cdev = dev_get_drvdata(dev);
    if (NULL == led_cdev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_cdev\n");
        return LED_ERROR;
    }

    led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    if (NULL == led_dev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev\n");
        return LED_ERROR;
    }

    if(NULL == led_dev->pdata)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev->pdata\n");
        return LED_ERROR;
    }

    return snprintf(buf, LED_SYSFS_MAX_STR_LENTH, "%lu\n", led_dev->pdata->fade_on);
}

static ssize_t led_fadeon_store(struct device *dev,	struct device_attribute *attr, const char *buf, size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct balong_led_device *led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    int ret = -EINVAL;
    char *after;
    unsigned long state = simple_strtoul(buf, &after, 10);
    size_t count = after - buf;

    if (isspace(*after))
        count++;

    if ((count == size) && led_cdev && led_dev && led_dev->pdata)
    {

        LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "%s full_on %d, full_off %d, fade_on %d, fade_off %d\n", led_cdev->name, led_dev->pdata->full_on,
                  led_dev->pdata->full_off, state, led_dev->pdata->fade_off);
        led_breath_set(led_cdev, &led_dev->pdata->full_on, &led_dev->pdata->full_off,
                       &state, &led_dev->pdata->fade_off);

        led_dev->pdata->fade_on = state;
        ret = count;
    }

    return ret;
}

static ssize_t led_fadeoff_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct led_classdev *led_cdev = NULL;

    struct balong_led_device *led_dev = NULL;

    if(NULL == dev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == dev\n");
        return LED_ERROR;
    }

    led_cdev = dev_get_drvdata(dev);
    if(NULL == led_cdev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_cdev\n");
        return LED_ERROR;
    }

    led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    if(NULL == led_dev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev\n");
        return LED_ERROR;
    }

    if(NULL == led_dev->pdata)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev->pdata\n");
        return LED_ERROR;
    }

    return snprintf(buf, LED_SYSFS_MAX_STR_LENTH, "%lu\n", led_dev->pdata->fade_off);
}

static ssize_t led_fadeoff_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    struct led_classdev *led_cdev = dev_get_drvdata(dev);
    struct balong_led_device *led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    int ret = -EINVAL;
    char *after;
    unsigned long state = simple_strtoul(buf, &after, 10);
    size_t count = after - buf;

    if (isspace(*after))
        count++;

    if ((count == size) && led_dev && led_dev->pdata)
    {

        LED_TRACE(LED_DEBUG_LEVEL(DEBUG), "%s full_on %d, full_off %d, fade_on %d, fade_off %d\n", led_cdev->name, led_dev->pdata->full_on,
                  led_dev->pdata->full_off, led_dev->pdata->fade_on, state);
        led_breath_set(led_cdev, &led_dev->pdata->full_on, &led_dev->pdata->full_off,
                       &led_dev->pdata->fade_on, &state);

        led_dev->pdata->fade_off = state;
        ret = count;
    }

    return ret;
}

static DEVICE_ATTR(full_on, 0644, led_fullon_show, led_fullon_store);
static DEVICE_ATTR(full_off, 0644, led_fulloff_show, led_fulloff_store);
static DEVICE_ATTR(fade_on, 0644, led_fadeon_show, led_fadeon_store);
static DEVICE_ATTR(fade_off, 0644, led_fadeoff_show, led_fadeoff_store);

static void breath_trig_activate(struct led_classdev *led_cdev)
{
    int rc;
    struct balong_led_device *led_dev = NULL;

    if(NULL == led_cdev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_cdev\n");
        return;
    }

    led_dev = (struct balong_led_device *)container_of(led_cdev, struct balong_led_device, cdev);
    if(NULL == led_dev)
    {
        LED_TRACE(LED_DEBUG_LEVEL(ERROR), "NULL == led_dev\n");
        return;
    }

    led_cdev->trigger_data = (void *)0;

    rc = device_create_file(led_cdev->dev, &dev_attr_full_on);
    if (rc)
        return;
    rc = device_create_file(led_cdev->dev, &dev_attr_full_off);
    if (rc)
        goto err_out_fulloff;
    rc = device_create_file(led_cdev->dev, &dev_attr_fade_on);
    if (rc)
        goto err_out_fadeon;
    rc = device_create_file(led_cdev->dev, &dev_attr_fade_off);
    if (rc)
        goto err_out_fadeoff;

    if(led_dev->pdata)
    {
        led_breath_set(led_cdev, &led_dev->pdata->full_on, &led_dev->pdata->full_off, &led_dev->pdata->fade_on, &led_dev->pdata->fade_off);
    }

    led_cdev->trigger_data = (void *)1;

    return;

err_out_fadeoff:
    device_remove_file(led_cdev->dev, &dev_attr_fade_on);
err_out_fadeon:
    device_remove_file(led_cdev->dev, &dev_attr_full_off);
err_out_fulloff:
    device_remove_file(led_cdev->dev, &dev_attr_full_on);
}

static void breath_trig_deactivate(struct led_classdev *led_cdev)
{
    if (led_cdev->trigger_data)
    {
        device_remove_file(led_cdev->dev, &dev_attr_full_on);
        device_remove_file(led_cdev->dev, &dev_attr_full_off);
        device_remove_file(led_cdev->dev, &dev_attr_fade_on);
        device_remove_file(led_cdev->dev, &dev_attr_fade_off);
    }

    /* Stop breathing */
    led_set_brightness(led_cdev, LED_OFF);
}

static struct led_trigger breath_led_trigger =
{
    .name     = "breath",
    .activate = breath_trig_activate,
    .deactivate = breath_trig_deactivate,
};

static int __init breath_trig_init(void)
{
    return led_trigger_register(&breath_led_trigger);
}

static void __exit breath_trig_exit(void)
{
    led_trigger_unregister(&breath_led_trigger);
}

module_init(breath_trig_init);
module_exit(breath_trig_exit);

MODULE_DESCRIPTION("Breath LED trigger");
MODULE_LICENSE("GPL");
