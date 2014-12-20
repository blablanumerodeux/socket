#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netdb.h>
#include <signal.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <X11/Xlib.h>

int MAXDATASIZE = 100;
int descGuiToClient;
int descServerToGui;	

/* Variables globales */
int damier[8][8];	// tableau associe au damier
int couleur;		// 0 : pour noir, 1 : pour blanc
int nbCoup;			// Nombre de coup joué
int msgConnect = 0; // Message connexion établie montré déjà une fois (1) ou pas (0)

int port;		// numero port passe a l'appel

int pid;
int pidClient;

char *addr_j2, *port_j2;	// Info sur adversaire

/* Variables globales associées à l'interface graphique */
GtkBuilder  *  p_builder   = NULL;
GError      *  p_err       = NULL;

// Entetes des fonctions  

/* Fonction permettant de changer l'image d'une case du damier (indiqué par sa colonne et sa ligne) */
void change_img_case(int col, int lig, int couleur_j);

/* Fonction permettant changer nom joueur blanc dans cadre Score */
void set_label_J1(char *texte);

/* Fonction permettant récupérer nom joueur noir dans cadre Score */
char *get_label_J1(void);

/* Fonction permettant de changer nom joueur noir dans cadre Score */
void set_label_J2(char *texte);

/* Fonction permettant récupérer nom joueur blanc dans cadre Score */
char *get_label_J2(void);

/* Fonction permettant de changer score joueur blanc dans cadre Score */
void set_score_J1(int score);

/* Fonction permettant de récupérer score joueur blanc dans cadre Score */
int get_score_J1(void);

/* Fonction permettant de changer score joueur noir dans cadre Score */
void set_score_J2(int score);

/* Fonction permettant de récupérer score joueur noir dans cadre Score */
int get_score_J2(void);

/* Fonction pour mettre le joueur actif en gras */
void bold_label_player(int player);

/* Fonction transformant coordonnees du damier graphique en indexes pour matrice du damier */
void coord_to_indexes(const gchar *coord, int *col, int *lig);

/* Fonction transformant indexes en coordonnees du damier graphique pour matrice du damier */
void indexes_to_coord(int col, int lig, char *coord);

/* Fonction calculant les scores courants */
void calcul_scores();

/* Fonction d'encadrement de la droite vers la gauche */
void encadrement_D_G(int col_piece, int lig_piece, int couleur_joueur);

/* Fonction d'encadrement de la gauche vers la droite */
void encadrement_G_D(int col_piece, int lig_piece, int couleur_joueur);

/* Fonction d'encadrement de haut vers bas */
void encadrement_H_B(int col_piece, int lig_piece, int couleur_joueur);

/* Fonction d'encadrement de bas vers haut */
void encadrement_B_H(int col_piece, int lig_piece, int couleur_joueur);

/* Fonction d'encadrement diagonale vers Nord Est */
void encadrement_NE(int col_piece, int lig_piece, int couleur_joueur);

/* Fonction d'encadrement diagonale vers Sud Ouest */
void encadrement_SO(int col_piece, int lig_piece, int couleur_joueur);

/* Fonction d'encadrement diagonale vers Nord Ouest */
void encadrement_NO(int col_piece, int lig_piece, int couleur_joueur);

/* Fonction d'encadrement diagonale vers Sud Est */
void encadrement_SE(int col_piece, int lig_piece, int couleur_joueur);

/* Fonction pour vérifier si le damier est entierement rempli */
int damier_complet();

/* Fonction appelee lors du clique sur une case du damier */
static void coup_joueur(GtkWidget *p_case);

/* Fonction retournant texte du champs adresse du serveur de l'interface graphique */
char *lecture_addr_serveur(void);

/* Fonction retournant texte du champs port du serveur de l'interface graphique */
char *lecture_port_serveur(void);

/* Fonction retournant texte du champs login de l'interface graphique */
char *lecture_login(void);

/* Fonction retournant texte du champs adresse du cadre Joueurs de l'interface graphique */
char *lecture_addr_adversaire(void);

/* Fonction retournant texte du champs port du cadre Joueurs de l'interface graphique */
char *lecture_port_adversaire(void);

/* Fonction affichant boite de dialogue si partie gagnee */
void affiche_fenetre_gagne(void);

/* Fonction affichant boite de dialogue si partie perdue */
void affiche_fenetre_perdu(void);

/* Fonction affichant boite de dialogue pour confirmer la connexion au serveur de joueurs */
void affiche_connexion_etablie(void);

/* Fonction affichant boite de dialogue si action impossible */
void affiche_fenetre_action_impossible(void);

/* Fonction affichant boite de dialogue si tous les champs de connexion ne sont pas renseignes */
void affiche_fenetre_connexion_impossible(void);

/* Fonction affichant boite de dialogue si le joueur renseigne son propre port */
void affiche_fenetre_demarrer_partie_impossible(void);

/* Fonction affichant boite de dialogue si adversaire refuse la partie */
void affiche_message_refus_jouer(void);

/* Fonction affichant boite de dialogue pour demander une partie */
int confirm_game(void);

