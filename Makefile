TARGET=video-rotater
CXX = g++
LIBS=opencv libgflags

all:
	g++ src/main.cpp -o bin/$(TARGET) `pkg-config $(LIBS) --libs --cflags`

debug:
	g++ src/main.cpp -o bin/$(TARGET) `pkg-config $(LIBS) --libs --cflags` -g

clean:
	rm $(TARGET)
