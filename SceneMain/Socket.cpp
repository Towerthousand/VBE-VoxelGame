#include "Socket.hpp"
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
#include "commons.hpp"
#include <fcntl.h>

Socket::Socket() : sockfd(-1)
{

}

void Socket::connect(std::string host, int port)
{
	VBE_ASSERT(sockfd == -1, "Socket is already connected");

	struct sockaddr_in serv_addr;
	struct hostent* server;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Could not create socket!");
		VBE_ASSERT(false, "fail");
	}

	server = gethostbyname(host.c_str());

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	if(::connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("connect fail");
		VBE_ASSERT(false, "fail");
	}
}



void Socket::setBlocking(bool blocking)
{
#ifdef WIN32
	unsigned long mode = blocking ? 0 : 1;
	if (ioctlsocket(sockfd, FIONBIO, &mode) != 0)
	{
		perror("ioctlsocket fail");
		VBE_ASSERT(false, "fail");
	}
#else
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags < 0)
	{
		perror("fcntl F_GETFL fail");
		VBE_ASSERT(false, "fail");
	}
	flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
	if (fcntl(sockfd, F_SETFL, flags) < 0)
	{
		perror("fcntl F_SETFL fail");
		VBE_ASSERT(false, "fail");
	}
#endif
}

void Socket::close()
{
	VBE_ASSERT(sockfd != -1, "Socket is not connected");
	if(::close(sockfd) < 0)
	{
		perror("close fail");
		VBE_ASSERT(false, "fail");
	}

	sockfd = -1;
}

void Socket::read(void* dest, int len)
{
	VBE_ASSERT(sockfd != -1, "Socket is not connected");

	char* cdest = (char*)dest;
	int done = 0;
	while(done < len)
	{
		int r = ::read(sockfd, cdest, len-done);
		if(r == -1 && errno != EWOULDBLOCK)
		{
			perror("read fail");
			VBE_ASSERT(false, "fail");
		}
		done += r;
		cdest += r;
	}
}

char Socket::readByteNonblock()
{
	VBE_ASSERT(sockfd != -1, "Socket is not connected");
	setBlocking(false);

	char res = -1;
	if(::read(sockfd, &res, sizeof(res)) == -1 && errno != EWOULDBLOCK)
	{
		perror("read fail");
		VBE_ASSERT(false, "fail");
	}

	setBlocking(true);

	return res;
}

char Socket::readByte()
{
	char r;
	read(&r, sizeof(r));
	return r;
}

short Socket::readShort()
{
	short r;
	read(&r, sizeof(r));
	r = ntohs(r);
	return r;
}

std::string Socket::readString()
{
	char str[64];
	read(str, sizeof(str));
	int len = 64;
	while(len > 0 && str[len-1] == ' ') len--;
	return std::string(str+0, str+len);
}

void Socket::readBytes(char* bytes)
{
	read(bytes, 1024);
}

void Socket::write(const void* src, int len)
{
	VBE_ASSERT(sockfd != -1, "Socket is not connected");
	int r = ::write(sockfd, src, len);
	if(r != len)
	{
		perror("write fail");
		VBE_ASSERT(false, "fail");
	}
}

void Socket::writeByte(char c)
{
	write(&c, sizeof(c));
}

void Socket::writeShort(short c)
{
	c = htons(c);
	write(&c, sizeof(c));
}

void Socket::writeString(const std::string& s)
{
	if(s.size() > 64)
	{
		printf("Tried to send a string longer than 64 chars :(\n");
		VBE_ASSERT(false, "fail");
	}

	char str[64];
	for(int i = 0; i < 64; i++)
		if(i < s.size())
			str[i] = s[i];
		else
			str[i] = ' ';

	write(str, sizeof(str));
}

void Socket::writeBytes(char* bytes)
{
	write(bytes, 1024);
}
