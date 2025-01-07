build:
	gcc adv.c -Wall -Wpedantic -lSDL2 -lSDL2_ttf -o adv-game

run:
	./adv-game

clean:
	rm adv-game
