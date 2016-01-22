CC = clang++
CFLAGS = -Wall
CFLAGS_DEBUG = -ldc1394 -Wall -g
LDFLAGS=`pkg-config --cflags --libs opencv` -ldc1394 -Wall
LDFLAGS_DEBUG=`pkg-config --cflags --libs opencv` -ldc1394 -g -Wall

all: segment

segment: segment.cpp
	${CC} ${LDFLAGS} segment.cpp -o segment
