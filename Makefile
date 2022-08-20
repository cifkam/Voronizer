CC = g++
PROJECT = voronoi
CPP = *.cpp
HPP = *.hpp
LIBS = `pkg-config --cflags --libs opencv4`
$(PROJECT) : $(CPP) $(HPP)
	$(CC) -std=c++17 -g $(CPP) -o $(PROJECT) $(LIBS)

clean:
	rm voronoi