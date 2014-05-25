#ifndef SOCKET_HPP
#define SOCKET_HPP

#include<string>

class Socket
{
	private:
		int sockfd;
	public:
		Socket();
		void connect(std::string host, int port);

		void read(void* dest, int len);
		char readByte();
		short readShort();
		std::string readString();
		void readBytes(char* bytes);

		char readByteNonblock();

		void write(const void* src, int len);
		void writeByte(char c);
		void writeShort(short c);
		void writeString(const std::string& s);
		void writeBytes(char* bytes);

		void setBlocking(bool blocking);
		void close();
};

#endif // SOCKET_HPP
