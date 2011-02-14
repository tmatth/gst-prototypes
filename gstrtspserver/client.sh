gst-launch -v uridecodebin uri=rtsp://localhost:8554/test ! ffmpegcolorspace ! xvimagesink sync=false
