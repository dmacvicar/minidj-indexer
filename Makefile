all: dj-index

dj-index: dj-index.cpp
	g++ -DUSE_LIB64_PATHES -DQT_SHARED -DQT_THREAD_SUPPORT -L/usr/lib/qt3/lib64 -I/usr/lib/qt3/include -lqt-mt `taglib-config --cflags --libs` -o dj-index dj-index.cpp

clean:
	rm dj-index

