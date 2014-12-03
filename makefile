all: mainserver client gui  

mainserver: modele_serveur.c  
	gcc modele_serveur.c -o server.o 

client: modele_client.c  
	gcc modele_client.c -o client.o 

gui: othello_GUI.c 
	gcc -lpthread -lX11 -o othello_GUI.o othello_GUI.c $(shell pkg-config --cflags --libs gtk+-3.0)

clean:
	rm -rf *.o

#rm *.fifo ../socket/*.fifo || cp ../socket/*.o .
