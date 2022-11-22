#Butilca Rares

HEADERS=map-reducer.h powers.h
CPPS=powers.cpp

build: $(HEADERS) $(CPPS)
	g++ powers.cpp -o powers -Wall -Werror -lpthread

clean:
	rm -rf power
	
	
