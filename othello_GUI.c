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

/*  Global Variables */
int checkerboard[8][8];		// array representing the checkerboard
int color;								// 0 : BLACK, 1 : WHITE
int nbMove;								// Number of moves
int msgConnect = 0;				// Message 'connection established' shown once (1) or not (0)

int port;									// port number, passes into arguments

int pid;
int pidClient;

char *addr_j2, *port_j2;	// Info about opponent

/* Global Variables about graphic interface */
GtkBuilder  *  p_builder   = NULL;
GError      *  p_err       = NULL;

// Function headers 

/* Function allowing to change the image of a square of the checkerboard (indicated by its row and column) */
void change_img_square(int col, int row, int color_p);

/* Function allowing to change the name of the white player in the Score part */
void set_label_J1(char *text);

/* Function allowing to get the name of the white player in the Score part */
char *get_label_J1(void);

/* Function allowing to change the name of the black player in the Score part */
void set_label_J2(char *text);

/* Function allowing to get the name of the black player in the Score part */
char *get_label_J2(void);

/* Function allowing to change the score of the white player in the Score part */
void set_score_J1(int score);

/* Function allowing to get the score of the white player in the Score part */
int get_score_J1(void);

/* Function allowing to change the score of the black player in the Score part */
void set_score_J2(int score);

/* Function allowing to get the score of the black player in the Score part */
int get_score_J2(void);

/* Function to put the current player name to bold */
void bold_label_player(int player);

/* Function transforming position on the graphic checkerboard into indexes for matricial checkerboard */
void coord_to_indexes(const gchar *coord, int *col, int *row);

/* Function transforming indexes into position on the graphic checkerboard for matricial checkerboard */
void indexes_to_coord(int col, int row, char *coord);

/* Function calculating the current scores */
void scores_calculation();

/* Surrounding function from right to left */
void surrounding_R_L(int col_piece, int row_piece, int color_player);

/* Surrounding function from left to right */
void surrounding_L_R(int col_piece, int row_piece, int color_player);

/* Surrounding function from top to bottom */
void surrounding_T_B(int col_piece, int row_piece, int color_player);

/* Surrounding function from bottom to top */
void surrounding_B_T(int col_piece, int row_piece, int color_player);

/* Surrounding function to North East */
void surrounding_NE(int col_piece, int row_piece, int color_player);

/* Surrounding function to South West */
void surrounding_SW(int col_piece, int row_piece, int color_player);

/* Surrounding function to North West */
void surrounding_NW(int col_piece, int row_piece, int color_player);

/* Surrounding function to South East */
void surrounding_SE(int col_piece, int row_piece, int color_player);

/* Function to check if the checkerboard is full */
int isCheckerboardFull();

/* Function called when player is clicking on a square of the checkerboard */
static void player_move(GtkWidget *p_square);

/* Function returning the text of the address field of the server of the graphic interface */
char *read_addr_server(void);

/* Function returning the text of the port field of the server of the graphic interface */
char *read_port_server(void);

/* Function returning the text of the login field of the graphic interface the graphic interface */
char *read_login(void);

/* Function returning the text of the address field of the Players part of the graphic interface */
char *read_addr_opponent(void);

/* Function returning the text of the port field of the Players part of the graphic interface */
char *read_port_opponent(void);

/* Function displaying a dialog box if player won the game */
void display_won(void);

/* Function displaying a dialog box if player lost the game */
void display_lost(void);

/* Function displaying a dialog box to confirm the connection to game server */
void display_connection_established(void);

/* Function displaying a dialog box if the action is cannot be performed */
void display_invalid_action(void);

/* Function displaying a dialog box if information for connection is missing */
void display_invalid_information_for_connection(void);

/* Function displaying a dialog box if the player provide its own port number */
void display_invalid_port_number(void);

/* Function displaying a dialog box if oppponent refused to play */
void display_opponent_refused(void);

/* Function displaying a dialog box to ask for a game */
int display_confirm_game(void);

/* Function called when the player is clicking on the button Log In */
static void click_connect_server(GtkWidget *b);

/* Function disabling button Start Game */
void disable_button_start(void);

/* Function enabling button Start Game */
void enable_button_start(void);

/* Function called when the player is clicking on the button Start Game */
static void click_connect_opponent(GtkWidget *b);

