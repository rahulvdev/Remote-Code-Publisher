#pragma once
#pragma once
/////////////////////////////////////////////////////////////////////////////////////////
//// TypeTable.h - stores file names, type names ,namesspace for each type defined     //
////                                                                                   //
//// Rahul Vijaydev,CSE687 - Object Oriented Design, Spring 2017					     // 
////                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////
///*
//Module Operations:
//==================
//This module defines an TypeAnalysis class
//
//TypeAnalysis class defines functions to:
//build dependency table based on type info stored in type table
//return keys stored in the database
//return row for a given key from the db
//show the contents of the dependecy table
//
//Public Interface:
//=================
//TypeAnalysis();
//std::vector<std::string> returnDependentFiles(std::string token);
//void startTypeAnalysis();
//bool doDisplay(ASTNode* pNode);
//void searchAST(ASTNode* pNode);
//void showTypeTable();
//using uMap = std::unordered_map <std::string, std::string>;
//
//void showTypeTable();
//std::vector<std::string> getTypeDefinationFnames(std::string typeName);
//
//* Required Files:
//* ---------------
//*   - AbstrSynTree.h
//*   - ActionsAndRules.h
//*
//* Build Process:
//* --------------
//*   devenv TypeTable.sln /debug rebuild
//*
//* Maintenance History:
//* --------------------
//ver 0.1 : 7nd March 2017
//*
///*
//* -
//*/
#pragma warning (disable: 4503)
#include<iostream>
#include<vector>
#include <functional>
#include<string>
#include<stack>
#include "../Parser/ActionsAndRules.h"
#include "../AbstractSyntaxTree/AbstrSynTree.h"

using namespace CodeAnalysis;

class TypeAnalysis {

public:

	using uMap = std::unordered_map <std::string, std::string>;
	using vectStr = std::vector<std::string>;
	using vectOfMap = std::vector<std::unordered_map <std::string, std::string>>;


	TypeAnalysis();
	std::vector<std::string> returnDependentFiles(std::string token);
	void startTypeAnalysis();
	bool doDisplay(ASTNode* pNode);
	void searchAST(ASTNode* pNode);
	void showTypeTable();
	void getGlobalFunctionInfo(ASTNode * globalNode);
	void storeGlobalFunction(ASTNode* pNode);
	std::unordered_map<std::string, std::vector<std::unordered_map<std::string,std::string>>>& getGlobFuncMap();
	void displayLineNoInfo();
	std::unordered_map<std::string, vectOfMap>& getLineNumMap();
	bool checkIfLineStEnEqual(std::string start, std::string end);


private:
	std::unordered_map<std::string, std::vector<uMap>> typeTable;
	std::unordered_map<std::string, std::vector<uMap>> lineNoMap;
	std::unordered_map<std::string,vectOfMap> globalfunctionMap;
	std::vector<uMap> valueVect;
	std::stack<std::string> namespaceStack;
	AbstrSynTree& ASTref_;
	ScopeStack<ASTNode*> scopeStack_;
	Scanner::Toker& toker_;
};
