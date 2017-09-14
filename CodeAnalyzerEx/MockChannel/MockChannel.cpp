
/////////////////////////////////////////////////////////////////////////////
// MockChannel.cpp - Demo for CSE687 Project #4, Spring 2015               //
// - build as DLL to show how C++\CLI client can use native code channel   //
// - MockChannel reads from sendQ and writes to recvQ                      //
//                                                                         //
// Rahul Vijaydev, CSE687 - Object Oriented Design, Spring 2015 //
/////////////////////////////////////////////////////////////////////////////

#define IN_DLL
#include "MockChannel.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Client/MsgClient.h"
#include <string>
#include <thread>
#include <iostream>

using BQueue = BlockingQueue <Message>;

/////////////////////////////////////////////////////////////////////////////
// Sendr class
// - accepts messages from client for consumption by MockChannel
//
class Sendr : public ISendr
{
public:
	void postMessage(const Message& msg);
	BQueue& queue();
private:
	BQueue sendQ_;
};

void Sendr::postMessage(const Message& msg)
{
	sendQ_.enQ(msg);
}

BQueue& Sendr::queue() { return sendQ_; }

/////////////////////////////////////////////////////////////////////////////
// Recvr class
// - accepts messages from MockChanel for consumption by client
//
class Recvr : public IRecvr
{
public:
	Message getMessage();
	BQueue& queue();
private:
	BQueue recvQ_;
};

Message Recvr::getMessage()
{
	return recvQ_.deQ();
}

BQueue& Recvr::queue()
{
	return recvQ_;
}
/////////////////////////////////////////////////////////////////////////////
// MockChannel class
// - reads messages from Sendr and writes messages to Recvr
//
class MockChannel : public IMockChannel
{
public:
	MockChannel(ISendr* pSendr, IRecvr* pRecvr, int clientPort, std::string clientRepoPath);
	void start();
	void stop();
private:
	std::thread thread_;
	std::vector<std::string> split(const std::string &s, char delim);
	Message processMessage(Message message);
	ISendr* pISendr_;
	IRecvr* pIRecvr_;
	bool stop_ = false;
	MsgClient c;
};

//----< pass pointers to Sender and Receiver >-------------------------------

MockChannel::MockChannel(ISendr* pSendr, IRecvr* pRecvr, int clientPort, std::string clientRepoPath) : pISendr_(pSendr), pIRecvr_(pRecvr), c(clientPort, clientRepoPath) {}

//----< creates thread to read from sendQ and echo back to the recvQ >-------

void MockChannel::start()
{
	std::cout << "\n  MockChannel starting up";
	thread_ = std::thread(
		[this] {
		Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
		Recvr* pRecvr = dynamic_cast<Recvr*>(pIRecvr_);
		if (pSendr == nullptr || pRecvr == nullptr)
		{
			std::cout << "\n  failed to start Mock Channel\n\n";
			return;
		}
		BQueue& sendQ = pSendr->queue();
		BQueue& recvQ = pRecvr->queue();
		while (!stop_)
		{
			std::cout << "\n  channel deQing message";
			Message msg = sendQ.deQ();  // will block here so send quit message when stopping
			std::cout << "\n  channel got message: " << msg;
			Message res = processMessage(msg);
			recvQ.enQ(res);
		}
		std::cout << "\n  Server stopping\n\n";
	});
}
//----< signal server thread to stop >---------------------------------------

void MockChannel::stop() { stop_ = true; }


/*
checks for the delimiter in the given string aand splits them and returns a vector
*/

std::vector<std::string> MockChannel::split(const std::string & s, char delim)
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
/*
Processes the message and performs the appropriat operation by calling the client functions
*/
Message MockChannel::processMessage(Message msg)
{
	Message res;
	std::vector<std::string> m = split(msg, ',');
	if (m[0] == "PUBLISH") {
		res = c.publishFiles(std::stoi(m[1]));
		res = "PUBLISH," + res;
	}
	else if (m[0] == "DELETE") {
		res = c.deleteFiles(std::stoi(m[1]));
		res = "DELETE," + res;
	}
	else if (m[0] == "DISPLAY") {
		res = c.listFiles(std::stoi(m[1]));
		res = "DISPLAY," + res;
	}
	else if (m[0] == "UPLOAD") {
		std::string files;
		int actualsize = (int)m.size() - 1;
		for (int i = 2; i < actualsize; i++) {
			files += m[i] + ",";
		}
		files.pop_back();
		res = c.uploadFiles(files, std::stoi(m[1]));
		std::cout << "\n   got response: " << res;
		res = "UPLOAD," + res;
		std::cout << "\n   sent response: " << res;
	}
	else if (m[0] == "DOWNLOAD") {
		if (m[2] != "ALL") {
			std::string files;
			int actualsize = (int)m.size() - 1;
			for (int i = 2; i < actualsize; i++) {
				files += m[i] + ",";
			}
			files.pop_back();
			res = c.downloadFiles(std::stoi(m[1]), files);
		}
		else {
			res = c.downloadFiles(std::stoi(m[1]), std::string("ALL"));
		}
		res = "DOWNLOAD," + res;
	}
	return res;
}

//----< factory functions >--------------------------------------------------

ISendr* ObjectFactory::createSendr() { return new Sendr; }

IRecvr* ObjectFactory::createRecvr() { return new Recvr; }

IMockChannel* ObjectFactory::createMockChannel(ISendr* pISendr, IRecvr* pIRecvr, int clientPort, std::string clientRepoPath)
{
	return new MockChannel(pISendr, pIRecvr, clientPort, clientRepoPath);
}

#ifdef TEST_MOCKCHANNEL

//----< test stub >----------------------------------------------------------

int main()
{
	ObjectFactory objFact;
	ISendr* pSendr = objFact.createSendr();
	IRecvr* pRecvr = objFact.createRecvr();
	IMockChannel* pMockChannel = objFact.createMockChannel(pSendr, pRecvr);
	pMockChannel->start();
	pSendr->postMessage("Hello World");
	pSendr->postMessage("CSE687 - Object Oriented Design");
	Message msg = pRecvr->getMessage();
	std::cout << "\n  received message = \"" << msg << "\"";
	msg = pRecvr->getMessage();
	std::cout << "\n  received message = \"" << msg << "\"";
	pSendr->postMessage("stopping");
	msg = pRecvr->getMessage();
	std::cout << "\n  received message = \"" << msg << "\"";
	pMockChannel->stop();
	pSendr->postMessage("quit");
	std::cin.get();
}
#endif
