oth: othello_GUI.c 
	gcc -Wall -lpthread -o othello_GUI.o othello_GUI.c $(pkg-config --cflags --libs gtk+-3.0)
