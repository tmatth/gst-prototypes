DEPS=gstreamer-0.10 gst-rtsp-server-0.10 gtk+-2.0
CXXFLAGS=`pkg-config --cflags $(DEPS)` -Wall -Werror -Wfatal-errors -Wextra -O2
CFLAGS=$(CXXFLAGS)
LDADD=`pkg-config --libs $(DEPS)`
APPS=camera_server test_client

all: $(APPS)

%.o : %.cpp %.c %.h
	$(CXX) -c $(CXXFLAGS) $^ -o $@
 
camera_server: camera_server.o rtsp-media-factory-custom.o
	$(CXX) -Wall -g $(LDADD) $^ -o $@

test_client: test-client.o
	$(CXX) -Wall -g $(LDADD) $^ -o $@

.PHONY: clean

clean:
	rm -f $(APPS) *.o