/* Fonction appelee lors du clique du bouton Se connecter */
static void clique_connect_serveur(GtkWidget *b);

/* Fonction desactivant bouton demarrer partie */
void disable_button_start(void);

/* Fonction activant bouton demarrer partie */
void enable_button_start(void);

/* Fonction appelee lors du clique du bouton Demarrer partie */
static void clique_connect_adversaire(GtkWidget *b);

/* Fonction desactivant les cases du damier */
void gele_damier(void);

/* Fonction activant les cases du damier */
void degele_damier(void);

/* Fonction permettant d'initialiser le plateau de jeu */
void init_interface_jeu(void);

/* Fonction reinitialisant l'interface apres une partie */
void reset_interface(void);

/* Fonction reinitialisant la liste des joueurs sur l'interface graphique */
void reset_liste_joueurs(void);

/* Fonction permettant d'ajouter un joueur dans la liste des joueurs sur l'interface graphique */
void affich_joueur(char *login, char *adresse, char *port);

/*function that read pipe and modify the gui in funtion of what's read*/
void * read_pipe_and_modify_gui();

/*funtion that open the pipe and don't block the interface*/
void * write_to_client();

/*funtion that connect */
void * connect_server();

/*function to cast char to int*/
int ctoi(char c);

/*Function ttriggered when the gui is shut down*/
static void close_game();


/* Fonction permettant de changer l'image d'une case du damier (indiqué par sa colonne et sa ligne) */
void change_img_case(int col, int lig, int couleur_j)
{
	char * coord;

	coord=malloc(3*sizeof(char));

	indexes_to_coord(col, lig, coord);
	damier[col][lig] = couleur_j;

	switch(couleur_j)
	{
		case 1: // image pion blanc
			gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_blanc.png");
		break;
		case 0: // image pion noir
			gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_noir.png");
		break;
		case -1: // image default
			gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_def.png");
		break;
	}
}

/* Fonction permettant changer nom joueur blanc dans cadre Score */
void set_label_J1(char *texte)
{
	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J1")), texte);
}

/* Fonction permettant de récupérer le label du joueur noir dans cadre Score */
char *get_label_J1(void)
{
	return (char *)gtk_label_get_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J1")));
}

/* Fonction permettant de changer nom joueur noir dans cadre Score */
void set_label_J2(char *texte)
{
	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J2")), texte);
}

/* Fonction permettant de récupérer le label du joueur noir dans cadre Score */
char *get_label_J2(void)
{
	return (char *)gtk_label_get_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J2")));
}

/* Fonction permettant de changer score joueur blanc dans cadre Score */
void set_score_J1(int score)
{
	char *s;

	s=malloc(5*sizeof(char));
	sprintf(s, "%d\0", score);

	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ1")), s);
}

/* Fonction permettant de récupérer score joueur blanc dans cadre Score */
int get_score_J1(void)
{
	const gchar *c;

	c=gtk_label_get_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ1")));

	return atoi(c);
}

/* Fonction permettant de changer score joueur noir dans cadre Score */
void set_score_J2(int score)
{
	char *s;

	s=malloc(5*sizeof(char));
	sprintf(s, "%d\0", score);

	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ2")), s);
}

/* Fonction permettant de récupérer score joueur noir dans cadre Score */
int get_score_J2(void)
{
	const gchar *c;

	c=gtk_label_get_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ2")));

	return atoi(c);
}

/* Fonction pour mettre le joueur actif en gras */
void bold_label_player(int player){

	char label_player[20];
	strcpy(label_player, "<b>");

	switch(player){
		case 0:
			strcat(label_player, get_label_J2());
			strcat(label_player, "</b>");

			gtk_label_set_markup(GTK_LABEL((GtkWidget *) gtk_builder_get_object (p_builder, "label_J2")), label_player);
			gtk_label_set_markup(GTK_LABEL((GtkWidget *) gtk_builder_get_object (p_builder, "label_J1")), get_label_J1());
			break;
		case 1:
			strcat(label_player, get_label_J1());
			strcat(label_player, "</b>");

			gtk_label_set_markup(GTK_LABEL((GtkWidget *) gtk_builder_get_object (p_builder, "label_J1")), label_player);
			gtk_label_set_markup(GTK_LABEL((GtkWidget *) gtk_builder_get_object (p_builder, "label_J2")), get_label_J2());
			break;
		case -1:
			gtk_label_set_markup(GTK_LABEL((GtkWidget *) gtk_builder_get_object (p_builder, "label_J1")), get_label_J1());
			gtk_label_set_markup(GTK_LABEL((GtkWidget *) gtk_builder_get_object (p_builder, "label_J2")), get_label_J2());
		break;
	}
}

