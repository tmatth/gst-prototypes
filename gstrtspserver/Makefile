DEPS=gstreamer-0.10 gst-rtsp-server-0.10 gtk+-2.0
CXXFLAGS=`pkg-config --cflags $(DEPS)` -Wall -Werror -Wfatal-errors -Wextra -O2
LIBS=`pkg-config --libs $(DEPS)`

all: camera_server test_client

%.o : %.cpp %.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

camera_server: camera_server.o
	$(CXX) -Wall -g $(LIBS) $^ -o $@

test_client: test-client.o
	$(CXX) -Wall -g $(LIBS) $^ -o $@

.PHONY: clean

clean:
	rm -f camera_server *.o

