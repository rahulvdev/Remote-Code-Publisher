///////////////////////////////////////////////////////////////////////////
// TypeTable.cpp - It is used to generate type information from AST data //
//                                                                       //
// Rahul Vijaydev, CSE687 - Object Oriented Design, Spring 2017          //
///////////////////////////////////////////////////////////////////////////
//
//Description: This file is used to generate a typetable that contains information about
//classes,methods,global functions,enums and structs
//

#include "typetable.h"


//Constructor for TypeAnalysis
//Initializes AST,ScopeStack and Toker instances
TypeAnalysis::TypeAnalysis() :ASTref_(Repository::getInstance()->AST()),
scopeStack_(Repository::getInstance()->scopeStack()),
toker_(*(Repository::getInstance()->Toker())) {
}

//Returns a list of all files from the typetable
//by attempting to match the token against each entry in the type table
std::vector<std::string> TypeAnalysis::returnDependentFiles(std::string token) {
	std::vector<std::string> depFileVect;
	std::vector<std::string> emptyVect;
	if (typeTable.find(token) != typeTable.end()) {
		std::vector<std::unordered_map<std::string, std::string>> mapVect = typeTable[token];
		for (std::vector<std::unordered_map<std::string, std::string>>::iterator it = mapVect.begin(); it != mapVect.end(); it++) {
			std::unordered_map<std::string, std::string> mapForTok = *it;
			depFileVect.push_back(mapForTok["filename"]);
		}
	}
	else {
	}
	return depFileVect;
}

//Iniitializes type analysis
void TypeAnalysis::startTypeAnalysis()
{
	std::cout << "\n  starting type analysis:\n";
	std::cout << "\n  scanning AST and displaying important things:";
	std::cout << "\n -----------------------------------------------";
	ASTNode* pRoot = ASTref_.root();
	searchAST(pRoot);
	getGlobalFunctionInfo(Repository::getInstance()->getGlobalScope());
}

//Checks if node is either of class,struct or enum
bool TypeAnalysis::doDisplay(ASTNode* pNode)
{
	static std::string toDisplay[] = {
		"class","function","struct", "enum"
	};
	for (std::string type : toDisplay)
	{
		if (pNode->type_ == type)
			return true;
	}
	return false;
}