/* Fonction transformant coordonnees du damier graphique en indexes pour matrice du damier */
void coord_to_indexes(const gchar *coord, int *col, int *lig)
{
	char *c;

	c=malloc(3*sizeof(char));

	c=strncpy(c, coord, 1);
	c[1]='\0';

	if(strcmp(c, "A")==0)
	{
		*col=0;
	}
	if(strcmp(c, "B")==0)
	{
		*col=1;
	}
	if(strcmp(c, "C")==0)
	{
		*col=2;
	}
	if(strcmp(c, "D")==0)
	{
		*col=3;
	}
	if(strcmp(c, "E")==0)
	{
		*col=4;
	}
	if(strcmp(c, "F")==0)
	{
		*col=5;
	}
	if(strcmp(c, "G")==0)
	{
		*col=6;
	}
	if(strcmp(c, "H")==0)
	{
		*col=7;
	}

	*lig=atoi(coord+1)-1;
}

/* Fonction transformant coordonnees du damier graphique en indexes pour matrice du damier */
void indexes_to_coord(int col, int lig, char *coord)
{
	char c;

	if(col==0)
	{
		c='A';
	}
	if(col==1)
	{
		c='B';
	}
	if(col==2)
	{
		c='C';
	}
	if(col==3)
	{
		c='D';
	}
	if(col==4)
	{
		c='E';
	}
	if(col==5)
	{
		c='F';
	}
	if(col==6)
	{
		c='G';
	}
	if(col==7)
	{
		c='H';
	}

	sprintf(coord, "%c%d\0", c, lig+1);
}

/* Fonction calculant les scores courants */
void calcul_scores(){
	int i, j;
	int score_J1 = 0, score_J2 = 0;

	for(i = 0 ; i < 8 ; i++){
		for(j = 0 ; j < 8 ; j++){
			switch(damier[i][j]){
				case 0:
					score_J1++;
					break;

				case 1:
					score_J2++;
					break;								
			}
		}
	}

	set_score_J1(score_J2);
	set_score_J2(score_J1);
}

/* Fonction d'encadrement de la droite vers la gauche */
void encadrement_D_G(int col_piece, int lig_piece, int couleur_joueur){
	int couleur_adverse = (couleur_joueur == 0) ? 1 : 0;
	int i = col_piece - 1;

	while (i > 0 && damier[i][lig_piece] == couleur_adverse){
		i--;
	}
	if (i >= 0 && damier[i][lig_piece] == couleur_joueur && i != col_piece){
		for (i = i + 1; i < col_piece; i++){
			change_img_case(i, lig_piece, couleur_joueur);
		}
	}
}

/* Fonction d'encadrement de la gauche vers la droite */
void encadrement_G_D(int col_piece, int lig_piece, int couleur_joueur){
	int couleur_adverse = (couleur_joueur == 0) ? 1 : 0;
	int i = col_piece + 1;

	while (i < 8 && damier[i][lig_piece] == couleur_adverse){
		i++;
	}
	if (i < 8 && damier[i][lig_piece] == couleur_joueur && i != col_piece){
		for (i = i - 1; i > col_piece; i--){
			change_img_case(i, lig_piece, couleur_joueur);
		}
	}
}

/* Fonction d'encadrement de haut vers bas */
void encadrement_H_B(int col_piece, int lig_piece, int couleur_joueur){
	int couleur_adverse = (couleur_joueur == 0) ? 1 : 0;
	int i = lig_piece + 1;

	while (i < 8 && damier[col_piece][i] == couleur_adverse){
		i++;
	}
	if (i < 8 && damier[col_piece][i] == couleur_joueur && i != lig_piece){
		for (i = i - 1; i > lig_piece; i--){
			change_img_case(col_piece, i, couleur_joueur);
		}
	}
}

/* Fonction d'encadrement de bas vers haut */
void encadrement_B_H(int col_piece, int lig_piece, int couleur_joueur){
	int couleur_adverse = (couleur_joueur == 0) ? 1 : 0;
	int i = lig_piece - 1;

	while (i > 0 && damier[col_piece][i] == couleur_adverse){
		i--;
	}
	if (i >= 0 && damier[col_piece][i] == couleur_joueur && i != lig_piece){
		for (i = i + 1; i < lig_piece; i++){
			change_img_case(col_piece, i, couleur_joueur);
		}
	}
}

/* Fonction d'encadrement diagonale vers Nord Est */
void encadrement_NE(int col_piece, int lig_piece, int couleur_joueur){
	int couleur_adverse = (couleur_joueur == 0) ? 1 : 0;
	int i = col_piece + 1;
	int j = lig_piece - 1;

	while (i < 8 && j > 0 && damier[i][j] == couleur_adverse){
		i++;
		j--;
	}
	if (i < 8 && j >= 0 && damier[i][j] == couleur_joueur && i != col_piece && j != lig_piece){
		for (i = i - 1, j = j + 1; i > col_piece, j < lig_piece; i--, j++){
			change_img_case(i, j, couleur_joueur);
		}
	}
}

/* Fonction d'encadrement diagonale vers Sud Ouest */
void encadrement_SO(int col_piece, int lig_piece, int couleur_joueur){
	int couleur_adverse = (couleur_joueur == 0) ? 1 : 0;
	int i = col_piece - 1;
	int j = lig_piece + 1;

	while (i > 0 && j < 8 && damier[i][j] == couleur_adverse){
		i--;
		j++;
	}
	if (i >= 0 && j < 8 && damier[i][j] == couleur_joueur && i != col_piece && j != lig_piece){
		for (i = i + 1, j = j - 1; i < col_piece, j > lig_piece; i++, j--){
			change_img_case(i, j, couleur_joueur);
		}
	}
}

