DEPS=gstreamer-0.10 gst-rtsp-server-0.10 gtk+-2.0
CXXFLAGS=`pkg-config --cflags $(DEPS)` -Wall -Werror -Wfatal-errors -Wextra -O2
CFLAGS=$(CXXFLAGS)
LDADD=`pkg-config --libs $(DEPS)`

all: camera_server test_client

%.o : %.cpp %.c %.h
	$(CXX) -c $(CXXFLAGS) $^ -o $@
 
#rtsp-my-media-factory.o
camera_server: camera_server.o rtsp-my-media-factory.o
	$(CXX) -Wall -g $(LDADD) $^ -o $@

test_client: test-client.o
	$(CXX) -Wall -g $(LDADD) $^ -o $@

.PHONY: clean

clean:
	rm -f camera_server *.o

