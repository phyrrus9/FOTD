all:
	cc -o server server.c -lncurses
	cc -o client client.c -lncurses
fotd:
	cd files; zip ../file *
