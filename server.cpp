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

#include "curses.h"
#include <signal.h>

#include "convert.c"
#include "buffers.h"
#include "Tpool/Thread.h"

#define VERSION "001"

#define log_out(); if (debug_logging) fprintf(logfile, "%s", log_statement);

void update_main(WINDOW *w);
void file_cache(void);
unsigned char get_error_code(void);

char SENDFILE[50];
int PORT;

WINDOW *w;
WINDOW *p;
WINDOW *a; //admin window
//define server stat variables
int client_total = 0;
int client_error = 0;
double bandwidth_total = 0;
double bandwidth_waste = 0;
unsigned int v_p = 0;
time_t startuptime;
double avg_speed = 0;
double avg_speed_total = 0;

char file_caching = false;
char block_connections = false;
char curses_off = false;

char debug_logging = false;
char log_statement[100];
FILE *logfile;

unsigned char get_error_code(void)
{
    if (file_caching)
    {
        return 1;
    }
    if (block_connections)
    {
        return 2;
    }
    return 0;
}

void file_cache(void)
{
    file_caching = true;
    v_p = 0;
    FILE *f;
    f = fopen(SENDFILE, "rb");
    if (f == NULL)
    {
        printf("Error! File not open!\n");
    }
	if (f == NULL)
	{
        if (!curses_off)
        {
            endwin();
        }
		printf("Error! File does not exist!\n");
		exit(-1);
	}
    
    if (!curses_off)
    {
        wclear(w);
        wrefresh(w);
        wprintw(w, "Scanning file...\n");
        wrefresh(w);
    }
    else
    {
        printf("Scanning file...\n");
    }
	unsigned char v_c = 0;
	while ( fscanf(f, "%c", &v_c) != EOF )
		v_p++;
	fclose(f);
    if (!curses_off)
    {
        wclear(w);
        wrefresh(w);
    }
    file_caching = false;
    if (!curses_off)
    {
        update_main(w);
    }
}

class admin_thread : public tpool::Thread
{
    void Entry(void)
    {
        if (curses_off)
        {
            //return;
        }
        char action = 0;
        char c = 0;
        int recache_position = 0;
        if (!curses_off)
        {
            wclear(a);
            wrefresh(a);
        }
        else
        {
            printf("Admin panel: [b=block connections r=recache q=quit]\n");
        }
        while (true)
        {
            if (!curses_off)
            {
                wclear(a);
                wprintw(a, "Admin panel: [b=block connections r=recache q=quit]\n");
            }
            if (block_connections)
            {
                if (!curses_off)
                {
                    wprintw(a, "Connections blocked\n");
                    wrefresh(a);
                }
            }
            if (!curses_off)
            {
                wrefresh(a);
                action = getch();
            }
            else
            {
                scanf("%c", &action);
            }
            switch (action)
            {
                    case 'b':
                        block_connections = block_connections ? false : true;
                        if (curses_off)
                        {
                            printf("Connections blocked: %d\n", block_connections);
                        }
                        break;
                    case 'r':
                        if (!curses_off)
                        {
                            wprintw(a, "File recache: ");
                            wrefresh(a);
                            while (c != '\n')
                            {
                                c = getch();
                                wprintw(a, "%c", c);
                                SENDFILE[recache_position] = c;
                                recache_position++;
                                wrefresh(a);
                            }
                            SENDFILE[--recache_position] = 0;
                        }
                        else
                        {
                            printf("File recache: ");
                            scanf("%s", SENDFILE);
                        }
                        if (!curses_off)
                        {
                            wprintw(a, "File: %s\n", SENDFILE);
                            wprintw(a, "Recaching (clients will be dropped)\n");
                            wrefresh(a);
                        }
                        else
                        {
                            printf("File: %s\n", SENDFILE);
                            printf("Recaching (clients will be dropped)\n");
                        }
                        c = 0;
                        file_cache();
                        if (!curses_off)
                        {
                            wclear(a);
                            wrefresh(a);
                        }
                        else
                        {
                            printf("Recache complete\n");
                        }
                        break;
                    case 'q':
                        if (!curses_off)
                        {
                            endwin();
                        }
                        printf("Server shutdown..\n");
                        exit(1);
                        break;
            }
        }
    }
};