//Displays typetable content 
void TypeAnalysis::showTypeTable() {
	for (auto it : typeTable) {
		std::cout << std::endl;
		std::cout << it.first;
		std::cout << std::endl;
		std::cout << "----------------------------------" << std::endl;
		std::vector<std::unordered_map<std::string, std::string>> valueVect = it.second;
		for (std::vector<std::unordered_map <std::string, std::string>>::iterator it = valueVect.begin(); it != valueVect.end(); it++) {
			std::unordered_map <std::string, std::string> inst = *it;
			for (auto item : inst) {
				std::cout << std::setw(20)<<item.first << std::setw(20)<< item.second << std::endl;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "-------------------------------------" << std::endl;
	}
}

//Used to get global function information
void TypeAnalysis::getGlobalFunctionInfo(ASTNode * globalNode) {
	std::cout << "\n---------------------------------------\n";
	for (auto node : globalNode->children_) {
		if (node->type_ == "function") {
			if (node->name_ != "void")
				storeGlobalFunction(node);
		}
	}
	std::cout << "\n---------------------------------------\n";
}


//Stores global function information
void TypeAnalysis::storeGlobalFunction(ASTNode* pNode)
{
	std::vector<std::unordered_map<std::string, std::string>> glFuncVect;
	std::unordered_map<std::string, std::string> glFuncMap;
	if (globalfunctionMap.find(pNode->name_) == globalfunctionMap.end()) {
		glFuncMap["file"] = pNode->path_;
		std::ostringstream ost;
		ost << pNode->startLineCount_;
		std::string lineNum = ost.str();
		ost.str();
		ost.clear();
		glFuncMap["lineNumber"] = lineNum;
		ost << pNode->endLineCount_;
		std::string endLine = ost.str();
		glFuncMap["lineNumberEnd"] = endLine;
		glFuncVect.push_back(glFuncMap);

	}
	else {
		glFuncVect = globalfunctionMap[pNode->name_];
		std::unordered_map<std::string, std::string> glFuncMap;
		glFuncMap["file"] = pNode->path_;
		std::ostringstream ost;
		ost << pNode->startLineCount_;
		std::string linNum = ost.str();
		ost.str("");
		ost.clear();
		glFuncMap["lineNumber"] = linNum;
		ost << pNode->endLineCount_;
		std::string endLine = ost.str();
		glFuncMap["lineNumberEnd"] = endLine;
		glFuncVect.push_back(glFuncMap);
	}
	globalfunctionMap[pNode->name_] = glFuncVect;
}


//Returns instance of globalfunction map
std::unordered_map<std::string,std::vector<std::unordered_map<std::string,std::string>>>& TypeAnalysis::getGlobFuncMap()
{
	return globalfunctionMap;
}

//Depth first search over the abstract syntax tree
//Builds type table
void TypeAnalysis::searchAST(ASTNode* pNode)
{	static std::string path = "";
	if (pNode->path_ != path)
	{
		std::cout << "\n    -- " << pNode->path_ << "\\" << pNode->package_;
		path = pNode->path_;}
	if (doDisplay(pNode))
	{
		std::unordered_map<std::string, std::string> mapElem;
		std::unordered_map<std::string, std::string> lineNoVal;
		mapElem["type"] = pNode->type_;
		mapElem["filename"] = pNode->path_;
		mapElem["namespace"] = namespaceStack.top();
		std::ostringstream ost;
		ost << pNode->startLineCount_;
		std::string lineNumber = ost.str();
		ost.str("");
		ost.clear();
		mapElem["lineNum"] = lineNumber;
		ost << pNode->endLineCount_;
		std::string lineNumberEnd = ost.str();
		mapElem["lineNumEnd"] = lineNumberEnd;
		bool eqCheck=checkIfLineStEnEqual(lineNumber, lineNumberEnd);
			lineNoVal["startLine"] = lineNumber;
			lineNoVal["endLine"] = lineNumberEnd;
		std::vector<std::unordered_map<std::string, std::string>> lineNoVect;
		if (!eqCheck) {
			if (lineNoMap.find(pNode->path_) == lineNoMap.end()) {
				lineNoVect.push_back(lineNoVal);}
			else {
				lineNoVect = lineNoMap[pNode->path_];
				lineNoVect.push_back(lineNoVal);}
			lineNoMap[pNode->path_] = lineNoVect;}
		else {}
		std::vector<std::unordered_map<std::string, std::string>> initVect;
		if (typeTable.find(pNode->name_) == typeTable.end()) {
			initVect.push_back(mapElem);
		}
		else {
			initVect = typeTable[pNode->name_];
			initVect.push_back(mapElem);}
		typeTable[pNode->name_] = initVect;}
	if (pNode->type_ == "namespace") {
		namespaceStack.push(pNode->name_);}
	for (auto pChild : pNode->children_) {
		searchAST(pChild);}
	if (pNode->type_ == "namespace") {
		namespaceStack.pop();}
}

//Checks if start and end lines of nodes are equal
bool TypeAnalysis::checkIfLineStEnEqual(std::string start, std::string end)
{
	int startVal = std::stoi(start);
	int endVal = std::stoi(end);
	if (startVal == endVal)
		return true;
	else
		return false;
	
}

//Method to display line numbers for classes,methods and functions
void TypeAnalysis::displayLineNoInfo()
{
	for (auto it : lineNoMap) {
		std::cout << std::endl;
		std::cout << it.first;
		std::cout << std::endl;
		std::cout << "----------------------------------" << std::endl;
		std::vector<std::unordered_map<std::string, std::string>> valueVect = it.second;
		for (std::vector<std::unordered_map <std::string, std::string>>::iterator it = valueVect.begin(); it != valueVect.end(); it++) {
			std::unordered_map <std::string, std::string> inst = *it;
			for (auto item : inst) {
				std::cout << std::setw(20) << item.first << std::setw(20) << item.second << std::endl;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "-------------------------------------" << std::endl;
	}
}

std::unordered_map<std::string,TypeAnalysis::vectOfMap>& TypeAnalysis::getLineNumMap()
{
	return lineNoMap;
}


#ifdef TYPETABLE

int main() {
	TypeAnalysis typAnal;
	typAnal.startTypeAnalysis();
	std::cout << "-------------------------------\n";
	std::cout << "Displaying Type Table Content\n";
	typAnal.showTypeTable();
	std::cout << "-------------------------------\n";
}

#endif // TYPETABLE
