build:
	gcc cyg-x1.c -Wall -Wpedantic -lSDL2 -lSDL2_ttf -o cyg-x1 -fsanitize=address

run:
	./cyg-x1

clean:
	rm cyg-x1
