all:out
out:mysqltool.cpp server.cpp client.cpp
	g++ mysqltool.cpp server.cpp -o server.out -I/usr/include/mysql -lmysqlclient
	g++ client.cpp -o client.out -lssl -lcrypto
