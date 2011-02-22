gst-launch -v uridecodebin uri=rtsp://localhost:8554/test name=bin ! ffmpegcolorspace ! xvimagesink sync=false \
              bin. ! audioconvert ! autoaudiosink 
