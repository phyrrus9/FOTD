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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

#include <curses.h>
#include <signal.h>

#include "convert.c"
#include "buffers.h"

char SENDFILE[50];
int PORT;

WINDOW *w;
WINDOW *p;
//define server stat variables
int client_total = 0;
int client_error = 0;
double bandwidth_total = 0;
double bandwidth_waste = 0;
unsigned int v_p = 0;
time_t startuptime;
double avg_speed = 0;
double avg_speed_total = 0;

void print_size_fmt(WINDOW *w, double _value, const char *message) //print a formatted file size with a description
{
    int level = 0;
    double o_length = _value;
    while (o_length > 1024)
    {
        level++;
        o_length /= 1024;
    }
    wprintw(w, "%s %0.2f", message, o_length);
    switch (level)
    {
        case 0:
            wprintw(w,"B");
            break;
        case 1:
            wprintw(w,"KB");
            break;
        case 2:
            wprintw(w,"MB");
            break;
        case 3:
            wprintw(w,"GB");
            break;
    }
    wprintw(w, "\n");
    wrefresh(w);
}

void update_main(WINDOW *w) //prints the server stats
{
    struct tm *stimeinfo = localtime(&startuptime);
    wclear(w);
    wrefresh(w);
    wprintw(w, "=================PROGRM INFORMATION=================\n");
    wprintw(w, "Port number:                %d\n", PORT);
    wprintw(w, "File name:                  %s\n", SENDFILE);
    print_size_fmt(w, v_p, "File size:                 ");
    wprintw(w, "Start time:                 %s", asctime(stimeinfo));
    wprintw(w, "=================CLIENT INFORMATION=================\n");
    wprintw(w, "Total clients served today: %d\nTotal client disconnects:   %d\nNet clients:                %d\n", client_total, client_error, (client_total - client_error) );
    wprintw(w, "=================SERVER INFORMATION=================\n");
    print_size_fmt(w, bandwidth_total, "Total bandwidth:           ");
    print_size_fmt(w, bandwidth_waste, "Total bandwidth waste:     ");
    print_size_fmt(w, bandwidth_total - bandwidth_waste, "Net bandwidth:             ");
    wprintw(w, "Average network speed:      %0.2f KB/S\n", avg_speed);
    wrefresh(w);
}

int main(int argc, char *argv[])
{
    startuptime = time(0);
    signal(SIGPIPE, SIG_IGN);
    strcpy(SENDFILE, "./file.zip");
    PORT = PORTNO;
    initscr();
    noecho();
    refresh();
    w = newwin(15, 140, 0, 0);
    p = newwin(2, 140, 16, 0);
    wclear(w);
    wrefresh(w);
    wclear(p);
    wrefresh(p);
	FILE *f;
    unsigned char fbuf[15];

    //developer mods
    if (argc > 2)
    {
        strcpy(SENDFILE, argv[1]);
        PORT = atoi(argv[2]);
    }
	
	f = fopen(SENDFILE, "r");
	if (f == NULL)
	{
        endwin();
		printf("Error! File does not exist!\n");
		exit(-1);
	}

    wclear(w);
    wrefresh(w);
    wprintw(w, "Scanning file...\n");
    wrefresh(w);
	unsigned char v_c = 0;
	while ( fscanf(f, "%c", &v_c) != EOF )
		v_p++;
	fclose(f);
    wclear(w);
    wrefresh(w);

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10);

    while(1)
    {
        int size = 0;
        int n = 0;
        update_main(w);
        wprintw(w,"Waiting for client..\n");
        wrefresh(w);
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        client_total++;
        long stime = time(0);
        wprintw(w, "Client connected, writing file\n");
        wrefresh(w);
        f = fopen(SENDFILE, "rb");
        char ic;
        if (f == NULL)
        {
            printf("Error! File not open!\n");
        }
        
        place_size(fbuf, v_p);
        write(connfd, fbuf, 4);
        place_size(fbuf, (unsigned int)stime);
        write(connfd, fbuf, 4);
        memset(fbuf, 0, 4);
        place_size(fbuf, (unsigned int)startuptime);
        write(connfd, fbuf, 4);
        if (stime == startuptime)
        {
            endwin();
            exit(0);
        }
        
        int lcount = 0;
        n = 0;
        int t_n = 0;
        double netspeed = 0;
        double instance_avg_speed = 1;
        while ( fscanf(f, "%c", &fbuf[0]) != EOF )
        {
            t_n = write(connfd, fbuf, 1);
            if (t_n < 0)
            {
                bandwidth_waste += n;
                client_error++;
                close(connfd);
                fclose(f);
                continue;
            }
            n += t_n;
            bandwidth_total += t_n;
            if (lcount % (1024 * 3) == 0) // (1024 * x) where x is # of KB per update
            {
                double progress = ( (double)n / v_p ) * 100;
                wclear(p);
                wprintw(p, "Progress: %0.0f%%\n", progress );
                netspeed = ( (double)n / (time(0) - stime) ) / 1024;
                if (lcount % (1024 * 15) == 0)
                {
                    instance_avg_speed = netspeed;
                }
                wprintw(p, "Speed: %0.2f kb/s\n", netspeed);
                wrefresh(p);
                update_main(w);
            }
            lcount++;
        }
        avg_speed_total += instance_avg_speed;
        avg_speed = avg_speed_total / (client_total - client_error);
        wclear(p);
        wrefresh(p);
        fclose(f);
        
        close(connfd);
        sleep(1);
    }
    endwin();
    
}