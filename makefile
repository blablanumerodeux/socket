othello : mainserver gui  

mainserver: modele_serveur.c  
	gcc modele_serveur.c -o server.o 

gui: othello_GUI.c 
	gcc -lpthread -lX11 -o othello_GUI.o othello_GUI.c $(shell pkg-config --cflags --libs gtk+-3.0)
