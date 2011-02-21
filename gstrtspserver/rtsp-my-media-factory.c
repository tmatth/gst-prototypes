/* GStreamer
 * Copyright (C) 2008 Wim Taymans <wim.taymans at gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gst/rtsp-server/rtsp-media-factory.h>
#include "rtsp-my-media-factory.h"

enum
{
  PROP_0,
  PROP_BIN,
  PROP_LAST
};

GST_DEBUG_CATEGORY_STATIC (rtsp_my_media_debug);
#define GST_CAT_DEFAULT rtsp_my_media_debug

static void gst_rtsp_my_media_factory_get_property (GObject * object, guint propid,
    GValue * value, GParamSpec * pspec);
static void gst_rtsp_my_media_factory_set_property (GObject * object, guint propid,
    const GValue * value, GParamSpec * pspec);
static void gst_rtsp_my_media_factory_finalize (GObject * obj);
static GstElement *
my_get_element (GstRTSPMyMediaFactory * factory, const GstRTSPUrl * url);

G_DEFINE_TYPE (GstRTSPMyMediaFactory, gst_rtsp_my_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY);

static void
gst_rtsp_my_media_factory_class_init (GstRTSPMyMediaFactoryClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->get_property = gst_rtsp_my_media_factory_get_property;
  gobject_class->set_property = gst_rtsp_my_media_factory_set_property;
  gobject_class->finalize = gst_rtsp_my_media_factory_finalize;

  /**
   * GstRTSPMediaFactory::bin
   *
   * The bin used in the default prepare vmethod.
   *
   * The pipeline should have payloaders named pay0, pay1,
   * etc.. Each of the payloaders will result in a stream.
   *
   * Support for dynamic payloaders can be accomplished by adding payloaders
   * named dynpay0, dynpay1, etc..
   */
  g_object_class_install_property (gobject_class, PROP_BIN,
      g_param_spec_object ("bin", "Bin", "Bin object used", 
          GST_TYPE_ELEMENT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  klass->get_element = my_get_element;

  GST_DEBUG_CATEGORY_INIT (rtsp_my_media_debug, "rtspmymediafactory", 0,
      "GstRTSPMyMediaFactory");
}

static void
gst_rtsp_my_media_factory_init (GstRTSPMyMediaFactory * factory)
{
  factory->bin = NULL;
  factory->lock = g_mutex_new ();
}

static void
gst_rtsp_my_media_factory_finalize (GObject * obj)
{
  GstRTSPMyMediaFactory *factory = GST_RTSP_MY_MEDIA_FACTORY (obj);

  if (factory->bin)
      g_object_unref (factory->bin);
  g_mutex_free (factory->lock);

  G_OBJECT_CLASS (gst_rtsp_my_media_factory_parent_class)->finalize (obj);
}

static void
gst_rtsp_my_media_factory_get_property (GObject * object, guint propid,
    GValue * value, GParamSpec * pspec)
{
  GstRTSPMyMediaFactory *factory = GST_RTSP_MY_MEDIA_FACTORY (object);

  switch (propid) {
    case PROP_BIN:
      g_value_set_object (value, factory->bin);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propid, pspec);
  }
}

static void
gst_rtsp_my_media_factory_set_property (GObject * object, guint propid,
    const GValue * value, GParamSpec * pspec)
{
  GstRTSPMyMediaFactory *factory = GST_RTSP_MY_MEDIA_FACTORY (object);

  switch (propid) {
    case PROP_BIN:
      gst_rtsp_my_media_factory_set_bin (factory, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propid, pspec);
  }
}

/**
 * gst_rtsp_my_media_factory_new:
 *
 * Create a new #GstRTSPMyMediaFactory instance.
 *
 * Returns: a new #GstRTSPMyMediaFactory object.
 */
GstRTSPMyMediaFactory *
gst_rtsp_my_media_factory_new (void)
{
  GstRTSPMyMediaFactory *result;

  result = g_object_new (GST_TYPE_RTSP_MY_MEDIA_FACTORY, NULL);

  return result;
}

/**
 * gst_rtsp_my_media_factory_set_bin:
 * @factory: a #GstRTSPMyMediaFactory
 * @bin: the bin object
 *
 * The bin to use in the default prepare vmethod.
 *
 * The bin should contain a pipeline with payloaders named pay0, pay1,
 * etc.. Each of the payloaders will result in a stream.
 */
void
gst_rtsp_my_media_factory_set_bin (GstRTSPMyMediaFactory * factory,
    GstBin * bin)
{
  g_return_if_fail (GST_IS_RTSP_MY_MEDIA_FACTORY (factory));
  g_return_if_fail (bin != NULL);

  GST_RTSP_MY_MEDIA_FACTORY_LOCK (factory);
  if (bin != factory->bin) {
      GstBin *old;

      old = factory->bin;
      if (bin)
          gst_object_ref_sink (bin);

      factory->bin = bin;
      if (old)
          gst_object_unref (old);
  }
  GST_RTSP_MY_MEDIA_FACTORY_UNLOCK (factory);
}

/**
 * gst_rtsp_my_media_factory_get_bin:
 * @factory: a #GstRTSPMyMediaFactory
 *
 * Get the bin that will be used in the
 * default prepare vmethod.
 *
 * Returns: the configured launch description. g_free() after usage.
 */
GstBin *
gst_rtsp_media_factory_get_bin (GstRTSPMyMediaFactory * factory)
{
  GstBin *bin; 

  g_return_val_if_fail (GST_IS_RTSP_MY_MEDIA_FACTORY (factory), NULL);

  GST_RTSP_MY_MEDIA_FACTORY_LOCK (factory);
  bin = factory->bin;
  GST_RTSP_MY_MEDIA_FACTORY_UNLOCK (factory);

  return bin;
}

static GstElement *
my_get_element (GstRTSPMyMediaFactory * factory, const GstRTSPUrl * url)
{
  GstElement *element;
  (void) url; // unused

  GST_RTSP_MY_MEDIA_FACTORY_LOCK (factory);
  /* we need a parse syntax */
  if (factory->bin == NULL)
    goto no_bin;

  /* get the user provided bin */
  element = GST_ELEMENT(factory->bin);

  GST_RTSP_MY_MEDIA_FACTORY_UNLOCK (factory);

  return element;

  /* ERRORS */
no_bin:
  {
    GST_RTSP_MY_MEDIA_FACTORY_UNLOCK (factory);
    g_critical ("no bin specified");
    return NULL;
  }
}
