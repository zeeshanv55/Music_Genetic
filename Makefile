all:
	g++ midi2notes.cpp -I include -Llib -lmidifile -o bin/midi2notes
	g++ main.cpp -I include -o bin/main
