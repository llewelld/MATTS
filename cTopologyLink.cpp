/*$T MATTS/cTopologyLink.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopologyLink.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Organise data concerning links between virtual machines ;
 * NOTE: This is a double linked list ;
 */
#include "cTopology.h"
#include "cTopologyLink.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cTopologyLink::cTopologyLink (cTopologyNode* psSetLinkFrom, cTopologyNode* psSetLinkTo, int nLinkID) {

	/* Set up "Out/From" details */
	pFromPrevious = NULL;
	pFromNext = NULL;
	if (psSetLinkFrom) {
		pFromNext = psSetLinkFrom->pLinksOut;
		psSetLinkFrom->pLinksOut = this;
	}

	if (pFromNext) {
		pFromNext->pFromPrevious = this;
	}

	psLinkFrom = psSetLinkFrom;
	nChannelOut = 0;
	szLinkOutput[0] = 0;

	/* Set up "In/To" details */
	pToPrevious = NULL;
	pToNext = NULL;
	if (psSetLinkTo) {
		pToNext = psSetLinkTo->pLinksIn;
		psSetLinkTo->pLinksIn = this;
	}

	if (pToNext) {
		pToNext->pToPrevious = this;
	}

	psLinkTo = psSetLinkTo;
	nChannelIn = 0;
	szLinkInput[0] = 0;
	this->nLinkID = nLinkID;

	nSensitivityLevel = 0;
	nSecurity = 0;

	/* Start with an empty buffer */
	nBufferCount = 0;
	boAdHoc = false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyLink::~cTopologyLink () {

	/* Set up "Out/From" details */
	if (psLinkFrom) {
		if (psLinkFrom->pLinksOut == this) {
			psLinkFrom->pLinksOut = pFromNext;
		}
	}

	if (pFromPrevious) {
		pFromPrevious->pFromNext = pFromNext;
	}

	if (pFromNext) {
		pFromNext->pFromPrevious = pFromPrevious;
	}

	/* Set up "In/To" details */
	if (psLinkTo) {
		if (psLinkTo->pLinksIn == this) {
			psLinkTo->pLinksIn = pToNext;
		}
	}

	if (pToPrevious) {
		pToPrevious->pToNext = pToNext;
	}

	if (pToNext) {
		pToNext->pToPrevious = pToPrevious;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
CASCADE cTopologyLink::CheckCascadeLink (void) {

	/*
	 * Check for cascade vulnerabilities ;
	 * based on the security levels in the in and out tables ;
	 * of the nodes at either end and the security level of the node
	 */
	CASCADE eResult;
	int nMax;
	int nMin;
	int nRisk;

	eResult = CASCADE_SAFE;

	/* The security level of the link must fall within that of the nodes at either end */
	if (psLinkFrom && psLinkTo) {
		if ((nSecurity > psLinkFrom->nSecurityMax) || (nSecurity < psLinkFrom->nSecurityMin)
		|| (nSecurity > psLinkTo->nSecurityMax) || (nSecurity < psLinkTo->nSecurityMin)) {
			eResult = CASCADE_LINKOUTSIDERANGE;
		}

		/* Now the tricky part */
		if (eResult == CASCADE_SAFE) {
			for (nMax = (int) SEC_MAX; nMax >= nSecurity; nMax--) {
				for (nMin = nSecurity; nMin >= SEC_MIN; nMin--) {

					/* Ensure the check falls within the bounds of a possible path */
					if ((psLinkFrom->anRiskIn[nSecurity][nMax] != RISK_INFINITY)
					&& (psLinkTo->anRiskOut[nMin][nSecurity] != RISK_INFINITY)) {

						/* Establish the risk of this downgrade */
						nRisk = psLinkFrom->ganRisk[nMin][nMax];

						/* Check if there is a cascade path for these security levels */
						if ((nRisk > psLinkFrom->anRiskIn[nSecurity][nMax]) && (nRisk > psLinkTo->anRiskOut[nMin][nSecurity])) {
							eResult = CASCADE_VULNERABILITY;
						}
					}
				}
			}
		}
	}

	return eResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyLink::UpdateCascadeTables (void) {

	/*
	 * Update the Input and Output tables based on the given link ;
	 * Update the Input table of the downstream nodes ;
	 * This needs to be performed recursively
	 */
	if (psLinkTo) {
		psLinkTo->UpdateCascadeInputTableRecurse ();
	}

	/*
	 * Update the Output table of the upstream nodes ;
	 * This needs to be performed recursively
	 */
	if (psLinkFrom) {
		psLinkFrom->UpdateCascadeOutputTableRecurse ();
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyLink::bringToFront (void) {
	cTopologyLink*	pLast;

	if ((this->psLinkFrom) && (this->pFromNext)) {
		pLast = this->psLinkFrom->pLinksOut;
		while (pLast->pFromNext) {
			pLast = pLast->pFromNext;
		}

		this->pFromNext->pFromPrevious = this->pFromPrevious;
		if (this->pFromPrevious) {
			this->pFromPrevious->pFromNext = this->pFromNext;
		}

		pLast->pFromNext = this;
		this->pFromPrevious = pLast;

		if (this->psLinkFrom->pLinksOut == this) {
			this->psLinkFrom->pLinksOut = this->pFromNext;
		}

		this->pFromNext = NULL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopologyLink::sendData (long int nData) {
	bool boReturn;

	if (nBufferCount < TOPOLOGYLINK_BUFFER_SIZE) {
		anBuffer[nBufferCount] = nData;
		nBufferCount++;
		boReturn = true;
	} else {
		boReturn = false;
	}

	return boReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopologyLink::receiveData (long int* pnData) {
	bool boReturn;
	int nCount;

	if (nBufferCount > 0) {
		*pnData = anBuffer[0];
		for (nCount = 1; nCount < TOPOLOGYLINK_BUFFER_SIZE; nCount++) {
			anBuffer[nCount - 1] = anBuffer[nCount];
		}

		nBufferCount--;
		boReturn = true;
	} else {
		boReturn = false;
	}

	return boReturn;
}
