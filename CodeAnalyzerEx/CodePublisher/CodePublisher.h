#pragma once
#pragma warning (disable: 4503)
/////////////////////////////////////////////////////////////////////////////////////////
//// CodePublisher.h - Generates HTML pages for files whose type and dependancy        //
//// analysis  is done                                                                 //
//																					   //
//// Rahul Vijaydev,CSE687 - Object Oriented Design, Spring 2017					   // 
////                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
///*
//Module Operations:
//==================
//This module is used to generate HTML pages for all files
//whose type and dependancy analysis is carried out.It also provides functionality for
//hiding and unhiding class,method and global function scopes.
//
//Public Interface:
//=================
//using lineNoMap = std::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>>;
//using valVect = std::vector<std::unordered_map<std::string, std::string>>;
//using mapInstance = std::unordered_map<std::string, std::string>;
//CodePublisher(DependancyAnalysis& dpObj);
//void fileCollector(std::vector<std::string> listOfFiles, lineNoMap& lMap);
//std::stack<std::string> CodePublisher::getFileNameOnly(const std::string s, char delim);
//void generateHTMlfile(std::string file, lineNoMap& lMap, std::ofstream& ostIndex);
//void writeToHTMLfile(std::string fileToRead, std::string htmlFileName, lineNoMap& lMap);
//std::string convertIdtoString(int id);
//std::vector<std::string> getDependantFiles(std::string file);
//void printDependancy(std::ofstream& ost, std::string fileNameOnly, std::vector<std::string> dependancyVect);
//void printInitBody(std::ofstream& ost, std::string file);
//std::vector<int> getStartLineVect(std::vector<std::unordered_map<std::string, std::string>> valueVect);
//std::vector<int> getEndLineVect(std::vector<std::unordered_map<std::string, std::string>> valueVect);
//void printButtonAndDiv(std::ofstream& ost, std::vector<std::string> elem);
//void openFilesInBrowser(std::string htmlFileName);
//void printHtmlEnd(std::ofstream& myfile);
//void printHtmlPrologue(std::string htmlFileName, std::ofstream& ost);
//void generateTags(std::ofstream& myfile);
//valVect removeElementsWithLineDiffOne(valVect refineVect);
//void displayLineNoInfo(valVect);
//void writeToIndex(std::string htmlFileName, std::ofstream& ostIndex);
//void writeIndexInit(std::ofstream& ostIndex);
//
//* Required Files:
//* ---------------
//*   -DependancyAnalyzer.h
//*
//* Build Process:
//* --------------
//*   devenv CodePublisher.sln /debug rebuild
//*
//* Maintenance History:
//* --------------------
//ver 0.1 : 3rd April 2017
//*
///*
//* -
//*/
#include<string>
#include<fstream>
#include<stack>
#include<sstream>
#include<vector>
#include<iostream>
#include"..\DependencyAnalyzer\DependencyAnalysis.h"


class CodePublisher {

public:
	using lineNoMap = std::unordered_map<std::string,std::vector<std::unordered_map<std::string, std::string>>>;
	using valVect = std::vector<std::unordered_map<std::string, std::string>>;
	using mapInstance = std::unordered_map<std::string, std::string>;
	CodePublisher(DependancyAnalysis& dpObj,std::string category);
	void fileCollector(std::vector<std::string> listOfFiles,lineNoMap& lMap,std::string category);
	std::stack<std::string> CodePublisher::getFileNameOnly(const std::string s, char delim);
	void generateHTMlfile(std::string file,lineNoMap& lMap,std::ofstream& ostIndex,std::string category);
	void writeToHTMLfile(std::string fileToRead,std::string htmlFileName,lineNoMap& lMap,std::string category);
	std::string convertIdtoString(int id);
	std::vector<std::string> getDependantFiles(std::string file);
	void printDependancy(std::ofstream& ost,std::string fileNameOnly,std::vector<std::string> dependancyVect,std::string category);
	void printInitBody(std::ofstream& ost,std::string file);
	std::vector<int> getStartLineVect(std::vector<std::unordered_map<std::string, std::string>> valueVect);
	std::vector<int> getEndLineVect(std::vector<std::unordered_map<std::string, std::string>> valueVect);
	void printButtonAndDiv(std::ofstream& ost, std::vector<std::string> elem);
	void openFilesInBrowser(std::string htmlFileName,std::string category);
	void printHtmlEnd(std::ofstream& myfile);
	void printHtmlPrologue(std::string htmlFileName,std::ofstream& ost);
	void generateTags(std::ofstream& myfile);
	valVect removeElementsWithLineDiffOne(valVect refineVect);
	void displayLineNoInfo(valVect);
	void writeToIndex(std::string htmlFileName,std::ofstream& ostIndex);
	void writeIndexInit(std::ofstream& ostIndex);


private:
	std::string htmlHeader = "<html>\n<head>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"..\\CSSfiles\\CSSstart.css\">\n<script src=\"..\\JavaScript\\JSFile.js\">\n</script>\n</head>\n";
	std::string htmlFilePath = "..\\..\\Repository\\PublishedFiles\\";
	std::vector<std::string> generateUniqueButtonAndDiv();
	int uniqueId = 0;
	DependancyAnalysis& dpAnal;
	DBHandler<std::string> dbH;
	std::string indexStart = "<html>\n<head>\n<title>\"Index Page\"</title>\n</head>\n<body>\n";
	
};