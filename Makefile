CC = g++
PROJECT = Voronizer
CPP = *.cpp
HPP = *.hpp
LIBS = `pkg-config --cflags --libs opencv4`
$(PROJECT) : $(CPP) $(HPP)
	$(CC) -std=c++17 -g $(CPP) -o $(PROJECT) $(LIBS) -I include

clean:
	rm -f $(PROJECT)