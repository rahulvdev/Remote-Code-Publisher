/////////////////////////////////////////////////////////////////////
// CodePublisher.cpp -Generates HTML pages for each C++ source file//
//																   //
//																   //
// Rahul Vijaydev, CSE687 - Object Oriented Design, Spring 2017    //
/////////////////////////////////////////////////////////////////////
//
//Description: This file contains methods to generate HTML files for each C++ source file along with
//links to its dependancies.Also, an index page is generated that contains links to each individual 
//HTML page
//

#include "CodePublisher.h"

//constructor for CodePublisher
//Intitializes DependancyAnalyzer object
CodePublisher::CodePublisher(DependancyAnalysis& dpObj,std::string category) :dpAnal(dpObj) {
	htmlFilePath = htmlFilePath + category+"\\";
	std::cout << "\n\n";
	std::cout << "------------------------------------------------------------\n";
	std::cout << "HTML files are created in path ..\\..\\Repository\\PublishedFiles\\ \n";
	std::cout << "------------------------------------------------------------\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 5:CSS FILE for styling is present in path: ..\\..\\CSSfiles\\CSSstart.css\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENTS 4 AND 5:JavaScript file for hiding and unhiding is present in path: ..\\..\\JavaScript\\JSFile.js\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 6:LINKS to CSS and JS files:<head>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"..\\..\\CSSfiles\\CSSstart.css\">\n<script src=\"..\\..\\JavaScript\\JSFile.js\">\n</script>\n</head>\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "\n\n";
}

//Receives all files in the directory for HTML file generation
void CodePublisher::fileCollector(std::vector<std::string> listOfFiles, lineNoMap& lMap,std::string category)
{	
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::cout << "REQUIREMENT 3(Proj 4):Repository program that provides functionality to publish, as linked web pages, the contents of a set of C++ source code files.\n";
	std::cout << "----------------------------------------------------------------------------------------------------------\n";
	std::ofstream ostIndex;
	ostIndex.open(htmlFilePath+"index.htm");
	writeIndexInit(ostIndex);
	ostIndex << "<ul style=\"list - style - type:disc\">";
	if (listOfFiles.size() != 0) {
		for (std::string file : listOfFiles) {
			generateHTMlfile(file,lMap,ostIndex,category);
		}
		ostIndex << "</body>\n";
		ostIndex << "</html>";
		ostIndex.close();
		openFilesInBrowser("index",category);
	}
}


