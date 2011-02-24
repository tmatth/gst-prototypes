/* GStreamer
 * Copyright (C) 2011 Tristan Matthews <le.businessman at gmail.com>
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
#include "rtsp-media-factory-custom.h"

enum
{
  PROP_0,
  PROP_BIN,
  PROP_LAST
};

GST_DEBUG_CATEGORY_STATIC (rtsp_media_factory_custom_debug);
#define GST_CAT_DEFAULT rtsp_media_factory_custom_debug

static void gst_rtsp_media_factory_custom_get_property (GObject * object, guint propid,
    GValue * value, GParamSpec * pspec);
static void gst_rtsp_media_factory_custom_set_property (GObject * object, guint propid,
    const GValue * value, GParamSpec * pspec);
static void gst_rtsp_media_factory_custom_finalize (GObject * obj);
static GstElement *
custom_get_element (GstRTSPMediaFactory * factory, const GstRTSPUrl * url);

G_DEFINE_TYPE (GstRTSPMediaFactoryCustom, gst_rtsp_media_factory_custom, GST_TYPE_RTSP_MEDIA_FACTORY /*parent class*/);

static void
gst_rtsp_media_factory_custom_class_init (GstRTSPMediaFactoryCustomClass * klass)
{
  GObjectClass *gobject_class;
  GstRTSPMediaFactoryClass *gstrtspmediafactory_class;

  gobject_class = (GObjectClass *) klass;
  gstrtspmediafactory_class = (GstRTSPMediaFactoryClass *) klass;

  gobject_class->get_property = gst_rtsp_media_factory_custom_get_property;
  gobject_class->set_property = gst_rtsp_media_factory_custom_set_property;
  gobject_class->finalize = gst_rtsp_media_factory_custom_finalize;

  /**
   * GstRTSPMediaFactoryCustom::bin
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

  gstrtspmediafactory_class->get_element = custom_get_element;

  GST_DEBUG_CATEGORY_INIT (rtsp_media_factory_custom_debug, "rtspmediafactorycustom", 0,
      "GstRTSPMediaFactoryCustom");
}

static void
gst_rtsp_media_factory_custom_init (GstRTSPMediaFactoryCustom * factory)
{
  factory->bin = NULL;
}

static void
gst_rtsp_media_factory_custom_finalize (GObject * obj)
{
  GstRTSPMediaFactoryCustom *factory = GST_RTSP_MEDIA_FACTORY_CUSTOM (obj);

  if (factory->bin)
      gst_object_unref (factory->bin);

  G_OBJECT_CLASS (gst_rtsp_media_factory_custom_parent_class)->finalize (obj);
}

static void
gst_rtsp_media_factory_custom_get_property (GObject * object, guint propid,
    GValue * value, GParamSpec * pspec)
{
  GstRTSPMediaFactoryCustom *factory = GST_RTSP_MEDIA_FACTORY_CUSTOM (object);

  switch (propid) {
    case PROP_BIN:
      g_value_set_object (value, gst_rtsp_media_factory_custom_get_bin(factory));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propid, pspec);
  }
}

static void
gst_rtsp_media_factory_custom_set_property (GObject * object, guint propid,
    const GValue * value, GParamSpec * pspec)
{
  GstRTSPMediaFactoryCustom *factory = GST_RTSP_MEDIA_FACTORY_CUSTOM (object);

  switch (propid) {
    case PROP_BIN:
      gst_rtsp_media_factory_custom_set_bin (factory, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propid, pspec);
  }
}

/**
 * gst_rtsp_media_factory_custom_new:
 *
 * Create a new #GstRTSPMediaFactoryCustom instance.
 *
 * Returns: a new #GstRTSPMediaFactoryCustom object.
 */
GstRTSPMediaFactoryCustom *
gst_rtsp_media_factory_custom_new (void)
{
  GstRTSPMediaFactoryCustom *result;

  result = g_object_new (GST_TYPE_RTSP_MEDIA_FACTORY_CUSTOM, NULL);

  return result;
}

/**
 * gst_rtsp_media_factory_custom_set_bin:
 * @factory: a #GstRTSPMediaFactoryCustom
 * @bin: the bin object
 *
 * The bin to use in the default prepare vmethod.
 *
 * The bin should contain a pipeline with payloaders named pay0, pay1,
 * etc.. Each of the payloaders will result in a stream.
 */
void
gst_rtsp_media_factory_custom_set_bin (GstRTSPMediaFactoryCustom * factory,
    GstElement * bin)
{
  GstElement *old;

  g_return_if_fail (GST_IS_RTSP_MEDIA_FACTORY_CUSTOM (factory));
  g_return_if_fail (bin != NULL);

  g_mutex_lock (GST_RTSP_MEDIA_FACTORY(factory)->lock);

  old = factory->bin;

  if (old != bin) {
      if (bin)
          gst_object_ref (bin);
      factory->bin = bin;
      if (old)
          gst_object_unref (old);
  }
  g_mutex_unlock (GST_RTSP_MEDIA_FACTORY(factory)->lock);
}

/**
 * gst_rtsp_media_factory_custom_get_bin:
 * @factory: a #GstRTSPMediaFactoryCustom
 *
 * Get the bin that will be used in the
 * default prepare vmethod.
 *
 * Returns: the configured bin.
 */
GstElement *
gst_rtsp_media_factory_custom_get_bin (GstRTSPMediaFactoryCustom * factory)
{
  GstElement *bin; 

  g_return_val_if_fail (GST_IS_RTSP_MEDIA_FACTORY_CUSTOM (factory), NULL);

  g_mutex_lock (GST_RTSP_MEDIA_FACTORY(factory)->lock);

  if ((bin = factory->bin))
      gst_object_ref (bin);

  g_mutex_unlock (GST_RTSP_MEDIA_FACTORY(factory)->lock);

  return bin;
}

static GstElement *
custom_get_element (GstRTSPMediaFactory * factory, const GstRTSPUrl * url)
{
  GstElement *element;
  GError *error = NULL;
  (void) url; // unused

  g_mutex_lock (factory->lock);

  /* we need a bin */
  if (GST_RTSP_MEDIA_FACTORY_CUSTOM(factory)->bin == NULL) {
      if (factory->launch == NULL)
          goto no_launch_or_bin;
      else {
          /* parse the user provided launch line */
          element = gst_parse_launch (factory->launch, &error);
          if (element == NULL)
              goto parse_error;
      }
  }
  else /* get the user provided bin */
      element = GST_RTSP_MEDIA_FACTORY_CUSTOM(factory)->bin;
    
  g_mutex_unlock (factory->lock);

  if (error != NULL) {
    /* a recoverable error was encountered */
    GST_WARNING ("recoverable parsing error: %s", error->message);
    g_error_free (error);
  }
  return element;

  /* ERRORS */
no_launch_or_bin:
  {
    g_mutex_unlock (factory->lock);
    g_critical ("no launch line or bin specified");
    return NULL;
  }
parse_error:
  {
    g_mutex_unlock (factory->lock);
    g_critical ("could not parse launch syntax (%s): %s", factory->launch,
        (error ? error->message : "unknown reason"));
    if (error)
      g_error_free (error);
    return NULL;
  }
}
