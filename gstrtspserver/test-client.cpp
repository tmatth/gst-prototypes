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
}

gboolean
timeout (GMainLoop *loop, gboolean /*ignored*/)
{
    if (interrupted)
    {
        g_print("Interrupted\n");
        g_main_loop_quit(loop);
    }
    return TRUE;
}

int main (int argc, char *argv[])
{
    attachInterruptHandlers();
    gst_init(&argc, &argv);

    GstElement *pipeline = gst_parse_launch("uridecodebin uri=rtsp://localhost:8554/test ! ffmpegcolorspace ! xvimagesink sync=false", 0);
    /* run */
    GstStateChangeReturn ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print ("Failed to start up pipeline!\n");
        return 1;
    }

    GMainLoop *loop = g_main_loop_new (NULL, FALSE);

    /* add a timeout to check the interrupted variable */
    g_timeout_add_seconds(1, (GSourceFunc) timeout, loop);

    /* start loop */
    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    g_print("Client exitting...\n");

    return 0;
}