/* Fonction d'encadrement diagonale vers Nord Ouest */
void encadrement_NO(int col_piece, int lig_piece, int couleur_joueur){
	int couleur_adverse = (couleur_joueur == 0) ? 1 : 0;
	int i = col_piece - 1;
	int j = lig_piece - 1;

	while (i > 0 && j > 0 && damier[i][j] == couleur_adverse){
		i--;
		j--;
	}
	if (i >= 0 && j >= 0 && damier[i][j] == couleur_joueur && i != col_piece && j != lig_piece){
		for (i = i + 1, j = j + 1; i < col_piece, j < lig_piece; i++, j++){
			change_img_case(i, j, couleur_joueur);
		}
	}
}

/* Fonction d'encadrement diagonale vers Sud Est */
void encadrement_SE(int col_piece, int lig_piece, int couleur_joueur){
	int couleur_adverse = (couleur_joueur == 0) ? 1 : 0;
	int i = col_piece + 1;
	int j = lig_piece + 1;

	while (i < 8 && j < 8 && damier[i][j] == couleur_adverse){
		i++;
		j++;
	}
	if (i < 8 && j < 8 && damier[i][j] == couleur_joueur && i != col_piece && j != lig_piece){
		for (i = i - 1, j = j - 1; i > col_piece, j > lig_piece; i--, j--){
			change_img_case(i, j, couleur_joueur);
		}
	}
}

/* Check if game is ended */
int damier_complet(){
	int i, j;
	
	for(i = 0 ; i < 8 ; i++)
	{
		for(j = 0 ; j < 8 ; j++)
		{
			if(damier[i][j] == -1)
			{
				return 0;
			}
		}
	}
	return 1;	
}

/* Fonction appelee lors du clique sur une case du damier */
static void coup_joueur(GtkWidget *p_case)
{
	int col, lig, type_msg, nb_piece, score;
	char buf[MAXDATASIZE];

	// Traduction coordonnees damier en indexes matrice damier
	coord_to_indexes(gtk_buildable_get_name(GTK_BUILDABLE(gtk_bin_get_child(GTK_BIN(p_case)))), &col, &lig);

	if(damier[col][lig] != -1)
	{
		affiche_fenetre_action_impossible();
	}
	else
	{
		nbCoup++;
		change_img_case(col, lig, couleur);

		char coord[2];
		indexes_to_coord(col, lig, coord);
		printf("Player add piece to : %s\n", coord);
		fflush(stdout);

		// Appel des fonctions d'encadrement
		encadrement_D_G(col, lig, couleur);
		encadrement_G_D(col, lig, couleur);
		encadrement_H_B(col, lig, couleur);
		encadrement_B_H(col, lig, couleur);
		encadrement_NO(col, lig, couleur);
		encadrement_NE(col, lig, couleur);
		encadrement_SE(col, lig, couleur);
		encadrement_SO(col, lig, couleur);

		calcul_scores();

		//we send the movement to the other player
		char message[5];
		strcpy(message, "c-");

		char position[5];
		char ligInChar[2];
		char colInChar[2];

		memset(position, 0, sizeof(position));

		sprintf(ligInChar, "%d", lig);
		sprintf(colInChar, "%d", col);
		strcat(position, colInChar);
		strcat(position, ligInChar);
		strcat(message, position);
		write(descGuiToClient, message, strlen(message));

		gele_damier();

		int opponent_color = (couleur == 0) ? 1 : 0;
		bold_label_player(opponent_color);
	}


	// Fin de jeu
	if(nbCoup == 32 || damier_complet() == 1)
	{
		if((couleur == 0 && get_score_J1() < get_score_J2()) ||
		(couleur == 1 && get_score_J1() > get_score_J2()))
		{
			affiche_fenetre_gagne();
		}
		else
		{
			affiche_fenetre_perdu();
		}

		printf("The game is done !");
		fflush(stdout);
		
		reset_interface();
		enable_button_start();
	}
}

/* Fonction retournant texte du champs adresse du serveur de l'interface graphique */
char *lecture_addr_serveur(void)
{
	GtkWidget *entry_addr_srv;

	entry_addr_srv = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_adr");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_addr_srv));
}

/* Fonction retournant texte du champs port du serveur de l'interface graphique */
char *lecture_port_serveur(void)
{
	GtkWidget *entry_port_srv;

	entry_port_srv = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_port");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_port_srv));
}

/* Fonction retournant texte du champs login de l'interface graphique */
char *lecture_login(void)
{
	GtkWidget *entry_login;

	entry_login = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_login");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_login));
}

/* Fonction retournant texte du champs adresse du cadre Joueurs de l'interface graphique */
char *lecture_addr_adversaire(void)
{
	GtkWidget *entry_addr_j2;

	entry_addr_j2 = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_addr_j2");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_addr_j2));
}

