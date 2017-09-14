#pragma once
//////////////////////////////////////////////////////////////////////
// DBAccess.h - It provides member functions for database operations//
//                                                                  //
// Rahul Vijaydev, CSE687 - Object Oriented Design, Spring 2017     //
/////////////////////////////////////////////////////////////////////
//
//Description: 
//----------------
//This file provides memeber functions used to access all database operations
//like insertion of database elements,deletion of elements,updating metadata and data values
//and querying the database content.
//
//
//Public Interface:
//-------------------
//DBHandler()
//DBHandler(std::string fname, int writeNum)
//bool deleteElement(std::string key);
//void showElement(Key key);
//std::vector<Key> getKeys();
//Element<Data> getValue(Key key);
//void saveDBElement(std::string name, std::string category, std::string desc, Data data, std::vector<Key> childrenVect);
//void saveFileDependancy(std::string name, std::vector<std::string> depFileVect);
//bool modifyMetaData(Key key, std::string property, std::string newVal, bool typeOfOp);
//bool modifyData(Key key, Data data);
//void addChild(Key key, std::string child);
//time_t DateTimeFromString(std::string Dt);
//std::string DateTimeToString(time_t time);
//void toXMl(std::string nameOfFile);
//void fromXml(std::string nameOfFile);
//void showDbContent();
//std::vector<std::string> queryTime(std::string startTime, std::string endTime);
//std::vector<std::string> retrieveChildrenList(Key key);
//std::vector<std::string> queryElement(std::vector<std::string> keyList, std::string pattern, std::string property);
//std::vector<std::string> formKeyUnion(std::vector<std::string> keySet1, std::vector<std::string> keySet2);
//std::vector<std::string> queryElemRegex(std::vector<std::string> keyList, std::string regexPattern, std::string property);
//
//Required Files :
//-------------- -
//-NoSqlDb.h
//
// Build Process :
//--------------
//   devenv DBWrapper.sln / debug rebuild
//
//Maintenance History :
//--------------------
//ver 0.1 : 2nd Feb 2017
/*
* -
*/

#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include "../NoSqlDb/NoSqlDb.h"


template<typename Data>
class DBHandler {

private:
	NoSqlDb<Data> dbObj;
	int noOfWrites;
	int currentWrites = 0;
	std::string nameOffile;

public:

	using StrData = std::string;
	using intData = int;
	using Key = std::string;
	using Keys = std::vector<Key>;

	//Non-Parameterized DBHandler constructor
	DBHandler() {

	}

	//DBHandler contructor
	//initializes path for writing DB content to XML file
	DBHandler(std::string fname, int writeNum)
	{
		nameOffile = fname;
		noOfWrites = writeNum;
		dbObj.setxmlFilePath(fname);
	}


	bool deleteElement(std::string key);
	void showElement(Key key);
	std::vector<Key> getKeys();
	Element<Data> getValue(Key key);
	void saveDBElement(std::string name, std::string category, std::string desc, Data data, std::vector<Key> childrenVect);
	void saveFileDependancy(std::string name, std::vector<std::string> depFileVect);
	bool modifyMetaData(Key key, std::string property, std::string newVal, bool typeOfOp);
	bool modifyData(Key key, Data data);
	void addChild(Key key, std::string child);
	time_t DateTimeFromString(std::string Dt);
	std::string DateTimeToString(time_t time);
	void toXMl(std::string nameOfFile);
	void fromXml(std::string nameOfFile);
	void showDbContent();
	std::vector<std::string> queryTime(std::string startTime, std::string endTime);
	std::vector<std::string> retrieveChildrenList(Key key);
	std::vector<std::string> formKeyUnion(std::vector<std::string> keySet1, std::vector<std::string> keySet2);
};



//Delete Element from database
template<typename Data>
bool DBHandler<Data>::deleteElement(std::string key) {
	std::cout << "Deleting element with key= " << key << "\n";
	if (dbObj.deleteElement(key))
		return true;
	else
		return false;
}


//Returns the list of all keys from teh database
template<typename Data>
std::vector<std::string> DBHandler<Data>::getKeys() {
	return dbObj.keys();
}

//Used to write DB content to XML file
template<typename Data>
void DBHandler<Data>::toXMl(std::string nameOfFile) {
	dbObj.toXml(nameOfFile);
}

//Used to read XML file
template<typename Data>
void  DBHandler<Data>::fromXml(std::string nameOfFile) {
	dbObj.fromXml(nameOfFile);
}

//Displays database content
template<typename Data>
inline void DBHandler<Data>::showDbContent()
{
	std::cout << "\n  size of db = " << dbObj.count() << "\n";
	Keys keys = dbObj.keys();
	for (Key key : keys)
	{
		//std::cout << "\n  " << key << ":";
		std::cout << dbObj.value(key).show();
	}
}

//Dsiplays one record from the database
template<typename Data>
void DBHandler<Data>::showElement(Key key) {
	std::cout << "\n";
	std::cout << "Displaying contents of element with key " << key << "\n";
	std::cout << dbObj.value(key).show();
	std::cout << "-----------------------------------------------------------------------" << "\n";
	std::cout << "\n";
}

//Returns the value associated with a particular key
template<typename Data>
Element<Data> DBHandler<Data>::getValue(Key key) {
	
	return dbObj.value(key);
}

