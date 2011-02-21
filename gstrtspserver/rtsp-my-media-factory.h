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

#include <gst/gst.h>
#include <gst/rtsp/gstrtspurl.h>

#include <gst/rtsp-server/rtsp-media-factory.h>

#ifndef __GST_RTSP_MY_MEDIA_FACTORY_H__
#define __GST_RTSP_MY_MEDIA_FACTORY_H__

G_BEGIN_DECLS

/* types for the media factory */
#define GST_TYPE_RTSP_MY_MEDIA_FACTORY              (gst_rtsp_my_media_factory_get_type ())
#define GST_IS_RTSP_MY_MEDIA_FACTORY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_RTSP_MY_MEDIA_FACTORY))
#define GST_IS_RTSP_MY_MEDIA_FACTORY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_RTSP_MY_MEDIA_FACTORY))
#define GST_RTSP_MY_MEDIA_FACTORY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_RTSP_MY_MEDIA_FACTORY, GstRTSPMyMediaFactoryClass))
#define GST_RTSP_MY_MEDIA_FACTORY(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_RTSP_MY_MEDIA_FACTORY, GstRTSPMyMediaFactory))
#define GST_RTSP_MY_MEDIA_FACTORY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_RTSP_MY_MEDIA_FACTORY, GstRTSPMyMediaFactoryClass))
#define GST_RTSP_MY_MEDIA_FACTORY_CAST(obj)         ((GstRTSPMyMediaFactory*)(obj))
#define GST_RTSP_MY_MEDIA_FACTORY_CLASS_CAST(klass) ((GstRTSPMyMediaFactoryClass*)(klass))

typedef struct _GstRTSPMyMediaFactory GstRTSPMyMediaFactory;
typedef struct _GstRTSPMyMediaFactoryClass GstRTSPMyMediaFactoryClass;

#define GST_RTSP_MY_MEDIA_FACTORY_GET_LOCK(f)       (GST_RTSP_MY_MEDIA_FACTORY_CAST(f)->lock)
#define GST_RTSP_MY_MEDIA_FACTORY_LOCK(f)           (g_mutex_lock(GST_RTSP_MY_MEDIA_FACTORY_GET_LOCK(f)))
#define GST_RTSP_MY_MEDIA_FACTORY_UNLOCK(f)         (g_mutex_unlock(GST_RTSP_MY_MEDIA_FACTORY_GET_LOCK(f)))

/**
 * GstRTSPMyMediaFactory:
 * @lock: mutex protecting the datastructure.
 * @bin: the bin used for streaming
 * @shared: if media from this factory can be shared between clients
 *
 * The definition and logic for constructing the pipeline for a media. The media
 * can contain multiple streams like audio and video.
 */
struct _GstRTSPMyMediaFactory {
  GstRTSPMediaFactory parent;
  GMutex       *lock;
  GstBin        *bin;
};

/**
 * GstRTSPMyMediaFactoryClass:
 * @get_element: Construct and return a #GstElement that is a #GstBin containing
 *       the elements to use for streaming the media. The bin should contain
 *       payloaders pay%d for each stream. The default implementation of this
 *       function returns the bin created from the launch parameter.
 *
 * The #GstRTSPMyMediaFactory class structure.
 */
struct _GstRTSPMyMediaFactoryClass {
  GstRTSPMediaFactoryClass  parent_class;

  GstElement *      (*get_element)    (GstRTSPMyMediaFactory *factory, const GstRTSPUrl *url);
};

GType                 gst_rtsp_my_media_factory_get_type        (void);

/* creating the factory */
GstRTSPMyMediaFactory * gst_rtsp_my_media_factory_new           (void);

/* configuring the factory */
void                  gst_rtsp_my_media_factory_set_bin (GstRTSPMyMediaFactory *factory,
                                                           GstBin *bin);
GstBin *              gst_rtsp_my_media_factory_get_bin (GstRTSPMyMediaFactory *factory);

G_END_DECLS

#endif /* __GST_RTSP_MY_MEDIA_FACTORY_H__ */
