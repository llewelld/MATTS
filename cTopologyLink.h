/*$T MATTS/cTopologyLink.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopologyLink.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for Topology *Link* List ;
 * Which also happens to be a linked list ;
 * Members ;
 * pHead - Pointer to the head node ;
 * pTail - Pointer to the tail node ;
 * pCurrent - Pointer to the current node ;
 * count - Total number of nodes ;
 * nextID - ID for the next node ;
 * Methods ;
 */
#ifndef CTOPOLOGYLINK_H
#define CTOPOLOGYLINK_H

#include "cProperties.h"

#define TOPOLOGYLINK_BUFFER_SIZE	(10)

typedef enum
{
	CASCADE_INVALID			= -1,
	CASCADE_VULNERABILITY,
	CASCADE_SAFE,
	CASCADE_LINKOUTSIDERANGE,
	CASCADE_NUM
} CASCADE;

class cTopologyNode;

class cTopologyLink : public cProperties
{
public:
	cTopologyLink (cTopologyNode* psSetLinkFrom, cTopologyNode* psSetLinkTo, int nLinkID);
	~ cTopologyLink ();

	void bringToFront (void);
	CASCADE CheckCascadeLink (void);
	void UpdateCascadeTables (void);

	cTopologyLink*	pFromPrevious;
	cTopologyLink*	pFromNext;

	cTopologyLink*	pToPrevious;
	cTopologyLink*	pToNext;

	cTopologyNode*	psLinkFrom;
	cTopologyNode*	psLinkTo;
	int nChannelOut;
	int nChannelIn;
	int nSensitivityLevel;
	bool boAdHoc;

	char szLinkInput[1024];
	char szLinkOutput[1024];

	int nSecurity;
	int nLinkID;

	int nLinkInIndexTemp;
	int nLinkOutIndexTemp;

	bool sendData (long int nData);
	bool receiveData (long int* pnData);
private:
	long int anBuffer[TOPOLOGYLINK_BUFFER_SIZE];
	int nBufferCount;
};
#endif
