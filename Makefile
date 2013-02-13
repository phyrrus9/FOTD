native: Tpool/Thread.o server.cpp client.c
	c++ -o ncpserver -g server.cpp Tpool/Thread.o -lncurses
	cc -o ncpclient -g client.c -lncurses
all: native
	cd ncp-server-ios; xcodebuild
	cd ncp-client-ios; xcodebuild
	cp ncp-server-ios/build/Release-iphoneos/ncp-server-ios.app/ncp-server-ios ./ncp-ios/bin/ncpserver
	cp ncp-client-ios/build/Release-iphoneos/ncp-client-ios.app/ncp-client-ios ./ncp-ios/bin/ncpclient
	dpkg -b ncp-ios
clean:
	rm -rf ncpserver ncpclient ncp-ios/bin/* file.zip ncp-ios.deb
	rm -rf ncp-server-ios/build
	rm -rf ncp-client-ios/build
fotd:
	cd files; zip ../file *