/* Fonction retournant texte du champs port du cadre Joueurs de l'interface graphique */
char *lecture_port_adversaire(void)
{
	GtkWidget *entry_port_j2;

	entry_port_j2 = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_port_j2");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_port_j2));
}

/* Fonction affichant boite de dialogue si partie gagnee */
void affiche_fenetre_gagne(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	
	char message[70];
	int score_J1 = get_score_J1();
	int score_J2 = get_score_J2();
	
	if(score_J1 > score_J2)
	{
		sprintf(message, "Fin de la partie.\n\n Vous avez gagné (%d-%d) !!!", score_J1, score_J2);
	}
	else
	{
		sprintf(message, "Fin de la partie.\n\n Vous avez gagné (%d-%d) !!!", score_J2, score_J1);
	}

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, message, NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

/* Fonction affichant boite de dialogue si partie perdue */
void affiche_fenetre_perdu(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	
	char message[70];
	int score_J1 = get_score_J1();
	int score_J2 = get_score_J2();
	
	if(score_J1 < score_J2)
	{
		sprintf(message, "Fin de la partie.\n\n Vous avez perdu (%d-%d) !", score_J1, score_J2);
	}
	else
	{
		sprintf(message, "Fin de la partie.\n\n Vous avez perdu (%d-%d) !", score_J2, score_J1);
	}

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, message, NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

/* Fonction affichant boite de dialogue pour confirmer la connexion au serveur de joueurs */
void affiche_connexion_etablie(void)
{
	if(msgConnect == 0)
	{
		GtkWidget *dialog;

		GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

		dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Connexion avec le serveur de joueurs établie !\n\n Pour rafraîchir la liste de joueurs, veuillez cliquer sur le bouton [Se Connecter].", NULL);
		gtk_dialog_run(GTK_DIALOG (dialog));

		gtk_widget_destroy(dialog);
		
		msgConnect = 1;
	}
}

/* Fonction affichant boite de dialogue si action impossible */
void affiche_fenetre_action_impossible(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Action impossible.\n\n Veuillez sélectionner une case vide.", NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

/* Fonction affichant boite de dialogue si tous les champs de connexion ne sont pas renseignes */
void affiche_fenetre_connexion_impossible(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Connexion au serveur de joueurs impossible.\n\n Veillez à bien renseigner tous les champs demandés.", NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

/* Fonction affichant boite de dialogue si le joueur renseigne son propre port */
void affiche_fenetre_demarrer_partie_impossible(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Démarrage de la partie impossible...\n\n Vous devez renseigner un numéro de port différent du votre.", NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

/* Fonction affichant boite de dialogue si le joueur refuse la partie */
void affiche_message_refus_jouer(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Opponent refused to play...\n\n Try another player at your height.", NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

/* Fonction affichant boite de dialogue pour demander une partie */
int confirm_game(void)
{
	GtkWidget *dialog;
	
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Another player wanna play with you.\n\n Accept ?");

	switch(gtk_dialog_run(GTK_DIALOG(dialog)))
	{
		case GTK_RESPONSE_YES:
			gtk_widget_destroy(dialog);
			return 1;
		break;
		case GTK_RESPONSE_NO:
			gtk_widget_destroy(dialog);
			return 0;
		break;
	}
}

/* Fonction appelee lors du clique du bouton Se connecter */
static void clique_connect_serveur(GtkWidget *b)
{
	char * portServer = lecture_port_serveur();
	char * login = lecture_login();

	//we launch a thread that will just write to the client to comunicate our position to the oponent
	pthread_t thread_connect_server_players;
	int desc_thread_connect_server_players = pthread_create(&thread_connect_server_players, NULL, connect_server, 0);
}

/* Fonction desactivant bouton demarrer partie */
void disable_button_start(void)
{
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_start"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_connect"), FALSE);
}

/* Fonction activant bouton demarrer partie */
void enable_button_start(void)
{
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_start"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_connect"), TRUE);
}

/* Fonction appelee lors du clique du bouton Demarrer partie */
static void clique_connect_adversaire(GtkWidget *b)
{
	char* portToConnect = lecture_port_adversaire();

	char portInChar[6]; 
	sprintf(portInChar, "%d", port);

	if(strcmp(portInChar, portToConnect) == 0)
	{
		affiche_fenetre_demarrer_partie_impossible();
	}
	else{
		//lancer un modele_client et ecouter sur un pipe nomme pour la MAJ de l'interface
		pid_t pid_client = fork();
		if(pid_client != 0)
		{
			//I am the father
			pidClient = (int) pid_client;
		}
		else
		{		
			if (execlp("./client.o", "client.o", portToConnect, portInChar, "0", NULL)==-1)
			{
				printf("\nOthello : Execlp didn't work\n");
				strerror(errno);
				fflush(stdout);
			}
		}
	}
}

/* Fonction desactivant les cases du damier */
void gele_damier(void)
{
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA1"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB1"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC1"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD1"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE1"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF1"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG1"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH1"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA2"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB2"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC2"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD2"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE2"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF2"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG2"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH2"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA3"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB3"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC3"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD3"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE3"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF3"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG3"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH3"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA4"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB4"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC4"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD4"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE4"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF4"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG4"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH4"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA5"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB5"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC5"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD5"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE5"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF5"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG5"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH5"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA6"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB6"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC6"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD6"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE6"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF6"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG6"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH6"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA7"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB7"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC7"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD7"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE7"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF7"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG7"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH7"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA8"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB8"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC8"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD8"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE8"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF8"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG8"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH8"), FALSE);
}

