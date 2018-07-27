/*$T MATTS/cTopologyMirror.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopologyMirror.cpp ;
 * Last Modified: 12/09/08 ;
 * ;
 * Purpose: Provide a mirrored version of the topology including all properties ;
 * needed for an analysis. Ensures that the analysis is safe even if ;
 * the real topology changes. ;
 */
#include "cTopologyMirror.h"
#include "cTopologyList.h"
#include <malloc.h>

/**
 =======================================================================================================================
 *  cTopologyMirrorNode
 =======================================================================================================================
 */
cTopologyMirrorNode::cTopologyMirrorNode (cTopologyNode const * const psNode, int nEnumeration) {
	this->nEnumerationTemp = nEnumeration;

	/* Reset all of the variables to default values */
	virtualMachine = NULL;
	nSensitivityLevel = 0;
	szEncryptAlgorithm = NULL;
	szStaffSkills = NULL;
	boFirewall = false;
	boIDS = false;
	boExternal = false;
	szCertProps = NULL;

	/* Make a copy of the cTopologyNode object */
	if (psNode) {
		// Copy across the properties
		PropertyCopy (psNode);

		virtualMachine = psNode->virtualMachine;
		nSensitivityLevel = psNode->nSensitivityLevel;
		if (psNode->szEncryptAlgorithm) {
			szEncryptAlgorithm = (char*) malloc (strlen (psNode->szEncryptAlgorithm) + 1);
			strcpy (szEncryptAlgorithm, psNode->szEncryptAlgorithm);
		}

		if (psNode->szStaffSkills) {
			szStaffSkills = (char*) malloc (strlen (psNode->szStaffSkills) + 1);
			strcpy (szStaffSkills, psNode->szStaffSkills);
		}

		boFirewall = psNode->boExternal;
		boIDS = psNode->boIDS;
		boExternal = psNode->boExternal;
		if (psNode->szCertProps) {
			szCertProps = (char*) malloc (strlen (psNode->szCertProps) + 1);
			strcpy (szCertProps, psNode->szCertProps);
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyMirrorNode::~cTopologyMirrorNode () {
	if (virtualMachine) {

		/* Don't free the virtual machine! */
		virtualMachine = NULL;
	}

	if (szEncryptAlgorithm) {
		free (szEncryptAlgorithm);
		szEncryptAlgorithm = NULL;
	}

	if (szStaffSkills) {
		free (szStaffSkills);
		szStaffSkills = NULL;
	}

	if (szCertProps) {
		free (szCertProps);
		szCertProps = NULL;
	}
}

/**
 =======================================================================================================================
 *  cTopologyMirrorLink
 =======================================================================================================================
 */
cTopologyMirrorLink::cTopologyMirrorLink (cTopologyLink const *const psLink, int nLinkOutIndex, int nLinkInIndex,
										  cTopologyMirrorNode *const *const apsBox) {
	nLinkOutIndexTemp = nLinkOutIndex;
	nLinkInIndexTemp = nLinkInIndex;

	/* Reset all of the variables to default values */
	psLinkFrom = NULL;
	nChannelIn = 0;
	nChannelOut = 0;
	psLinkTo = NULL;
	szLinkInput = NULL;
	szLinkOutput = NULL;
	nSensitivityLevel = 0;

	/* Make a copy of the cTopologyNode object */
	if (psLink) {
		// Copy across the properties
		PropertyCopy (psLink);

		psLinkFrom = apsBox[psLink->psLinkFrom->nEnumerationTemp + LINKLIST_OFFSET];
		nChannelIn = psLink->nChannelIn;
		nChannelOut = psLink->nChannelOut;
		psLinkTo = apsBox[psLink->psLinkTo->nEnumerationTemp + LINKLIST_OFFSET];
		if (psLink->szLinkInput) {
			szLinkInput = (char*) malloc (strlen (psLink->szLinkInput) + 1);
			strcpy (szLinkInput, psLink->szLinkInput);
		}

		if (psLink->szLinkOutput) {
			szLinkOutput = (char*) malloc (strlen (psLink->szLinkOutput) + 1);
			strcpy (szLinkOutput, psLink->szLinkOutput);
		}

		nSensitivityLevel = psLink->nSensitivityLevel;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyMirrorLink::~cTopologyMirrorLink () {
	if (szLinkInput) {
		free (szLinkInput);
		szLinkInput = NULL;
	}

	if (szLinkOutput) {
		free (szLinkOutput);
		szLinkOutput = NULL;
	}
}

/**
 =======================================================================================================================
 *  cTopologyMirror
 =======================================================================================================================
 */
cTopologyMirror::cTopologyMirror (cTopology* psTopology) {
	cTopologyList*	pList;
	int nCount;
	int nLink;
	int nLinkEnumeration;
	cTopologyNode*	psNode;
	cTopologyLink*	pLinks;
	cTopologyNode*	psIterator;

	/* Ensure everything is reset to default values */
	anBoxLinkInCount = NULL;
	anBoxLinkOutCount = NULL;
	apsBox = NULL;
	appsBoxLinkBoxOut = NULL;
	appsBoxLinkBoxIn = NULL;
	apsLink = NULL;

	pList = psTopology->GetpList ();

	if (pList) {
		nPoints = pList->getCount ();

		/* Allocate and clear memory for the nodes and count lists */
		apsBox = (cTopologyMirrorNode**) calloc ((nPoints + LINKLIST_OFFSET), (sizeof (cTopologyMirrorNode *)));
		anBoxLinkInCount = (int*) calloc ((nPoints + LINKLIST_OFFSET), (sizeof (int)));
		anBoxLinkOutCount = (int*) calloc ((nPoints + LINKLIST_OFFSET), (sizeof (int)));
		appsBoxLinkBoxOut = (cTopologyMirrorLink***) calloc ((nPoints + LINKLIST_OFFSET),
															  (sizeof (cTopologyMirrorLink **)));
		appsBoxLinkBoxIn = (cTopologyMirrorLink***) calloc ((nPoints + LINKLIST_OFFSET),
															 (sizeof (cTopologyMirrorLink **)));

		/* Create a copy of each node */
		nLinks = 0;
		nCount = 0;
		psIterator = pList->startIterator ();
		while (pList->isIteratorValid (psIterator)) {
			psNode = pList->getCurrentNode (psIterator);
			psNode->nEnumerationTemp = nCount;

			/* Create a copy and give the mirrored node an enumeration number */
			apsBox[nCount + LINKLIST_OFFSET] = new cTopologyMirrorNode (psNode, nCount);

			/* Enumerate all of the links out */
			pLinks = pList->getLinksOut (psIterator);
			while (pLinks) {
				nLinks++;
				pLinks->nLinkOutIndexTemp = anBoxLinkOutCount[nCount + LINKLIST_OFFSET];
				anBoxLinkOutCount[nCount + LINKLIST_OFFSET]++;
				pLinks = pLinks->pFromNext;
			}

			/* Allocate and clear memory for the list of output links */
			appsBoxLinkBoxOut[nCount + LINKLIST_OFFSET] = (cTopologyMirrorLink**) calloc (anBoxLinkOutCount[nCount + LINKLIST_OFFSET],
																						   sizeof (cTopologyMirrorLink *));

			/* Enumerate all of the links in */
			pLinks = pList->getLinksIn (psIterator);
			while (pLinks) {
				pLinks->nLinkInIndexTemp = anBoxLinkInCount[nCount + LINKLIST_OFFSET];
				anBoxLinkInCount[nCount + LINKLIST_OFFSET]++;
				pLinks = pLinks->pToNext;
			}

			/* Allocate and clear memory for the list of input links */
			appsBoxLinkBoxIn[nCount + LINKLIST_OFFSET] = (cTopologyMirrorLink**) calloc (anBoxLinkInCount[nCount + LINKLIST_OFFSET],
																						  sizeof (cTopologyMirrorLink *));

			/* Move to next node */
			nCount++;
			psIterator = pList->moveForward (psIterator);
		}

		/* Allocate and clear memory for all of the links */
		apsLink = (cTopologyMirrorLink**) calloc (nLinks, sizeof (cTopologyMirrorLink *));

		/* Enumerate all of the links between the VMs */
		nCount = 0;
		nLinkEnumeration = 0;
		psIterator = pList->startIterator ();
		while (pList->isIteratorValid (psIterator)) {

			/* Enumerate all of the links out and create cTopologyMirrorLink objects */
			nLink = 0;
			pLinks = pList->getLinksOut (psIterator);
			while (pLinks) {
				apsLink[nLinkEnumeration] = new cTopologyMirrorLink (pLinks, pLinks->nLinkOutIndexTemp,
																	 pLinks->nLinkInIndexTemp, apsBox);
				(appsBoxLinkBoxOut[nCount + LINKLIST_OFFSET])[nLink] = apsLink[nLinkEnumeration];
				nLink++;
				nLinkEnumeration++;
				pLinks = pLinks->pFromNext;
			}

			nCount++;
			psIterator = pList->moveForward (psIterator);
		}

		/* Link up the output links with the input links */
		nCount = 0;
		psIterator = pList->startIterator ();
		while (pList->isIteratorValid (psIterator)) {

			/*
			 * Enumerate all of the links in and link them to the created cTopologyMirrorLink
			 * objects
			 */
			nLink = 0;
			pLinks = pList->getLinksIn (psIterator);
			while (pLinks) {

				/* Link the input with the output link */
				(appsBoxLinkBoxIn[nCount + LINKLIST_OFFSET])[nLink] = (appsBoxLinkBoxOut[(pLinks->psLinkFrom->nEnumerationTemp) + LINKLIST_OFFSET])[pLinks->nLinkOutIndexTemp];
				nLink++;
				pLinks = pLinks->pToNext;
			}

			nCount++;
			psIterator = pList->moveForward (psIterator);
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cTopologyMirror::~cTopologyMirror () {
	int nCount;

	if (anBoxLinkInCount) {

		/* Free up the memory allocated to the list */
		free (anBoxLinkInCount);
		anBoxLinkInCount = NULL;
	}

	if (anBoxLinkOutCount) {

		/* Free up the memory allocated to the list */
		free (anBoxLinkOutCount);
		anBoxLinkOutCount = NULL;
	}

	if (apsBox) {

		/* Delete all of the cTopologyMirrorNode objects */
		for (nCount = 0; nCount < nPoints; nCount++) {
			if (apsBox[nCount + LINKLIST_OFFSET]) {
				delete apsBox[nCount + LINKLIST_OFFSET];
				apsBox[nCount + LINKLIST_OFFSET] = NULL;
			}
		}

		/* Free up the memory allocated to the list */
		free (apsBox);
		apsBox = NULL;
	}

	if (appsBoxLinkBoxOut) {

		/* Free up the memory for the list of links pointed to by each element */
		for (nCount = 0; nCount < (nPoints + LINKLIST_OFFSET); nCount++) {
			if (appsBoxLinkBoxOut[nCount]) {
				free (appsBoxLinkBoxOut[nCount]);
				appsBoxLinkBoxOut[nCount] = NULL;
			}
		}

		/* Free up the memory allocated to the list */
		free (appsBoxLinkBoxOut);
		apsBox = NULL;
	}

	if (appsBoxLinkBoxIn) {

		/* Free up the memory for the list of links pointed to by each element */
		for (nCount = 0; nCount < (nPoints + LINKLIST_OFFSET); nCount++) {
			if (appsBoxLinkBoxIn[nCount]) {
				free (appsBoxLinkBoxIn[nCount]);
				appsBoxLinkBoxIn[nCount] = NULL;
			}
		}

		/* Free up the memory allocated to the list */
		free (appsBoxLinkBoxIn);
		apsBox = NULL;
	}

	if (apsLink) {

		/* Delete all of the cTopologyMirrorLink objects */
		for (nCount = 0; nCount < nLinks; nCount++) {
			if (apsLink[nCount]) {
				delete apsLink[nCount];
				apsLink[nCount] = NULL;
			}
		}

		/* Free up the memory allocated to the list */
		free (apsLink);
		apsLink = NULL;
	}
}

/*
 =======================================================================================================================
 *  Return the actual node data structure given the node index
 =======================================================================================================================
 */
cTopologyMirrorNode * cTopologyMirror::GetNode (int nPoint) {
	return apsBox[nPoint + LINKLIST_OFFSET];
}

/*
 =======================================================================================================================
 *  Return the actual link data structure given the node and link in indices
 =======================================================================================================================
 */
cTopologyMirrorLink * cTopologyMirror::GetLinkIn (int nPoint, int nLinkNum) {
	return appsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum];
}

/*
 =======================================================================================================================
 *  Return the actual link data structure given the node and link out indices
 =======================================================================================================================
 */
cTopologyMirrorLink * cTopologyMirror::GetLinkOut (int nPoint, int nLinkNum) {
	return appsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum];
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::LinksInCount (int nPoint) {

	/*
	 * gnBoxLinkInCount[nPoint + LINKLIST_OFFSET];
	 */
	return anBoxLinkInCount[nPoint + LINKLIST_OFFSET];
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::LinkFrom (int nPoint, int nLinkNum) {
	int nReturn = -1;

	/*
	 * gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->psLinkFrom->nEnumerationTemp;
	 */
	if ((nPoint >= 0) && (nLinkNum >= 0)) {
		nReturn = appsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->psLinkFrom->nEnumerationTemp;
	}

	return nReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::GetLinkInputChannelIn (int nPoint, int nLinkNum) {

	/*
	 * gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelIn;
	 */
	return appsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelIn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::LinksOutCount (int nPoint) {

	/*
	 * gnBoxLinkOutCount[nPoint + LINKLIST_OFFSET];
	 */
	return anBoxLinkOutCount[nPoint + LINKLIST_OFFSET];
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::GetLinkOutputChannelOut (int nPoint, int nLinkNum) {

	/*
	 * gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelOut;
	 */
	return appsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelOut;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::LinkTo (int nPoint, int nLinkNum) {
	int nReturn = -1;
	/*
	 * gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->psLinkTo->nEnumerationTemp;
	 */
	if ((nPoint >= 0) && (nLinkNum >= 0)) {
		nReturn = appsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->psLinkTo->nEnumerationTemp;
	}

	return nReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::GetLinkOutputInputIndex (int nPoint, int nLinkNum) {

	/*
	 * gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nLinkInIndexTemp;
	 */
	return appsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nLinkInIndexTemp;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cVM* cTopologyMirror::GetComponentVM (int nPoint) {
	cVM * psReturn = NULL;
	/*
	 * gpsBox[nPoint + LINKLIST_OFFSET]->virtualMachine;
	 */
	if (nPoint >= 0) {
		psReturn = apsBox[nPoint + LINKLIST_OFFSET]->virtualMachine;
	}

	return psReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::GetLinkInputChannelOut (int nPoint, int nLinkNum) {

	/*
	 * gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelOut;
	 */
	return appsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelOut;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::GetLinkOutputChannelIn (int nPoint, int nLinkNum) {

	/*
	 * gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelIn;
	 */
	return appsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelIn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::GetLinkInputOutputIndex (int nPoint, int nLinkNum) {

	/*
	 * gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nLinkOutIndexTemp;
	 */
	return appsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->nLinkOutIndexTemp;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cTopologyMirror::GetLinkInput (int nPoint, int nLinkNum) {

	/*
	 * gpsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->szLinkInput;
	 */
	return appsBoxLinkBoxIn[nPoint + LINKLIST_OFFSET][nLinkNum]->szLinkInput;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cTopologyMirror::GetLinkOutput (int nPoint, int nLinkNum) {

	/*
	 * gpsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->szLinkOutput;
	 */
	return appsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->szLinkOutput;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::GetPredefinedProperty (int nPoint, PREPROP ePreProp) {
	int nReturn;
	cTopologyMirrorNode*  psBox;

	nReturn = 0;
	if (nPoint >= 0) {
		psBox = apsBox[nPoint + LINKLIST_OFFSET];

		switch (ePreProp) {
		case PREPROP_SENSITIVITYLEVEL:
			nReturn = psBox->nSensitivityLevel;
			break;
		case PREPROP_ENCRYPTIONSTRENGTH:
			if (strcmp (psBox->szEncryptAlgorithm, "Unspecified") == 0) {
				nReturn = 1;
			} else if (strcmp (psBox->szEncryptAlgorithm, "WEP-40") == 0) {
				nReturn = 2;
			} else if (strcmp (psBox->szEncryptAlgorithm, "RC2-40") == 0) {
				nReturn = 3;
			} else if (strcmp (psBox->szEncryptAlgorithm, "RC4-40") == 0) {
				nReturn = 4;
			} else if (strcmp (psBox->szEncryptAlgorithm, "WEP-114") == 0) {
				nReturn = 5;
			} else if (strcmp (psBox->szEncryptAlgorithm, "DES-56") == 0) {
				nReturn = 6;
			} else if (strcmp (psBox->szEncryptAlgorithm, "RC5-56") == 0) {
				nReturn = 7;
			} else if (strcmp (psBox->szEncryptAlgorithm, "RC2-128") == 0) {
				nReturn = 8;
			} else if (strcmp (psBox->szEncryptAlgorithm, "RC4-128") == 0) {
				nReturn = 9;
			} else if (strcmp (psBox->szEncryptAlgorithm, "RC5-128") == 0) {
				nReturn = 10;
			} else if (strcmp (psBox->szEncryptAlgorithm, "TDES-168") == 0) {
				nReturn = 11;
			} else if (strcmp (psBox->szEncryptAlgorithm, "IDEA-128") == 0) {
				nReturn = 12;
			} else if (strcmp (psBox->szEncryptAlgorithm, "Skipjack-80") == 0) {
				nReturn = 13;
			}

			break;
		case PREPROP_STAFFSKILLS:
			if (strcmp (psBox->szStaffSkills, "Low") == 0) {
				nReturn = 1;
			} else if (strcmp (psBox->szStaffSkills, "Mid") == 0) {
				nReturn = 2;
			} else if (strcmp (psBox->szStaffSkills, "High") == 0) {
				nReturn = 3;
			}

			break;
		case PREPROP_FIREWALL:
			nReturn = psBox->boFirewall;
			break;
		case PREPROP_IDS:
			nReturn = psBox->boIDS;
			break;
		case PREPROP_EXTERNAL:
			nReturn = psBox->boExternal;
			break;
		default:
			nReturn = 0;
			break;
		}
	}

	return nReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cTopologyMirror::GetPredefinedProperty (int nPoint, int nLinkNum, PREPROP ePreProp) {
	int nReturn;
	cTopologyMirrorNode*  psBox;

	nReturn = 0;
	psBox = apsBox[nPoint + LINKLIST_OFFSET];

	switch (ePreProp) {
	case PREPROP_SENSITIVITYLEVEL:
		nReturn = appsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nSensitivityLevel;
		break;
	case PREPROP_CHANNELOUT:
		nReturn = appsBoxLinkBoxOut[nPoint + LINKLIST_OFFSET][nLinkNum]->nChannelOut;
		break;
	default:
		nReturn = 0;
		break;
	}

	return nReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cTopologyMirror::GetComponentProperties (int nPoint) {
	char * szReturn = NULL;
	/*
	 * gpsBox[nPoint + LINKLIST_OFFSET]->szCertProps;
	 */
	if (nPoint >= 0) {
		szReturn = apsBox[nPoint + LINKLIST_OFFSET]->szCertProps;
	}

	return szReturn;
}
