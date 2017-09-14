/////////////////////////////////////////////////////////////////////////
// MsgClient.cpp - Demonstrates simple one-way HTTP messaging          //
//                                                                     //
// Rahul Vijaydev, CSE687 - Object Oriented Design, Spring 2016        //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
* This package implements a client that sends HTTP style messages and
* files to a server that simply displays messages and stores files.
*
* It's purpose is to provide a very simple illustration of how to use
* the Socket Package provided for Project #4.
*/
/*
* Required Files:
*   MsgClient.cpp, MsgServer.cpp
*   HttpMessage.h, HttpMessage.cpp
*   Cpp11-BlockingQueue.h
*   Sockets.h, Sockets.cpp
*   FileSystem.h, FileSystem.cpp
*   Logger.h, Logger.cpp
*   Utilities.h, Utilities.cpp
*/
/*
* ToDo:
* - pull the sending parts into a new Sender class
* - You should create a Sender like this:
*     Sender sndr(endPoint);  // sender's EndPoint
*     sndr.PostMessage(msg);
*   HttpMessage msg has the sending adddress, e.g., localhost:8080.
*/

#include "../HttpMessage/HttpMessage.h"
#include "Sockets.h"
#include "FileSystem.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include <thread>
#include "MsgClient.h"

using Show = Logging::StaticLogger<1>;
using namespace Utilities;
using Utils = StringHelper;

/////////////////////////////////////////////////////////////////////
// ClientCounter creates a sequential number for each client
//
class ClientCounter
{
public:
	ClientCounter() { ++clientCount; }
	size_t count() { return clientCount; }
private:
	static size_t clientCount;
};


size_t ClientCounter::clientCount = 0;

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
		std::string fileDirectory = category + "\\" + filename;
		if (filename != "")
		{
			size_t contentSize;
			std::string sizeString = msg.findValue("content-length");
			if (sizeString != "")
				contentSize = Converter<size_t>::toValue(sizeString);
			else
				return msg;

			readFile(fileDirectory, contentSize, socket);
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
bool ClientHandler::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{

	////////////Split file name////////////////////
	std::string fqname = repoPath + "PublishedFiles\\" + filename;
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
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


//Utility method used to frame a message 
HttpMessage MsgClient::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:8080";  // ToDo: make this a member of the sender
											 // given to its constructor.
	switch (n)
	{
	case 1:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("POST", "Message"));
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));

		msg.addBody(body);
		if (body.size() > 0)
		{
			attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			msg.addAttribute(attrib);
		}
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}

//Utility method for making custom message
HttpMessage MsgClient::makeCustomMessage(size_t n, const std::string& body, const EndPoint& ep,std::string type,std::string category)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:8080";  // ToDo: make this a member of the sender
											 // given to its constructor.
	switch (n)
	{
	case 1:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("POST", "Message"));
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		msg.addAttribute(HttpMessage::attribute("Type", type));
		msg.addAttribute(HttpMessage::attribute("Category", category));
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));

		msg.addBody(body);
		if (body.size() > 0)
		{
			attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			msg.addAttribute(attrib);
		}
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}


//----< send message using socket >----------------------------------

void MsgClient::sendMessage(HttpMessage& msg, Socket& socket)
{
	std::cout << "\n\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 6(Proj 4):Shall provide a message - passing communication system, based on Sockets, used to access the Repository's functionality from another process or machine. \n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "\n\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 7(Proj 4):The communication system shall provide support for passing HTTP style messages using either synchronous request/response or asynchronous one-way messaging. \n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::string msgString = msg.toString();
	socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}


//----< send file using socket >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool MsgClient::sendFile(const std::string& filename, Socket& socket, std::string toAddress, std::string category)
{
	std::cout << "\n\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 8(Proj 4):The communication system shall also support sending and receiving streams of bytes6.Streams will be established with an initial exchange of messages.\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::string fname = filename;
	FileSystem::FileInfo fi(filename);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);
	FileSystem::File file(filename);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;

	HttpMessage msg = makeMessage(1, "", toAddress);
	msg.addAttribute(HttpMessage::Attribute("FileName", split(fname, '\\').top()));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(msg, socket);
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	while (true)
	{
		FileSystem::Block blk = file.getBlock(BlockSize);
		if (blk.size() == 0)
			break;
		for (size_t i = 0; i < blk.size(); ++i)
			buffer[i] = blk[i];
		socket.send(blk.size(), buffer);
		if (!file.isGood())
			break;
	}
	file.close();
	return true;
}


