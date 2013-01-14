/**
 * File Of The Day copyright © 2013 Ethan Laur (phyrrus9) [Open Source]
 * File of the day was a purely entertainment project (non-piracy) that
 * aims to send a random file to any client that connects during the day
 * You must change the files on your own (and restart the server so that
 * the checksums validate fine), but I hope to include some random files
 * like funny photos, GNU/GPL music files, and little text files I write
 * or have permission to use. PLEASE do NOT send files that you are not
 * licensed to send, this is a violation of copyright and we don't do 
 * things like that here. You may make any changes to this program if
 * you like, but please include credit to the authors listed in this
 * document. If you make a change and would like it to be added to the
 * main distribution, please email me (Ethan Laur) with your change and
 * I will add it to some part of the source code (even if I do not add
 * it to the main distribution) and give you credit in all files.
 * contributors:
 * Ethan Laur (phyrrus9) <phyrrus9@gmail.com>
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#define HOSTNAME_SERVER "97.85.65.20"

#ifndef _WIN32
    #include <curses.h>
#endif

#include <time.h>

#include "buffers.h"
#include "convert.c"

#define OFFSET MOFFSET

#ifndef _WIN32
    WINDOW *w;
    WINDOW *p;
#endif

int PORT;

void showprogress(double progress)
{
#ifndef _WIN32
    wclear(p);
    wrefresh(p);
    wprintw(p, "Downloading....\nProgress: %0.0f%%\n", progress);
    wrefresh(p);
#else
    system("cls");
    printf("Downloading...Prigress: %0.0f%%\n", progress);
#endif
}

int main(int argc, char *argv[])
{
    char waitforuser_exit = 1;
    char hostip[20];
    PORT = PORTNO;
    strcpy(hostip, HOSTNAME_SERVER);
#ifndef _WIN32
    //initialize the ncurses windows
    initscr();
    noecho();
    refresh();
    w = newwin(5,140,0,0); //lines cols start_y start_x
    p = newwin(6, 140, 5, 0); //look above
    wclear(p);
    wclear(w);
    wrefresh(w);
    wrefresh(p);
    //display status message
    wprintw(w, "Connecting to server...\n");
    wrefresh(w);
#else
    system("cls")
    //display status message
    printf("Connecting to server...\n");
#endif
    //define buffers and main networking variables
    int sockfd = 0, n = 0;
    unsigned char recvBuff[15]; //1 mb filesize
    struct sockaddr_in serv_addr; 

    //custom settings check
    if (argc > 1)
    {
        PORT = atoi(argv[1]);
    }
    if (argc > 2)
    {
        if (strcmp(argv[2], "-fast") == 0)
        {
            waitforuser_exit = 0;
        }
    }

    memset(recvBuff, 0,sizeof(recvBuff)); //initialize the buffer
    //create a socket (pipe)
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
    #ifndef _WIN32
        endwin();
    #endif
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, 0, sizeof(serv_addr)); //initialize the server details

    //set up the socket
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); 

    if(inet_pton(AF_INET, hostip, &serv_addr.sin_addr)<=0)
    {
    #ifndef _WIN32
        endwin();
    #endif
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    //connect to the remote server
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
    #ifndef _WIN32
        endwin();
    #endif
        printf("\n Error : Connect Failed \n");
        return 1;
    } 

#ifndef _WIN32
    //wclear(w);
    wrefresh(w);
#endif
    //begin downloading the file
	FILE *f;
    long stime = time(0); //get the start time (for speed calculation)
	char firstcall = 1;
    {
        //recvBuff[n] = 0;
        unsigned int length = 0;
        unsigned int sendtime = 0;
        unsigned int serveruptime = 0;
        unsigned char magic_size[4]; //header variable
        n = read(sockfd, magic_size, 4); //read the header from the pipe
        length = c4toi(magic_size); //determine the size of the file to come
        n = read(sockfd, magic_size, 4); //read the header from the pipe
        time_t rawtime_s = c4toi(magic_size); //determine the size of the file to come
        n = read(sockfd, magic_size, 4); //read the header from the pipe
        time_t rawtime_u = c4toi(magic_size); //determine the size of the file to come
        
        //display the file size (formatted)
		int level = 0;
		double o_length = length;
		while (o_length > 1024)
		{
			level++;
			o_length /= 1024;
		}
#ifndef _WIN32
        struct tm * timeinfo;
        struct tm * uptime;
        timeinfo = localtime(&rawtime_s);
        //wprintw(w, "Send time: %d\n", asctime(timeinfo));
        uptime = localtime(&rawtime_u);
        //wprintw(w, "Server up: %d\n", asctime(uptime));
		wprintw(w, "File size: %0.2f", o_length);
#define printstatement(b) wprintw(w, b)
#else
        printf("File size: %0.02f", o_length);
#define printstatement(b) printf(b)
#endif
		switch (level)
		{
			case 0:
                printstatement("B");
				break;
			case 1:
				printstatement("KB");
				break;
			case 2:
				printstatement("MB");
				break;
		}
#ifndef _WIN32
        wrefresh(w);
#endif
        f = fopen("./fotd.zip", "wb"); //open the output file in write+binary+trunc mode
        sleep(1);
        //initialize data variables
        n = 0;
        int lcount = 0;
        unsigned char c[3];
        double netspeed;
        while (n < length)
        {
            if ( lcount % ( 1024 * 5 ) == 0) //( 1024 * x ) x=KB per update
            {
                double progress = ( (double)n / length ) * 100; //calculate % done
                showprogress(progress); //see function
                if ( lcount % ( 1024 * 15 ) == 0 )
                {
                    netspeed = ( (double)n / (time(0) - stime) ) / 1024; //calculate speed in kbps
                }
                wprintw(p, "Speed: %0.0f kb/s", netspeed);
                wrefresh(p);
            }
            //read the stream 1 byte at a time and write it to the file
            n += read(sockfd, c, 1);
            fprintf(f, "%c", c[0]);
            lcount++;
        }
    }

    if(n < 0)
    {
        printf("\n Read error \n");
    } 

	fclose(f);
#ifndef _WIN32
    if (waitforuser_exit)
    {
        wprintw(p, "\nFile downloaded, press any key to exit..\n");
        wrefresh(p);
        getch();
    }
    endwin();
#else
    printf("\nFile downloaded...\n");
#endif
    return 0;
}
