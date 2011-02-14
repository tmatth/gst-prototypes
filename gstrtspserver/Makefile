DEPS=gstreamer-0.10 gst-rtsp-server-0.10
CXXFLAGS=`pkg-config --cflags $(DEPS)` -Wall -Werror -Wfatal-errors -Wextra -O2
LIBS=`pkg-config --libs $(DEPS)`
OBJS=camera_server.o

all: camera_server

%.o : %.cpp %.h
	$(CC) -c $(CXXFLAGS) $< -o $@

camera_server: $(OBJS)
	g++ -Wall -g $(LIBS) $^ -o $@

.PHONY: clean

clean:
	rm -f camera_server *.o

