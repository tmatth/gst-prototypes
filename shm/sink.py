#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2011 Tristan Matthews <le.businessman@gmail.com>
# Some parts derived from:
#
# PiTiVi , Non-linear video editor
# Copyright (c) 2009, Edward Hervey <bilboed@bilboed.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

import sys
import os
import gobject # for mainloop
gobject.threads_init()
import pygst
pygst.require('0.10')
import gst

from time import sleep
    
class Pipeline(object):
    def __init__(self):
        self._mainloop = gobject.MainLoop()
        self._pipeline = gst.Pipeline()
        self._bus = self._pipeline.get_bus()
        self._bus.add_signal_watch()
        self._bus.connect("message", self._busMessageCb)
        videotestsrc = gst.element_factory_make('videotestsrc')
        shmsink = gst.element_factory_make('shmsink')
        shmsink.set_property('socket-path', 'test_shm')
        shmsink.set_property('shm-size', 2 ** 20)
        shmsink.get_pad('sink').connect('notify::caps', self._onNotifyCaps)
        self._pipeline.add(videotestsrc, shmsink)
        gst.element_link_many(videotestsrc, shmsink)

    def _onNotifyCaps(self, pad, unused_property):
        """
        Callback when caps are set for the shmsink element's
        sink pad. We then write these caps to a file.
        """
        caps = pad.props.caps
        if caps is None or not caps.is_fixed():
            return
        else:
            print "Caps are %s" % (caps)
            # write them to file
            with open('caps.txt', 'w') as f:
                read_data = f.write(caps.to_string() + '\n')
            assert f.closed

    
    def setState(self, state):
        """
        Set the L{Pipeline} to the given state.

        @raises PipelineError: If the C{gst.Pipeline} could not be changed to
        the requested state.
        """
        res = self._pipeline.set_state(state)
        if res == gst.STATE_CHANGE_FAILURE:
            # reset to NULL
            self._pipeline.set_state(gst.STATE_NULL)
            raise RuntimeError("Failure changing state of the gst.Pipeline to %r, currently reset to NULL" % state)
    
    def play(self):
        """
        Sets the L{Pipeline} to PLAYING
        """
        self.setState(gst.STATE_PLAYING)
        self._mainloop.run()
    
    def stop(self):
        """
        Sets the L{Pipeline} to READY
        """
        self.setState(gst.STATE_READY)

    def release(self):
            """
            Release the L{Pipeline} and all used L{ObjectFactory} and
            L{Action}s.

            Call this method when the L{Pipeline} is no longer used. Forgetting to do
            so will result in memory loss.

            @postcondition: The L{Pipeline} will no longer be usable.
            """
            self._bus.disconnect_by_func(self._busMessageCb)
            self._bus.remove_signal_watch()
            self._bus.set_sync_handler(None)
            self.setState(gst.STATE_NULL)
            self._bus = None
            self._pipeline = None

    def _handleErrorMessage(self, error, detail, source):
        print "error from %s: %s (%s)" % (source, error, detail)

    def _busMessageCb(self, unused_bus, message):
        if message.type == gst.MESSAGE_ERROR:
            error, detail = message.parse_error()
            self._handleErrorMessage(error, detail, message.src)
            self._mainloop.quit()           
        elif message.type == gst.MESSAGE_EOS:
            self._mainloop.quit()           
        else:
            pass
        
def run():
    """
    Starts a pipeline that will output video to a shared memory sink.
    """
    pipeline = Pipeline()
    try: 
        pipeline.play() # this will block
    except KeyboardInterrupt:
        print "Interrupted"
    finally:
        print "Exitting"
        
    pipeline.stop()
    pipeline.release() # this MUST be called to free resources
    os.remove('caps.txt')

if __name__ == '__main__':
    sys.exit(run())