//Starts listener on client
void MsgClient::startListener(int port)
{
	::SetConsoleTitle(L"HttpMessage Client - Runs Forever");

	Show::attach(&std::cout);
	Show::start();
	Show::title("\n  HttpMessage Client started");

	BlockingQueue<HttpMessage> msgQ;
	std::string type;
	try
	{
		SocketSystem ss;
		SocketListener sl(port, Socket::IP6);
		ClientHandler cp(msgQ,repoPath);
		sl.start(cp);
		while (true)
		{
			HttpMessage msg = msgQ.deQ();
			type = msg.findValue("Type");
			if ((type == "Publish") || (type == "Download") || (type == "Delete") || (type == "ListFiles") || (type == "Upload"))
			{
				break;
			}
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}


//----< this defines the behavior of the client >--------------------
void MsgClient::execute(const size_t TimeBetweenMessages, const size_t NumMessages)
{
	// send NumMessages messages
	ClientCounter counter;
	size_t myCount = counter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	Show::attach(&std::cout);
	Show::start();
	Show::title(
		"Starting HttpMessage client" + myCountString +
		" on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id())
	);
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8081))
		{
			Show::write("\n client waiting to connect");
			::Sleep(100);
		}
		::Sleep(5000);
		Show::write("\n");
		Show::write("\n  All done folks");
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}


////////////////////Custom code//////////////////////////
std::vector<std::string> MsgClient::splitVect(const std::string & s, char delim)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

//utility method that splits the given string into substrings based on the delimiter specified
std::stack<std::string> MsgClient::split(const std::string s, char delim)
{
	std::stack<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		if (item.length() > 0) {
			elems.push(item);
		}
	}
	return elems;
}

//Start Listener on client
HttpMessage MsgClient::beginListener()
{
	Async::BlockingQueue<HttpMessage> msgQ;
	HttpMessage msg;
	try
	{
		std::string type;
		SocketSystem ss;
		SocketListener sl(portNumber, Socket::IP6);
		ClientHandler cp(msgQ, repoPath);
		sl.start(cp);
		while (true)
		{
			msg = msgQ.deQ();
			type = msg.findValue("Type");
			if ((type == "Publish") || (type == "Download") || (type == "Delete") || (type == "ListFiles") || (type == "Upload"))
			{
				break;
			}
		}
		sl.close();
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}

	return msg;
}

std::string MsgClient::getCategory(int category)
{
	switch (category) {
	case 1:
		return "Category1";
		break;
	case 2:
		return "Category2";
		break;
	case 3:
		return "Category3";
		break;
	default:
		return "none";
		break;
	}
}

void MsgClient::sendFilesToServer(std::string category, std::string toAddr, std::vector<std::string> files)
{
	//std::string fpath = repoPath + "SourceFiles\\" + category + "\\";
	std::string toadd = split(toAddr, ':').top();
	SocketSystem ss;
	SocketConnecter si;
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	for (size_t i = 0; i < files.size(); ++i)
	{
		Show::write("\n\n  sending file " + files[i]);
		sendFile(files[i], si, toAddr, category);
	}
	HttpMessage terminateMsg;
	terminateMsg = makeMessage(1, "Terminating Upload", toAddr);
	terminateMsg.addAttribute(HttpMessage::Attribute("Type", "Upload"));
	terminateMsg.addAttribute(HttpMessage::Attribute("Status", "complete"));
	terminateMsg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(terminateMsg, si);
	si.close();
}


std::string MsgClient::uploadFiles(std::string files, int Category)
{
	std::cout << "\n\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 5(Proj 4):Client program that can upload files3, and view Repository contents, as described in the Purpose section, above\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "\n--------------------------------- Sending Upload Request to Server---------------------------------\n";
	std::vector <std::string> filesVector = splitVect(files, ',');
	std::string category = getCategory(Category);
	sendFilesToServer(category, serverAddress, filesVector);
	HttpMessage message = beginListener();
	std::cout << "\n--------------------------------Received Message From Server------------------------\n" << message.bodyString() << "\n-----------------------\n";
	return message.bodyString();
}

