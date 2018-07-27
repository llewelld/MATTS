/*$T MATTS/cList.h GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cList.h ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Header file for Linked List ;
 * NOTE: none ;
 * Members ;
 * pHead - Pointer to the head node ;
 * pTail - Pointer to the tail node ;
 * pCurrent - Pointer to the current node ;
 * count - Total number of nodes ;
 * nextID - ID for the next node ;
 * Methods ;
 * startIterator() - Moves list to the begining node ;
 * endIterator() - Moves list to the end node ;
 * moveForward() - Moves to next node ;
 * moveBackward() - Moves to previous node ;
 * getCurrentItem() - Returns a pointer to current nodes VM ;
 * getCurrentID() - Returns ID of current node ;
 * isIteratorValid() - Returns true if current node is valid ;
 * removeCurrentItem() - Removes current node ;
 * newNode() - Creates a new node ;
 * getCount() - Returns count ;
 * getNextID() - Returns next ID ;
 */
#ifndef CLIST_H
#define CLIST_H

#include "cNode.h"
#include "cTopologyNode.h"

#define NEXT_ID_START	(0)

class cList
{
public:
	cList ();
	virtual ~cList ();
	virtual cTopologyNode*	startIterator ();
	virtual cTopologyNode*	endIterator ();
	virtual cTopologyNode*	moveForward (cTopologyNode* psCurrent);
	virtual cTopologyNode*	moveBackward (cTopologyNode* psCurrent);
	virtual cVM*  getCurrentItem (cTopologyNode* psCurrent);
	int getCurrentID (cTopologyNode* psCurrent);
	bool isIteratorValid (cTopologyNode* psCurrent);
	cTopologyNode*	removeCurrentItem (cTopologyNode* psCurrent);
	virtual cTopologyNode* cList:: newNode (int nNodeID);
	int getCount ();
	int getNextID ();
	void resetNextID ();
protected:
	cTopologyNode*	psHead;
	cTopologyNode*	psTail;
	int nCount;
	int nNextID;
};
#endif
