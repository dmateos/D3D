objects = \
					src/main.o \
					src/network.o \
					src/util.o \
					src/D3DObject.o\

flags = -lglfw3 -lglew -lassimp -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo 

d3d: $(objects)
	$(CXX) $(flags) -o $@ $(objects)

src/main.o: src/main.cpp
	$(CC) -c -o $@ src/main.cpp

src/network.o: src/network.cpp
	$(CC) -c -o $@ src/network.cpp

src/util.o: src/util.cpp
	$(CC) -c -o $@ src/util.cpp

src/D3DObject.o: src/D3DObject.cpp
	$(CC) -c -o $@ src/D3DObject.cpp

clean:
	rm -rf src/*.o d3d
