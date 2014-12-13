all: playersserver mainserver client gui clean copy_executable

playersserver: players_server.c
	gcc players_server.c -o players_server.o -lpthread

mainserver: modele_serveur.c
	gcc -lpthread modele_serveur.c -o server.o 

client: modele_client.c
	gcc modele_client.c -o client.o 

gui: othello_GUI.c
	gcc -lpthread -lX11 -o othello_GUI.o othello_GUI.c $(shell pkg-config --cflags --libs gtk+-3.0 x11)

clean:
	rm ../OthelloBis/*.fifo # TODO update the path
	rm ../OthelloBis/*.o # TODO update the path
	
copy_executable:
	cp *.o ../JB/ # TODO update the path