//Used to save elements into the database
template<typename Data>
void DBHandler<Data>::saveDBElement(std::string name, std::string category, std::string desc, Data data, std::vector<Key> childrenVect) {
	Element<Data> elem;
	elem.name = name;
	elem.children = childrenVect;
	dbObj.save(elem.name, elem);
	currentWrites++;
	if (currentWrites > noOfWrites) {
		toXMl(nameOffile);
		currentWrites = 0;
	}
}

//Used to update or modify metadata informtation
template<typename Data>
bool DBHandler<Data>::modifyMetaData(Key key, std::string property, std::string newVal, bool typeOfOp) {
	std::cout << " Property " << property << " is being updated to new value " << newVal << "\n";
	std::cout << "--------------------------------- \n";
	if (dbObj.updateMetadata(key, property, newVal, typeOfOp))
		return true;
	else
		return false;
}

//Used to modify data 
template<typename Data>
bool DBHandler<Data>::modifyData(Key key, Data data) {
	if (dbObj.updateData(key, data))
		return true;
	else
		return false;

}

//Called when file dependancies need to be stored in the database
template<typename Data>
void DBHandler<Data>::saveFileDependancy(std::string fName, std::vector<std::string> depFileVect) {
	Element<Data> elem;
	elem.name = fName;
	elem.children = depFileVect;
	bool flag=dbObj.save(fName, elem);
	if (!flag) {
		dbObj.updateChildVal(fName, depFileVect);
	}
}

//returns the list of children associated with a particular key
template<typename Data>
std::vector<std::string> DBHandler<Data>::retrieveChildrenList(Key key) {
	std::vector<Key> childrenVect = dbObj.getChildren(key);
	if (childrenVect.size() != 0) {
		return childrenVect;
	}
	else {
		std::vector<Key> emptyVect;
		std::cout << "\n";
		std::cout << " Element does not contain any children" << "\n";
		std::cout << "\n";
		return emptyVect;
	}
}

//Used to add a child value to a database element
template<typename Data>
void DBHandler<Data>::addChild(Key key, std::string child) {
	Element<Data> elem = dbObj.value(key);
	std::vector<Key> childrenVect = elem.children;
	childrenVect.push_back(child);
	elem.children = childrenVect;
	dbObj.insertChild(key, elem);
}



//Convert from string to DateTime format
template<typename Data>
time_t  DBHandler<Data>::DateTimeFromString(std::string Dt) {
	time_t tmDat;
	try {
		struct tm dtim = { 0 };
		int year = 0;
		int month = 0;
		int day = 0;
		int hour = 0;
		int min = 0;
		int sec = 0;
		const char* DtStr = Dt.c_str();
		sscanf_s(DtStr, "%d-%d-%d.%d:%d:%d", &year, &month, &day, &hour, &min, &sec);
		dtim.tm_year = year - 1900;
		dtim.tm_mon = month - 1;
		dtim.tm_mday = day;
		dtim.tm_hour = hour;
		dtim.tm_min = min;
		dtim.tm_sec = sec;
		tmDat = mktime(&dtim);
	}
	catch (std::exception e) {
		std::cout << e.what();
	}
	return tmDat;
}


//Convert DateTime Format to string
template<typename Data>
std::string DBHandler<Data>::DateTimeToString(time_t time) {
	try {
		struct tm tstruct = {};
		char charArr[80];
		localtime_s(&tstruct, &t);
		strftime(charArr, sizeof(charArr), "%Y-%m-%d.%X", &tstruct);
		std::string dTim(charArr);
	}
	catch (std::exception e) {
		std::cout << e.what();
	}
	return dTim;
}


//Querying for elements in the database
//that were added with the specified time frame
template<typename Data>
std::vector<std::string> DBHandler<Data>::queryTime(std::string startTime, std::string endTime) {
	std::vector<Key> retVect;
	try {
		std::vector<Key> dtVect = dbObj.keys();
		std::cout << "Number of elements in the database are " << dtVect.size() << "\n";
		std::cout << "\n";
		time_t stTime = DateTimeFromString(startTime);
		for (Key k : dtVect) {
			Element<Data> elem = dbObj.value(k);
			time_t storeDt = elem.timeDate;
			if (endTime == "DEFAULT") {
				time_t eTime = time(0);
				if (storeDt > stTime && storeDt < eTime) {
					retVect.push_back(k);
				}
				else
					std::cout << "Element not stored within specified time" << "\n";
			}
			else {
				time_t enTime = DateTimeFromString(endTime);
				if (storeDt > stTime && storeDt < enTime) {
					retVect.push_back(k);
				}
				else
					std::cout << "Element not stored within specified time" << "\n";
			}
		}
	}
	catch (std::exception e) {
		std::cout << e.what();
	}
	return retVect;
}


//Used to form a union of keys
//Separate keysets obtained from different querying results are merged 
//and a new keyset is generated
template<typename Data>
std::vector<std::string> DBHandler<Data>::formKeyUnion(std::vector<std::string> keySet1, std::vector<std::string> keySet2) {
	bool keyPresent = false;
	int outerLoop = 0;
	int innerLoop = 0;
	int keySet1_size = keySet1.size();
	int keySet2_size = keySet2.size();
	std::vector<Key> keyVect;
	for (outerLoop = 0; outerLoop < keySet1_size; outerLoop++) {
		for (innerLoop = 0; innerLoop < keySet2_size; innerLoop++) {
			if (keySet1[outerLoop] == keySet2[innerLoop]) {
				keyPresent = true;
			}
			else
				keyPresent = false;
		}
		if (innerLoop == (keySet2_size - 1) && keyPresent == false) {
			keySet2.push_back(keySet1[outerLoop]);
		}
	}
	return keySet2;
}

