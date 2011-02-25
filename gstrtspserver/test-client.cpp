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
#include <unistd.h>

struct Client {
    Client () : pipeline(0), rtpbin(0), loop(0) {}
    GstElement *pipeline;
    GstElement *rtpbin;
    GMainLoop *loop;
};

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

gboolean
timeout (Client *client, gboolean /*ignored*/)
{
    if (interrupted)
    {
        g_print("Interrupted\n");
        if (client->loop)
            g_main_loop_quit(client->loop);
        return FALSE;
    }
    return TRUE;
}

gboolean bus_call(GstBus * /*bus*/, GstMessage *msg, void *user_data)
{
    Client *context = static_cast<Client*>(user_data);
    if (interrupted)
    {
        g_print("Interrupted\n");
        if (context->loop)
            g_main_loop_quit(context->loop);
        return FALSE;
    }

    switch (GST_MESSAGE_TYPE(msg)) 
    {
        case GST_MESSAGE_EOS: 
            {
                g_message("End-of-stream");
                if (context->loop)
                    g_main_loop_quit(context->loop);
                break;
            }

        case GST_MESSAGE_LATENCY:
            {
                // when pipeline latency is changed, this msg is posted on the bus. we then have
                // to explicitly tell the pipeline to recalculate its latency
                // FIXME: this never works!
#if 0
                if (!gst_bin_recalculate_latency (GST_BIN(context->pipeline)))
                    g_print("Could not reconfigure latency.\n");
                else
                    g_print("Reconfigured latency.\n");
                break;
#endif
            }
        case GST_MESSAGE_ERROR: 
            {
                GError *err;
                gst_message_parse_error(msg, &err, NULL);
                g_error("%s", err->message);
                g_error_free(err);

                if (context->loop)
                    g_main_loop_quit(context->loop);

                break;

            }

        default:
            break;
    }

    return TRUE;
}
} // end anonymous namespace

int main (int argc, char *argv[])
{
    attachInterruptHandlers();
    gst_init(&argc, &argv);
    Client client;

    client.pipeline = gst_parse_launch("uridecodebin uri=rtsp://localhost:8554/test name=decode ! queue ! ffmpegcolorspace ! timeoverlay halignment=right ! xvimagesink decode. ! queue ! audioconvert ! autoaudiosink buffer-time=15000", 0);

    // add bus call
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(client.pipeline));
    gst_bus_add_watch(bus, bus_call, &client);
    gst_object_unref(bus);

    /* run */
    GstStateChangeReturn ret = gst_element_set_state (client.pipeline, GST_STATE_PLAYING);

    int tries = 0;
    while (client.rtpbin == 0 and tries < 10)
    {
        client.rtpbin = gst_bin_get_by_name (GST_BIN(client.pipeline),
                "rtpbin0");
        usleep(1000);
        tries++;
    }
    g_assert(client.rtpbin);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print ("Failed to start up pipeline!\n");
        return 1;
    }

    client.loop = g_main_loop_new (NULL, FALSE);

    /* add a timeout to check the interrupted variable */
    g_timeout_add_seconds(1, (GSourceFunc) timeout, &client);
    
    g_object_set(client.rtpbin, "latency", 15, NULL);

    /* start loop */
    g_main_loop_run (client.loop);

    /* clean up */
    gst_element_set_state (client.pipeline, GST_STATE_NULL);
    gst_object_unref (client.pipeline);

    g_print("Client exitting...\n");

    return 0;
}
