/* 
 * Based on:
 * GStreamer
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
#include <signal.h>
#include <string>

#include <gst/rtsp-server/rtsp-server.h>
#include "rtsp-media-factory-custom.h"

namespace {
volatile int interrupted = 0; // caught signals will be stored here
void terminateSignalHandler(int sig)
{
    interrupted = sig;
}

void attachInterruptHandlers()
{
    // attach interrupt handlers
    signal(SIGINT, &terminateSignalHandler);
    signal(SIGTERM, &terminateSignalHandler);
}

struct Data {
    GstRTSPServer *server;
    GMainLoop *loop;
};

void cleanSessions(GstRTSPServer *server)
{
  GstRTSPSessionPool *pool;

  pool = gst_rtsp_server_get_session_pool (server);
  gst_rtsp_session_pool_cleanup (pool);
  g_object_unref (pool);
}

/* this timeout is periodically run to clean up the expired sessions from the
 * pool. This needs to be run explicitly currently but might be done
 * automatically as part of the mainloop. */
gboolean
timeout (Data *data, gboolean /*ignored*/)
{
  gboolean result = FALSE;
  if (interrupted)
  {
      g_print("Interrupted, quitting...\n");
      if (data->loop)
          g_main_loop_quit(data->loop);
  }
  else
  {
      cleanSessions(data->server);
      result = TRUE;
  }
  return result;
}

} // end anonymous namespace

int
main (int argc, char *argv[])
{
  attachInterruptHandlers();

  Data data;
  GstRTSPMediaMapping *mapping;
  GstRTSPMediaFactory *factory;

  gst_init (&argc, &argv);


  /* create the main loop */
  data.loop = g_main_loop_new (NULL, FALSE);
  /* create a server instance */
  data.server = gst_rtsp_server_new ();

  /* get the mapping for this server, every server has a default mapper object
   * that be used to map uri mount points to media factories */
  mapping = gst_rtsp_server_get_media_mapping (data.server);
  /* don't need the ref to the mapper anymore */
  g_object_unref (mapping);


  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines. 
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  factory = GST_RTSP_MEDIA_FACTORY(gst_rtsp_media_factory_custom_new());

  // allow multiple clients to see the same video
  gst_rtsp_media_factory_set_shared (factory, TRUE);

  static const std::string launchLine("( v4l2src ! video/x-raw-yuv,width=640,height=480,framerate=30/1,format=(fourcc)UYVY ! "
      "ffmpegcolorspace ! timeoverlay ! ffenc_mpeg4 bitrate=3000000 ! rtpmp4vpay name=pay0 pt=96 "
      "autoaudiosrc ! audioconvert ! rtpL16pay max-ptime=2000000 name=pay1 pt=97 )");

  GstElement *pipeline = gst_parse_launch(launchLine.c_str(), 0);

  g_object_set(factory, "bin", pipeline, NULL);

  /* attach the test factory to the /test url */
  gst_rtsp_media_mapping_add_factory (mapping, "/test", factory);

  guint id;
  /* attach the server to the default maincontext */
  if ((id = gst_rtsp_server_attach (data.server, g_main_loop_get_context(data.loop))) == 0)
  {
    g_print ("failed to attach the server\n");
    return -1;
  }

  /* add a timeout for the session cleanup */
  g_timeout_add_seconds(1, (GSourceFunc) timeout, &data);

  /* start serving, this never stops */
  g_main_loop_run (data.loop);

  // cleanup
  
  //g_source_remove(id);
  g_object_unref(data.server);
  g_print("Exitting...\n");

  return 0;
}
