#include "FingerList.h"
#include "FingerVector.h"
#include <vector>

FingerList::FingerList()
{
}

FingerList::FingerList(std::vector<FingerVector> vec)
{
	g_fingers = vec; 
}

FingerVector FingerList::operator[] (int a)
{ 
	return g_fingers[a]; 
}
	
int FingerList::size()
{ 
	return g_fingers.size();
}
	
void FingerList::add(FingerVector p)
{ 
	g_fingers.push_back(p); 
}

void FingerList::clear()
{
	g_fingers.clear();
}