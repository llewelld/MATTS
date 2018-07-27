/*$T MATTS/ServerSocket.cpp GC 1.140 07/01/09 21:12:11 */
#include <cstdlib>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sstream>
#include "serverSocket.h"
#include "Socket.h"

#using < System.dll > #using < system.windows.forms.dll > #using < mscorlib.dll > using namespace std;
using namespace System;
using namespace System::Net:: Sockets;
using namespace System:: Net;
using namespace System:: Threading;
using namespace System:: IO;

/* Socket notification commands. */
int port = 1984;

/*
 * Socket* _listen;
 * Socket* _socket;
 */
enum NotifyCommand
{

	/* Listen, */
	Connected,
	Disconnected,
	ReceivedData
}

/*/
 =======================================================================================================================
 *  <summary> Listens and connects to socket connection request. Receives data from socket and sends back an
 *  acknowledgement. Creates worker thread so UI is not blocked. </summary>
 =======================================================================================================================
 */
ServerSocket () {

	/* InetAddress* pBindAddr */
	int port;
	Sockets::Socket ^ socket;
	Sockets::Socket ^ listiner;
	return Connected;
}

/*/
 =======================================================================================================================
 *  <summary> Shutdown the socket. </summary>
 =======================================================================================================================
 */
void Stop () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void Start (int portNumber) {

	/* save port number */
	port = portNumber;

	/* make sure stop if currently connected */
	Stop ();

	/*
	 * create a new thread that handle client connection ;
	 * ThreadPool::QueueUserWorkItem(gcnew WaitCallback(Listiner));
	 * ThreadPool::QueueUserWorkItem(gcnew WaitCallback(Listen));
	 */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void bind (int port) {
	port = 1984;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void Restart () {
	Stop ();
	Start (port);
}

/*
 =======================================================================================================================
 *  void Listen(Object state) ;
 *  { ;
 *  establish connection with client ;
 *  establish connection with client ;
 *  IPHostEntry hostInfo = Dns::GetHostByName("www.contoso.com");
 *  System::Net::IPHostEntry ipHost = Dns::Resolve(Dns::GetHostName());
 *  IPAddress ipAddress = ipHost.AddressList[0];
 *  IPEndPoint endPoint = gcnew IPEndPoint(ipAddress, port);
 *  Sockets::Socket ^listiner;
 *  ;
 *  try ;
 *  { ;
 *  listen for client connection ;
 *  listiner = gcnew Sockets::Socket(AddressFamily::InterNetwork, SocketType::Stream, ProtocolType::Tcp);
 *  listiner->Blocking = true;
 *  listiner->Bind(endPoint);
 *  listiner->Listen(0);
 *  ;
 *  listening ;
 *  RaiseNotifyEvent(NotifyCommand.listiner, endPoint.Address.ToString());
 *  ;
 *  block here until establish a connection ;
 *  socket = listiner.Accept();
 *  ;
 *  we connected with a client, shutdown the listen socket ;
 *  so we won't connect with another client ;
 *  listiner.Close();
 *  ;
 *  connected ;
 *  RaiseNotifyEvent(NotifyCommand.Connected,socket.RemoteEndPoint.ToString());
 *  ;
 *  sit in this loop until connection is broken ;
 *  handle client commands and send back response ;
 *  bool bListen = true;
 *  while (bListen) ;
 *  { ;
 *  reset everything for the next read message ;
 *  bool bReadMessage = true;
 *  int bytesRead = 0;
 *  int totalBytes = 0;
 *  ;
 *  hold incoming data ;
 *  byte[] buf = new byte[1024];
 *  MemoryStream streamRead = new MemoryStream();
 *  ;
 *  while (bReadMessage) ;
 *  { ;
 *  loop that reads incoming message ;
 *  buf is temp holder and the MemoryStream ;
 *  contains all of the bits ;
 *  bytesRead = socket.Receive(buf);
 *  if (bytesRead > 0) ;
 *  { ;
 *  streamRead.Write(buf, 0, bytesRead);
 *  bReadMessage = !Network.CheckForTerminator(streamRead.ToArray());
 *  totalBytes += bytesRead;
 *  } ;
 *  else ;
 *  { ;
 *  client disconnected ;
 *  Restart();
 *  throw (new Exception("Client disconnected."));
 *  } ;
 *  } ;
 *  done reading incoming message, now process the command ;
 *  ProcessCommand(streamRead);
 *  streamRead.Close();
 *  } ;
 *  } ;
 *  catch (Exception ex) ;
 *  { ;
 *  Debug.WriteLine(ex.Message);
 *  } ;
 *  }
 =======================================================================================================================
 */
void RunSever () {
	Sockets::Socket ^ clientSoc;
	try
	{

		/* Set the TcpListener on port 1984. */
		Int32 port;
		port = 1984;

		/*
		 * IPAddress^ localAddr = IPAddress::Parse( "127.0.0.1" );
		 */
		TcpListener ^ server = gcnew TcpListener (port);

		/*
		 * TcpListener^ server = gcnew TcpListener( localAddr,port );
		 * Start listening for client requests.
		 */
		server->Start ();

		/* Buffer for reading data */
		array<Byte> ^ bytes = gcnew array<Byte> (256);
		String ^ data = nullptr;

		/* Enter the listening loop. */
		while (true) {
			Int32 i;

			/*
			 * MessageBox.Show("Waiting for a connection... ");
			 * Perform a blocking call to accept requests.
			 */
			clientSoc = server->AcceptSocket ();	/* You could also user server.AcceptSocket() here. */
			TcpClient ^ client = server->AcceptTcpClient ();

			/*
			 * MessageBox.Show("Connected!" );
			 */
			data = nullptr;

			/*
			 * Get a stream Object* for reading and writing ;
			 * NetworkStream^ stream=clientSoc->
			 */
			NetworkStream ^ stream = client->GetStream ();

			/* Loop to receive all the data sent by the client. */
			while (i = stream->Read (bytes, 0, bytes->Length)) {

				/* Translate data bytes to a ASCII String*. */
				data = Text::Encoding::ASCII->GetString (bytes, 0, i);

				/*
				 * MessageBox.Show( "Received: {0}", data );
				 * Process the data sent by the client.
				 */
				data = data->ToUpper ();
				array<Byte> ^ msg = Text::Encoding::ASCII->GetBytes (data);

				/* Send back a response. */
				stream->Write (msg, 0, msg->Length);

				/*
				 * MessageBox.Show( "Sent: {0}", data );
				 */
			}

			/*
			 * Shutdown and end connection ;
			 * clientSoc->Close();
			 */
			client->Close ();
		}
	}

	catch (SocketException ^ e) {

		/*
		 * MessageBox.Show( "SocketException: {0}", e );
		 */
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
CServerSocketApp::CServerSocketApp () {

	/*
	 * TODO: add construction code here, ;
	 * Place all significant initialization in InitInstance
	 */
}

/* The one and only CServerSocketApp object */
CServerSocketApp theApp;
