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

#ifndef _WIN32
    #include "curses.h"
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
char HOSTNAME_SERVER[25] = "127.0.0.1";
char ncurses_off = false;

void showprogress(double progress)
{
#ifndef _WIN32
    if (!ncurses_off)
    {
        wclear(p);
        wrefresh(p);
        wprintw(p, "Downloading....\nProgress: %0.0f%%\n", progress);
        wrefresh(p);
    }
    else
    {
        printf("Downloading...Prigress: %0.0f%%\n", progress);
    }
#else
    system("cls");
    printf("Downloading...Prigress: %0.0f%%\n", progress);
#endif
}

int main(int argc, char *argv[])
{
    char waitforuser_exit = 1;
    char hostip[20];
    double progress;
    PORT = PORTNO;
    strcpy(hostip, HOSTNAME_SERVER);
    
    //custom settings check
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-fast") == 0)
            {
                waitforuser_exit = 0;
            }
            if (strcmp(argv[i], "-port") == 0)
            {
                PORT = atoi(argv[++i]);
            }
            if (strcmp(argv[i], "-host") == 0)
            {
                strcmp(hostip, argv[++i]);
            }
            if (strcmp(argv[i], "-nocurses") == 0)
            {
                printf("No curses mode\n");
                ncurses_off = true;
            }
            if (strcmp(argv[i], "-help") == 0)
            {
                printf("Usage: %s [options] <parameters>\n"
                       "Option: -fast (dont display all the crap)\n"
                       "Option: -port n (change to port n)\n"
                       "Option: -host a.b.c.d (change to ip a.b.c.d)\n"
                       "Option: -nocurses (run text-based only)\n"
                       "Option: -help (display this dialog\n"
                       "Copyright © 2012-13 Ethan Laur (phyrrus9)\n"
                       "<phyrrus9@gmail.com>\n",
                       argv[0]);
                exit(0);
            }
        }
    }
    
#ifndef _WIN32
    //initialize the ncurses windows
    if (!ncurses_off)
    {
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
    }
    else
    {
        printf("Connecting to server...\n");
    }
#else
    system("cls")
    //display status message
    printf("Connecting to server...\n");
#endif
    //define buffers and main networking variables
    int sockfd = 0, n = 0;
    unsigned char recvBuff[15]; //1 mb filesize
    struct sockaddr_in serv_addr; 

    memset(recvBuff, 0,sizeof(recvBuff)); //initialize the buffer
    //create a socket (pipe)
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
    #ifndef _WIN32
        if (!ncurses_off)
        {
            endwin();
        }
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
        if (!ncurses_off)
        {
            endwin();
        }
    #endif
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    //connect to the remote server
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
    #ifndef _WIN32
        if (!ncurses_off)
        {
            endwin();
        }
    #endif
        printf("\n Error : Connect Failed \n");
        return 1;
    } 

#ifndef _WIN32
    if (!ncurses_off)
    {
        //wclear(w);
        wrefresh(w);
    }
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
        unsigned char error_code[1]; //between the magic size and the time
        n = read(sockfd, magic_size, 4); //read the header from the pipe
        length = c4toi(magic_size); //determine the size of the file to come
        n = read(sockfd, error_code, 1); //one-byte header error codes
        if (length == 0)
        {
            if (!ncurses_off)
            {
                endwin();
            }
            printf("Server denied your connection, try again later\n");
            printf("Error code: %d\n", error_code[0]);
            exit(0);
        }
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
        if (!ncurses_off)
        {
            wprintw(w, "File size: %0.2f", o_length);
        }
        else
        {
            printf("File size: %0.02f", o_length);
        }
#define printstatement(b)\
if (!ncurses_off)\
{\
    wprintw(w, b);\
}\
else\
{\
    printf("b\n");\
}
#else
        printf("File size: %0.02f", o_length);
#define printstatement(b) printf(b);
#endif
		switch (level)
		{
			case 0:
                printstatement("B")
				break;
			case 1:
				printstatement("KB")
				break;
			case 2:
				printstatement("MB")
				break;
		}
#ifndef _WIN32
        if (!ncurses_off)
        {
            wrefresh(w);
        }
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
                progress = ( (double)n / length ) * 100; //calculate % done
                showprogress(progress); //see function
                if ( lcount % ( 1024 * 15 ) == 0 )
                {
                    netspeed = ( (double)n / (time(0) - stime) ) / 1024; //calculate speed in kbps
                }
                if (!ncurses_off)
                {
                    wprintw(p, "Speed: %0.0f kb/s", netspeed);
                    wrefresh(p);
                }
            }
            //read the stream 1 byte at a time and write it to the file
            int t = 0;
            t = read(sockfd, c, 1);
            if (t == 0)
                n = length;
            n += t;
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
    showprogress(progress);
    if (waitforuser_exit)
    {
        if (!ncurses_off)
        {
            wprintw(p, "\nFile downloaded, press any key to exit..\n");
            wrefresh(p);
            getch();
        }
        else
        {
            printf("\nFile downloaded..\n");
        }
    }
    if (!ncurses_off)
    {
        endwin();
    }
#else
    printf("\nFile downloaded...\n");
#endif
    return 0;
}
