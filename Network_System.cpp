//-----------------------------------------------------------------
#include "global.h"
//-----------------------------------------------------------------
///*****************************************************************
//Network_System - CONSTRUCTORS
///*****************************************************************
Network_System::Network_System(void)
{
	//-------------------------------------------------------------
	///Send new track info?
	bNewTrack = false;

	//Interface changes
	bInterfaceUpdate = true;
	bSpectrum_Reset = true;

	///Connection
	socket.setBlocking(false);
	listener.setBlocking(false);
	ServerPort = 50001;
	State = 0;
	//-------------------------------------------------------------
}
Network_System::~Network_System(void)
{
	//-------------------------------------------------------------
	//-------------------------------------------------------------
}
///*****************************************************************
//Network_System - StopServer
///*****************************************************************
void Network_System::StopServer(void)
{
	//-------------------------------------------------------------
	//Disconnect Network
	listener.close();
	socket.disconnect();
	//-------------------------------------------------------------
}
///*****************************************************************
//Network_System - Update
///*****************************************************************
void Network_System::Update(void)
{
	//-------------------------------------------------------------
	//idle
	if (State == 0)
	{
		//Do absolutley Nothing
	}
	//-------------------------------------------------------------
	//Wait for connection
	else if (State == 1)
	{
		// Listen to the given port for incoming connections
		if (listener.listen(ServerPort) != sf::Socket::Done)
		{
		}
		// Wait for a connection
		if (listener.accept(socket) != sf::Socket::Done)
		{
		}
		//Check for Connection and obtain IP from client
		if (socket.getRemoteAddress() != sf::IpAddress::None)
		{
			//Ip obtained, connected
			if (cBass.Stream_State != BASS_ACTIVE_STOPPED)
			{
			  bNewTrack = true;
			}
			bInterfaceUpdate = true;
			bSpectrum_Reset = true;
			State = 2;
		}
	}
	//-------------------------------------------------------------
	//Connection Establised
	else if (State == 2)
	{
		SendRecieveData();
	}
	//-------------------------------------------------------------
	//Disconnect
	else if (State == -1)
	{
		StopServer();
		State = 0;
	}
	//-------------------------------------------------------------
}
///*****************************************************************
///Network_System - SendRecieveData
///*****************************************************************
void Network_System::SendRecieveData(void)
{
	//-------------------------------------------------------------
	Packet_Send.clear();
	Packet_Recieve.clear();
	Socket_Status = socket.receive(Packet_Recieve);
	//-------------------------------------------------------------
	//Monitor connection
	if (Socket_Status == sf::Socket::Status::Disconnected)
	{
		State = -1;
	}
	else
	{
		//-------------------------------------------------------------
		//Recieve
		while (Packet_Recieve.getDataSize() > 0) //Always use latest packet
		{
			//null
			bool bTest = false;
			Packet_Recieve >> bTest;

			Packet_Recieve.clear();
			Socket_Status = socket.receive(Packet_Recieve);
		}

		//-------------------------------------------------------------
		//Send

		/////Update Rate (Oe2 logic loop target)
		//Packet_Send << float(updateTimerSpeed);

		//Interface changes
		Packet_Send << bInterfaceUpdate;
		if (bInterfaceUpdate)
		{
			//Spectrum Falloff speed
			Packet_Send << cBass.fSpectrum_Falloff_TendTo;
			//Spectrum Colours
			Packet_Send << int(gSpectrum_R);
			Packet_Send << int(gSpectrum_G);
			Packet_Send << int(gSpectrum_B);

			bInterfaceUpdate = false;
		}

		//Spectrum Reset
		Packet_Send << bSpectrum_Reset;
		bSpectrum_Reset = false;

		//Play state
		Packet_Send << int(cBass.Stream_State);
		if (cBass.Stream_State > 0) // 0 = stopped | 1 = playing | 3 = paused
		{
			///Play Duration
			Packet_Send << float(cBass.Current_Position);
			///FFT's
			for (int i = 0; i < MAX_FFT_USED; i++)
			{
				Packet_Send << cBass.FFT[i];
			}
			///Output Channels
			Packet_Send << cBass.Output_Level_L;
			Packet_Send << cBass.Output_Level_R;
		}
		///Send new track info?
		Packet_Send << bNewTrack;
		if (bNewTrack)
		{
			char Temp[128] = { 0 };
			strcpy_s(Temp, gPlaylist_Entry[gPlay_CurrentId].artist.c_str());
			Packet_Send << Temp;

			strcpy_s(Temp, gPlaylist_Entry[gPlay_CurrentId].title.c_str());
			if (gPlaylist_Entry[gPlay_CurrentId].title == "No Title")
			{
				strcpy_s(Temp, gPlaylist_Entry[gPlay_CurrentId].displayname.c_str());
			}
			Packet_Send << Temp;
			bNewTrack = false;
		}
		socket.send(Packet_Send);
	}
	//-------------------------------------------------------------
}
