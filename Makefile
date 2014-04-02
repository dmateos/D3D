client_objects = \
					src/main.o \
					src/network.o \
					src/util.o \
					src/D3DObject.o\

client_flags = -lglfw3 -lglew -lassimp -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

server_objects = \
					src/mserver/main.cpp

server_flags =

all: d3d d3d-server

#Client
d3d: $(client_objects)
	$(CXX) $(client_flags) -o $@ $(client_objects)

src/main.o: src/main.cpp
	$(CC) -c -o $@ src/main.cpp

src/network.o: src/network.cpp
	$(CC) -c -o $@ src/network.cpp

src/util.o: src/util.cpp
	$(CC) -c -o $@ src/util.cpp

src/D3DObject.o: src/D3DObject.cpp
	$(CC) -c -o $@ src/D3DObject.cpp

#Server
d3d-server: $(server_objects)
	$(CXX) $(server_flags) -o $@ $(server_objects)

src/mserver/main.o: src/mserver.main.cpp
	$(CC) -c -o $@ src/mserver/main.cpp

clean:
	rm -rf src/*.o d3d
	rm -rf src/mserver/*.o d3d-server
