#include "NetworkManager.hpp"

#include<iostream>
#include <zlib.h>
#include<vector>
#include "SceneMain.hpp"
#include "world/World.hpp"
#include "Player.hpp"

using namespace std;


NetworkManager::NetworkManager(string host, int port, string nickname)
{
	setName("NetworkManager");

	s.connect(host, port);

	//Player identification
	s.writeByte(0x00);
	s.writeByte(0x07);
	s.writeString(nickname);
	s.writeString("blarg");
	s.writeByte(0x00); //Unused

	scene = nullptr;
	world = nullptr;
}

void writeToFile(const vector<unsigned char>& data, const char* filename)
{
	FILE *fp;
	fp=fopen(filename, "wb");
	fwrite(&data[0], sizeof(data[0]), data.size(), fp);
	fclose(fp);
}

bool gzipInflate( const std::vector<unsigned char>& compressedBytes, std::vector<unsigned char>& uncompressedBytes ) {
	if ( compressedBytes.size() == 0 ) {
		uncompressedBytes = compressedBytes ;
		return true ;
	}

	uncompressedBytes.clear() ;

	unsigned full_length = compressedBytes.size() ;
	unsigned half_length = compressedBytes.size() / 2;

	unsigned uncompLength = full_length ;
	char* uncomp = (char*) calloc( sizeof(char), uncompLength );

	z_stream strm;
	strm.next_in = (Bytef *) &compressedBytes[0];
	strm.avail_in = compressedBytes.size() ;
	strm.total_out = 0;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;

	bool done = false ;

	if (inflateInit2(&strm, (16+MAX_WBITS)) != Z_OK) {
		free( uncomp );
		return false;
	}

	while (!done) {
		// If our output buffer is too small
		if (strm.total_out >= uncompLength ) {
			// Increase size of output buffer
			char* uncomp2 = (char*) calloc( sizeof(char), uncompLength + half_length );
			memcpy( uncomp2, uncomp, uncompLength );
			uncompLength += half_length ;
			free( uncomp );
			uncomp = uncomp2 ;
		}

		strm.next_out = (Bytef *) (uncomp + strm.total_out);
		strm.avail_out = uncompLength - strm.total_out;

		// Inflate another chunk.
		int err = inflate (&strm, Z_SYNC_FLUSH);
		if (err == Z_STREAM_END) done = true;
		else if (err != Z_OK)  {
			break;
		}
	}

	if (inflateEnd (&strm) != Z_OK) {
		free( uncomp );
		return false;
	}

	for ( size_t i=0; i<strm.total_out; ++i ) {
		uncompressedBytes.push_back(uncomp[i]);
	}
	free( uncomp );
	return true ;
}

