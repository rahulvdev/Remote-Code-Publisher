#pragma once
#pragma once
///////////////////////////////////////////////////////////////////////////////////////////
//// DisplayEx.h - Displays the type table, dependency table and SCC                     //
////                                                                                    //
//// Rahul Vijaydev,CSE687 - Object Oriented Design, Spring 2017					   // 
////                                                                                  //
///////////////////////////////////////////////////////////////////////////////////////
///*
//Module Operations:
//==================
//This module defines a DisplayItems Class
//DisplayItems class provides functions to:
//show the type table
//show the dependency table
//show Strongly connected Components
//
//Public Interface:
//=================
//void displayTypeTable(TypeTable &objtypeTable);
//void displaDependancyTable(DependencyTable &objDepTable);
//Disp();
//* Required Files:
//* ---------------
//*   - Typtable.h
//*   - DependencyAnalysis.h
//*   -StronglyConnectedComp.h
//*
//* Build Process:
//* --------------
//*   devenv DisplayEx.sln /debug rebuild
//*
//* Maintenance History:
//* --------------------
//ver 0.1 : 7nd March 2017
//*
///*
//* -
//*/

#include"../TypeTable/typetable.h"
#include "../DependencyAnalyzer/DependencyAnalysis.h"
#include "../StronglyConnectedComp/StronglyConnectedComp.h"


	class DisplayItems
	{
	public:
		void displayTypeTable(TypeAnalysis& typTable);
		void displayDependancyTable(DependancyAnalysis& depTable);
		DisplayItems();

	};