//Returns the name of the file name only
//Eliminates fully qualified structure
	std::stack<std::string> CodePublisher::getFileNameOnly(const std::string s, char delim) {
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

	//Helper function for initially writing to the index HTML file
	void CodePublisher::writeIndexInit(std::ofstream& ostIndex) {
			ostIndex << indexStart;
			ostIndex << "<hr/>\n";
			ostIndex << "<h3>INDEX</h3>\n";
			ostIndex << "<hr/>\n";
	}

	//Initiates HTML file generation
	void CodePublisher::generateHTMlfile(std::string file,lineNoMap& lMap,std::ofstream& ostIndex,std::string category)
	{
		std::stack<std::string> fileComponents=getFileNameOnly(file, '\\');
		std::string fileName;
		if (fileComponents.size()) {
			fileName = fileComponents.top();
		}
		writeToIndex(fileName,ostIndex);
		writeToHTMLfile(file,fileName,lMap,category);
	}

	//Reads the given file line by line and constrcuts string objects out of every line
	//Writes each character from the string to a html file in between html 'pre' tags
	//For preservation of white space
	void CodePublisher::writeToHTMLfile(std::string fileToRead, std::string htmlFileName,lineNoMap& lMap,std::string category)
	{	int counter = 1;
		std::vector<std::string> dependantFileVect=getDependantFiles(fileToRead);
		std::vector<std::unordered_map<std::string, std::string>> lineInfoVal;
		std::vector<std::unordered_map<std::string, std::string>> processVect;
		std::vector<int> startLineVect;
		std::vector<int> endLineVect;
		if (lMap.find(fileToRead) != lMap.end()) {
			lineInfoVal=lMap[fileToRead];
			processVect=removeElementsWithLineDiffOne(lineInfoVal);}
		else {
			std::cout << "File Not found in the map\n";}
		startLineVect = getStartLineVect(processVect);
		endLineVect = getEndLineVect(processVect);
		std::ofstream myfile(htmlFilePath+htmlFileName+".htm");
		if (myfile.is_open())
		{
			myfile << htmlHeader;
			myfile << "<body>";
			myfile << "\n";
			printHtmlPrologue(htmlFileName + ".htm", myfile);
			printInitBody(myfile,fileToRead);
			std::cout << "-------------------REQUIREMENT 7:Including links to dependant HTML files-----------------\n";
			printDependancy(myfile, htmlFileName, dependantFileVect,category);
			generateTags(myfile);
			std::ifstream file(fileToRead);
			std::string str;
			myfile << "<pre>";
			myfile << "\n";
			while (std::getline(file, str))
			{
				for (int lineNo : startLineVect) {
					if (counter == (lineNo + 1)) {
						std::vector<std::string> elem=generateUniqueButtonAndDiv();
						printButtonAndDiv(myfile, elem);}}
				for (int lineNo : endLineVect) {
					if (counter == lineNo + 1) {
						myfile << "</div>";}}
				for (char& c : str) {
					if (c == '<') {
						myfile << "&lt;";}
					else if (c == '>') {
						myfile << "&gt;";}
					else
						myfile << c;}
				myfile << "\n";
				counter++;}
			printHtmlEnd(myfile);}
		else 
			std::cout << "Unable to open file";}


	//Used to remove elements whose start and end lines differed by one
	//Typically for inline functions
	CodePublisher::valVect CodePublisher::removeElementsWithLineDiffOne(CodePublisher::valVect refineVect) {
		CodePublisher::valVect::iterator it = refineVect.begin();
		while (it != refineVect.end()) {
			std::unordered_map<std::string, std::string> mapInst = *it;
			std::string start = mapInst["startLine"];
			std::string end = mapInst["endLine"];
			int startInt = std::stoi(start);
			int endInt = std::stoi(end);
			if ((endInt-startInt) == 1) {
				it=refineVect.erase(it);
			}
			else {
				it++;
			}
		}
		return refineVect; 
	}


	//Creates an anchor tag for each HTML file linking
	//it to its appropriate HTMl web page
	void CodePublisher::writeToIndex(std::string htmlFileName,std::ofstream& ostIndex) {
		ostIndex << "<li>\n";
		ostIndex << "<a href=\"" +htmlFileName + ".htm" + "\">" + htmlFileName + ".htm" + "</a>\n";
		ostIndex << "</li>\n";
	}

	//Generates unique button and div tags
	std::vector<std::string> CodePublisher::generateUniqueButtonAndDiv()
	{
		std::vector<std::string> elemVect;
		std::string numInString= convertIdtoString(uniqueId);
		std::string button = "<button id=\"but"+ numInString +"\" type=\"button\" onclick=\"triggerMethod(this)\">+</button>";
		std::string divElement = "<div class=\"noShow\" id=\"but"+ numInString +"_div\">";
		elemVect.push_back(button);
		elemVect.push_back(divElement);
		uniqueId++;
		return elemVect;
	}

	//Generates string representation of integer id
	std::string  CodePublisher::convertIdtoString(int id)
	{
		std::string intRepresentation;
		std::ostringstream convert; 
		convert << id;      
		intRepresentation = convert.str();
		return intRepresentation;
	}

	//Returns the list of all files that the given file is dependant upon
	//HTMl variants of these files are added as dependancies in the form of links on the HTML page
	std::vector<std::string> CodePublisher::getDependantFiles(std::string file)
	{
		dbH=dpAnal.getDbInstance();
		Element<std::string> elem=dbH.getValue(file);
		std::vector<std::string> dependantFileVect = elem.children;
		std::cout << "\n";
		std::cout << "File is: " << file << std::endl;
		std::cout << "\n";
		return dependantFileVect;
	}

	//Prints anchor tags and links to dependant files
void CodePublisher::printDependancy(std::ofstream& ost,std::string fileNameOnly,std::vector<std::string> dependancyVect,std::string category)
	{
	ost << "<ul style=\"list - style - type:disc\">";
	ost << "\n";
	//std::string htmlFilePath = "..\\..\\Filestobepublished\\HTML\\";
	std::string htmlFilePath = "..\\..\\Repository\\PublishedFiles\\"+ category+"\\";
	if (ost.is_open()) {
		for (std::string file : dependancyVect) {
			std::stack<std::string> fileStack=getFileNameOnly(file,'\\');
			std::string fNameOnly=fileStack.top();
			std::string depFile = "<li><a href=\"" +fNameOnly+ ".htm" + "\">" + fNameOnly + "</a></li>";
			ost << depFile;
			ost << "\n";
		}
		ost << "</ul>";
		ost << "\n";
	}
}

//Prints initiial HTML body content
void CodePublisher::printInitBody(std::ofstream& ost,std::string file)
{
	std::stack<std::string> fileStack = getFileNameOnly(file, '\\');
	std::string fNameOnly = fileStack.top();
		std::string initBody = "<h3>"+fNameOnly +"</h3>\n<hr/>\n<div class = \"indent\">\n<h4>Dependencies:\n";
		ost << initBody;
		ost << "\n";
}

//Returns the list of all lines after which appropriate
//button and div tags are added
std::vector<int> CodePublisher::getStartLineVect(std::vector<std::unordered_map<std::string,std::string>> valueVect)
{
	std::vector<int> startLineNo;
	for (std::unordered_map<std::string, std::string> mapInst : valueVect) {
		std::string val = mapInst["startLine"];
		int iVal = std::stoi(val);
		startLineNo.push_back(iVal);
	}
	return startLineNo;
}

//Returns the list of all lines after which appropriate
//div end tags are added
std::vector<int> CodePublisher::getEndLineVect(std::vector<std::unordered_map<std::string, std::string>> valueVect)
{
	std::vector<int> endLineNo;
	for (std::unordered_map<std::string, std::string> mapInst : valueVect) {
		std::string val = mapInst["endLine"];
		int iVal = std::stoi(val);
		endLineNo.push_back(iVal);
	}
	return endLineNo;
}

//prints all button and div tags
//Done for classes,methods and global functions
void CodePublisher::printButtonAndDiv(std::ofstream& ost,std::vector<std::string> elem) {

	for (std::string elemVal : elem) {
		ost << elemVal;
		ost << "\n";
	}
}

//Opens all HTML pages on the browser
void CodePublisher::openFilesInBrowser(std::string htmlFileName,std::string category) {
	std::string path = "file:///" + FileSystem::Path::getFullFileSpec("..\\..\\Repository\\PublishedFiles\\"+ category+"\\"+ htmlFileName + ".htm");
	std::wstring spath = std::wstring(path.begin(), path.end());
	LPCWSTR swpath = spath.c_str();
	LPCWSTR a = L"open";
	LPCWSTR ie = L"iexplore.exe";
	ShellExecute(NULL, a, ie, swpath, NULL, SW_SHOWDEFAULT);
}

//Helper methid for generating tags
void CodePublisher::printHtmlEnd(std::ofstream& myfile) {
	myfile << "</pre>";
	myfile << "\n";
	myfile << "</body>";
	myfile << "\n";
	myfile << "</html>";
	myfile.close();
}

//Used to print prologues for HTML files
void CodePublisher::printHtmlPrologue(std::string htmlFileName,std::ofstream& ost)
{
	std::string htmlPro = " !----------------------------------------------------------------------------\n"
		+ htmlFileName + " - Help file for Project #3\n"
		"Published 19 Mar 2017\n"
		"Jim Fawcett, CSE687 - Object Oriented Design, Spring 2017\n\n\n"
		"Note:\n"
		"-Markup characters in the text part, enclosed in &lt;pre&gt;...&lt;/pre&gt; have to be\n"
		"replaced with escape sequences, e.g., < becomes &lt and > becomes &gt\n"
		"-Be careful that you don't replace genuine markup characters with escape\n"
		"sequences, e.g., everything outside of the &lt;pre&gt;...&lt;/pre&gt; section.\n"
		"----------------------------------------------------------------------------->\n ";
	ost << "<pre>\n";
	ost << htmlPro;
	ost << "</pre>";
	ost << "\n";
}

//helper function for generating tags
void CodePublisher::generateTags(std::ofstream& myfile) {
	myfile << "</div>";
	myfile << "\n";
	myfile << "<hr/>";
	myfile << "\n";
}

//displays line information to identify beginning of classes,methods and global functions
void CodePublisher::displayLineNoInfo(CodePublisher::valVect vect)
{
	for (mapInstance mapINst : vect) {
		std::cout << std::endl;
		for (auto it : mapINst) {
			std::cout << std::setw(20) << it.first << std::setw(20) << it.second << "\n";
		}
		std::cout << "\n";
		std::cout << "\n";
	}
}

#ifdef CODEPUBLISHER
int main() {
	CodeAnalysisExecutive exec;
	DependancyAnalysis dp(exec.depXmlPath, 50);
	CodePublisher cPub(dp);
	TypeTable tb;
	td::unordered_map<std::string, std::vector<std::unordered_map<std::string, std::string>>>& lineMapInst = tb.getLineNumMap();
	cPub.fileCollector(exec,lineMapInst);
}
#endif // CODEPUBLISHER



