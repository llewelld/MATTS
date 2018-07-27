/*$T MATTS/cTopologyList.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopologyList.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for Topology Linked List ;
 * NOTE: Inherits from cList ;
 * Members ;
 * pHead - Pointer to the head node ;
 * pTail - Pointer to the tail node ;
 * pCurrent - Pointer to the current node ;
 * count - Total number of nodes ;
 * nextID - ID for the next node ;
 * Methods ;
 */
#ifndef CTOPOLOGYLIST_H
#define CTOPOLOGYLIST_H

#include "cList.h"
#include "cTopologyNode.h"

class cTopologyList : public cList
{
public:
	cTopologyList ();
	virtual ~cTopologyList ();
	virtual cTopologyNode*	newNode (int nNodeID);
	cTopologyNode*	getCurrentNode (cTopologyNode* psCurrent);

	char const * cTopologyList::getName (cTopologyNode* psCurrent);
	float getXPos (cTopologyNode* psCurrent);
	float getYPos (cTopologyNode* psCurrent);
	float getZPos (cTopologyNode* psCurrent);
	float getXRot (cTopologyNode* psCurrent);
	float getYRot (cTopologyNode* psCurrent);
	float getZRot (cTopologyNode* psCurrent);
	float getXScale (cTopologyNode* psCurrent);
	float getYScale (cTopologyNode* psCurrent);
	float getZScale (cTopologyNode* psCurrent);
	int getCol (cTopologyNode* psCurrent);
	int getAnalyseCol (cTopologyNode* psCurrent);
	int getShape (cTopologyNode* psCurrent);
	int getType (cTopologyNode* psCurrent);
	cTopologyLink*	getLinksOut (cTopologyNode* psCurrent);
	cTopologyLink*	getLinksIn (cTopologyNode* psCurrent);
protected:

	/*
	 * cTopologyNode *pCurrent;
	 */
};
#endif
