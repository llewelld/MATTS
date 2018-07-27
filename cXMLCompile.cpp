/*$T MATTS/cXMLCompile.cpp GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: cXMLCompile.cpp ;
 * Last Modified: 15/07/08 ;
 * ;
 * Purpose: Compile Compose XML files into structures for execution ;
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include "cXMLCompile.h"
#include "cAnalyse.h"

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

XERCES_CPP_NAMESPACE_USE

/** cXMLCompile */
cXMLCompile::cXMLCompile () {
	psRoot = NULL;
	pOptions = cOptions::create ();
	psTopologyMirror = NULL;
	psHandler = new CompileHandler (&psRoot);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cXMLCompile::cXMLCompile (XMLCCompose* psRoot) {
	this->psRoot = psRoot;
	pOptions = cOptions::create ();
	psTopologyMirror = NULL;
	psHandler = new CompileHandler (&psRoot);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cXMLCompile::~cXMLCompile () {
	delete psHandler;
}

/*
 =======================================================================================================================
 *  Load the Compose XML file and convert it into a hierarchy of classes
 =======================================================================================================================
 */
void cXMLCompile::LoadFile (char* szFilename) {
	XMLCTag*  psSequential;

	SAX2XMLReader*	psSAXParser;
	psSequential = NULL;
	XMLPlatformUtils::Initialize ();

	/* Create a new parser */
	psSAXParser = XMLReaderFactory::createXMLReader ();
	psHandler->LoadStart ("");

	psSAXParser->setContentHandler (psHandler);
	psSAXParser->setErrorHandler (psHandler);

	/* Parse the document */
	try
	{
		psSAXParser->parse (szFilename);
	}

	catch (const XMLException&toCatch) {
		char*  szMessage;
		szMessage = XMLString::transcode (toCatch.getMessage ());

		/*
		 * LogString (szMessage);
		 */
		MessageBox (NULL, szMessage, NULL, MB_OK);
		XMLString::release (&szMessage);
	}

	catch (const SAXParseException&toCatch) {
		char*  szMessage;
		szMessage = XMLString::transcode (toCatch.getMessage ());

		/*
		 * LogString (szMessage);
		 */
		MessageBox (NULL, szMessage, NULL, MB_OK);
		XMLString::release (&szMessage);
	}

	catch (...) {

		/*
		 * LogString ("Unexpected Error");
		 */
		MessageBox (NULL, "Unexpected Error", NULL, MB_OK);
	}

	psHandler->LoadEnd ("");
	///* Populate the list of sandbox pointers */
	//psRoot->nSandboxes = psHandler->nSandboxes;
	//if (psRoot->nSandboxes > 0) {

	//	/* Allocate memory for the sandbox pointer list */
	//	psRoot->apsSandbox = (XMLCSandbox**) calloc (psRoot->nSandboxes, sizeof (XMLCSandbox *));
	//	if (psRoot->apsSandbox) {

	//		/* Add all of the sandbox elements to the list of pointers */
	//		nSandboxCount = 0;
	//		psSequential = psRoot;
	//		while ((psSequential) && (nSandboxCount < psRoot->nSandboxes)) {
	//			if (psSequential->GetTag () == XMLCTAG_SANDBOX) {
	//				psRoot->apsSandbox[nSandboxCount] = (XMLCSandbox*) psSequential;
	//			}

	//			psSequential = GetSequential (psSequential);
	//		}
	//	}
	//}

	/* Clean up */
	delete psSAXParser;
	XMLPlatformUtils::Terminate ();
}

/*
 =======================================================================================================================
 *  This method allows us to cycle sequentially through all of the tags in the structure
 =======================================================================================================================
 */
XMLCTag* cXMLCompile::GetSequential (XMLCTag* psCurrent) {
	XMLCTag*  psSequential;

	if (psCurrent->psChild) {
		psSequential = psCurrent->psChild;
	} else if (psCurrent->psNext) {
		psSequential = psCurrent->psNext;
	} else {
		psSequential = psCurrent->psParent;
		while (psSequential && (psSequential->psNext == NULL)) {
			psSequential = psSequential->psParent;
		}

		if (psSequential && (psSequential->psNext)) {
			psSequential = psSequential->psNext;
		}
	}

	return psSequential;
}

/*
 =======================================================================================================================
 *  This method allows us to cycle sequentially through all of the tags in the structure
 =======================================================================================================================
 */
XMLCTag* CompileHandler::GetSequential (XMLCTag* psCurrent) {
	XMLCTag*  psSequential;

	if (psCurrent->psChild) {
		psSequential = psCurrent->psChild;
	} else if (psCurrent->psNext) {
		psSequential = psCurrent->psNext;
	} else {
		psSequential = psCurrent->psParent;
		while (psSequential && (psSequential->psNext == NULL)) {
			psSequential = psSequential->psParent;
		}

		if (psSequential && (psSequential->psNext)) {
			psSequential = psSequential->psNext;
		}
	}

	return psSequential;
}

/*
 =======================================================================================================================
 *  Find the tag with the given ID ;
 *  This is currently a slow way of doing it. It needs to be sped up using a hash table
 =======================================================================================================================
 */
XMLCTag* cXMLCompile::FindTagWithID (char const *const szID) {
	XMLCTag*  psCurrent;
	XMLCTag*  psFound;

	psCurrent = psRoot;
	psFound = NULL;
	while (psCurrent && (psFound == NULL)) {
		if (psCurrent->GetID ().compare (szID) == 0) {
			psFound = psCurrent;
		}

		psCurrent = GetSequential (psCurrent);
	}

	return psFound;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
XMLCCompose* cXMLCompile::GetCompiled (void) {
	return psRoot;
}

/**
 =======================================================================================================================
 *  CompileHandler
 =======================================================================================================================
 */
CompileHandler::CompileHandler (XMLCCompose ** ppsRoot) {

	/*
	 * Store the Root node pointer;
	 * this is the return value
	 */
	this->ppsRoot = ppsRoot;
	psCurrent = NULL;
	if (ppsRoot) {

		/* We start the process without a root node */
		*ppsRoot = NULL;
	}

	eCurrent = XMLCTAG_INVALID;

	/* Clear the character buffer */
	szCharacters = "";
	nSandboxes = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
CompileHandler::~CompileHandler () {
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void CompileHandler::startElement (XMLCh const *const uri, XMLCh const *const localname, XMLCh const *const qname,
								   Attributes const&  attrs) {
	char*  szTag;
	XMLCTAG eCheckTag;
	unsigned int uAttribute;
	XMLCTag*  psNew;

	/* There are quite a few attribute variables */
	XMLCTYPE eType;
	string szID;
	string szInit;
	string szCond;
	string szAction;
	string szConfig;
	string szFormat;
	XMLCCYCLE eCycle;
	XMLCFOLLOW eFollow;

	szTag = XMLString::transcode (localname);
	psNew = NULL;
	eType = XMLCTYPE_STANDARD;
	szID = "";
	szInit = "";
	szCond = "";
	szAction = "";
	szConfig = "";
	szFormat = "";
	eCycle = XMLCCYCLE_OPTIONAL;
	eFollow = XMLCFOLLOW_FRESH;

	/* Figure out which tag this is */
	eCurrent = XMLCTAG_INVALID;
	eCheckTag = (XMLCTAG) ((int) XMLCTAG_INVALID + 1);
	while ((eCheckTag < XMLCTAG_NUM) && (eCurrent == XMLCTAG_INVALID)) {
		if (_stricmp (szTag, szXMLCTag[eCheckTag]) == 0) {
			eCurrent = eCheckTag;
		}

		eCheckTag = (XMLCTAG) ((int) eCheckTag + 1);
	}

	/* Store the attribute values into the appropriate variables */
	uAttribute = 0;
	while (uAttribute < attrs.getLength ()) {

		/* Get the attribute and value strings in a form we can use */
		char*  szAttribute;
		char*  szValue;
		szAttribute = XMLString::transcode (attrs.getLocalName (uAttribute));
		szValue = XMLString::transcode (attrs.getValue (uAttribute));

		/*
		 * This just checks for all of the attributes we care about ;
		 * It's a messy, but at least straightforward approach
		 */
		if (_stricmp (szAttribute, "type") == 0) {
			if (_stricmp (szValue, "extended") == 0) {
				eType = XMLCTYPE_EXTENDED;
			}
		} else if (_stricmp (szAttribute, "id") == 0) {
			szID.assign (szValue);
		} else if (_stricmp (szAttribute, "init") == 0) {
			szInit.assign (szValue);
		} else if (_stricmp (szAttribute, "cond") == 0) {
			szCond.assign (szValue);
		} else if (_stricmp (szAttribute, "action") == 0) {
			szAction.assign (szValue);
		} else if (_stricmp (szAttribute, "config") == 0) {
			szConfig.assign (szValue);
		} else if (_stricmp (szAttribute, "format") == 0) {
			szFormat.assign (szValue);
		} else if (_stricmp (szAttribute, "cycle") == 0) {
			if (_stricmp (szValue, "required") == 0) {
				eCycle = XMLCCYCLE_REQUIRED;
			} else if (_stricmp (szValue, "disallow") == 0) {
				eCycle = XMLCCYCLE_DISALLOW;
			}
		} else if (_stricmp (szAttribute, "follow") == 0) {
			if (_stricmp (szValue, "yes") == 0) {
				eFollow = XMLCFOLLOW_YES;
			} else if (_stricmp (szValue, "no") == 0) {
				eFollow = XMLCFOLLOW_NO;
			}
		}

		/* Release the temporary attribute and value strings */
		XMLString::release (&szValue);
		XMLString::release (&szAttribute);
		uAttribute++;
	}

	/* Create a new class based on the tag information */
	switch (eCurrent) {
	case XMLCTAG_COMPOSE:
		if ((ppsRoot) && ((*ppsRoot) == NULL)) {

			/* Create new root Compose class */
			*ppsRoot = new XMLCCompose (szID, szInit, eType);
			psNew = *ppsRoot;
		}

		break;
	case XMLCTAG_SANDBOX:

		/* Create new Sandbox class */
		psNew = new XMLCSandbox (szID, "", szConfig);

		/* Reset the character buffer for the description */
		szCharacters.clear ();
		nSandboxes++;
		break;
	case XMLCTAG_PROPERTY:

		/* Create new Property class */
		psNew = new XMLCProperty (szID, "");

		/* Reset the character buffer for the description */
		szCharacters.clear ();
		break;
	case XMLCTAG_CONFIGURATION:

		/* Create new Configuration class */
		psNew = new XMLCConfiguration (szID);
		break;
	case XMLCTAG_COMPONENT:

		/* Create new Component class */
		psNew = new XMLCComponent (szID, szInit, szCond, szAction);
		break;
	case XMLCTAG_INPUT:

		/* Create new Input class */
		psNew = new XMLCInput (szID, szInit, szCond, szAction, szConfig, szFormat, eCycle, eFollow);
		break;
	case XMLCTAG_OUTPUT:

		/* Create new Output class */
		psNew = new XMLCOuput (szID, szInit, szCond, szAction, szConfig, szFormat, eCycle, eFollow);
		break;
	case XMLCTAG_PROCESS:

		/* Create new Process class */
		psNew = new XMLCProcess (szID, szInit, szCond, szAction, szConfig);
		break;
	default:

		/* Don't create a new class */
		break;
	}

	/* Add the new class into the hierarchy */
	if (psNew) {
		if (psCurrent) {

			/* We add it as a child of the current class */
			psCurrent->AddChild (psNew);
		}

		/*
		 * Then set it to be the current class ;
		 * Any new nodes will be added as children of this class until the element is
		 * closed
		 */
		psCurrent = psNew;
	}

	XMLString::release (&szTag);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void CompileHandler::characters (const XMLCh *const chars, const unsigned int length) {
	char*  szData;
	szData = XMLString::transcode (chars);

	/* Append the characters onto the end of the buffer */
	szCharacters.append (szData);
	XMLString::release (&szData);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void CompileHandler::fatalError (const SAXParseException& ) {

	/* Something went wrong */
	if (ppsRoot) {

		/*
		 * The structure is most likely corrupted ;
		 * So we're going to get rid of it
		 */
		if (*ppsRoot) {

			/* Fingers crossed this isn't going to cause problems */
			delete *ppsRoot;
		}

		*ppsRoot = NULL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void CompileHandler::endElement (const XMLCh *const uri, const XMLCh *const localname, const XMLCh *const qname) {
	switch (eCurrent) {
	case XMLCTAG_SANDBOX:

		/* Set the description for the Sandbox class */
		if (psCurrent->GetTag () == XMLCTAG_SANDBOX) {
			((XMLCSandbox*) psCurrent)->szDescription = szCharacters;
		}

		/* Reset the character buffer for the description */
		szCharacters.clear ();
		break;
	case XMLCTAG_PROPERTY:

		/* Set the description for the Property class */
		if (psCurrent->GetTag () == XMLCTAG_PROPERTY) {
			((XMLCProperty*) psCurrent)->szDescription = szCharacters;
		}

		/* Reset the character buffer for the description */
		szCharacters.clear ();
		break;
	default:

		/* Don't need to do anything special */
		break;
	}

	/* We're done with this tag */
	eCurrent = XMLCTAG_INVALID;

	/* Move to the parent in the hierarchy */
	if (psCurrent) {
		psCurrent = psCurrent->psParent;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void CompileHandler::Save (FILE * hFile, int nIndent) {
	if (ppsRoot && (*ppsRoot)) {
		(*ppsRoot)->Save (hFile, nIndent);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void CompileHandler::LoadStart (char const * szChunkName) {
	/*
	 * Store the Root node pointer;
	 * this is the return value
	 */
	this->ppsRoot = ppsRoot;
	psCurrent = NULL;
	if (ppsRoot) {

		/* We start the process without a root node */
		*ppsRoot = NULL;
	}

	eCurrent = XMLCTAG_INVALID;

	/* Clear the character buffer */
	szCharacters = "";
	nSandboxes = 0;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void CompileHandler::LoadEnd (char const * szChunkName) {
	XMLCTag*  psSequential;
	int nSandboxCount;

	if (ppsRoot && (*ppsRoot)) {
		/* Populate the list of sandbox pointers */
		(*ppsRoot)->nSandboxes = nSandboxes;
		if ((*ppsRoot)->nSandboxes > 0) {

			/* Allocate memory for the sandbox pointer list */
			(*ppsRoot)->apsSandbox = (XMLCSandbox**) calloc ((*ppsRoot)->nSandboxes, sizeof (XMLCSandbox *));
			if ((*ppsRoot)->apsSandbox) {

				/* Add all of the sandbox elements to the list of pointers */
				nSandboxCount = 0;
				psSequential = (*ppsRoot);
				while ((psSequential) && (nSandboxCount < (*ppsRoot)->nSandboxes)) {
					if (psSequential->GetTag () == XMLCTAG_SANDBOX) {
						(*ppsRoot)->apsSandbox[nSandboxCount] = (XMLCSandbox*) psSequential;
					}

					psSequential = GetSequential (psSequential);
				}
			}
		}
	}
}

/**
 =======================================================================================================================
 *  Analysis engine
 =======================================================================================================================
 */
void cXMLCompile::CheckComposition (cTopologyMirror* psTopologyMirror) {
	int nSandbox;
	XMLCTag*  psStart;
	int nNodeCount;

	this->psTopologyMirror = psTopologyMirror;

	if (psRoot) {

		/*
		 * Travel through the tree recursively to discover if the pattern is satisfied at
		 * each stage ;
		 * There are a couple of things to watch out for:- ;
		 * - If more than one pattern is satisfied, we must test them all (recursively) ;
		 * - At each stage we must note that a component has been dealt with, so that
		 * cycles can be recognised ;
		 * psTopology->CalculateLinkList ();
		 */
		for (nSandbox = 0; nSandbox < psRoot->nSandboxes; nSandbox++) {

			/* Check each sandbox */
			psStart = FindTagWithID (psRoot->apsSandbox[nSandbox]->szConfig.c_str ());
			if (psStart) {

				/* Reset the count for the number of times a node has been visited */
				for (nNodeCount = -LINKLIST_OFFSET; nNodeCount < POINTS_MAX; nNodeCount++) {
					nNodeVisited[nNodeCount + LINKLIST_OFFSET] = 0;
				}

				/* This is where all of the actual checking work is done */
				if (psRoot->eType == XMLCTYPE_EXTENDED) {
					nRecurseLevel = 0;
					nCurrentLinkOut = -1;
					psRoot->apsSandbox[nSandbox]->boTestResult = TestExtended (psStart, 0);
				} else {

					/* We've not yet completed the standard test */
					psRoot->apsSandbox[nSandbox]->boTestResult = FALSE; /* Test (psStart,
																		 * 0);
																		 * */
				}
			}
		}

		/* Reset the structure ready for the next time */
		psRoot->Reset ();
	} else {

		/*
		 * LogString ("No XML document loaded");
		 */
		MessageBox (NULL, "No Compose XML document loaded", NULL, MB_OK);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLCompile::EvaluateCond (XMLCComponent* psCurrentNode) {
	bool boReturn;

	boReturn = TRUE;

	/*
	 * LogString ("Evaluating condition");
	 * LogString (psCurrentNode->szCond);
	 */
	boReturn = CheckCondition (psCurrentNode, (psCurrentNode->szCond).c_str ());

	/*
	 * if (boReturn) { ;
	 * LogString ("Evaluated to TRUE");
	 * } ;
	 * else { ;
	 * LogString ("Evaluated to FALSE");
	 * }
	 */
	if (boReturn) {

		/*
		 * LogString ("Applying action");
		 * LogString (psCurrentNode->szAction);
		 */
		ApplyAction (psCurrentNode, (psCurrentNode->szAction).c_str ());
	}

	return boReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLCompile::TestExtended (XMLCTag* psCurrentNode, int nCurrentComponent) {
	XMLCTag*  psComponent;
	XMLCTag*  psNextNode;
	int nLinkNum;
	int nLinksIn;
	int nLinksOut;
	int nStartOfLink;
	int nEndOfLink;
	int nChannel;
	XMLCCYCLE eCycleIn;
	XMLCCYCLE eCycleOut;
	XMLCFOLLOW eFollowIn;
	XMLCFOLLOW eFollowOut;

	nRecurseLevel++;

	/*
	 * LogValue ("\n Current component is %d ", nCurrentComponent);
	 * LogValue ("Recursion level %d", gnRecurseLevel);
	 * ;
	 * Set the global component variable, as it may be used when evaluating ;
	 * to resolve in[] and out[] variables ;
	 * Establish the global stack variables
	 */
	GlobalStackVariables (nCurrentComponent);

	for (psComponent = psCurrentNode->psChild; psComponent; psComponent = psComponent->psNext) {
		if (psComponent->GetTag () == XMLCTAG_COMPONENT) {

			/*
			 * LogString ("Node");
			 * LogNode (psComponent);
			 * ;
			 * This is a component, so we need to check it
			 */
			if (EvaluateCond ((XMLCComponent*) psComponent)) {
				XMLCTag*  psInputTag;

				/* Undertake any processes */
				ProcessScan (psComponent);

				for (psInputTag = psComponent->psChild; psInputTag; psInputTag = psInputTag->psNext) {
					if (psInputTag->GetTag () == XMLCTAG_INPUT) {
						XMLCInput*	psInput;
						bool boInputsMatch;
						int nTokenCount;

						string szToken;
						bool boMultiple;
						int nRepeatCount;
						bool boAny;
						psInput = (XMLCInput*) psInputTag;

						/*
						 * LogString ("Input");
						 * This is an input, so we need to check it ;
						 * Check the attributes and see if they fit the box
						 */
						eCycleIn = psInput->eCycle;

						/*
						 * LogString ("Input format");
						 * Check the format attribute ;
						 * LogString (psInput->szFormat);
						 * Compare the inputs against this
						 */
						boInputsMatch = TRUE;

						nLinksIn = psTopologyMirror->LinksInCount (nCurrentComponent);
						nTokenCount = 0;

						if ((psInput->psFormat->GetTokensNum () != nLinksIn) && !(psInput->psFormat->FindAny ("*"))) {
							boInputsMatch = FALSE;

							/*
							 * LogValue ("Input links don't match (%d link in)", nLinksIn);
							 */
						}

						/* Check if there is an asterisk postfixed */
						szToken = psInput->psFormat->GetToken (nTokenCount);

						nTokenCount++;
						boMultiple = FALSE;
						nRepeatCount = 0;
						boAny = FALSE;

						if ((szToken.length () > 0) && (szToken.at (szToken.length () - 1) == '*')) {

							/* Remove the asterisk */
							szToken.resize (szToken.length () - 1);
							boMultiple = TRUE;
							if (szToken.length () == 0) {
								boAny = TRUE;
							}
						}

						for (nLinkNum = 0; (boInputsMatch && (nLinkNum < nLinksIn) && (!boAny)); nLinkNum++) {

							/*
							 * psTopology->GetLinkInput (nCurrentComponent, nLinkNum) ;
							 * Establish the box at the start of the link
							 */
							nStartOfLink = psTopologyMirror->LinkFrom (nCurrentComponent, nLinkNum);
							if (((!boMultiple) || (nRepeatCount == 0))) {
								if (!AnalyseComponent (szToken.c_str (), nCurrentComponent, nLinkNum, psInput,
									XMLCLINKTYPE_INPUT)) {
									boInputsMatch = FALSE;

									/*
									 * LogValue ("Input links (not multiple) don't match on link %d (Analysis
									 * failed)", nLinkNum);
									 */
								}
							}

							if (boMultiple && (nRepeatCount > 0)) {
								nChannel = psTopologyMirror->GetLinkInputChannelIn (nCurrentComponent, nLinkNum);
								if (!AnalyseComponent (szToken.c_str (), nCurrentComponent, nLinkNum, psInput,
									XMLCLINKTYPE_INPUT)) {
									szToken = psInput->psFormat->GetToken (nTokenCount);
									nTokenCount++;
									nRepeatCount = 0;
									boMultiple = FALSE;
									boAny = FALSE;
									if ((szToken.length () > 0) && (szToken.at (szToken.length () - 1) == '*')) {

										/* Remove the asterisk */
										szToken.resize (szToken.length () - 1);
										boMultiple = TRUE;
										if (szToken.length () == 0) {
											boAny = TRUE;
										}
									}

									if (!boAny) {
										nChannel = psTopologyMirror->GetLinkInputChannelIn (nCurrentComponent, nLinkNum);
										if (!AnalyseComponent (szToken.c_str (), nCurrentComponent, nLinkNum, psInput,
											XMLCLINKTYPE_INPUT)) {
											boInputsMatch = FALSE;

											/*
											 * LogValue ("Input links (multiple) don't match on link %d (Analysis
											 * failed)", nLinkNum);
											 */
										}
									}
								}
							}

							if (boInputsMatch && boMultiple) {
								nRepeatCount++;
							}

							if (!boMultiple) {
								szToken = psInput->psFormat->GetToken (nTokenCount);
								nTokenCount++;
								nRepeatCount = 0;
								boMultiple = FALSE;
								boAny = FALSE;
								if ((szToken.length () > 0) && (szToken.at (szToken.length () - 1) == '*')) {

									/* Remove the asterisk */
									szToken.resize (szToken.length () - 1);
									boMultiple = TRUE;
									if (szToken.length () == 0) {
										boAny = TRUE;
									}
								}
							}

							if ((nNodeVisited[nStartOfLink + LINKLIST_OFFSET] > 0) && ((
									(nNodeVisited[nCurrentComponent + LINKLIST_OFFSET] > 0)
							&& (eCycleIn == XMLCCYCLE_DISALLOW)) || (
									(nNodeVisited[nCurrentComponent + LINKLIST_OFFSET] <= 0)
							&& (eCycleIn == XMLCCYCLE_REQUIRED)))) {
								boInputsMatch = FALSE;

								/*
								 * LogValue ("Input links don't match on link %d (Cycle failed)", nLinkNum);
								 */
							}
						}

						if ((nTokenCount < (int) psInput->psFormat->GetTokensNum ()) && (!boAny)) {
							boInputsMatch = FALSE;

							/*
							 * LogValue ("Input links don't match on link %d (More tokens)", nLinkNum);
							 */
						}

						if (((!boMultiple) && (szToken.length () != 0)) || ((!boAny) && boMultiple && (nRepeatCount == 0))) {
							boInputsMatch = FALSE;

							/*
							 * LogValue ("Input links don't match on link %d (Repeat count failed)", nLinkNum);
							 */
						}

						/* Find out if we should follow the link or not */
						eFollowIn = psInput->eFollow;

						if (boInputsMatch && (eFollowIn == XMLCFOLLOW_NO)) {

							/*
							 * Return backwards ;
							 * LogString ("Node already visited;
							 * returning");
							 */
							nRecurseLevel--;
							return TRUE;
						}

						if (boInputsMatch) {

							/*/
							 *  Inputs are okay, check the outputs
							 */
							if (EvaluateCond (psInput)) {

								/* Check the outputs */
								XMLCTag*  psOutputTag;
								for (psOutputTag = psComponent->psChild; psOutputTag; psOutputTag = psOutputTag->psNext) {
									if (psOutputTag->GetTag () == XMLCTAG_OUTPUT) {
										XMLCOuput*	psOutput;

										/*
										 * LogString ("Output");
										 * This is an output, so we need to check it ;
										 * Check the attributes and see if they fit the box ;
										 * LogString ("Output format");
										 * ;
										 * Check the format attribute ;
										 * LogString (psOutput->szFormat);
										 */
										bool boOutputsMatch;

										psOutput = (XMLCOuput*) psOutputTag;
										boOutputsMatch = TRUE;

										if (nCurrentComponent == POINT_EXTERNAL) {
											if (psOutput->szFormat.compare ("external") != 0) {

												/* This shouldn't be an external link */
												boOutputsMatch = FALSE;

												/*
												 * LogString ("Output links don't match (Not external)");
												 */
											} else {

												/*
												 * This should be an external link;
												 * we don't need to recurse any further
												 */
												nRecurseLevel--;
												return TRUE;
											}
										} else {
											bool boMultiple;
											int nRepeatCount;
											bool boAny;

											/* Compare the outputs against this */
											nLinksOut = psTopologyMirror->LinksOutCount (nCurrentComponent);
											if ((psOutput->psFormat->GetTokensNum () != nLinksOut)
											&& !(psOutput->psFormat->FindAny ("*"))) {
												boOutputsMatch = FALSE;

												/*
												 * LogValue ("Output links don't match (%d link out)", nLinksOut);
												 */
											}

											/* Check if there is an asterisk postfixed */
											szToken = psInput->psFormat->GetToken (nTokenCount);
											nTokenCount++;
											boMultiple = FALSE;
											nRepeatCount = 0;
											boAny = FALSE;

											if ((szToken.length () > 0) && (szToken.at (szToken.length () - 1) == '*')) {

												/* Remove the asterisk */
												szToken.resize (szToken.length () - 1);
												boMultiple = TRUE;
												if (szToken.length () == 0) {
													boAny = TRUE;
												}
											}

											for (nLinkNum = 0; (boOutputsMatch && (nLinkNum < nLinksOut) && (!boAny));
												 nLinkNum++) {

												/*
												 * psTopology->GetLinkOutput (nCurrentComponent, nLinkNum);
												 */
												if (((!boMultiple) || (nRepeatCount == 0))) {

													/* Check for the property */
													nChannel = psTopologyMirror->GetLinkOutputChannelOut (nCurrentComponent,
																										  nLinkNum);
													if (!AnalyseComponent (szToken.c_str (), nCurrentComponent,
														nLinkNum, psOutput, XMLCLINKTYPE_OUTPUT)) {
														boOutputsMatch = FALSE;

														/*
														 * LogValue ("Output links don't match on link %d (Analysis
														 * failed)", nLinkNum);
														 */
													}
												}

												if ((boMultiple && (nRepeatCount > 0))) {
													nChannel = psTopologyMirror->GetLinkOutputChannelOut (nCurrentComponent,
																										  nLinkNum);
													if (!AnalyseComponent (szToken.c_str (), nCurrentComponent,
														nLinkNum, psOutput, XMLCLINKTYPE_OUTPUT)) {
														psInput->psFormat->GetToken (nTokenCount);
														nTokenCount++;
														nRepeatCount = 0;
														boMultiple = FALSE;
														boAny = FALSE;

														if ((szToken.length () > 0)
														&& (szToken.at (szToken.length () - 1) == '*')) {

															/* Remove the asterisk */
															szToken.resize (szToken.length () - 1);
															boMultiple = TRUE;
															if (szToken.length () == 0) {
																boAny = TRUE;
															}
														}

														if (!boAny) {
															nChannel = psTopologyMirror->GetLinkOutputChannelOut (nCurrentComponent,
																												  nLinkNum);
															if (!AnalyseComponent (szToken.c_str (), nCurrentComponent,
																nLinkNum, psOutput, XMLCLINKTYPE_OUTPUT)) {
																boOutputsMatch = FALSE;

																/*
																 * LogValue ("Output links (not any) don't match on
																 * link %d (Analysis failed)", nLinkNum);
																 */
															}
														}
													}
												}

												if (boOutputsMatch && boMultiple) {
													nRepeatCount++;
												}

												if (!boMultiple) {
													szToken = psInput->psFormat->GetToken (nTokenCount);
													nTokenCount++;
													nRepeatCount = 0;
													boMultiple = FALSE;
													boAny = FALSE;
													if ((szToken.length () > 0) && (szToken.at (szToken.length () - 1) == '*')) {

														/* Remove the asterisk */
														szToken.resize (szToken.length () - 1);
														boMultiple = TRUE;
														if (szToken.length () == 0) {
															boAny = TRUE;
														}
													}
												}
											}

											if ((nTokenCount < (int) psOutput->psFormat->GetTokensNum ()) && (!boAny)) {
												boOutputsMatch = FALSE;

												/*
												 * LogValue ("Output links don't match on link %d (More tokens)", nLinkNum);
												 */
											}

											if (((!boMultiple) && (szToken.length () != 0))
											|| ((!boAny) && boMultiple && (nRepeatCount == 0))) {
												boOutputsMatch = FALSE;

												/*
												 * LogValue ("Output links (not any) don't match on link %d (Repeat
												 * count failed)", nLinkNum);
												 */
											}
										}

										/* Compare the outputs against this */
										if (boOutputsMatch) {
											if (EvaluateCond (psOutput)) {
												bool boSuccess;

												boSuccess = TRUE;

												/* Get the config atribute if there is one */
												if (psOutput->szConfig.length () != 0) {
													psNextNode = FindTagWithID (psOutput->szConfig.c_str ());
													if (!psNextNode) {

														/*
														 * LogString ("Failed to find config id");
														 */
														MessageBox (NULL, "Failed to find config id", NULL, MB_OK);
														psNextNode = psComponent;
													}
												} else {
													psNextNode = psComponent;
												}

												/* Find out if we're okay to cycle or not */
												eCycleOut = psOutput->eCycle;

												nNodeVisited[nCurrentComponent + LINKLIST_OFFSET]++;

												/* Cycle through each output (the recursive bit) */
												for (nLinkNum = 0; ((nLinkNum < nLinksOut) && boSuccess); nLinkNum++) {

													/* Establish the box at the end of the link */
													nEndOfLink = psTopologyMirror->LinkTo (nCurrentComponent, nLinkNum);

													/*
													 * LogValue ("Component at end of link is %d", nEndOfLink);
													 * Find out if this is a cycle
													 */
													if (((nNodeVisited[nEndOfLink + LINKLIST_OFFSET] > 0)
													&& (eCycleOut == XMLCCYCLE_DISALLOW)) || ((nNodeVisited[nEndOfLink + LINKLIST_OFFSET] <= 0)
													&& (eCycleOut == XMLCCYCLE_REQUIRED))) {
														boSuccess = FALSE;

														/*
														 * LogValue ("Output links don't match on link %d (Cycle
														 * failed)", nLinkNum);
														 */
													}

													/* Find out if we should follow the link or not */
													eFollowOut = psOutput->eFollow;

													if (boSuccess) {
														if ((eFollowOut == XMLCFOLLOW_YES) || (
																(eFollowOut == XMLCFOLLOW_FRESH)
														&& (nNodeVisited[nEndOfLink + LINKLIST_OFFSET] <= 0))) {
															GlobalLinkOut (nLinkNum);

															/*
															 * This is the actual recursive bit ;
															 * LogString ("Recursively calling TestExtended");
															 */
															if (!TestExtended (psNextNode, nEndOfLink)) {
																boSuccess = FALSE;

																/*
																 * LogString ("Failure (in recursion)");
																 */
															}

															/* Establish the global stack variables */
															GlobalStackVariables (nCurrentComponent);
															GlobalLinkOut (nLinkNum);
														} else {

															/*
															 * LogString ("Link success, but not followed");
															 */
														}
													}
												}

												if (boSuccess) {

													/*
													 * LogString ("Success");
													 */
													nRecurseLevel--;
													return TRUE;
												}

												nNodeVisited[nCurrentComponent + LINKLIST_OFFSET] = 0;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	/*
	 * LogString ("Failure (final)");
	 */
	nRecurseLevel--;
	return FALSE;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLCompile::GlobalStackVariables (int nCurrentComponent) {
	nCurrentComponentOn = nCurrentComponent;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLCompile::GlobalLinkOut (int nCurrentLinkOut) {
	this->nCurrentLinkOut = nCurrentLinkOut;
	this->nCurrentLinkIn = psTopologyMirror->GetLinkOutputInputIndex (nCurrentComponentOn, nCurrentLinkOut);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLCompile::ProcessScan (XMLCTag* psCurrentNode) {
	XMLCTag*  psProcessTag;
	XMLCTag*  psNextNode;

	/* Act on any processing instructions within the current node */
	for (psProcessTag = psCurrentNode->psChild; psProcessTag; psProcessTag = psProcessTag->psNext) {
		if (psProcessTag->GetTag () == XMLCTAG_PROCESS) {
			XMLCProcess*  psProcess;
			psProcess = (XMLCProcess*) psProcessTag;

			/*
			 * LogString ("\nProcess node");
			 * LogNode (psProcess);
			 * ;
			 * This is a process, so we need to check it
			 */
			if (EvaluateCond (psProcess)) {
				if (psProcess->szConfig.length () != 0) {
					psNextNode = FindTagWithID (psProcess->szConfig.c_str ());
				} else {
					psNextNode = psProcess;
				}

				if (psNextNode) {

					/* This is a recursive bit */
					ProcessScan (psNextNode);
				}
			}
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLCompile::CheckCondition (XMLCTag* psCurrentNode, char const *const szCondition) {
	bool boError;
	bool boReturn;

	boError = FALSE;

	psEvalNode = psCurrentNode;
	if ((strlen (szCondition) == 0) || (evaluate (szCondition, &boError))) {
		boReturn = TRUE;
	} else {
		boReturn = FALSE;
	}

	if (boError) {
		boReturn = FALSE;

		/*
		 * LogString ("Error interpreting condition");
		 */
		MessageBox (NULL, "Error interpreting condition", NULL, MB_OK);
	}

	return boReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLCompile::ApplyAction (XMLCTag* psCurrentNode, char const *const szAction) {
	bool boError;

	boError = FALSE;

	psEvalNode = psCurrentNode;
	if (strlen (szAction) > 0) {
		evaluate (szAction, &boError);
	}

	if (boError) {

		/*
		 * LogString ("Error interpreting action");
		 */
		MessageBox (NULL, "Error interpreting action", NULL, MB_OK);
	}
}

/*/
 */
typedef enum _OPERATOR
{
	OPERATOR_INVALID		= -1,
	OPERATOR_MULTIPLY,
	OPERATOR_DIVIDE,
	OPERATOR_REMAINDER,
	OPERATOR_ADD,
	OPERATOR_SUB,
	OPERATOR_LESSEQUAL,
	OPERATOR_GREATEREQUAL,
	OPERATOR_LESS,
	OPERATOR_GREATER,
	OPERATOR_EQUAL,
	OPERATOR_NOTEQUAL,
	OPERATOR_LAND,
	OPERATOR_LOR,
	OPERATOR_AND,
	OPERATOR_OR,
	OPERATOR_ASSIGN,
	OPERATOR_NUM
} OPERATOR;

static char const aszOperator[OPERATOR_NUM][3] =
{
	"*",
	"/",
	"%",
	"+",
	"-",
	"<=",
	">=",
	"<",
	">",
	"==",
	"!=",
	"&&",
	"||",
	"&",
	"|",
	"="
};

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

comptype cXMLCompile::evaluate (char const *const szEval, bool* pboError) {
	comptype xOutput;
	int nLength;
	char szClean[1024];
	int nPos;
	int nTransPos;

	nLength = (int) strlen (szEval);

	/* Remove any spaces from the expression */
	nTransPos = 0;
	for (nPos = 0; ((nPos < nLength) && (nTransPos < (int) (sizeof (szClean) - 1))); nPos++) {
		if (szEval[nPos] != ' ') {
			szClean[nTransPos] = szEval[nPos];
			nTransPos++;
		}

		/* TODO: Change the code so that it's not necessary to convert underscores to spaces
		 * by not removing spaces in the first place
		 */
		if (szEval[nPos] == '~') {
			szClean[nTransPos - 1] = ' ';
		}
	}

	if (nTransPos < (int) (sizeof (szClean) - 1)) {
		szClean[nTransPos] = 0;
		boError = FALSE;
		xOutput = eval (szClean, nTransPos);
	} else {
		char szError[1024];
		sprintf (szError, "Error - expressions cannot be larger than %d\n", (int) (sizeof (szClean)));

		/*
		 * LogString (szError);
		 */
		MessageBox (NULL, szError, NULL, MB_OK);
		boError = TRUE;
		xOutput = 0;
	}

	if (!boError) {
		if (pboError) {
			*pboError = FALSE;
		}
	} else {
		if (pboError) {
			*pboError = TRUE;
		}

		xOutput = 0;
	}

	return xOutput;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
comptype cXMLCompile::eval (char const *const szEval, int nLength) {
	int nBrackets;
	int nPos;
	char cCurrent;
	bool boResult;
	comptype xResult;
	char szCopy[1024];
	char const*	 szLHS;
	int nLHSLen;
	char const*	 szRHS;
	int nRHSLen;
	OPERATOR eOperator;
	int nOpLen;
						float nLeft = 0;
						float nRight = 0;

	if (boError) {
		return 0;
	}

	if (nLength == 0) {

		/*
		 * LogString ("Empty constant");
		 */
		MessageBox (NULL, "Empty constant", NULL, MB_OK);
		boError = TRUE;
		return 0;
	}

	strncpy (szCopy, szEval, nLength);
	szCopy[nLength] = 0;

	boResult = FALSE;

	/* Binary operators */
	eOperator = OPERATOR (0);
	while ((!boResult) && (eOperator < OPERATOR_NUM)) {
		nOpLen = (int) strlen (aszOperator[(int) (eOperator)]);
		nBrackets = 0;
		nPos = 0;
		while ((!boResult) && (nPos < nLength)) {
			cCurrent = szEval[nPos];
			if (cCurrent == '(') {
				nBrackets++;
			}

			if (cCurrent == ')') {
				nBrackets--;
				if (nBrackets < 0) {

					/*
					 * LogString ("Error - too few opening brackets");
					 */
					MessageBox (NULL, "Error - too few opening brackets", NULL, MB_OK);
					boError = true;
					return 0;
				}
			}

			if ((nBrackets == 0) && (nPos <= (nLength - nOpLen))) {
				if (strncmp (szEval + nPos, aszOperator[(int) (eOperator)], nOpLen) == 0) {

					/* Do the recursive bit */
					szLHS = szEval;
					nLHSLen = nPos;
					szRHS = szEval + nPos + nOpLen;
					nRHSLen = nLength - nPos - nOpLen;
					switch (eOperator) {
					case OPERATOR_MULTIPLY:
						xResult = eval (szLHS, nLHSLen) * eval (szRHS, nRHSLen);
						break;
					case OPERATOR_DIVIDE:
						xResult = eval (szLHS, nLHSLen) / eval (szRHS, nRHSLen);
						break;
					case OPERATOR_REMAINDER:
						xResult = (int)eval (szLHS, nLHSLen) % (int)eval (szRHS, nRHSLen);
						break;
					case OPERATOR_ADD:
						xResult = eval (szLHS, nLHSLen) + eval (szRHS, nRHSLen);
						break;
					case OPERATOR_SUB:
						xResult = eval (szLHS, nLHSLen) - eval (szRHS, nRHSLen);
						break;
					case OPERATOR_LESS:
						xResult = eval (szLHS, nLHSLen) < eval (szRHS, nRHSLen);
						break;
					case OPERATOR_GREATER:
						xResult = eval (szLHS, nLHSLen) > eval (szRHS, nRHSLen);
						break;
					case OPERATOR_LESSEQUAL:
						xResult = eval (szLHS, nLHSLen) <= eval (szRHS, nRHSLen);
						break;
					case OPERATOR_GREATEREQUAL:
						xResult = eval (szLHS, nLHSLen) >= eval (szRHS, nRHSLen);
						break;
					case OPERATOR_EQUAL:
						nLeft = (eval (szLHS, nLHSLen) + 0.5);
						nRight = (eval (szRHS, nRHSLen) + 0.5);
						xResult = ((int)nLeft == (int)nRight);
						//xResult = eval (szLHS, nLHSLen) == eval (szRHS, nRHSLen);
						break;
					case OPERATOR_NOTEQUAL:
						nLeft = (eval (szLHS, nLHSLen) + 0.5);
						nRight = (eval (szRHS, nRHSLen) + 0.5);
						xResult = ((int)nLeft != (int)nRight);
						//xResult = eval (szLHS, nLHSLen) != eval (szRHS, nRHSLen);
						break;
					case OPERATOR_AND:
						xResult = (int)eval (szLHS, nLHSLen) & (int)eval (szRHS, nRHSLen);
						break;
					case OPERATOR_OR:
						xResult = (int)eval (szLHS, nLHSLen) | (int)eval (szRHS, nRHSLen);
						break;
					case OPERATOR_LAND:
						nLeft = (eval (szLHS, nLHSLen) + 0.5);
						nRight = (eval (szRHS, nRHSLen) + 0.5);
						xResult = ((int)nLeft && (int)nRight);
//						xResult = (int)eval (szLHS, nLHSLen) && (int)eval (szRHS, nRHSLen);
						break;
					case OPERATOR_LOR:
						nLeft = (eval (szLHS, nLHSLen) + 0.5);
						nRight = (eval (szRHS, nRHSLen) + 0.5);
						xResult = ((int)nLeft || (int)nRight);
						//xResult = (int)eval (szLHS, nLHSLen) || (int)eval (szRHS, nRHSLen);
						break;
					case OPERATOR_ASSIGN:
						xResult = SetVariableValue (szLHS, nLHSLen, eval (szRHS, nRHSLen));
						break;
					default:

						/*
						 * LogString ("Error - operator failure");
						 */
						MessageBox (NULL, "Error - operator failure", NULL, MB_OK);
						break;
					}

					boResult = true;
				}
			}

			nPos++;
		}

		eOperator = (OPERATOR) ((int) (eOperator) + 1);
	}

	/* Unary operators */
	if (!boResult) {
		switch (szEval[0]) {
		case '!':
			xResult = !(int)eval (szEval + 1, nLength - 1);
			boResult = true;
			break;
		case '~':
			xResult = ~(int)eval (szEval + 1, nLength - 1);
			boResult = true;
			break;
		default:
			break;
		}
	}

	if ((!boResult) && ((szEval[0] == '(') && (szEval[nLength - 1] == ')'))) {
		xResult = eval (szEval + 1, nLength - 2);
		boResult = true;
	}

	if (!boResult) {
		if (nBrackets != 0) {
			char szError[1024];
			sprintf (szError, "Error - brackets do not match (%d)\n", nBrackets);

			/*
			 * LogString (szError);
			 */
			MessageBox (NULL, szError, NULL, MB_OK);
			boError = true;
			return 0;
		}

		xResult = GetVariableValue (szCopy, nLength);
	}

	if (boError) {
		return 0;
	}

	return xResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
comptype cXMLCompile::AnalyseComponent (char const *const szPropertyID, int nCurrentComponent, int nLinkNum,
								   XMLCTag*	 psCurrentNode, XMLCLINKTYPE eLinkType) {
	cAnalyse*  psAnalyse;
	cVM*  psVM;
	comptype xReturn;
	comptype xInput;
	bool boReturn;

	/*
	 * char szPrint[64];
	 */
	int nChannel;
	bool boCertified;
	bool boFound;

	psAnalyse = NULL;
	psVM = NULL;
	xReturn = 0;
	boReturn = FALSE;

	psEvalNode = psCurrentNode;

	/* Obtain the input from the XML as a variable */
	xInput = GetInitVal ("@v", 2, nLinkNum, -1, NULL);

	psVM = psTopologyMirror->GetComponentVM (nCurrentComponent);

	switch (eLinkType) {
	case XMLCLINKTYPE_INPUT:

		/*
		 * sprintf (szPrint, "\nComponent %d, input link %d, property ID %s\n",
		 * nCurrentComponent, nLinkNum, szPropertyID);
		 */
		boCertified = CheckLinkInputProperties (szPropertyID, nCurrentComponent, nLinkNum, &xReturn);
		nChannel = psTopologyMirror->GetLinkInputChannelIn (nCurrentComponent, nLinkNum);
		break;
	case XMLCLINKTYPE_OUTPUT:

		/*
		 * sprintf (szPrint, "\nComponent %d, output link %d, property ID %s\n",
		 * nCurrentComponent, nLinkNum, szPropertyID);
		 */
		boCertified = CheckLinkOutputProperties (szPropertyID, nCurrentComponent, nLinkNum, &xReturn);
		nChannel = psTopologyMirror->GetLinkOutputChannelOut (nCurrentComponent, nLinkNum);
		break;
	case XMLCLINKTYPE_NONE:

		/*
		 * sprintf (szPrint, "\nComponent %d, property ID %s\n", nCurrentComponent, szPropertyID);
		 */
		boCertified = CheckNodeProperties (szPropertyID, nCurrentComponent, &xReturn);
		nChannel = -1;
		break;
	default:

		/*
		 * sprintf (szPrint, "\nComponent %d, (not supported) link %d, property ID %s\n",
		 * nCurrentComponent, nLinkNum, szPropertyID);
		 */
		boCertified = FALSE;
		nChannel = -1;

		/*
		 * LogString ("Error - link type not supported in AnalyseComponent");
		 */
		MessageBox (NULL, "Error - link type not supported in AnalyseComponent", NULL, MB_OK);
		break;
	}

	/*
	 * psVM->ConsolePrint (szPrint);
	 * sprintf (szPrint, "Input value %d\n", nInput);
	 * psVM->ConsolePrint (szPrint);
	 */
	boFound = FALSE;

	/* Do the analysis */
	if ((!boFound) && (szPropertyID == "id1")) {
		boFound = TRUE;

		/* Check if the property has been pre-certified */
		if (boCertified) {
			boReturn = (xInput <= xReturn);
		} else {
			psAnalyse = new cAnalyse (psVM);
			psAnalyse->AnalyseCode_id1 (psVM, nChannel, (int)xInput);
			if (psAnalyse->gboCompleted) {
				xReturn = (comptype)psAnalyse->gnResult;
				boReturn = (xReturn != 0);
			}
		}
	}

	if ((!boFound) && (szPropertyID == "id2")) {
		boFound = TRUE;

		if (boCertified) {
			boReturn = TRUE;
		} else {
			psAnalyse = new cAnalyse (psVM);
			psAnalyse->AnalyseCode_id2 (psVM, nChannel);
			if (psAnalyse->gboCompleted) {
				xReturn = (comptype)psAnalyse->gnResult;
				boReturn = TRUE;
			}
		}
	}

	if ((!boFound) && (szPropertyID == "id3")) {
		boFound = TRUE;

		/* Check if the property has been pre-certified */
		if (boCertified) {
			boReturn = TRUE;
		} else {
			psAnalyse = new cAnalyse (psVM);
			psAnalyse->AnalyseCode_id3 (psVM, nChannel);
			if (psAnalyse->gboCompleted) {
				xReturn = (comptype)psAnalyse->gnResult;
				boReturn = (psAnalyse->gnResult == TRUE);
			}
		}
	}

	if ((!boFound) && (szPropertyID == "id4")) {
		boFound = TRUE;

		/* Check if the property has been pre-certified */
		if (boCertified) {
			boReturn = TRUE;
		} else {
			psAnalyse = new cAnalyse (psVM);
			psAnalyse->AnalyseCode_id4 (psVM, nChannel, (int)xInput);
			if (psAnalyse->gboCompleted) {
				xReturn = (comptype)psAnalyse->gnResult;
				boReturn = (psAnalyse->gnResult == TRUE);
			}
		}
	}

	if (!boFound) {

		/* Catch everything else */
		boReturn = boCertified;
	}

	/* Store the result back into the XML as a variable */
	SetInitVal ("@v", 2, nLinkNum, xReturn);

	/*
	 * sprintf (szPrint, "\nOutput value %d\nResult is %d\n\n", nReturn, boReturn);
	 * psVM->ConsolePrint (szPrint);
	 */
	return xReturn; /* used to be boReturn, may cause problem later */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLCompile::CheckCertifiedProperties (char const *const szPropertyID, char* szLinkProperties, comptype* pxReturn) {
	char*  pcFound;
	bool boReturn;
	int nLinkPropsLen;
	int nIDLen;
	comptype xValue;
	int nFound;
	char*  pcSearch;
	bool boFound;

	boReturn = FALSE;

	/* Search for the property ID within the link properties string */
	nLinkPropsLen = (int) strlen (szLinkProperties);
	nIDLen = (int) strlen (szPropertyID);
	*pxReturn = 0;

	pcSearch = szLinkProperties;
	pcFound = NULL;
	do {
		pcFound = strstr (pcSearch, szPropertyID);
		pcSearch = pcFound + 1;
		if (pcFound) {
			boFound = TRUE;

			/* Is this really an instance of the property? */
			if ((pcFound > szLinkProperties) && (pcFound[-1] != ' ')) {
				boFound = FALSE;
			}

			if (((pcFound + nIDLen) < (szLinkProperties + nLinkPropsLen)) && (pcFound[nIDLen] != ' ')
			&& (pcFound[nIDLen] != '=')) {
				boFound = FALSE;
			}

			if (boFound) {

				/* Success! */
				boReturn = TRUE;
				*pxReturn = 1;
				if (pcFound[nIDLen] == '=') {
					nFound = sscanf (pcFound + nIDLen + 1, compconv, &xValue);
					if (nFound > 0) {
						*pxReturn = xValue;
					}
				}

				pcFound = NULL;
			}
		}
	} while (pcFound != NULL);

	return boReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
comptype cXMLCompile::GetVariableValue (char const *const szEval, int nLength) {
	comptype xResult;
	int nScan;
	char szVarName[1024];
	char szIndexVal[1024];
	char const*	 pcBracketOpen[3];
	char const*	 pcBracketClose;
	int nIndexLen[3];
	int nIndexVal1;
	int nIndexVal2;
	int nVarNameLen;
	int nBracketCount;
	int nVarNum;
	char szError[1024];
	char*  pcPropertyID;
	bool boMore;

	xResult = 0;

	if (szEval[nLength - 1] == ']') {

		/*
		 * This is an array variable ;
		 * Start by establishing the variable name
		 */
		pcBracketOpen[0] = strchr (szEval, '[');
		if (pcBracketOpen) {
			nVarNameLen = (int) (pcBracketOpen[0] - szEval);
			strncpy (szVarName, szEval, nVarNameLen);
			szVarName[nVarNameLen] = 0;

			if ((strcmp (szVarName, "in") == 0) || (strcmp (szVarName, "out") == 0)) {

				/* in[] or out[] special array */
				xResult = GetInOutCount (szEval, nLength);
			} else {

				/* Standard array var[] */
				nVarNum = 0;
				boMore = TRUE;
				pcBracketClose = pcBracketOpen[0];
				do
				{

					/* Establish the indices */
					nIndexLen[nVarNum] = 0;
					nBracketCount = 1;
					pcBracketOpen[nVarNum] = pcBracketClose;
					pcBracketClose++;
					while ((pcBracketClose < (szEval + nLength)) && (nBracketCount > 0)) {
						nIndexLen[nVarNum]++;
						pcBracketClose++;
						if (*pcBracketClose == '[') {
							nBracketCount++;
						}

						if (*pcBracketClose == ']') {
							nBracketCount--;
						}
					}

					if (nBracketCount != 0) {
						sprintf (szError, "Error - mismatched brackets array (array index %d)\n", nVarNum);

						/*
						 * LogString (szError);
						 */
						MessageBox (NULL, szError, NULL, MB_OK);
						boError = true;
						xResult = 0;
						boMore = FALSE;
					} else {
						if (nIndexLen[nVarNum] == (nLength - (pcBracketOpen[nVarNum] - szEval) - 2)) {

							/* No more indices */
							boMore = FALSE;
						}
					}

					pcBracketClose++;
					nVarNum++;
				} while ((boMore) && (nVarNum < 3));

				if (boMore) {

					/*
					 * LogString ("Error - too many array arguments/indices");
					 */
					MessageBox (NULL, "Error - too many array arguments/indices", NULL, MB_OK);
					boError = true;
					xResult = 0;
				}

				/* Evaluate the indices */
				pcPropertyID = NULL;
				nIndexVal1 = -1;
				if (nVarNum > 0) {
					strncpy (szIndexVal, pcBracketOpen[0] + 1, nIndexLen[0]);
					szIndexVal[nIndexLen[0]] = 0;
					nIndexVal1 = (int)eval (szIndexVal, nIndexLen[0]);
				}

				nIndexVal2 = -1;
				if (nVarNum > 1) {
					strncpy (szIndexVal, pcBracketOpen[1] + 1, nIndexLen[1]);
					szIndexVal[nIndexLen[1]] = 0;
					if ((nVarNameLen == 2) && (strncmp (szVarName, "@a", 2) == 0)) {

						/* The second index is actually the property for @a */
						nIndexVal2 = 0;
						pcPropertyID = szIndexVal;
					} else {
						nIndexVal2 = (int)eval (szIndexVal, nIndexLen[1]);
					}
				}

				if (nVarNum > 2) {

					/* This is the property for @ai or @ao */
					strncpy (szIndexVal, pcBracketOpen[2] + 1, nIndexLen[2]);
					szIndexVal[nIndexLen[2]] = 0;
					pcPropertyID = szIndexVal;
				}

				if (!boError) {
					xResult = GetInitVal (szVarName, nVarNameLen, nIndexVal1, nIndexVal2, pcPropertyID);
				}
			}
		} else {
			xResult = 0;
		}
	} else {

		/*
		 * Parse the value as a number ;
		 * Check to see whether the value is in hex or decimal
		 */
		if (szEval[0] == '#') {

			/* Try hex */
			nScan = sscanf (szEval + 1, "%x", &xResult);
		} else {

			/* Try decimal */
			nScan = sscanf (szEval, compconv, &xResult);
		}

		if (nScan != 1) {

			/* We reckon it must be a variable */
			xResult = GetInitVal (szEval, nLength, -1, -1, NULL);
		}
	}

	return xResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
comptype cXMLCompile::GetInitVal (char const *const szEval, int nLength, int nIndex1, int nIndex2,
							 char const *const szPropertyID) {
	comptype xResult;
	XMLCComponent*	psVarNode;
	bool boFound;

	xResult = 0;
	psVarNode = NULL;
	boFound = FALSE;

	/* Find the variable */
	if ((nLength > 1) && (szEval[0] == '@')) {

		/* Special variable */
		if ((nLength == 2) && (strncmp (szEval, "@v", 2) == 0)) {

			/* Get the variable for the current node */
			psVarNode = (XMLCComponent*) psEvalNode;
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 2) && (strncmp (szEval, "@n", 2) == 0)) {

			/* Current component number */
			psVarNode = NULL;
			xResult = (comptype)nCurrentComponentOn;
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "@nv", 3) == 0)) {

			/* Component's number of times visited */
			psVarNode = NULL;
			xResult = (comptype)nNodeVisited[nIndex1 + LINKLIST_OFFSET];
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "@il", 3) == 0)) {

			/* Current input link index used to arrive at this component */
			psVarNode = NULL;
			xResult = (comptype)nCurrentLinkIn;
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "ol", 3) == 0)) {

			/* Current ouput link index used to arrive at this component */
			psVarNode = NULL;
			xResult = (comptype)nCurrentLinkOut;
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 6) && (strncmp (szEval, "@ilnum", 6) == 0)) {

			/* Component's number of input links */
			psVarNode = NULL;
			xResult = (comptype)psTopologyMirror->LinksInCount (nIndex1);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 6) && (strncmp (szEval, "@olnum", 6) == 0)) {

			/* Component's number of output links */
			psVarNode = NULL;
			xResult = (comptype)psTopologyMirror->LinksOutCount (nIndex1);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@ilic", 5) == 0)) {

			/* The input link's input channel */
			psVarNode = NULL;
			xResult = (comptype)psTopologyMirror->GetLinkInputChannelIn (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@iloc", 5) == 0)) {

			/* The input link's output channel */
			psVarNode = NULL;
			xResult = (comptype)psTopologyMirror->GetLinkInputChannelOut (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@olic", 5) == 0)) {

			/* The output link's input channel */
			psVarNode = NULL;
			xResult = (comptype)psTopologyMirror->GetLinkOutputChannelIn (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@oloc", 5) == 0)) {

			/* The output link's input channel */
			psVarNode = NULL;
			xResult = (comptype)psTopologyMirror->GetLinkOutputChannelOut (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 4) && (strncmp (szEval, "@iln", 4) == 0)) {

			/* The node at the start of the input link */
			psVarNode = NULL;
			xResult = (comptype)psTopologyMirror->LinkFrom (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 4) && (strncmp (szEval, "@oln", 4) == 0)) {

			/* The node at the start of the input link */
			psVarNode = NULL;
			xResult = (comptype)psTopologyMirror->LinkTo (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "@ai", 3) == 0)) {

			/* Analyse the component on input link */
			psVarNode = NULL;
			xResult = AnalyseComponent (szPropertyID, nIndex1, nIndex2, psEvalNode, XMLCLINKTYPE_INPUT);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "@ao", 3) == 0)) {

			/* Analyse the component on output link */
			psVarNode = NULL;
			xResult = AnalyseComponent (szPropertyID, nIndex1, nIndex2, psEvalNode, XMLCLINKTYPE_OUTPUT);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 2) && (strncmp (szEval, "@a", 2) == 0)) {

			/* Analyse the component */
			psVarNode = NULL;
			xResult = AnalyseComponent (szPropertyID, nIndex1, nIndex2, psEvalNode, XMLCLINKTYPE_NONE);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@ilol", 5) == 0)) {

			/* The equivalent output index of the link for the given input index */
			psVarNode = NULL;
			xResult = psTopologyMirror->GetLinkInputOutputIndex (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@olil", 5) == 0)) {

			/* The equivalent input index of the link for the given ouput index */
			psVarNode = NULL;
			xResult = psTopologyMirror->GetLinkOutputInputIndex (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 2) && (strncmp (szEval, "@r", 2) == 0)) {

			/* The current depth of recursion within the file */
			psVarNode = NULL;
			xResult = (comptype)nRecurseLevel;
			boFound = TRUE;
		}

		/* This part added for the demo */
		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@ncol", 5) == 0)) {

			/* Change the node colour, use 0 to highlight */
			psVarNode = NULL;

			/*
			 * nResult = psTopology->SetNodeAnalyseColour (nIndex1, nIndex2);
			 */
			boFound = TRUE;

			/*
			 * if (nIndex2 == 0) { ;
			 * Sleep (pOptions->getSelectSpeed ());
			 * } ;
			 * else { ;
			 * Sleep (pOptions->getColourSpeed ());
			 * }
			 */
		}
	}

	if (!boFound) {
		char szNodeID[1024];
		strncpy (szNodeID, szEval, nLength);
		szNodeID[nLength] = 0;
		psVarNode = (XMLCComponent*) FindTagWithID (szNodeID);
	}

	if (psVarNode) {

		/* Must only use the first index */
		if (nIndex2 >= 0) {

			/*
			 * LogString ("Error - only one dimensional arrays are available");
			 */
			MessageBox (NULL, "Error - only one dimensional arrays are available", NULL, MB_OK);
			nIndex2 = -1;
		}

		if ((psVarNode->psInit->GetNumElements () <= 1) || (nIndex1 <= 0)) {

			/* We only have one element, so return it */
			xResult = psVarNode->psInit->GetElement (0);
		} else {

			/* More than one element, so pick the correct one */
			xResult = psVarNode->psInit->GetElement (nIndex1);
		}
	} else {
		if (!boFound) {

			/*
			 * LogString ("Error - variable id not found");
			 */
			MessageBox (NULL, "Error - variable id not found", NULL, MB_OK);
			boError = true;
			xResult = 0;
		}
	}

	/*
	 * char szVarName[1024];
	 * char szLog[1024];
	 * strncpy (szVarName, szEval, nLength);
	 * szVarName[nLength] = 0;
	 * sprintf (szLog, "Resolution: %s = %d", szVarName, nResult);
	 * LogString (szLog);
	 */
	return xResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
comptype cXMLCompile::SetInitVal (char const *const szEval, int nLength, int nIndex, comptype xValue) {
	comptype xResult;
	XMLCComponent*	psVarNode;

	xResult = 0;

	/* Find the variable */
	if ((nLength == 2) && (strncmp (szEval, "@v", 2) == 0)) {

		/* Get the variable for the current node */
		psVarNode = (XMLCComponent*) psEvalNode;
	} else {
		char szNodeID[1024];

		strncpy (szNodeID, szEval, nLength);
		szNodeID[nLength] = 0;
		psVarNode = (XMLCComponent*) FindTagWithID (szNodeID);
	}

	if (psVarNode) {
		if (nIndex < 0) {
			psVarNode->psInit->ClearAll ();
			psVarNode->psInit->SetElement (xValue, 0);
		} else {
			psVarNode->psInit->SetElement (xValue, nIndex);
		}

		xResult = xValue;
	} else {

		/*
		 * LogString ("Error - variable id not found");
		 */
		MessageBox (NULL, "Error - variable id not found", NULL, MB_OK);
		boError = true;
		xResult = 0;
	}

	return xResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
comptype cXMLCompile::SetVariableValue (char const *const szEval, int nLength, comptype xValue) {
	comptype xResult;
	char szVarName[1024];
	char szIndexVal[1024];
	char const*	 pcBracket;
	int nIndexLen;
	int nIndexVal;
	int nVarNameLen;

	xResult = 0;

	/*
	 * char szLog[1024];
	 * strncpy (szVarName, szEval, nLength);
	 * szVarName[nLength] = 0;
	 * sprintf (szLog, "Assignment: %s = %d", szVarName, nValue);
	 * LogString (szLog);
	 * ;
	 * Find the variable
	 */
	if (szEval[nLength - 1] == ']') {

		/*
		 * This is an array variable ;
		 * Start by establishing the variable name
		 */
		pcBracket = strchr (szEval, '[');
		if (pcBracket) {
			nVarNameLen = (int) (pcBracket - szEval);
			strncpy (szVarName, szEval, nVarNameLen);
			szVarName[nVarNameLen] = 0;
			nIndexLen = (int) (nLength - (pcBracket - szEval) - 2);
			strncpy (szIndexVal, pcBracket + 1, nIndexLen);
			szIndexVal[nIndexLen] = 0;

			if ((strcmp (szVarName, "in") == 0) || (strcmp (szVarName, "out") == 0)) {

				/*
				 * in[] or out[] special array ;
				 * LogString ("Error - cannot set in[] and out[] variables");
				 */
				MessageBox (NULL, "Error - cannot set in[] and out[] variables", NULL, MB_OK);
				xResult = 0;
				boError = true;
			} else {

				/*
				 * Standard array var[] ;
				 * Evaluate the index
				 */
				nIndexVal = eval (szIndexVal, nIndexLen);
				xResult = SetInitVal (szVarName, nVarNameLen, nIndexVal, xValue);
			}
		} else {
			xResult = 0;
		}
	} else {

		/* Just a straightforward variable */
		xResult = SetInitVal (szEval, nLength, -1, xValue);
	}

	return xResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLCompile::GetInOutCount (char const *const szEval, int nLength) {
	char szLink[1024];
	int nLinkNum;
	int nLinksIn;
	int nLinksOut;
	int nResult;

	nResult = 0;

	if (strncmp (szEval, "in[", 3) == 0) {
		strncpy (szLink, szEval + 3, nLength - 4);
		szLink[nLength - 4] = 0;
		nLinksIn = psTopologyMirror->LinksInCount (nCurrentComponentOn);
		for (nLinkNum = 0; nLinkNum < nLinksIn; nLinkNum++) {
			if ((nLength <= 4) || (strcmp (szLink, psTopologyMirror->GetLinkInput (nCurrentComponentOn, nLinkNum)) == 0)) {
				nResult++;
			}
		}
	} else {
		if (strncmp (szEval, "out[", 4) == 0) {
			strncpy (szLink, szEval + 4, nLength - 5);
			szLink[nLength - 5] = 0;
			nLinksOut = psTopologyMirror->LinksOutCount (nCurrentComponentOn);
			for (nLinkNum = 0; nLinkNum < nLinksOut; nLinkNum++) {
				if ((nLength <= 5) || (strcmp (szLink, psTopologyMirror->GetLinkOutput (nCurrentComponentOn, nLinkNum)) == 0)) {
					nResult++;
				}
			}
		} else {

			/*
			 * LogString ("Error - input or output variable badly defined");
			 */
			MessageBox (NULL, "Error - input or output variable badly defined", NULL, MB_OK);
			boError = true;
			return 0;
		}
	}

	return nResult;
}

/*
 =======================================================================================================================
 *  Check whether an input link has a particular property, and what its value is if it exits
 =======================================================================================================================
 */
bool cXMLCompile::CheckLinkInputProperties (char const *const szPropertyID, int nCurrentComponent, int nLinkNum,
											comptype*  pxReturn) {
	bool boCertified;
	PREPROP ePreProp;
	comptype xReturn;
	cTopologyMirrorLink *  psLink;
	PROPTYPE ePropertyType;

	boCertified = FALSE;

	psLink = psTopologyMirror->GetLinkIn (nCurrentComponent, nLinkNum);

	boCertified = psLink->PropertyExists (szPropertyID);
	if (boCertified) {
		/* The property exists, so we need to extract its value */
		ePropertyType = psLink->PropertyGetType (szPropertyID);

		switch (ePropertyType) {
		case PROPTYPE_BOOL:
			xReturn = psLink->PropertyGetBool (szPropertyID);
			break;
		case PROPTYPE_FLOAT:
			xReturn = psLink->PropertyGetFloat (szPropertyID);
			break;
		case PROPTYPE_INT:
			xReturn = psLink->PropertyGetInt (szPropertyID);
			break;
		case PROPTYPE_OPTION:
			xReturn = psLink->PropertyGetOptionEnum (szPropertyID);
			xReturn = 1;
			break;
		case PROPTYPE_STRING:
			//nReturn = psLink->PropertyGetString (szPropertyID);
			xReturn = 1;
			break;
		default:
			// The property exists, but we don't have a value for it
			xReturn = 1;
			break;
		}
		// Return the value
		if (pxReturn) {
			*pxReturn = xReturn;
		}
	}

	if (!boCertified) {
		/* Check whether it's one of the predefined properties */
		for (ePreProp = (PREPROP) 0; ((ePreProp < PREPROP_NUM) && (!boCertified)); ePreProp = (PREPROP) ((int) ePreProp + 1)) {
			if (strcmp (szPropertyID, gszPreProp[ePreProp]) == 0) {
				xReturn = psTopologyMirror->GetPredefinedProperty (nCurrentComponent, nLinkNum, ePreProp);
				if (pxReturn) {
					*pxReturn = xReturn;
				}

				if (xReturn != 0) {
					boCertified = TRUE;
				}
			}
		}

		if (!boCertified) {

		/*
		 * Check whether it's one of the predefined properties
		 * There currently aren't any for input links
		 * Check if it's some other certified property
		 */
		boCertified = CheckCertifiedProperties (szPropertyID, psTopologyMirror->GetLinkInput (nCurrentComponent, nLinkNum),
												pxReturn);
		}
	}

	return boCertified;
}

/*
 =======================================================================================================================
 *  Check whether an output link has a particular property, and what its value is if it exits
 =======================================================================================================================
 */
bool cXMLCompile::CheckLinkOutputProperties (char const *const szPropertyID, int nCurrentComponent, int nLinkNum,
											 comptype*  pxReturn) {
	bool boCertified;
	PREPROP ePreProp;
	comptype xReturn;
	cTopologyMirrorLink *  psLink;
	PROPTYPE ePropertyType;

	boCertified = FALSE;

	psLink = psTopologyMirror->GetLinkOut (nCurrentComponent, nLinkNum);

	boCertified = psLink->PropertyExists (szPropertyID);
	if (boCertified) {
		/* The property exists, so we need to extract its value */
		ePropertyType = psLink->PropertyGetType (szPropertyID);

		switch (ePropertyType) {
		case PROPTYPE_BOOL:
			xReturn = psLink->PropertyGetBool (szPropertyID);
			break;
		case PROPTYPE_FLOAT:
			xReturn = psLink->PropertyGetFloat (szPropertyID);
			break;
		case PROPTYPE_INT:
			xReturn = psLink->PropertyGetInt (szPropertyID);
			break;
		case PROPTYPE_OPTION:
			xReturn = psLink->PropertyGetOptionEnum (szPropertyID);
			xReturn = 1;
			break;
		case PROPTYPE_STRING:
			//nReturn = psLink->PropertyGetString (szPropertyID);
			xReturn = 1;
			break;
		default:
			// The property exists, but we don't have a value for it
			xReturn = 1;
			break;
		}
		// Return the value
		if (pxReturn) {
			*pxReturn = xReturn;
		}
	}

	if (!boCertified) {
		/* Check whether it's one of the predefined properties */
		for (ePreProp = (PREPROP) 0; ((ePreProp < PREPROP_NUM) && (!boCertified)); ePreProp = (PREPROP) ((int) ePreProp + 1)) {
			if (strcmp (szPropertyID, gszPreProp[ePreProp]) == 0) {
				xReturn = psTopologyMirror->GetPredefinedProperty (nCurrentComponent, nLinkNum, ePreProp);
				if (pxReturn) {
					*pxReturn = xReturn;
				}

				if (xReturn != 0) {
					boCertified = TRUE;
				}
			}
		}

		/* Check if it's some other certified property */
		if (!boCertified) {
			boCertified = CheckCertifiedProperties (szPropertyID,
													psTopologyMirror->GetLinkOutput (nCurrentComponent, nLinkNum), pxReturn);
		}
	}

	return boCertified;
}

/*
 =======================================================================================================================
 *  Check whether a node has a particular property, and what its value is if it exits
 =======================================================================================================================
 */
bool cXMLCompile::CheckNodeProperties (char const *const szPropertyID, int nCurrentComponent, comptype* pxReturn) {
	bool boCertified;
	PREPROP ePreProp;
	comptype xReturn;
	cTopologyMirrorNode *  psBox;
	PROPTYPE ePropertyType;

	boCertified = FALSE;

	if (nCurrentComponent >= 0) {
		/* Check whether it's an assigned property */
		/* Check if the property exists */
		psBox = psTopologyMirror->GetNode (nCurrentComponent);

		boCertified = psBox->PropertyExists (szPropertyID);
		if (boCertified) {
			/* The property exists, so we need to extract its value */
			ePropertyType = psBox->PropertyGetType (szPropertyID);

			switch (ePropertyType) {
			case PROPTYPE_BOOL:
				xReturn = psBox->PropertyGetBool (szPropertyID);
				break;
			case PROPTYPE_FLOAT:
				xReturn = psBox->PropertyGetFloat (szPropertyID);
				break;
			case PROPTYPE_INT:
				xReturn = psBox->PropertyGetInt (szPropertyID);
				break;
			case PROPTYPE_OPTION:
				xReturn = psBox->PropertyGetOptionEnum (szPropertyID);
				break;
			case PROPTYPE_STRING:
				//nReturn = psBox->PropertyGetString (szPropertyID);
				xReturn = 1;
				break;
			default:
				// The property exists, but we don't have a value for it
				xReturn = 1;
				break;
			}
			// Return the value
			if (pxReturn) {
				*pxReturn = xReturn;
			}
		}

		if (!boCertified) {
			/* Check whether it's one of the predefined properties */
			for (ePreProp = (PREPROP) 0; ((ePreProp < PREPROP_NUM) && (!boCertified)); ePreProp = (PREPROP) ((int) ePreProp + 1)) {
				if (strcmp (szPropertyID, gszPreProp[ePreProp]) == 0) {
					xReturn = psTopologyMirror->GetPredefinedProperty (nCurrentComponent, ePreProp);
					if (pxReturn) {
						*pxReturn = xReturn;
					}

					if (xReturn != 0) {
						boCertified = TRUE;
					}
				}
			}

			/* Check if it's some other certified property */
			if (!boCertified) {
				boCertified = CheckCertifiedProperties (szPropertyID, psTopologyMirror->GetComponentProperties (nCurrentComponent), pxReturn);
			}
		}
	}

	return boCertified;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
CompileHandler * cXMLCompile::GetFileHandler () {
	return psHandler;
}