void print_size_fmt(WINDOW *w, double _value, const char *message) //print a formatted file size with a description
{
    if (curses_off)
    {
        return;
    }
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
    if (file_caching || curses_off)
    {
        return;
    }
    struct tm *stimeinfo = localtime(&startuptime);
    wclear(w);
    wrefresh(w);
    wprintw(w, "=================PROGRM INFORMATION=================\n");
    wprintw(w, "Build number:               %s\n", VERSION);
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
    PORT = PORTNO;
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-log") == 0)
            {
                logfile = fopen(argv[i + 1], "w");
                if (logfile == NULL)
                {
                    exit(-1);
                }
                debug_logging = true;
                sprintf(log_statement, "logging enabled\n");
                log_out();
            }
            if (strcmp(argv[i], "-file") == 0)
            {
                strcpy(SENDFILE, argv[i + 1]);
                sprintf(log_statement, "New send file: %s\n", SENDFILE);
                log_out();
            }
            if (strcmp(argv[i], "-port") == 0)
            {
                PORT = atoi(argv[i + 1]);
                sprintf(log_statement, "New port: %d\n", PORT);
                log_out();
            }
            if (strcmp(argv[i], "-nocurses") == 0)
            {
                printf("No curses mode\n");
                curses_off = true;
            }
            if (strcmp(argv[i], "-help") == 0)
            {
                printf("Usage: %s [options] <parameters>\n"
                       "Option: -file f (change to file f)\n"
                       "Option: -port n (change to port n)\n"
                       "Option: -nocurses (run text-based only)\n"
                       "Option: -help (display this dialog\n"
                       "Copyright © 2012-13 Ethan Laur (phyrrus9)\n"
                       "<phyrrus9@gmail.com>\n",
                       argv[0]);
                exit(0);
            }
        }
        
    }
    
    startuptime = time(0);
    signal(SIGPIPE, SIG_IGN);
    strcpy(SENDFILE, "./file.zip");
    if (!curses_off)
    {
        initscr();
        noecho();
        refresh();
        w = newwin(17, 100, 0, 0);
        p = newwin(2, 100, 18, 0);
        a = newwin(8, 100, 0, 102);
        wclear(w);
        wrefresh(w);
        wclear(p);
        wrefresh(p);
        wclear(a);
        wrefresh(a);
    }
	FILE *f;
    admin_thread administration;
    unsigned char fbuf[15];

    file_cache();
    
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10);

    administration.Run();
    
    while(1)
    {
        int n = 0;
        if (!curses_off)
        {
            update_main(w);
            wprintw(w,"Waiting for client..\n");
            wrefresh(w);
        }
        else
        {
            printf("Waiting for client...\n");
        }
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        client_total++;
        long stime = time(0);
        if (!file_caching)
        {
            if (!curses_off)
            {
                wprintw(w, "Client connected, writing file\n");
                wrefresh(w);
            }
            else
            {
                printf("Client connected, writing file...\n");
            }
        }
        
        f = fopen(SENDFILE, "rb");
        if (f == NULL)
        {
            if (!curses_off)
            {
                endwin();
            }
            printf("ERROR, file %s not open\n", SENDFILE);
            exit(-1);
        }
        
        place_size(fbuf, v_p);
        fbuf[4] = get_error_code(); //fixed byte header field
        if (get_error_code() != 0)
        {
            for (int i = 0; i < 4; i++)
                fbuf[i] = 0;
            write(connfd, fbuf, 5);
            close(connfd);
            continue;
        }
        else
        {
            write(connfd, fbuf, 4);
        }
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
        
        if (get_error_code() != 0)
        {
            close(connfd);
            continue;
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
                if (!curses_off)
                {
                    wclear(p);
                    wprintw(p, "Progress: %0.0f%%\n", progress );
                }
                else
                {
                    if (lcount % (1024 * 6) == 0)
                    {
                        printf("Progress: %0.0f%%\n", progress);
                    }
                }
                netspeed = ( (double)n / (time(0) - stime) ) / 1024;
                if (lcount % (1024 * 15) == 0)
                {
                    instance_avg_speed = netspeed;
                }
                if (!curses_off)
                {
                    wprintw(p, "Speed: %0.2f kb/s\n", netspeed);
                    wrefresh(p);
                }
                update_main(w);
            }
            lcount++;
        }
        avg_speed_total += instance_avg_speed;
        avg_speed = avg_speed_total / (client_total - client_error);
        if (!curses_off)
        {
            wclear(p);
            wrefresh(p);
        }
        fclose(f);
        
        close(connfd);
        sleep(1);
    }
    endwin();
    
}