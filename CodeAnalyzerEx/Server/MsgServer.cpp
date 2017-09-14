/////////////////////////////////////////////////////////////////////////
// MsgServer.cpp - Demonstrates simple one-way HTTP messaging          //
//                                                                     //
// Rahul Vijaydev CSE687 - Object Oriented Design, Spring 2016          //
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
* - pull the sending parts into a new Sender class
* - You should create a Sender like this:
*     Sender sndr(endPoint);  // sender's EndPoint
*     sndr.PostMessage(msg);
*   HttpMessage msg has the sending adddress, e.g., localhost:8080.
*/
#pragma warning( disable : 4267)
#include "../HttpMessage/HttpMessage.h"
#include "Sockets.h"
#include "FileSystem.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include <thread>
#include "MsgServer.h"

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

HttpMessage MsgServer::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:8081";  // ToDo: make this a member of the sender
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


//----< send message using socket >----------------------------------

void MsgServer::sendMessage(HttpMessage& msg, Socket& socket)
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

bool MsgServer::sendFile(const std::string& filename, Socket& socket, std::string addr, std::string type, std::string category)
{
	std::cout << "\n\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 8(Proj 4):The communication system shall also support sending and receiving streams of bytes6.Streams will be established with an initial exchange of messages.\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	FileSystem::FileInfo fi(filename);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);
	FileSystem::File file(filename);

	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;

	std::string fileNameOnly = split(filename);
	////////get from address from message attribute//////////
	HttpMessage msg = makeMessage(1, "", addr);
	msg.addAttribute(HttpMessage::Attribute("FileName", fileNameOnly));
	msg.addAttribute(HttpMessage::Attribute("Category", category));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
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


std::string MsgServer::split(std::string fileName) {
	std::stringstream ss;
	ss.str(fileName);
	char delim = '\\';
	std::string item;
	std::stack<std::string> addrStack;
	while (std::getline(ss, item, delim)) {
		addrStack.push(item);
	}
	return addrStack.top();
}

void MsgServer::startListener(int port)
{
	/////////////Repository path for reference///////
	::SetConsoleTitle(L"HttpMessage Server - Runs Forever");

	Show::attach(&std::cout);
	Show::start();
	Show::title("\n  HttpMessage Server started");

	BlockingQueue<HttpMessage> msgQ;

	try
	{
		SocketSystem ss;
		SocketListener sl(port, Socket::IP6);
		ClientHandler cp(msgQ,repositoryPath);
		sl.start(cp);
		/*
		* Since this is a server the loop below never terminates.
		* We could easily change that by sending a distinguished
		* message for shutdown.
		*/
		while (true)
		{
			HttpMessage msg = msgQ.deQ();
			//Process message(Parse and identify type of message)
			processMessage(msg);
			Show::write("\n\n  server recvd message with body contents:\n" + msg.bodyString());
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}

////////////////////////////////////////Custom methods///////////////////////////////////////////////

//Utility method that used for framing and sending message back to the client
void MsgServer::sendMessageToClient(std::string clientAddr, std::string msgBody, std::string typeOfMsg)
{
	SocketSystem ss;
	SocketConnecter si;
	std::string portStr = getclientPortNum(clientAddr);
	int portNum = std::stoi(portStr);
	while (!si.connect("localhost", portNum))
	{
		Show::write("\n client waiting to connect");
		::Sleep(100);
	}
	HttpMessage msg;
	msg = makeMessage(1, msgBody, clientAddr);
	msg.addAttribute(HttpMessage::attribute("Type", typeOfMsg));
	sendMessage(msg, si);
}


//Processes message based on the type of the message
//Action take is dependant on the message type
void MsgServer::processMessage(HttpMessage& msg){
	try
	{
		std::string typeOfMsg = msg.findValue("Type");
		std::string category = msg.findValue("Category");
		std::string clientAddr = msg.findValue("fromAddr");
		std::string fullPathInRepo = repositoryPath + "PublishedFiles\\" + category + "\\";
		if (typeOfMsg == "Download") {
			downloadFiles(fullPathInRepo, clientAddr, typeOfMsg, category);
		}

		else if (typeOfMsg == "ListFiles") {
			getListOfAllFilesOfCategory(fullPathInRepo, clientAddr);
		}
		else if (typeOfMsg == "Delete") {
			deleteFilesOfCategory(fullPathInRepo, category, clientAddr);
		}
		else if (typeOfMsg == "Upload") {
			std::string msgBody = "Upload Complete";
			sendMessageToClient(clientAddr, msgBody, typeOfMsg);
		}
		else if (typeOfMsg == "Publish") {
			char * array[8];
			std::string path = "..\\..\\Repository\\SourceFiles\\" + category;
			std::string x[] = { "messi",path,"*.h","*.cpp","*.cs","/m","/f","/r" };
			
			for (int i = 0; i < 8; i++) {
				const char* xx = x[i].c_str();
				array[i] = _strdup(xx);
			}

			CodeAnalysis::CodeAnalysisExecutive exec;
			exec.ProcessCommandLine(7, array);
			exec.startPublish(category);
			std::string msgBody = "Publish Complete";
			sendMessageToClient(clientAddr,msgBody, typeOfMsg);
		}
	}
	catch (std::exception& exc)
	{
	Show::write("\n  Exeception caught: ");
	std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
	Show::write(exMsg);
	}
	
}

//Method for dowloading files of a particular category from the server
void MsgServer::downloadFiles(std::string fullPath,std::string clientAddr,std::string type,std::string category){
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		/////////////////////Create socket using port number obtained from clientAddr/////////////////

		std::string portStr=getclientPortNum(clientAddr);
		int portNum = std::stoi(portStr);
		while (!si.connect("localhost", portNum))
		{
			Show::write("\n client waiting to connect");
			::Sleep(100);
		}

		std::vector<std::string> files = FileSystem::Directory::getFiles(fullPath, "*.htm");
		for (size_t i = 0; i < files.size(); ++i)
		{
			Show::write("\n\n  sending file " + files[i]);
			std::string filPathForDownload = fullPath + files[i];
			sendFile(filPathForDownload, si,clientAddr,type,category);
		}

		HttpMessage terminateMsg;
		terminateMsg = makeMessage(1, "Terminating download", clientAddr);
		terminateMsg.addAttribute(HttpMessage::Attribute("Type", type));
		terminateMsg.addAttribute(HttpMessage::Attribute("Status", "complete"));
		sendMessage(terminateMsg, si);

	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}

//Utility methhod that returns the port number
std::string MsgServer::getclientPortNum(std::string addr) {
	std::stringstream ss;
	ss.str(addr);
	char delim = ':';
	std::string item;
	std::stack<std::string> addrStack;
	while (std::getline(ss,item,delim)) {
		addrStack.push(item);
	}
	return addrStack.top();
}

//Fetches list of all files of a particular category in the repository
void MsgServer::getListOfAllFilesOfCategory(std::string fullPathInRepo,std::string clientAddr) {
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		std::string portStr = getclientPortNum(clientAddr);
		int portNum = std::stoi(portStr);
		while (!si.connect("localhost", portNum))
		{
			Show::write("\n client waiting to connect");
			::Sleep(100);
		}
		HttpMessage msg;
		std::vector<std::string> files = FileSystem::Directory::getFiles(fullPathInRepo, "*.*");

		std::string msgBody = "List of files in a category sought from client";
		msg = makeMessage(1, msgBody, clientAddr);
		msg.addAttribute(HttpMessage::attribute("Type", "ListFiles"));
		std::string fileCollection=buildFileList(files);
		msg.addAttribute(HttpMessage::attribute("FilesList",fileCollection));
		sendMessage(msg, si);
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}

std::string MsgServer::buildFileList(std::vector<std::string> fileList) {
	std::string listOfFiles;
	size_t vectorSize = fileList.size();
	if (vectorSize > 0) {
		for (size_t i = 0; i < vectorSize; i++) {
			if (i != vectorSize - 1) {
				listOfFiles = listOfFiles + fileList[i] + ",";
			}
			else {
				listOfFiles = listOfFiles + fileList[i];
			}
		}
	}
	else {
		listOfFiles = "No Files";
	}
	return listOfFiles;
}

void MsgServer::deleteFilesOfCategory(std::string filePath, std::string category,std::string addr) {

	////////////////////////////////////////Provide actual filepath/////////////////////////////////////
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		bool fdeleted=true;
		std::string portStr = getclientPortNum(addr);
		int portNum = std::stoi(portStr);
		while (!si.connect("localhost", portNum))	
		{
			Show::write("\n client waiting to connect");
			::Sleep(100);
		}

		HttpMessage msg;
		std::vector<std::string> files = FileSystem::Directory::getFiles(filePath, "*.*");
		for (std::string fileName : files) {
			std::string pathToFile = filePath + fileName;
			if (remove(pathToFile.c_str()))
				fdeleted = false;
		}
		if (fdeleted)
		{
			msg = makeMessage(1, "Files Deleted", addr);
			msg.addAttribute(HttpMessage::attribute("Category", category));
			msg.addAttribute(HttpMessage::attribute("Type", "Delete"));
			msg.addAttribute(HttpMessage::attribute("Files Deleted", "true"));
			sendMessage(msg, si);
		}
		else {
			msg = makeMessage(1, "Files Not Deleted", addr);
			msg.addAttribute(HttpMessage::attribute("Category", category));
			msg.addAttribute(HttpMessage::attribute("Type", "Delete"));
			msg.addAttribute(HttpMessage::attribute("Files Deleted", "false"));
			sendMessage(msg, si);
		}
		
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}



////////////////////////////////////////Custom methods///////////////////////////////////////////////

//----< this defines the behavior of the client >--------------------

void MsgServer::execute(const size_t TimeBetweenMessages, const size_t NumMessages)
{
	ClientCounter counter;
	size_t myCount = counter.count();
	std::string myCountString = Utilities::Converter<size_t>::toString(myCount);
	Show::attach(&std::cout);
	Show::start();
	Show::title(
		"Starting HttpMessage Server" + myCountString +" on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id())
	);
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		while (!si.connect("localhost", 8080))
		{
			Show::write("\n Server waiting to connect");
			::Sleep(100);
		}
		HttpMessage msg;
		for (size_t i = 0; i < NumMessages; ++i)
		{
			std::string msgBody =
				"<msg>Message #" + Converter<size_t>::toString(i + 1) + " from Server #" + myCountString + "</msg>";
			msg = makeMessage(1, msgBody, "localhost:8080");
			sendMessage(msg, si);
			Show::write("\n\n  Server" + myCountString + " sent\n" + msg.toIndentedString());
			::Sleep(TimeBetweenMessages);
		}
		std::vector<std::string> files = FileSystem::Directory::getFiles("../TestFiles", "*.cpp");
		for (size_t i = 0; i < files.size(); ++i)
		{
			Show::write("\n\n  sending file " + files[i]);
			sendFile(files[i], si,"localhost:8080","type","Category1");
		}
		// shut down server's client handler
		msg = makeMessage(1, "quit", "localhost:8080");
		sendMessage(msg, si);
		Show::write("\n\n  Server" + myCountString + " sent\n" + msg.toIndentedString());
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


//----< entry point - runs two clients each on its own thread >------

int main()
{
	CodeAnalysisExecutive cEx;
	::SetConsoleTitle(L"Server Running on Thread");

	////////////////////Intitlaize repository path in MsgServer object//////////////////////
	MsgServer s1("..\\..\\Repository\\");

	std::thread tserver(
		[&]() { s1.startListener(8081); }
	);
	tserver.detach();
	getchar();
}