void NetworkManager::update(float deltaTime)
{
	char packetId;
	while((packetId = s.readByteNonblock()) != -1)
	{
		cout<<"Packet "<<int(packetId)<<endl<<flush;
		switch (packetId)
		{
			case 0x00: //Server identification
			{
				cout<<"Server id"<<endl;
				char protoVersion = s.readByte();
				std::string serverName = s.readString();
				std::string serverMotd = s.readString();
				char userType = s.readByte();
				cout<<serverName<<" "<<serverMotd<<endl;
				break;
			}
			case 0x01: //Ping
				break;
			case 0x02: //Level init
			{
				levelData.resize(0);
				if(scene != nullptr)
				{
					scene->removeAndDelete();
					scene = nullptr;
					world = nullptr;
				}
				break;
			}
			case 0x03: //Level chunk
			{
				short length = s.readShort();
				char data[1024];
				s.readBytes(data);
				char percent = s.readByte();

				for(int i = 0; i < length; i++)
					levelData.push_back(data[i]);
				break;
			}
			case 0x04: //Level end
			{
				level.tx = s.readShort();
				level.ty = s.readShort();
				level.tz = s.readShort();

				cout<<"Level size: "<<level.tx<<" "<<level.ty<<" "<<level.tz<<endl;
				long unsigned int levelSize = level.tx*level.ty*level.tz;
				level.blocks = vector<unsigned char>(levelSize, 0xFF);
				writeToFile(levelData, "compressed.bin");
				gzipInflate(levelData, level.blocks);
				level.blocks.erase(level.blocks.begin(), level.blocks.begin()+4);

				levelData.resize(0);

				scene = new SceneMain();
				world = scene->world;
				scene->addTo(getGame());

				break;
			}
			case 0x06: //Setblock
			{
				short x = s.readShort();
				short y = s.readShort();
				short z = s.readShort();
				char block = s.readByte();
				world->setBlock(x, y, z, block);
				break;
			}
			case 0x07: //Spawn
			{
				char playerId = s.readByte();
				string playerName = s.readString();
				float x = s.readShort()/32.0f;
				float y = s.readShort()/32.0f;
				float z = s.readShort()/32.0f;
				char yaw = s.readByte();
				char pitch = s.readByte();

				cout<<"Spawn "<<int(playerId)<<" "<<playerName<<" "<<x<<" "<<y<<" "<<z<<endl;
				scene->spawnPlayer(playerId);
				PlayerBase* p = scene->getPlayer(playerId);
				if(p != nullptr)
				{
					p->pos = vec3f(x, y-0.9, z);
					p->pitch = pitch*360.0f/256;
					p->yaw = yaw*360.0f/256;
				}

				break;
			}
			case 0x08: //Player teleport
			{
				char playerId = s.readByte();
				float x = s.readShort()/32.0f;
				float y = s.readShort()/32.0f;
				float z = s.readShort()/32.0f;
				char yaw = s.readByte();
				char pitch = s.readByte();

				cout<<"Teleport "<<int(playerId)<<" "<<x<<" "<<y<<" "<<z<<endl;
				PlayerBase* p = scene->getPlayer(playerId);
				if(p != nullptr)
				{
					p->pos = vec3f(x, y-0.9, z);
					p->pitch = pitch*360.0f/256;
					p->yaw = yaw*360.0f/256;
				}
				break;
			}
			case 0x09: //Position+Orientation update
			{
				char playerId = s.readByte();
				float x = s.readByte()/32.0f;
				float y = s.readByte()/32.0f;
				float z = s.readByte()/32.0f;
				char yaw = s.readByte();
				char pitch = s.readByte();
				PlayerBase* p = scene->getPlayer(playerId);
				if(p != nullptr)
				{
					p->pos += vec3f(x, y, z);
					p->pitch += pitch*360.0f/256;
					p->yaw += yaw*360.0f/256;
				}

				break;
			}
			case 0x0A: //Position update
			{
				char playerId = s.readByte();
				float x = s.readByte()/32.0f;
				float y = s.readByte()/32.0f;
				float z = s.readByte()/32.0f;
				PlayerBase* p = scene->getPlayer(playerId);
				if(p != nullptr)
					p->pos = vec3f(x, y, z);

				break;
			}
			case 0x0B: //Orientation update
			{
				char playerId = s.readByte();
				char yaw = s.readByte();
				char pitch = s.readByte();
				PlayerBase* p = scene->getPlayer(playerId);
				if(p != nullptr)
				{
					p->pitch = pitch*360.0f/256;
					p->yaw = yaw*360.0f/256;
				}

				break;
			}
			case 0x0C: //Despawn player
			{
				char playerId = s.readByte();
				scene->despawnPlayer(playerId);
				break;
			}
			case 0x0D: //Message
			{
				char playerId = s.readByte();
				string message = s.readString();
				cout<<"Chat "<<message<<endl;
				break;
			}
			case 0x0E: //Disconnect
			{
				string message = s.readString();
				cout<<"Disconnected: "<<message<<endl;
				exit(1);
				break;
			}
			case 0x0F: //Update user type
			{
				char userType = s.readByte();
				break;
			}
			default:
			{
				cout<<"Unknown packet: "<<int(packetId)<<endl;
				exit(1);
			}
		}
	}
}

void NetworkManager::sendSetBlock(int x, int y, int z, bool create, unsigned char block)
{
	s.writeByte(0x05);
	s.writeShort(x);
	s.writeShort(y);
	s.writeShort(z);
	s.writeByte(create);
	s.writeByte(block);
}

void NetworkManager::sendPosition(float x, float y, float z, float yaw, float pitch)
{
	y += 0.9;
	s.writeByte(0x08);
	s.writeByte(-1);
	s.writeShort(x*32);
	s.writeShort(y*32);
	s.writeShort(z*32);

	//Yaw goes from 0 to 360
	yaw *= 256/360.0f;

	//Pitch goes from -90 to 90 but asdfasdf I dont understand how this is supposed to work!?
	pitch *= 256/360.0f;

	s.writeByte(yaw);
	s.writeByte(pitch);
}
