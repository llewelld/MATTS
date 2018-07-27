/*$T MATTS/cList.cpp GC 1.140 07/01/09 21:12:08 */
/*
 * ;
 * Name: cList.cpp ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Implementation file for list class ;
 */
#include "cList.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cList::cList () {
	psHead = NULL;
	psTail = NULL;
	nNextID = nCount = NEXT_ID_START;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cList::~cList () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cList::resetNextID () {
	nNextID = nCount = NEXT_ID_START;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode* cList::startIterator () {
	return psHead;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode* cList::endIterator () {
	return psTail;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode* cList::moveForward (cTopologyNode* psCurrent) {
	return (psCurrent ? (cTopologyNode*) psCurrent->next : NULL);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode* cList::moveBackward (cTopologyNode* psCurrent) {
	return (psCurrent ? (cTopologyNode*) psCurrent->previous : NULL);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cVM* cList::getCurrentItem (cTopologyNode* psCurrent) {
	return (psCurrent ? psCurrent->virtualMachine : NULL);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cList::isIteratorValid (cTopologyNode* psCurrent) {
	return (psCurrent != NULL);
}

/*
 =======================================================================================================================
 *  Function get call to re-link and deleted selected node with associated links and redraw the topology ;
 *  called in the cWindow.cpp -> void cWindow::DeleteSelecteNode (void) functions
 =======================================================================================================================
 */
cTopologyNode* cList::removeCurrentItem (cTopologyNode* psCurrent) {
	cTopologyNode*	psReturn;

	if (psCurrent) {

		/* Valid node */
		if (psCurrent == psHead) {

			/* Node is head */
			if (psCurrent == psTail) {

				/* Node is also tail (only node in list) */
				psHead = NULL;
				psTail = NULL;
				psReturn = NULL;
				delete psCurrent;

				/* Current node removed. */
			} else {

				/* Node has a next node */
				psReturn = (cTopologyNode*) psCurrent->next;
				psHead = psReturn;
				psHead->previous = NULL;
				delete psCurrent;

				/* Current node removed. */
			}
		} else {

			/* Node is not head */
			if (psCurrent == psTail) {

				/* Node is tail */
				psReturn = (cTopologyNode*) psCurrent->previous;
				psTail = psReturn;
				psTail->next = NULL;
				delete psCurrent;

				/* Current node removed. */
			} else {

				/* Node is a standard node */
				psCurrent->previous->next = psCurrent->next;
				psCurrent->next->previous = psCurrent->previous;
				psReturn = (cTopologyNode*) psCurrent->previous;
				delete psCurrent;

				/* Current node removed. */
			}
		}

		nCount--;

		/*
		 * nNextID--;
		 */
	}

	return psReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode* cList::newNode (int nNodeID) {
	cTopologyNode*	psNode;

	if (nNodeID < 0) {
		nNodeID = nNextID;
	}

	nNextID = MAX((nNodeID + 1), nNextID);

	psNode = (cTopologyNode*) new cNode (nNodeID);
	psNode->next = NULL;

	if (psHead) {
		psTail->next = psNode;
		psNode->previous = psTail;
		psTail = psNode;
	} else {

		/* List empty */
		psHead = psNode;
		psTail = psNode;
		psNode->previous = NULL;
	}

	nCount++;

	return psNode;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cList::getCount () {
	return nCount;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cList::getNextID () {
	return nNextID++;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cList::getCurrentID (cTopologyNode* psCurrent) {
	return (psCurrent ? psCurrent->getID () : 0);
}