/* Fonction activant les cases du damier */
void degele_damier(void)
{
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA1"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB1"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC1"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD1"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE1"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF1"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG1"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH1"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA2"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB2"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC2"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD2"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE2"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF2"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG2"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH2"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA3"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB3"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC3"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD3"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE3"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF3"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG3"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH3"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA4"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB4"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC4"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD4"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE4"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF4"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG4"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH4"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA5"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB5"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC5"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD5"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE5"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF5"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG5"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH5"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA6"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB6"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC6"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD6"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE6"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF6"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG6"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH6"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA7"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB7"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC7"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD7"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE7"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF7"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG7"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH7"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxA8"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxB8"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxC8"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxD8"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxE8"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxF8"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxG8"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object(p_builder, "eventboxH8"), TRUE);
}


/* Fonction permettant d'initialiser le plateau de jeu */
void init_interface_jeu(void)
{
	// Initilisation du damier (D4=blanc, E4=noir, D5=noir, E5=blanc)
	change_img_case(3, 3, 1);
	change_img_case(4, 3, 0);
	change_img_case(3, 4, 0);
	change_img_case(4, 4, 1);

	// Initialisation des scores et des joueurs
	if(couleur==1)
	{
		set_label_J1("Vous");
		set_label_J2("Adversaire");
	}
	else
	{
		set_label_J1("Adversaire");
		set_label_J2("Vous");
	}

	bold_label_player(0);

	calcul_scores();

	nbCoup = 0;

	// Le joueur Noir commence
	if(couleur == 0){
		degele_damier();
	}
}

/* Fonction reinitialisant l'interface apres une partie */
void reset_interface(void)
{
	int i, j;
	
	for(i = 0 ; i < 8 ; i++){
		for(j = 0 ; j < 8 ; j++){
			change_img_case(i, j, -1);
		}
	}
	
	bold_label_player(-1);
	
	calcul_scores();
	gele_damier();
}

/* Fonction reinitialisant la liste des joueurs sur l'interface graphique */
void reset_liste_joueurs(void)
{
	GtkTextIter start, end;

	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &end);

	gtk_text_buffer_delete(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &start, &end);
}

/* Fonction permettant d'ajouter un joueur dans la liste des joueurs sur l'interface graphique */
void affich_joueur(char *login, char *adresse, char *port)
{
	const gchar *joueur;

	joueur=g_strconcat(login, " - ", adresse, " : ", port, "\n", NULL);

	gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), joueur, strlen(joueur));
}

int main (int argc, char ** argv)
{
	int i, j, ret;

	if(argc!=2)
	{
		printf("\nPrototype : ./othello num_port\n\n");
		exit(1);
	}

	/* Initialisation de GTK+ */
	XInitThreads();
	gtk_init (& argc, & argv);

	/* Creation d'un nouveau GtkBuilder */
	p_builder = gtk_builder_new();

	if (p_builder != NULL)
	{
		/* Chargement du XML dans p_builder */
		gtk_builder_add_from_file (p_builder, "UI_Glade/Othello.glade", & p_err);

		if (p_err == NULL)
		{
			/* Recuparation d'un pointeur sur la fenetre. */
			GtkWidget * p_win = (GtkWidget *) gtk_builder_get_object (p_builder, "window1");

			/* Gestion evenement clic pour chacune des cases du damier */
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH1"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH2"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH3"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH4"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH5"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH6"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH7"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH8"), "button_press_event", G_CALLBACK(coup_joueur), NULL);

			/* Gestion clic boutons interface */
			g_signal_connect(gtk_builder_get_object(p_builder, "button_connect"), "clicked", G_CALLBACK(clique_connect_serveur), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "button_start"), "clicked", G_CALLBACK(clique_connect_adversaire), NULL);

			/* Gestion clic bouton fermeture fenetre */
			g_signal_connect_swapped(G_OBJECT(p_win), "destroy", G_CALLBACK(close_game), NULL);

			/* Recuperation numero port donne en parametre */
			port=atoi(argv[1]);

			/* Initialisation du damier de jeu */
			reset_interface();

			//here we create the two pipes we need to communicate between the gui the client and the server
			char serverToGui[] = "serverToGui.fifo";
			if(mkfifo(serverToGui, S_IRUSR | S_IWUSR ) != 0)  
			{
				fprintf(stderr, "Impossible de créer le tube nommé.\n");
				exit(EXIT_FAILURE);
			}

			char guiToClient[] = "guiToClient.fifo";
			if(mkfifo(guiToClient, S_IRUSR | S_IWUSR ) != 0)  
			{
				fprintf(stderr, "Impossible de créer le tube nommé.\n");
				exit(EXIT_FAILURE);
			}

			pid_t pid_serv = fork();
			if(pid_serv != 0)
			{ 	
				//I am the father
				pid = (int) pid_serv;

				//we launch a thread that will just read the first pipe and modify the gui
				pthread_t thread_read_pipe_and_modify_gui;
				int desc_thread_read_pipe_and_modify_gui = pthread_create (&thread_read_pipe_and_modify_gui, NULL, read_pipe_and_modify_gui, argv);

				//we launch a thread that will just write to the client to comunicate our position to the oponent
				pthread_t thread_write_to_client;
				int desc_thread_write_to_client = pthread_create (&thread_write_to_client, NULL, write_to_client, argv);

				//init the interface
				gtk_widget_show_all(p_win);
				gtk_main();
			}
			else
			{
				//we override the processe 
				if (execlp("./server.o", "server.o", argv[1], NULL))
				{
					printf("Othello : Execlp didn't work\n");
					strerror(errno);
					fflush(stdout);
				}
			}
		}
		else
		{
			/* Affichage du message d'erreur de GTK+ */
			g_error ("%s", p_err->message);
			g_error_free (p_err);
		}
	}


	return EXIT_SUCCESS;
}

