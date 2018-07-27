/*$T MATTS/cNode.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cNode.h ;
 * Last Modified: 10/11/04 ;
 * ;
 * Purpose: Header file for Linked List Node ;
 * Members ;
 * next - Pointer to next node ;
 * previous - Pointer to previous node ;
 * virtualMachine - Pointer to the node's Machine ;
 * id - ID of current node ;
 * Methods ;
 * getID() - Returns the node's ID ;
 * DecID() - Decrements the node's ID ;
 */
#ifndef CNODE_H
#define CNODE_H

#include "cVM.h"

class cNode
{
public:
	cNode (int ID);
	virtual ~cNode ();

	cNode*	next;
	cNode*	previous;
	cVM*  virtualMachine;
	int getID ()	{ return id; }

	void DecID ()	{ id--; }

private:
	int id;
};
#endif
