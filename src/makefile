CXX=g++
CXXFLAGS=-Wall -std=c++14 -I/usr/include/freetype2

LIBS=-lcurl -ljansson -lfreetype -lglfw -lIL -lGL

OBJ = curl_http.* disney_guide.*  json_parser.* text2bmp.* decompress_jpeg.* guide_obj.* parse_guide.*

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

all: DisneyGuide

DisneyGuide: $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)
	
install:
	cp DisneyGuide ../dependencies/bin

.PHONY: clean

clean:
	rm -f *.o *~ DisneyGuide 