void * read_pipe_and_modify_gui()
{
	char serverToGui[] = "serverToGui.fifo";

	if((descServerToGui = open(serverToGui, O_RDONLY)) == -1) 
	{   
		fprintf(stderr, "Impossible d'ouvrir la sortie du tube nommé.\n");
		exit(EXIT_FAILURE);
	}

	char stringToRead[5];
	int nbBRead;
	while(1)
	{
		if((nbBRead = read(descServerToGui, stringToRead, 5-1)) == -1)
		{
			perror("read error : ");
			exit(EXIT_FAILURE);
		}else if(nbBRead > 0)
		{
			stringToRead[nbBRead] = '\0';
			// message treatment
			// A header is contained in the message as follow :
			// j-XX means that the message is about the identity of the player (J1 or J2)
			// c-XX means that the message deals with a position
			//
			// Split the received string to separate header and content			
			char* token = strtok (stringToRead,"-");	
			char* header = token;
			token = strtok(NULL, "-");
			char* content = token;
			token = strtok(NULL, "-");

			if(strcmp(header, "j") == 0){

				// For this header, 3 types of content are possibles :
				// J2 : the player is suggested to be the player J2
				// ok : the player accepts to be player J2
				// no : the player refuses to play
				
				if(strcmp(content, "J2") == 0){
					
					int accept = confirm_game();
					
					char message[5];

					if(accept == 1)
					{
						couleur = 1;
						init_interface_jeu();
						
						// Disable connect button
						disable_button_start();
						
						strcpy(message, "j-ok");
						write(descGuiToClient, message, strlen(message));
					}
					else
					{
						strcpy(message, "j-no");
						write(descGuiToClient, message, strlen(message));
					}
				}
				else if(strcmp(content, "ok") == 0)
				{
					couleur = 0;
					init_interface_jeu();

					// Disable connect button
					disable_button_start();
				}
				else if(strcmp(content, "no") == 0)
				{
					affiche_message_refus_jouer();
				}
			}
			else if(strcmp(header, "c") == 0){
				// traduction of the position
				char coord[2];
				int col, lig;
				int opponent_color = (couleur == 0) ? 1 : 0;

				col = ctoi(content[0]);
				lig = ctoi(content[1]);
				// interpretation of the position
				indexes_to_coord(col, lig, coord);

				printf("Opponent add piece to : %s\n", coord);
				fflush(stdout);

				change_img_case(col, lig, opponent_color);

				// Appel des fonctions d'encadrement
				encadrement_D_G(col, lig, opponent_color);
				encadrement_G_D(col, lig, opponent_color);
				encadrement_H_B(col, lig, opponent_color);
				encadrement_B_H(col, lig, opponent_color);
				encadrement_NO(col, lig, opponent_color);
				encadrement_NE(col, lig, opponent_color);
				encadrement_SE(col, lig, opponent_color);
				encadrement_SO(col, lig, opponent_color);

				// Fin de jeu
				if(nbCoup == 31 || damier_complet() == 1)
				{
					calcul_scores();
					if((couleur == 0 && get_score_J1() < get_score_J2()) ||
					(couleur == 1 && get_score_J1() > get_score_J2()))
					{
						affiche_fenetre_gagne();
					}
					else
					{
						affiche_fenetre_perdu();
					}
					
					printf("The game is done !");
					fflush(stdout);
		
					reset_interface();
					enable_button_start();
				}
				else
				{
					calcul_scores();
					bold_label_player(couleur);
					degele_damier();
				}
			}
			else{
				printf("Wrong message header ...");
				fflush(stdout);
				
				exit(EXIT_FAILURE);
			}
		}
	}
}

void * write_to_client()
{

	char guiToClient[] = "guiToClient.fifo";

	//we also open the second pipe that will write to the client to communicate the move to the other player
	//this funcion will block untill we have a client that open the pipe on read
	if((descGuiToClient = open(guiToClient, O_WRONLY)) == -1) 
	{
		fprintf(stderr, "Impossible d'ouvrir l'entrée du tube nommé.\n");
		perror("open");
		exit(EXIT_FAILURE);
	}
}

