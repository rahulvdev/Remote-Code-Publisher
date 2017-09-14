#pragma once
#pragma once
#pragma warning(disable:4221)
/////////////////////////////////////////////////////////////////////////////////////////
// DependencyAnalysis.h - computes dependencies between files based on type table info //
//                                                                                    //
// Rahul Vijaydev,CSE687 - Object Oriented Design, Spring 2017						  
//                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This module defines an DDependencyTable Class
DDependencyTable class defines functions to:
build dependency table based on type info stored in type table
return keys stored in the database
return row for a given key from the db
show the contents of the dependecy table

Public Interface:
=================
DependancyAnalysis();
void startDependencyAnalysis(TypeAnalysis& typ,std::string tokFile);
void checkAgainstTypeTable(std::string fName, std::string token, TypeAnalysis& typ);
DBHandler<std::string> getDbInstance();
void displayDependancies();
* Required Files:
* ---------------
*   - Typtable.h
*   - DBAccess.h
*   -Tokenizer.h
* Build Process:
* --------------
*   devenv DependencyAnalyzer.sln /debug rebuild
*
* Maintenance History:
* --------------------
ver 0.1 : 8th March 2017
*
/*
* -
*/

#include<vector>
#include "../TypeTable/typetable.h"
#include "../DBAccess/DBAccess.h"

class DependancyAnalysis {

public:
	DependancyAnalysis(std::string fpsth,int persistcount);
	void startDependencyAnalysis(TypeAnalysis& typ,std::string tokFile);
	void checkAgainstTypeTable(std::string fName, std::string token, TypeAnalysis& typ);
	DBHandler<std::string>& getDbInstance();
	void displayDependancies();

private:
	DBHandler<std::string> dbH;

};