/* Function disabling checkerboard squares */
void freeze_checkerboard(void);

/* Function enabling checkerboard squares */
void unfreeze_checkerboard(void);

/* Function allowing to initialize game interface */
void init_interface_game(void);

/* Function reinitializing the interface after a game */
void reset_interface(void);

/* Function reinitializing the players list on the graphic interface */
void reset_players_list(void);

/* Function allowing to add a player to the list on the graphic interface */
void add_player(char *login, char *address, char *port);

/* Function that read pipe and modify the GUI in funtion of what's read */
void * read_pipe_and_modify_gui();

/* Funtion that open the pipe and doesn't block the interface */
void * write_to_client();

/* Funtion that connect to the game server */
void * connect_server();

/* Function to cast char into int */
int ctoi(char c);

/* Function triggered when the GUI is shut down */
static void close_game();


void change_img_square(int col, int row, int color_p)
{
	char * coord;

	coord=malloc(3*sizeof(char));

	indexes_to_coord(col, row, coord);
	checkerboard[col][row] = color_p;

	switch(color_p)
	{
		case 1: // image white piece
			gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_blanc.png");
		break;
		case 0: // image black piece
			gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_noir.png");
		break;
		case -1: // image default
			gtk_image_set_from_file(GTK_IMAGE(gtk_builder_get_object(p_builder, coord)), "UI_Glade/case_def.png");
		break;
	}
}

void set_label_J1(char *text)
{
	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J1")), text);
}

char *get_label_J1(void)
{
	return (char *)gtk_label_get_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J1")));
}

void set_label_J2(char *text)
{
	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J2")), text);
}

char *get_label_J2(void)
{
	return (char *)gtk_label_get_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_J2")));
}

void set_score_J1(int score)
{
	char *s;

	s=malloc(5*sizeof(char));
	sprintf(s, "%d\0", score);

	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ1")), s);
}

int get_score_J1(void)
{
	const gchar *c;

	c=gtk_label_get_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ1")));

	return atoi(c);
}

void set_score_J2(int score)
{
	char *s;

	s=malloc(5*sizeof(char));
	sprintf(s, "%d\0", score);

	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ2")), s);
}

int get_score_J2(void)
{
	const gchar *c;

	c=gtk_label_get_text(GTK_LABEL(gtk_builder_get_object (p_builder, "label_ScoreJ2")));

	return atoi(c);
}

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

void coord_to_indexes(const gchar *coord, int *col, int *row)
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

	*row=atoi(coord+1)-1;
}

void indexes_to_coord(int col, int row, char *coord)
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

	sprintf(coord, "%c%d\0", c, row+1);
}

