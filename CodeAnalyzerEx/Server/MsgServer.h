#pragma once
#pragma once
/////////////////////////////////////////////////////////////////////////
// MsgServer.h - Demonstrates simple one-way HTTP style messaging    //
//                 and file transfer                                   //
//                                                                     //
// Rahul Vijaydev, CSE687 - Object Oriented Design, Spring 2016        //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
* This package implements a server that receives HTTP style messages and
* files from multiple concurrent clients and simply displays the messages
* and stores files.
*
* It's purpose is to provide a very simple illustration of how to use
* the Socket Package provided for Project #4.
*/
/*
Public Interface(MsgServer):
* MsgServer(std::string repoPath) :repositoryPath(repoPath) {}
* using EndPoint = std::string;
* void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
* void startListener(int);
* void processMessage(HttpMessage& msg);
* void downloadFiles(std::string fullPathInRepo, std::string clientAddr, std::string type, std::string category);
* std::string getclientPortNum(std::string address);
* void getListOfAllFilesOfCategory(std::string fullPathInRepo, std::string clientAddr);
* std::string buildFileList(std::vector<std::string> fileList);
* std::string split(std::string fileName);
* void deleteFilesOfCategory(std::string filePath, std::string category, std::string addr);
* void sendMessageToClient(std::string clientAddr, std::string msgBody, std::string typeOfMsg);
*
*
*
* Public Interface(ClientHandler):
* ClientHandler(BlockingQueue<HttpMessage>& msgQ,std::string repoPath) : msgQ_(msgQ),repositoryPath(repoPath) {}
* void operator()(Socket socket);
*
* Required Files:
*   MsgServer.cpp, MsgServer.cpp
*   HttpMessage.h, HttpMessage.cpp
*   Cpp11-BlockingQueue.h
*   Sockets.h, Sockets.cpp
*   FileSystem.h, FileSystem.cpp
*   Logger.h, Logger.cpp
*   Utilities.h, Utilities.cpp
*/
/*
* ToDo:
* - pull the receiving code into a Receiver class
* - Receiver should own a BlockingQueue, exposed through a
*   public method:
*     HttpMessage msg = Receiver.GetMessage()
* - You will start the Receiver instance like this:
*     Receiver rcvr("localhost:8080");
*     ClientHandler ch;
*     rcvr.start(ch);
*/
#include "../Analyzer/Executive.h"
#include "../HttpMessage/HttpMessage.h"
#include "Sockets.h"
#include "FileSystem.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include <stack>

using Show = Logging::StaticLogger<1>;
using namespace Utilities;
using namespace CodeAnalysis;
using namespace Async;

/////////////////////////////////////////////////////////////////////
// MsgServer class
// - was created as a class so more than one instance could be 
//   run on child thread
//
//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - To do that the first argument is 1, e.g., index for the type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
class MsgServer
{
public:
	MsgServer(std::string repoPath) :repositoryPath(repoPath) {}
	using EndPoint = std::string;
	void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
	void startListener(int);
	void processMessage(HttpMessage& msg);
	void downloadFiles(std::string fullPathInRepo, std::string clientAddr, std::string type, std::string category);
	std::string getclientPortNum(std::string address);
	void getListOfAllFilesOfCategory(std::string fullPathInRepo, std::string clientAddr);
	std::string buildFileList(std::vector<std::string> fileList);
	std::string split(std::string fileName);
	void deleteFilesOfCategory(std::string filePath, std::string category, std::string addr);
	void sendMessageToClient(std::string clientAddr, std::string msgBody, std::string typeOfMsg);

private:
	std::string repositoryPath;
	HttpMessage makeMessage(size_t n, const std::string& msgBody, const EndPoint& ep);
	void sendMessage(HttpMessage& msg, Socket& socket);
	bool sendFile(const std::string& fqname, Socket& socket, std::string addr, std::string type, std::string category);
};

/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You no longer need to be careful using data members of this class
//   because each client handler thread gets its own copy of this 
//   instance so you won't get unwanted sharing.
// - I changed the SocketListener semantics to pass
//   instances of this class by value for version 5.2.
// - that means that all ClientHandlers need copy semantics.
//
class ClientHandler
{
public:
	ClientHandler(BlockingQueue<HttpMessage>& msgQ,std::string repoPath) : msgQ_(msgQ),repositoryPath(repoPath) {}
	void operator()(Socket socket);
private:
	std::string repositoryPath;
	bool connectionClosed_;
	HttpMessage readMessage(Socket& socket);
	bool readFile(const std::string& filename, std::string category,size_t fileSize, Socket& socket);
	BlockingQueue<HttpMessage>& msgQ_;
};
//----< this defines processing to frame messages >------------------

HttpMessage ClientHandler::readMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;

	// read message attributes

	while (true)
	{
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1)
		{
			HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
			msg.addAttribute(attrib);
		}
		else
		{
			break;
		}
	}
	// If client is done, connection breaks and recvString returns empty string

	if (msg.attributes().size() == 0)
	{
		connectionClosed_ = true;
		return msg;
	}
	// read body if POST - all messages in this demo are POSTs

	if (msg.attributes()[0].first == "POST")
	{
		// is this a file message?

		std::string filename = msg.findValue("FileName");
		std::string category = msg.findValue("Category");
		if (filename != "")
		{
			size_t contentSize;
			std::string sizeString = msg.findValue("content-length");
			if (sizeString != "")
				contentSize = Converter<size_t>::toValue(sizeString);
			else
				return msg;

			readFile(filename, category,contentSize, socket);
		}

		if (filename != "")
		{
			// construct message body

			msg.removeAttribute("content-length");
			std::string bodyString = "<file>" + filename + "</file>";
			std::string sizeString = Converter<size_t>::toString(bodyString.size());
			msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
			msg.addBody(bodyString);
		}
		else
		{
			// read message body

			size_t numBytes = 0;
			size_t pos = msg.findAttribute("content-length");
			if (pos < msg.attributes().size())
			{
				numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
				Socket::byte* buffer = new Socket::byte[numBytes + 1];
				socket.recv(numBytes, buffer);
				buffer[numBytes] = '\0';
				std::string msgBody(buffer);
				msg.addBody(msgBody);
				delete[] buffer;
			}
		}
	}
	return msg;
}
//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readFile(const std::string& filename, std::string category,size_t fileSize, Socket& socket)
{
	std::string fqname = repositoryPath+"SourceFiles\\"+category+"\\" + filename ;
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
		/*
		* This error handling is incomplete.  The client will continue
		* to send bytes, but if the file can't be opened, then the server
		* doesn't gracefully collect and dump them as it should.  That's
		* an exercise left for students.
		*/
		Show::write("\n\n  can't open file " + fqname);
		return false;
	}

	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];

	size_t bytesToRead;
	while (true)
	{
		if (fileSize > BlockSize)
			bytesToRead = BlockSize;
		else
			bytesToRead = fileSize;

		socket.recv(bytesToRead, buffer);

		FileSystem::Block blk;
		for (size_t i = 0; i < bytesToRead; ++i)
			blk.push_back(buffer[i]);

		file.putBlock(blk);
		if (fileSize < BlockSize)
			break;
		fileSize -= BlockSize;
	}
	file.close();
	return true;
}
//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
{
	while (true)
	{
		HttpMessage msg = readMessage(socket);
		if (connectionClosed_ || msg.bodyString() == "quit")
		{
			Show::write("\n\n  clienthandler thread is terminating");
			break;
		}
		msgQ_.enQ(msg);
	}
}
