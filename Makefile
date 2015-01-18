executable:=./src/Lab3Sistope2

build:=./build
src:=./src

objects:=$(build)/md5.o
sources:=$(src)/md5.cpp

cxx:=g++

includes:=-I./src/include -lpthread

main: $(objects)
	$(cxx) $(includes) $(objects) -pthread -o $(build)/laboratorio3 $(executable).cpp

$(build)/%.o: $(src)/%.cpp 
	test -d $(build) || mkdir $(build)
	$(cxx) $(includes) -c -o $(build)/$(*).o $(src)/$*.cpp

clean:
	rm $(build)/*;

run:
	./build/laboratorio3 -d L33t.dic -r TodoOferta.sql -h 8 -c 50


	
