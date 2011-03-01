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

#include <gst/gst.h>

#include <gst/rtsp-server/rtsp-media-factory.h>

#ifndef __GST_RTSP_MEDIA_FACTORY_CUSTOM_H__
#define __GST_RTSP_MEDIA_FACTORY_CUSTOM_H__

G_BEGIN_DECLS

/* types for the media factory */
#define GST_TYPE_RTSP_MEDIA_FACTORY_CUSTOM              (gst_rtsp_media_factory_custom_get_type ())
#define GST_IS_RTSP_MEDIA_FACTORY_CUSTOM(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_RTSP_MEDIA_FACTORY_CUSTOM))
#define GST_IS_RTSP_MEDIA_FACTORY_CUSTOM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_RTSP_MEDIA_FACTORY_CUSTOM))
#define GST_RTSP_MEDIA_FACTORY_CUSTOM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_RTSP_MEDIA_FACTORY_CUSTOM, GstRTSPMediaFactoryCustomClass))
#define GST_RTSP_MEDIA_FACTORY_CUSTOM(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_RTSP_MEDIA_FACTORY_CUSTOM, GstRTSPMediaFactoryCustom))
#define GST_RTSP_MEDIA_FACTORY_CUSTOM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_RTSP_MEDIA_FACTORY_CUSTOM, GstRTSPMediaFactoryCustomClass))
#define GST_RTSP_MEDIA_FACTORY_CUSTOM_CAST(obj)         ((GstRTSPMediaFactoryCustom*)(obj))
#define GST_RTSP_MEDIA_FACTORY_CUSTOM_CLASS_CAST(klass) ((GstRTSPMediaFactoryCustomClass*)(klass))

typedef struct _GstRTSPMediaFactoryCustom GstRTSPMediaFactoryCustom;
typedef struct _GstRTSPMediaFactoryCustomClass GstRTSPMediaFactoryCustomClass;

/**
 * GstRTSPMediaFactoryCustom:
 * @bin: the bin used for streaming
 *
 * The definition and logic for constructing the pipeline for a media. The media
 * can contain multiple streams like audio and video.
 */
struct _GstRTSPMediaFactoryCustom {
  GstRTSPMediaFactory parent;
  GstElement  *bin;
};

/**
 * GstRTSPMediaFactoryCustomClass:
 * @get_element: Construct and return a #GstElement that is a #GstBin containing
 *       the elements to use for streaming the media. The bin should contain
 *       payloaders pay%d for each stream. The default implementation of this
 *       function returns the bin created from the launch parameter.
 *
 * The #GstRTSPMediaFactoryCustom class structure.
 */
struct _GstRTSPMediaFactoryCustomClass {
  GstRTSPMediaFactoryClass  parent_class;
};

GType                 gst_rtsp_media_factory_custom_get_type        (void);

/* creating the factory */
GstRTSPMediaFactoryCustom * gst_rtsp_media_factory_custom_new           (void);

/* configuring the factory */
void                  gst_rtsp_media_factory_custom_set_bin (GstRTSPMediaFactoryCustom *factory,
                                                           GstElement *bin);
GstElement *          gst_rtsp_media_factory_custom_get_bin (GstRTSPMediaFactoryCustom *factory);

G_END_DECLS

#endif /* __GST_RTSP_MEDIA_FACTORY_CUSTOM_H__ */
