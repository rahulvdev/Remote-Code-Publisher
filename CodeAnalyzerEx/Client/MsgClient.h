#pragma once
#pragma once
/////////////////////////////////////////////////////////////////////////
// MsgClient.h - Demonstrates simple one-way HTTP style messaging	   //
//                 and file transfer                                   //
//                                                                     //
// Rahul Vijaydev, CSE687 - Object Oriented Design, Spring 2016        //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
* This package implements a Client that sends and receives HTTP style messages and
* files from the server and simply displays the messages
* and stores files.
*/
/*
* Public Interface(MsgClient):
* using EndPoint = std::string;
* void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
* void startListener(int);
* HttpMessage makeCustomMessage(size_t n, const std::string& body, const EndPoint& ep, std::string type, std::string category);
* void openOnBrowser(std::string fpath);
* std::stack<std::string> split(const std::string s, char delim);
* std::vector<std::string> splitVect(const std::string & s, char delim);
* std::string getCategory(int category);
* void sendFilesToServer(std::string category, std::string toAddr, std::vector<std::string> files);
* std::string deleteFiles(int Category);
* std::string publishFiles(int Category);
* std::string listFiles(int Category);
* std::string downloadFiles(int Category, std::string fileName);
* std::string uploadFiles(std::string files, int Category);
*
*
*
* Public Interface(Client Handler)
* ClientHandler(BlockingQueue<HttpMessage>& msgQ, std::string storagePath) : msgQ_(msgQ), repoPath(storagePath) {}
* void operator()(Socket socket);
*
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
* - pull the receiving code into a Receiver class
* - Receiver should own a BlockingQueue, exposed through a
*   public method:
*     HttpMessage msg = Receiver.GetMessage()
* - You will start the Receiver instance like this:
*     Receiver rcvr("localhost:8080");
*     ClientHandler ch;
*     rcvr.start(ch);
*/
#include "../HttpMessage/HttpMessage.h"
#include "Sockets.h"
#include "FileSystem.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include <stack>
#include<Windows.h>
#include <windows.h>
#include <ShellAPI.h>

using Show = Logging::StaticLogger<1>;
using namespace Utilities;
using namespace Async;

/////////////////////////////////////////////////////////////////////
// MsgClient class
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
class MsgClient
{
public:
	MsgClient(int port, std::string storagePath) :portNumber(port), repoPath(storagePath) {

		std::cout << "\n\n";
		std::cout << "----------------------------------------------------------------------------------------------------------\n";
		std::cout << "REQUIREMENT 9(Proj 4):Shall include an automated unit test suite that demonstrates you meet all the requirements of this project4 including the transmission of files. \n";
		std::cout << "----------------------------------------------------------------------------------------------------------\n";
		std::string fpath = "..\\..\\ClientRepository\\SourceFiles\\Category1\\";
		std::vector<std::string> files = FileSystem::Directory::getFiles(fpath, "*.*");
		std::string fileStr;
		for (std::string file : files)
		{
			fileStr = fpath + file + "," + fileStr;
			//fileStr =   file + "," + fileStr;
		}
		fileStr.pop_back();
		uploadFiles(fileStr, 1);
		::Sleep(1000);
		publishFiles(1);
		::Sleep(2000);
		listFiles(1);
		::Sleep(1000);
		downloadFiles(1, "Tokenizer.h.htm");
		::Sleep(1000);
		deleteFiles(1);
	
	};
	using EndPoint = std::string;
	void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
	void startListener(int);
	HttpMessage makeCustomMessage(size_t n, const std::string& body, const EndPoint& ep, std::string type, std::string category);
	void openOnBrowser(std::string fpath);
	std::stack<std::string> split(const std::string s, char delim);
	std::vector<std::string> splitVect(const std::string & s, char delim);
	std::string getCategory(int category);
	void sendFilesToServer(std::string category, std::string toAddr, std::vector<std::string> files);
	std::string deleteFiles(int Category);
	std::string publishFiles(int Category);
	std::string listFiles(int Category);
	std::string downloadFiles(int Category, std::string fileName);
	std::string uploadFiles(std::string files, int Category);


private:
	HttpMessage makeMessage(size_t n, const std::string& msgBody, const EndPoint& ep);
	std::string repoPath;
	int portNumber;
	std::string serverAddress = "localhost:8081";
	void sendMessage(HttpMessage& msg, Socket& socket);
	bool sendFile(const std::string& filename, Socket& socket, std::string toAddress, std::string category);
	HttpMessage beginListener();
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
	ClientHandler(BlockingQueue<HttpMessage>& msgQ, std::string storagePath) : msgQ_(msgQ), repoPath(storagePath) {}
	void operator()(Socket socket);
	std::string repoPath;
private:
	bool connectionClosed_;
	HttpMessage readMessage(Socket& socket);
	bool readFile(const std::string& filename, size_t fileSize, Socket& socket);
	BlockingQueue<HttpMessage>& msgQ_;
};
