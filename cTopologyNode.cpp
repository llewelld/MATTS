/*$T MATTS/cTopologyNode.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopologyNode.cpp ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Implementation file for Topology Node class ;
 */
#include <stdlib.h>
#include "cTopologyNode.h"

const int cTopologyNode:: ganRisk[SEC_NUM][SEC_NUM] =

/*
 *         Max ;
 * U, N, C, S, T, 1, M
 */
{{ 0, 1, 2, 3, 4, 5, 6 },  /* U   */
 { 0, 0, 1, 2, 4, 5, 6 },  /* N   */
 { 0, 0, 0, 1, 3, 4, 5 },  /* C M */
 { 0, 0, 0, 0, 2, 3, 4 },  /* S i */
 { 0, 0, 0, 0, 0, 2, 3 },  /* T n */
 { 0, 0, 0, 0, 0, 0, 1 },  /* 1   */
 { 0, 0, 0, 0, 0, 0, 0 }}; /* M */

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cTopologyNode::cTopologyNode (int ID) : cNode (ID), cProperties () {

	/* Geo render coordinates */
	fXPos = ((float) ((rand () % 100) - 50) / 1000.0f);
	fYPos = ((float) ((rand () % 100) - 50) / 1000.0f);
	fZPos = ((float) ((rand () % 100) - 50) / 1000.0f);

	/* Net render coordinates */
	fXPosNet = fXPos;
	fYPosNet = fYPos;
	fZPosNet = fZPos;
	fXPosNetRender = fXPosNet;
	fYPosNetRender = fYPosNet;
	fZPosNetRender = fZPosNet;

	fXVel = 0.0f;
	fYVel = 0.0f;
	fZVel = 0.0f;

	fXRot = (0.0f * ROT_SCALE);
	fYRot = (0.0f * ROT_SCALE);
	fZRot = (180.0f * ROT_SCALE);
	fXScale = (X_SIZE * SCALE_SCALE);
	fYScale = (Y_SIZE * SCALE_SCALE);
	fZScale = (Z_SIZE * SCALE_SCALE);
	nCol = NODECOL_DEFAULT;
	nAnalyseCol = 0;
	nShape = SHAPE_SPHERE;
	boAdHoc = false;
	fRangeTx = 30.0f;
	fRangeRx = 0.0f;

	pLinksOut = NULL;
	pLinksIn = NULL;

	nControlID = -1;

	szCertProps = (char*) malloc (1024);
	szCertProps[0] = 0;
	nSecurityMax = 0;
	nSecurityMin = 0;
	nSensitivityLevel = 9;

	szStaffSkills = (char*) malloc (STAFF_SKILLS);
	szStaffSkills[0] = 0;
	szEncryptAlgorithm = (char*) malloc (ENCRYPT_ALGORITHM);
	szEncryptAlgorithm[0] = 0;

	strncpy (szEncryptAlgorithm, "Unspecified", ENCRYPT_ALGORITHM);
	strncpy (szStaffSkills, "Low", STAFF_SKILLS);

	boFirewall = false;
	boIDS = false;
	boExternal = false;
	szData = (char*) malloc (DATA_NAMELEN);
	szData[0] = 0;

	/* Default type is external */
	nType = 0;
	PropertySetString ("Name", "Unknown Node");

	NodeSecurityReset ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyNode::~cTopologyNode () {

	/* Delete all of the links out */
	while (pLinksOut) {
		delete pLinksOut;
	}

	while (pLinksIn) {
		delete pLinksIn;
	}

	free (szCertProps);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyNode::NodeSecurityReset () {
	int nMin;
	int nMax;

	for (nMax = SEC_MIN; nMax <= SEC_MAX; nMax++) {
		for (nMin = SEC_MIN; nMin <= SEC_MAX; nMin++) {
			if (((nMax <= nSecurityMax) && (nMax >= nSecurityMin)) && ((nMin <= nSecurityMax) && (nMin >= nSecurityMin))) {
				if (nMax > nMin) {
					anRiskIn[nMin][nMax] = ganRisk[nSecurityMin][nSecurityMax];
					anRiskOut[nMin][nMax] = ganRisk[nSecurityMin][nSecurityMax];
				} else {
					anRiskIn[nMin][nMax] = 0;
					anRiskOut[nMin][nMax] = 0;
				}
			} else {
				anRiskIn[nMin][nMax] = RISK_INFINITY;
				anRiskOut[nMin][nMax] = RISK_INFINITY;
			}
		}
	}

	virtualMachine->ConsolePrint ("Reset Input table\n");
	PrintInputTable ();
	virtualMachine->ConsolePrint ("Reset Ouput table\n");
	PrintInputTable ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyNode::UpdateCascadeInputTableRecurse (void) {
	cTopologyLink*	pLinkOut;

	if (UpdateCascadeInputTable ()) {

		/* If the table has changed, we need to make sure we update any downstream nodes */
		pLinkOut = pLinksOut;
		while (pLinkOut) {
			pLinkOut->psLinkTo->UpdateCascadeInputTableRecurse ();
			pLinkOut = pLinkOut->pFromNext;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopologyNode::UpdateCascadeInputTable (void) {

	/* Update the Input table of the node */
	int nMax;
	int nMin;
	int nMinRisk;
	int nRisk;
	int nNodeRisk;
	bool boChange;
	cTopologyLink*	pLinkIn;
	cTopologyNode*	pUpstreamNode;

	boChange = false;
	nNodeRisk = ganRisk[nSecurityMin][nSecurityMax];
	for (nMax = SEC_MIN; nMax <= SEC_MAX; nMax++) {
		for (nMin = nSecurityMin; (nMin < MIN (nMax, (nSecurityMax + 1))); nMin++) {
			nMinRisk = RISK_INFINITY;

			/* Cycle through all of the links into the node */
			if (pLinksIn) {
				pLinkIn = pLinksIn;

				while (pLinkIn) {
					pUpstreamNode = pLinkIn->psLinkFrom;
					if (pLinkIn->nSecurity > nMin) {
						nRisk = pUpstreamNode->anRiskIn[pLinkIn->nSecurity][nMax];
						if (nNodeRisk > nRisk) {
							nRisk = nNodeRisk;
						}
					} else {
						nRisk = pUpstreamNode->anRiskIn[pLinkIn->nSecurity][nMax];
					}

					if (nRisk < nMinRisk) {
						nMinRisk = nRisk;
					}

					pLinkIn = pLinkIn->pToNext;
				}

				if ((nNodeRisk < nMinRisk) && (nMax <= nSecurityMax) && (nMin >= nSecurityMin)) {
					nMinRisk = nNodeRisk;
				}
			} else {
				if ((nMax <= nSecurityMax) && (nMin >= nSecurityMin)) {
					nMinRisk = nNodeRisk;
				} else {
					nMinRisk = RISK_INFINITY;
				}
			}

			/* Now we have the risk of the minimum risk path */
			if (anRiskIn[nMin][nMax] != nMinRisk) {
				boChange = true;
			}

			anRiskIn[nMin][nMax] = nMinRisk;
		}
	}

	/* Print the result */
	if (boChange) {
		virtualMachine->ConsolePrint ("Updated Input table\n");
		PrintInputTable ();
	} else {
		virtualMachine->ConsolePrint ("Input table not changed\n");
	}

	return boChange;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyNode::PrintInputTable (void) {
	int nMax;
	int nMin;
	char szTable[32];

	for (nMin = SEC_MIN; nMin <= SEC_MAX; nMin++) {
		szTable[0] = 0;
		for (nMax = SEC_MIN; nMax <= SEC_MAX; nMax++) {
			if (anRiskIn[nMin][nMax] == RISK_INFINITY) {
				virtualMachine->ConsolePrint (" x");
			} else {
				sprintf (szTable, " %d", anRiskIn[nMin][nMax]);
				virtualMachine->ConsolePrint (szTable);
			}
		}

		virtualMachine->ConsolePrint ("\n");
	}

	virtualMachine->ConsolePrint ("\n");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyNode::UpdateCascadeOutputTableRecurse (void) {
	cTopologyLink*	pLinkIn;

	if (UpdateCascadeOutputTable ()) {

		/* If the table has changed, we need to make sure we update any downstream nodes */
		pLinkIn = pLinksIn;
		while (pLinkIn) {
			pLinkIn->psLinkFrom->UpdateCascadeOutputTableRecurse ();
			pLinkIn = pLinkIn->pToNext;
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cTopologyNode::UpdateCascadeOutputTable (void) {

	/* Update the Output table of the node */
	int nMax;
	int nMin;
	int nMinRisk;
	int nRisk;
	int nNodeRisk;
	bool boChange;
	cTopologyLink*	pLinkOut;
	cTopologyNode*	pDownstreamNode;

	boChange = false;

	nNodeRisk = ganRisk[nSecurityMin][nSecurityMax];
	for (nMax = nSecurityMin; nMax <= nSecurityMax; nMax++) {
		for (nMin = SEC_MIN; nMin < nMax; nMin++) {
			nMinRisk = RISK_INFINITY;

			/* Cycle through all of the links out of the node */
			if (pLinksOut) {
				pLinkOut = pLinksOut;

				while (pLinkOut) {
					pDownstreamNode = pLinkOut->psLinkTo;
					if (pLinkOut->nSecurity < nMax) {
						nRisk = pDownstreamNode->anRiskOut[nMin][pLinkOut->nSecurity];
						if (nNodeRisk > nRisk) {
							nRisk = nNodeRisk;
						}
					} else {
						nRisk = pDownstreamNode->anRiskOut[nMin][pLinkOut->nSecurity];
					}

					if (nRisk < nMinRisk) {
						nMinRisk = nRisk;
					}

					pLinkOut = pLinkOut->pFromNext;
				}

				if ((nNodeRisk < nMinRisk) && (nMax <= nSecurityMax) && (nMin >= nSecurityMin)) {
					nMinRisk = nNodeRisk;
				}
			} else {
				if ((nMax <= nSecurityMax) && (nMin >= nSecurityMin)) {
					nMinRisk = nNodeRisk;
				} else {
					nMinRisk = RISK_INFINITY;
				}
			}

			/* Now we have the risk of the minimum risk path */
			if (anRiskOut[nMin][nMax] != nMinRisk) {
				boChange = true;
			}

			anRiskOut[nMin][nMax] = nMinRisk;
		}
	}

	/* Print the result */
	if (boChange) {
		virtualMachine->ConsolePrint ("Updated Output table\n");
		PrintOutputTable ();
	} else {
		virtualMachine->ConsolePrint ("Output table not changed\n");
	}

	return boChange;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyNode::PrintOutputTable (void) {
	int nMax;
	int nMin;
	char szTable[32];

	for (nMin = SEC_MIN; nMin <= SEC_MAX; nMin++) {
		szTable[0] = 0;
		for (nMax = SEC_MIN; nMax <= SEC_MAX; nMax++) {
			if (anRiskOut[nMin][nMax] == RISK_INFINITY) {
				virtualMachine->ConsolePrint (" x");
			} else {
				sprintf (szTable, " %d", anRiskOut[nMin][nMax]);
				virtualMachine->ConsolePrint (szTable);
			}
		}

		virtualMachine->ConsolePrint ("\n");
	}

	virtualMachine->ConsolePrint ("\n");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyLink* cTopologyNode::findLinkWithChannelOut (int nChannelOut) {
	cTopologyLink*	pLink;

	pLink = pLinksOut;
	while ((pLink) && (pLink->nChannelOut != nChannelOut)) {
		pLink = pLink->pFromNext;
	}

	return pLink;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyLink* cTopologyNode::findLinkWithChannelIn (int nChannelIn) {
	cTopologyLink*	pLink;

	pLink = pLinksIn;
	while ((pLink) && (pLink->nChannelIn != nChannelIn)) {
		pLink = pLink->pToNext;
	}

	return pLink;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char const * cTopologyNode::GetName () {
	return PropertyGetString ("Name");
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyNode::SetName (char const * szName) {
	PropertySetString ("Name", szName);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyNode::SetColour (int nColour) {
	this->nCol = nColour;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cTopologyNode::ApplyCharacteristics (void) {
	if (boExternal) {
		nCol = NODECOL_EXTERNAL;
	} else {
		nCol = NODECOL_DEFAULT;
	}
}