int ctoi(char c)
{
	int a = (int)c;	
	return a-48;
}

static void close_game()
{

	printf("GUI : closing game\n");
	fflush(stdout);
	
	/********************* Closing pipes *************************/

	int res;
	int pipeClosed = 1;
	
	printf("GUI : Closing pipes...\n");
	fflush(stdout);
	
	if((res = close(descServerToGui))==-1)
	{
		printf("Error while closing the pipe Server -> GUI\n");
		fflush(stdout);
		
		pipeClosed = 0;
	}
	if((res = close(descGuiToClient))==-1)
	{
		printf("Error while closing the pipe GUI -> Client\n");
		fflush(stdout);
		
		pipeClosed = 0;
	}	
	
	if(pipeClosed)
	{
		printf("GUI : Pipes closed.\n");
		fflush(stdout);
	}
	/********************* Killing process *************************/

	printf("GUI : Killing process...\n");
	fflush(stdout);

	int status = 0;
	pid_t w;
	int resk;
	
	if (pid != 0)
	{
		if (( resk = kill(pid, SIGTERM)) == -1) {
			perror("kill ");
			exit(EXIT_FAILURE);
		}
		if ((w = waitpid(pid, &status, 0)) == -1) {
			printf("GUI : waitpid on pid error\n");
			fflush(stdout);
		}
	}

	if (pidClient != 0)
	{
		if (( resk = kill(pidClient, SIGTERM)) == -1) {
			perror("kill ");
			exit(EXIT_FAILURE);
		}
		if ((w = waitpid(pidClient, &status, 0)) == -1) {
			printf("GUI : waitpid on pidClient error\n");
			fflush(stdout);
		}
	}

	printf("GUI : Process killed.\n");
	fflush(stdout);
	
	/********************* Removing pipes *************************/

	printf("GUI : Removing the pipes...\n");
	fflush(stdout);

	if((res = remove("./serverToGui.fifo"))==-1)
	{
		perror("GUI : remove");
		exit(EXIT_FAILURE);
	}
	if((res = remove("./guiToClient.fifo"))==-1)
	{
		perror("GUI : remove");
		exit(EXIT_FAILURE);
	}

	printf("GUI : Pipes removed.\n");
	fflush(stdout);
	
	printf("Game over\n");
	fflush(stdout);
	
	gtk_main_quit();
}


void * connect_server()
{
	char* SERVEUR = "127.0.0.1";
	char* PORTS = "2058";

	int sockfd, new_fd, rv, sin_size, numbytes;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr their_adr;
	char buf[100];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	rv = getaddrinfo(SERVEUR, PORTS, &hints, &servinfo);

	if(rv != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(0);
	}

	// Création  socket  et  attachement
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{   
			perror("client: socket");
			continue;
		}   
		if((connect(sockfd, p->ai_addr, p->ai_addrlen) == -1))
		{   
			close(sockfd);
			perror("client: connect");
			continue;
		}   

		break;
	}

	if(p == NULL)
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(0);
	}

	freeaddrinfo(servinfo);       // Libère structure

	//we send our ip, port and login
	char message[100];
	if (strcmp(lecture_addr_serveur(), "")==0 || strcmp(lecture_port_serveur(), "")==0 || strcmp(lecture_login(), "")==0)
	{
		affiche_fenetre_connexion_impossible();
		return NULL;
	}
	strcpy(message, "c,");
	strcat(message, lecture_addr_serveur());
	strcat(message, ",");
	strcat(message, lecture_port_serveur());
	strcat(message, ",");
	strcat(message, lecture_login());
	strcat(message, ",");
	send(sockfd, message, strlen(message), 0);

	printf("waiting for an answer from the server \n");
	fflush(stdout);

	//and we recv the list of all the players
	//we normaly have to do a while loop
	if((numbytes = recv(sockfd, buf, 100-1, 0)) == -1) 
	{
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';

	printf("Message reçu : %s\n",buf);
	fflush(stdout);
	
	// Cleaning the list on the interface
	reset_liste_joueurs();

	char* token = "";
	char* entete = "";
	token = strtok (buf,","); 

	entete = token;
	token = strtok(NULL, ",");

	while (strcmp(token, "c")!=0)
	{
		char* ip = token;
		token = strtok(NULL, ",");
		char* port = token;
		token = strtok(NULL, ",");
		char* login = token;
		token = strtok(NULL, ",");

		//verify that the token is not null 
		//if he is so malloc
		if (ip==NULL)
		{
			ip = (char*)malloc(sizeof(char*));
			strcpy(ip, "");
		}
		if (port==NULL)
		{
			port = (char*)malloc(sizeof(char*));
			strcpy(port, "");
		}
		if (login==NULL)
		{
			login = (char*)malloc(sizeof(char*));
			strcpy(login, "");
		}
		if (token==NULL)
		{
			token = (char*)malloc(sizeof(char*));
			strcpy(token, "");
		}
		
		affich_joueur(login, ip, port);
	}

	affiche_connexion_etablie();
}