/*
Connects to the server and Send a delete request to it, and listen for a message from the server
*/
std::string MsgClient::deleteFiles(int Category)
{
	std::cout << "\n----------------------------- Sending Delete Request to Server-----------------------------------------\n";
	SocketSystem ss;
	SocketConnecter si;
	std::string toadd = split(serverAddress, ':').top();
	std::string category = getCategory(Category);
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, "Delete Files", serverAddress);
	msg.addAttribute(HttpMessage::Attribute("Type", "Delete"));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(msg, si);
	si.close();
	HttpMessage message = beginListener();
	std::cout << "\n-----------------------Received Message From Server-------------\n" << message.bodyString() << "\n----------------\n";
	return message.bodyString();
}
/*
Connects to the server and Send a publish request to it, and listen for a message from the server
*/
std::string MsgClient::publishFiles(int Category)
{
	std::cout << "\n---------------------------------------Sending Publish Request to Server -----------------------------------------\n";
	SocketSystem ss;
	SocketConnecter si;
	std::string toadd = split(serverAddress, ':').top();
	std::string category = getCategory(Category);
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, "Publish Files", serverAddress);
	msg.addAttribute(HttpMessage::Attribute("Type", "Publish"));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(msg, si);
	si.close();
	HttpMessage message = beginListener();
	std::cout << "\n-----------------------Received Message From Server-------------\n" << message.bodyString() << "\n----------------\n";
	return message.bodyString();
}
/*
Connects to the server and Send a List Files request to it, and listen for a message from the server
*/
std::string MsgClient::listFiles(int Category)
{
	std::cout << "\n\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 5(Pro 4):Client program that can upload files3, and view Repository contents, as described in the Purpose section, above\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	SocketSystem ss;
	SocketConnecter si;
	std::string toadd = split(serverAddress, ':').top();
	std::string category = getCategory(Category);
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, "List Files", serverAddress);
	msg.addAttribute(HttpMessage::Attribute("Type", "ListFiles"));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	sendMessage(msg, si);
	si.close();
	HttpMessage message = beginListener();
	std::cout << "\n-----------------------Received Message From Server-------------\n" << message.findValue("FilesList") << "\n----------------\n";
	//std::cout << "\n\n" << message.findValue("FilesList");
	return "success," + message.findValue("FilesList");
}
/*
Connects to the server and Send a Download request to it, and listen for a message from the server
*/
std::string MsgClient::downloadFiles(int Category, std::string fileName)
{
	std::cout << "\n*--------------------Sending Download Request from to Server---------------------\n";
	SocketSystem ss;
	SocketConnecter si;
	std::string toadd = split(serverAddress, ':').top();
	std::string category = getCategory(Category);
	while (!si.connect("localhost", std::stoi(toadd)))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg = makeMessage(1, "Download files", serverAddress);
	msg.addAttribute(HttpMessage::Attribute("Type", "Download"));
	msg.addAttribute(HttpMessage::attribute("Category", category));
	msg.addAttribute(HttpMessage::attribute("FileName", fileName));

	sendMessage(msg, si);
	si.close();
	HttpMessage message = beginListener();
	std::cout << "\n-----------------------Received Message From Server-------------\n" << message.bodyString() << "\n----------------\n";
	openOnBrowser(category + "\\" + fileName);
	return message.bodyString();
}


void MsgClient::openOnBrowser(std::string fpath)
{
	fpath = repoPath + "PublishedFiles\\" + fpath;
	std::string path = "file:///" + FileSystem::Path::getFullFileSpec(fpath);
	std::wstring spath = std::wstring(path.begin(), path.end());
	LPCWSTR swpath = spath.c_str();
	ShellExecute(NULL, L"open", swpath, NULL, NULL, SW_SHOWNORMAL);
	//system("PAUSE");

}

#ifdef MSGCLIENT
int main()
{

	::Sleep(10000);

	::SetConsoleTitle(L"Clients Running on Threads");

	//Respository path
	MsgClient c1(8080, "..\\..\\ClientRepository\\");
	c1.deleteFiles(1);

	//std::thread tClient(
	//	[&]() { c1.startListener(8080); } // Start listener on the client c1
	//);

	//
	//::Sleep(10000);
	//tClient.join();
	getchar();

	/*std::thread t1(
	[&]() { c1.execute(100, 20); } // 20 messages 100 millisec apart
	);*/

	/*MsgClient c2;
	std::thread t2(
	[&]() { c2.execute(120, 20); } // 20 messages 120 millisec apart
	);
	t1.join();
	t2.join();*/
}
#endif // DEBUG


//----< entry point - runs two clients each on its own thread >------

