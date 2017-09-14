///////////////////////////////////////////////////////////////////////
// DependancyAnalysis.cpp - It is used to perform dependancy analysis//
// using typetable information                                       //
//																	 //
// Rahul Vijaydev, CSE687 - Object Oriented Design, Spring 2017      //
/////////////////////////////////////////////////////////////////////
//
//Description:This file provides implementation for generating the dependancy table
//from typetable information
//

#include "DependencyAnalysis.h"
#include "../Tokenizer/Tokenizer.h"
using namespace Scanner;


//DependancyAnalysis constructor
//Initializes path for persisiting dependancy information to XML file
DependancyAnalysis::DependancyAnalysis(std::string fpath, int persistcount):dbH(fpath,persistcount) {
}

//Initiates dependancy analysis
//Runs tokenizer on each input file
void DependancyAnalysis::startDependencyAnalysis(TypeAnalysis& typ,std::string tokFile) {
	try {
		std::string fileName = FileSystem::Path::getName(tokFile);
		std::ifstream in(tokFile);
		if (!in.good())
		{
			std::cout << "\n  can't open " << tokFile << "\n\n";
		}
		{
			Toker toker;
			toker.returnComments();
			toker.attach(&in);
			do
			{
				std::string tok = toker.getTok();
				if (tok == "\n")
					tok = "newline";
				checkAgainstTypeTable(tokFile, tok, typ);
				//std::cout << "\n -- " << tok;
			} while (in.good());
		}
	}
	catch (std::exception e) {
		std::cout << e.what();
	}
}

//Checks if token is present in typetable
//and returns list of all files that the token is present in
//Saves dependancy information in the database
void DependancyAnalysis::checkAgainstTypeTable(std::string fileName, std::string token, TypeAnalysis& typ) {
	std::vector<std::string> depFileVect = typ.returnDependentFiles(token);
	if (depFileVect.size() != 0) {
		dbH.saveFileDependancy(fileName, depFileVect);
	}
}

//Displays dependancy table
void DependancyAnalysis::displayDependancies() {
	std::cout << "------------------------------------" << std::endl;
	std::cout << "Displaying Database contents: " << std::endl;
	std::cout << "------------------------------------" << std::endl;
	std::cout << "\n";
	dbH.showDbContent();
}

//returns instance to dependancy table
DBHandler<std::string>& DependancyAnalysis::getDbInstance() {
	return dbH;
}


#ifdef DEPENDANCYANALYZER

int main() {
	Executive exec;
	std::vector<std::string> fileVect = exec.getAllSubFiles();
	DependancyAnalysis dpAnal("..\..\depInfo.xml", 20);
	for (std::string file : fileVect) {
		dpAnal.startDependancyAnalysis(file);
	}
	dpAnal.displayDependancies();
}


#endif // DEPENDANCYANALYZER