void scores_calculation()
{
	int i, j;
	int score_J1 = 0, score_J2 = 0;

	for(i = 0 ; i < 8 ; i++)
	{
		for(j = 0 ; j < 8 ; j++)
		{
			switch(checkerboard[i][j])
			{
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

void surrounding_R_L(int col_piece, int row_piece, int color_player)
{
	int color_opponent = (color_player == 0) ? 1 : 0;
	int i = col_piece - 1;

	while (i > 0 && checkerboard[i][row_piece] == color_opponent)
	{
		i--;
	}
	
	if (i >= 0 && checkerboard[i][row_piece] == color_player && i != col_piece)
	{
		for (i = i + 1; i < col_piece; i++)
		{
			change_img_square(i, row_piece, color_player);
		}
	}
}

void surrounding_L_R(int col_piece, int row_piece, int color_player)
{
	int color_opponent = (color_player == 0) ? 1 : 0;
	int i = col_piece + 1;

	while (i < 8 && checkerboard[i][row_piece] == color_opponent)
	{
		i++;
	}
	
	if (i < 8 && checkerboard[i][row_piece] == color_player && i != col_piece)
	{
		for (i = i - 1; i > col_piece; i--)
		{
			change_img_square(i, row_piece, color_player);
		}
	}
}

void surrounding_T_B(int col_piece, int row_piece, int color_player)
{
	int color_opponent = (color_player == 0) ? 1 : 0;
	int i = row_piece + 1;

	while (i < 8 && checkerboard[col_piece][i] == color_opponent)
	{
		i++;
	}
	
	if (i < 8 && checkerboard[col_piece][i] == color_player && i != row_piece)
	{
		for (i = i - 1; i > row_piece; i--)
		{
			change_img_square(col_piece, i, color_player);
		}
	}
}

void surrounding_B_T(int col_piece, int row_piece, int color_player)
{
	int color_opponent = (color_player == 0) ? 1 : 0;
	int i = row_piece - 1;

	while (i > 0 && checkerboard[col_piece][i] == color_opponent)
	{
		i--;
	}
	
	if (i >= 0 && checkerboard[col_piece][i] == color_player && i != row_piece)
	{
		for (i = i + 1; i < row_piece; i++)
		{
			change_img_square(col_piece, i, color_player);
		}
	}
}

void surrounding_NE(int col_piece, int row_piece, int color_player)
{
	int color_opponent = (color_player == 0) ? 1 : 0;
	int i = col_piece + 1;
	int j = row_piece - 1;

	while (i < 8 && j > 0 && checkerboard[i][j] == color_opponent)
	{
		i++;
		j--;
	}
	
	if (i < 8 && j >= 0 && checkerboard[i][j] == color_player && i != col_piece && j != row_piece)
	{
		for (i = i - 1, j = j + 1; i > col_piece, j < row_piece; i--, j++)
		{
			change_img_square(i, j, color_player);
		}
	}
}

void surrounding_SW(int col_piece, int row_piece, int color_player)
{
	int color_opponent = (color_player == 0) ? 1 : 0;
	int i = col_piece - 1;
	int j = row_piece + 1;

	while (i > 0 && j < 8 && checkerboard[i][j] == color_opponent)
	{
		i--;
		j++;
	}
	
	if (i >= 0 && j < 8 && checkerboard[i][j] == color_player && i != col_piece && j != row_piece)
	{
		for (i = i + 1, j = j - 1; i < col_piece, j > row_piece; i++, j--)
		{
			change_img_square(i, j, color_player);
		}
	}
}

void surrounding_NW(int col_piece, int row_piece, int color_player)
{
	int color_opponent = (color_player == 0) ? 1 : 0;
	int i = col_piece - 1;
	int j = row_piece - 1;

	while (i > 0 && j > 0 && checkerboard[i][j] == color_opponent)
	{
		i--;
		j--;
	}
	
	if (i >= 0 && j >= 0 && checkerboard[i][j] == color_player && i != col_piece && j != row_piece)
	{
		for (i = i + 1, j = j + 1; i < col_piece, j < row_piece; i++, j++)
		{
			change_img_square(i, j, color_player);
		}
	}
}

void surrounding_SE(int col_piece, int row_piece, int color_player)
{
	int color_opponent = (color_player == 0) ? 1 : 0;
	int i = col_piece + 1;
	int j = row_piece + 1;

	while (i < 8 && j < 8 && checkerboard[i][j] == color_opponent)
	{
		i++;
		j++;
	}
	
	if (i < 8 && j < 8 && checkerboard[i][j] == color_player && i != col_piece && j != row_piece)
	{
		for (i = i - 1, j = j - 1; i > col_piece, j > row_piece; i--, j--)
		{
			change_img_square(i, j, color_player);
		}
	}
}

int isCheckerboardFull()
{
	int i, j;
	
	for(i = 0 ; i < 8 ; i++)
	{
		for(j = 0 ; j < 8 ; j++)
		{
			if(checkerboard[i][j] == -1)
			{
				return 0;
			}
		}
	}
	return 1;	
}

static void player_move(GtkWidget *p_square)
{
	int col, row, type_msg, nb_piece, score;
	char buf[MAXDATASIZE];

	coord_to_indexes(gtk_buildable_get_name(GTK_BUILDABLE(gtk_bin_get_child(GTK_BIN(p_square)))), &col, &row);

	if(checkerboard[col][row] != -1)
	{
		display_invalid_action();
	}
	else
	{
		nbMove++;
		change_img_square(col, row, color);

		char coord[2];
		indexes_to_coord(col, row, coord);
		printf("Player add piece to : %s\n", coord);
		fflush(stdout);

		// Calling surrounding functions
		surrounding_R_L(col, row, color);
		surrounding_L_R(col, row, color);
		surrounding_T_B(col, row, color);
		surrounding_B_T(col, row, color);
		surrounding_NW(col, row, color);
		surrounding_NE(col, row, color);
		surrounding_SE(col, row, color);
		surrounding_SW(col, row, color);

		scores_calculation();

		// We send the move to the opponent
		char message[5];
		strcpy(message, "c-");

		char position[5];
		char rowInChar[2];
		char colInChar[2];

		memset(position, 0, sizeof(position));

		sprintf(rowInChar, "%d", row);
		sprintf(colInChar, "%d", col);
		strcat(position, colInChar);
		strcat(position, rowInChar);
		strcat(message, position);
		write(descGuiToClient, message, strlen(message));

		freeze_checkerboard();

		int opponent_color = (color == 0) ? 1 : 0;
		bold_label_player(opponent_color);
	}


	// End Game ?
	if(nbMove == 32 || isCheckerboardFull() == 1)
	{
		if((color == 0 && get_score_J1() < get_score_J2()) ||
		(color == 1 && get_score_J1() > get_score_J2()))
		{
			display_won();
		}
		else
		{
			display_lost();
		}

		printf("The game is done !");
		fflush(stdout);
		
		reset_interface();
		enable_button_start();
	}
}

char *read_addr_server(void)
{
	GtkWidget *entry_addr_srv;

	entry_addr_srv = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_adr");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_addr_srv));
}

char *read_port_server(void)
{
	GtkWidget *entry_port_srv;

	entry_port_srv = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_port");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_port_srv));
}

char *read_login(void)
{
	GtkWidget *entry_login;

	entry_login = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_login");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_login));
}

char *read_addr_opponent(void)
{
	GtkWidget *entry_addr_j2;

	entry_addr_j2 = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_addr_j2");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_addr_j2));
}

char *read_port_opponent(void)
{
	GtkWidget *entry_port_j2;

	entry_port_j2 = (GtkWidget *) gtk_builder_get_object(p_builder, "entry_port_j2");

	return (char *)gtk_entry_get_text(GTK_ENTRY(entry_port_j2));
}

void display_won(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	
	char message[70];
	int score_J1 = get_score_J1();
	int score_J2 = get_score_J2();
	
	if(score_J1 > score_J2)
	{
		sprintf(message, "End Game.\n\n You WON (%d-%d) !!!", score_J1, score_J2);
	}
	else
	{
		sprintf(message, "End Game.\n\n You WON (%d-%d) !!!", score_J2, score_J1);
	}

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, message, NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

void display_lost(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	
	char message[70];
	int score_J1 = get_score_J1();
	int score_J2 = get_score_J2();
	
	if(score_J1 < score_J2)
	{
		sprintf(message, "End Game.\n\n You LOST (%d-%d) ...", score_J1, score_J2);
	}
	else
	{
		sprintf(message, "End Game.\n\n You LOST (%d-%d) ...", score_J2, score_J1);
	}

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, message, NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

void display_connection_established(void)
{
	if(msgConnect == 0)
	{
		GtkWidget *dialog;

		GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

		dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Connection to game server established !\n\n To refresh players list, please click on the [Se Connecter] button.", NULL);
		gtk_dialog_run(GTK_DIALOG (dialog));

		gtk_widget_destroy(dialog);
		
		msgConnect = 1;
	}
}

void display_invalid_action(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Invalid Action.\n\n Please select an empty square.", NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

void display_invalid_information_for_connection(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Unable to connect to game server.\n\n Be careful to fill all required fields.", NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

void display_invalid_port_number(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Unable to start game...\n\n You have to provide a port number which is not yours.", NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

void display_opponent_refused(void)
{
	GtkWidget *dialog;

	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(p_builder, "window1")), flags, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Opponent refused to play...\n\n Try another player at your height.", NULL);
	gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);
}

int display_confirm_game(void)
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

static void click_connect_server(GtkWidget *b)
{
	char * portServer = read_port_server();
	char * login = read_login();

	// We launch a thread that will just write to the client to communicate our position to the opponent
	pthread_t thread_connect_server_players;
	int desc_thread_connect_server_players = pthread_create(&thread_connect_server_players, NULL, connect_server, 0);
}

void disable_button_start(void)
{
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_start"), FALSE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_connect"), FALSE);
}

void enable_button_start(void)
{
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_start"), TRUE);
	gtk_widget_set_sensitive((GtkWidget *) gtk_builder_get_object (p_builder, "button_connect"), TRUE);
}

static void click_connect_opponent(GtkWidget *b)
{
	char* portToConnect = read_port_opponent();

	char portInChar[6]; 
	sprintf(portInChar, "%d", port);

	if(strcmp(portInChar, portToConnect) == 0)
	{
		display_invalid_port_number();
	}
	else{
		// Launch a modele_client and listen on the named pipe to update the intreface
		pid_t pid_client = fork();
		if(pid_client != 0)
		{
			// I am the father
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

void freeze_checkerboard(void)
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

void unfreeze_checkerboard(void)
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


void init_interface_game(void)
{
	// Initializing the checkerboard (D4=white, E4=black, D5=black, E5=white)
	change_img_square(3, 3, 1);
	change_img_square(4, 3, 0);
	change_img_square(3, 4, 0);
	change_img_square(4, 4, 1);

	// Initializing scores and players
	if(color==1)
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

	scores_calculation();

	nbMove = 0;

	// Black player begins
	if(color == 0){
		unfreeze_checkerboard();
	}
}

void reset_interface(void)
{
	int i, j;
	
	for(i = 0 ; i < 8 ; i++)
	{
		for(j = 0 ; j < 8 ; j++)
		{
			change_img_square(i, j, -1);
		}
	}
	
	bold_label_player(-1);
	
	scores_calculation();
	freeze_checkerboard();
}

void reset_players_list(void)
{
	GtkTextIter start, end;

	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &end);

	gtk_text_buffer_delete(GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(p_builder, "textview_joueurs")))), &start, &end);
}

void add_player(char *login, char *address, char *port)
{
	const gchar *joueur;

	joueur=g_strconcat(login, " - ", address, " : ", port, "\n", NULL);

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

	/* Initializing GTK+ */
	XInitThreads();
	gtk_init (& argc, & argv);

	/* Creating a new GtkBuilder */
	p_builder = gtk_builder_new();

	if (p_builder != NULL)
	{
		/* Loading XML in p_builder */
		gtk_builder_add_from_file (p_builder, "UI_Glade/Othello.glade", & p_err);

		if (p_err == NULL)
		{
			/* Getting a pointer on the window */
			GtkWidget * p_win = (GtkWidget *) gtk_builder_get_object (p_builder, "window1");

			/* Handling the event when clicking on a square for each square of the checkerboard */
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA1"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB1"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC1"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD1"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE1"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF1"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG1"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH1"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA2"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB2"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC2"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD2"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE2"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF2"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG2"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH2"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA3"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB3"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC3"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD3"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE3"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF3"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG3"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH3"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA4"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB4"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC4"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD4"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE4"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF4"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG4"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH4"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA5"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB5"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC5"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD5"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE5"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF5"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG5"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH5"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA6"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB6"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC6"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD6"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE6"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF6"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG6"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH6"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA7"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB7"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC7"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD7"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE7"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF7"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG7"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH7"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxA8"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxB8"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxC8"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxD8"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxE8"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxF8"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxG8"), "button_press_event", G_CALLBACK(player_move), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "eventboxH8"), "button_press_event", G_CALLBACK(player_move), NULL);

			/* Handling clicking on the buttons of the interface */
			g_signal_connect(gtk_builder_get_object(p_builder, "button_connect"), "clicked", G_CALLBACK(click_connect_server), NULL);
			g_signal_connect(gtk_builder_get_object(p_builder, "button_start"), "clicked", G_CALLBACK(click_connect_opponent), NULL);

			/* Handling clicking on the button to close interface */
			g_signal_connect_swapped(G_OBJECT(p_win), "destroy", G_CALLBACK(close_game), NULL);

			/* Getting port number passed in the arguments */
			port=atoi(argv[1]);

			/* Initializing checkerboard */
			reset_interface();

			// Here we create the two pipes we need to communicate between the GUI, the client and the server
			char serverToGui[] = "serverToGui.fifo";
			if(mkfifo(serverToGui, S_IRUSR | S_IWUSR ) != 0)  
			{
				fprintf(stderr, "Unable to create the named pipe.\n");
				exit(EXIT_FAILURE);
			}

			char guiToClient[] = "guiToClient.fifo";
			if(mkfifo(guiToClient, S_IRUSR | S_IWUSR ) != 0)  
			{
				fprintf(stderr, "Unable to create the named pipe.\n");
				exit(EXIT_FAILURE);
			}

			pid_t pid_serv = fork();
			if(pid_serv != 0)
			{ 	
				// I am the father
				pid = (int) pid_serv;

				// We launch a thread that will just read the first pipe and modify the gui
				pthread_t thread_read_pipe_and_modify_gui;
				int desc_thread_read_pipe_and_modify_gui = pthread_create (&thread_read_pipe_and_modify_gui, NULL, read_pipe_and_modify_gui, argv);

				// We launch a thread that will just write to the client to communicate our position to the opponent
				pthread_t thread_write_to_client;
				int desc_thread_write_to_client = pthread_create (&thread_write_to_client, NULL, write_to_client, argv);

				// init the interface
				gtk_widget_show_all(p_win);
				gtk_main();
			}
			else
			{
				// We override the process
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
			/* Displaying error messages from GTK+ */
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
		fprintf(stderr, "Unable to open the exit of the named pipe.\n");
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
					
					int accept = display_confirm_game();
					
					char message[5];

					if(accept == 1)
					{
						color = 1;
						init_interface_game();
						
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
					color = 0;
					init_interface_game();

					// Disable connect button
					disable_button_start();
				}
				else if(strcmp(content, "no") == 0)
				{
					display_opponent_refused();
				}
			}
			else if(strcmp(header, "c") == 0){
				// translation of the position
				char coord[2];
				int col, row;
				int opponent_color = (color == 0) ? 1 : 0;

				col = ctoi(content[0]);
				row = ctoi(content[1]);
				// interpretation of the position
				indexes_to_coord(col, row, coord);

				printf("Opponent add piece to : %s\n", coord);
				fflush(stdout);

				change_img_square(col, row, opponent_color);

				// Calling surrounding functions
				surrounding_R_L(col, row, opponent_color);
				surrounding_L_R(col, row, opponent_color);
				surrounding_T_B(col, row, opponent_color);
				surrounding_B_T(col, row, opponent_color);
				surrounding_NW(col, row, opponent_color);
				surrounding_NE(col, row, opponent_color);
				surrounding_SE(col, row, opponent_color);
				surrounding_SW(col, row, opponent_color);

				// End Game
				if(nbMove == 31 || isCheckerboardFull() == 1)
				{
					scores_calculation();
					if((color == 0 && get_score_J1() < get_score_J2()) ||
					(color == 1 && get_score_J1() > get_score_J2()))
					{
						display_won();
					}
					else
					{
						display_lost();
					}
					
					printf("The game is done !");
					fflush(stdout);
		
					reset_interface();
					enable_button_start();
				}
				else
				{
					scores_calculation();
					bold_label_player(color);
					unfreeze_checkerboard();
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

	// We also open the second pipe that will write to the client to communicate the move to the opponent
	// this funcion will block until we have a client that open the pipe on read
	if((descGuiToClient = open(guiToClient, O_WRONLY)) == -1) 
	{
		fprintf(stderr, "Unable to open the entrance of the named pipe.\n");
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

	// Creating socket and linking
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

	freeaddrinfo(servinfo);       // free struct

	// We send our ip, port and login
	char message[100];
	if (strcmp(read_addr_server(), "")==0 || strcmp(read_port_server(), "")==0 || strcmp(read_login(), "")==0)
	{
		display_invalid_information_for_connection();
		return NULL;
	}
	strcpy(message, "c,");
	strcat(message, read_addr_server());
	strcat(message, ",");
	strcat(message, read_port_server());
	strcat(message, ",");
	strcat(message, read_login());
	strcat(message, ",");
	send(sockfd, message, strlen(message), 0);

	printf("Waiting for an answer from the server...\n");
	fflush(stdout);

	// And we receive the list of all the players
	// we normally have to do a while loop
	if((numbytes = recv(sockfd, buf, 100-1, 0)) == -1) 
	{
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';

	printf("Received message : %s\n",buf);
	fflush(stdout);
	
	// Cleaning the list on the interface
	reset_players_list();

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

		// Checking that the token is not null
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
		
		add_player(login, ip, port);
	}

	display_connection_established();
}

