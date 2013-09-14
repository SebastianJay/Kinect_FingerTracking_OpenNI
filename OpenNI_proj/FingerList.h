#ifndef __FINGER_LIST_INCLUDED__
#define __FINGER_LIST_INCLUDED__

#include "FingerVector.h"
#include <vector>

//wrapper class for std::vector<FingerVector>
//only necessary methods are implemented
/**
FingerList is a wrapper class for std::vector<FingerVector>, exposing all the necessary functions such as element access, adding, and clearing. No equivalent for std::vector<FingerVector>::iterator is provided, so looping through the list must be done through element access through square bracket notation, with an incrementing int.
*/
struct FingerList{
public:
	//struct iterator{
	//};

	/** 
	Default constructor
	*/
	FingerList();
	/**
	Constructor accepting an std::vector as initial values
	*/
	FingerList(std::vector<FingerVector>);
	
	FingerVector operator[] (int);	
	/**
	Provides number of elements in list
	*/
	int size();	
	/**
	Adds new element to list
	*/
	void add(FingerVector);
	/**
	Removes all elements from the list
	*/
	void clear();

private:
	std::vector<FingerVector> g_fingers;
};

#endif