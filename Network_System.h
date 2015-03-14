#pragma once
//-----------------------------------------------------------------
#include <SFML/Network.hpp>
//-----------------------------------------------------------------
class Network_System
{
	//-------------------------------------------------------------
public:
	//---------------------------------------------------------
	///TCP
	int State;
	///Send new track info?
	bool bNewTrack;

	//Interface Changes
	bool bInterfaceUpdate;
	bool bSpectrum_Reset;
	//---------------------------------------------------------
	//constructors
	Network_System(void);
	~Network_System(void);
	//---------------------------------------------------------
	void StopServer(void);
	void Update(void);
	//-------------------------------------------------------------
private:
	//-------------------------------------------------------------
	sf::TcpListener listener;
	sf::TcpSocket socket;
	sf::Packet Packet_Send;
	sf::Packet Packet_Recieve;
	sf::Socket::Status Socket_Status;

	unsigned short ServerPort;
	//-------------------------------------------------------------
	void SendRecieveData(void);
	//-------------------------------------------------------------
};
