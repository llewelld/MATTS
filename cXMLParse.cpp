/*$T MATTS/cXMLParse.cpp GC 1.140 07/01/09 21:12:10 */
/*
 * ;
 * Name: cTopology.cpp ;
 * Last Modified: 20/12/05 ;
 * ;
 * Purpose: Deal with network topology between agents ;
 * ;
 * include <iostream.h>
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <crtdbg.h>

#include "cXMLParse.h"
#include "cTopology.h"
#include "cAnalyse.h"
#include "resource.h"
#include "cTopologyMirror.h"

XERCES_CPP_NAMESPACE_USE

/*
 * void CalculateLinkList (void) {} ;
 * int LinksInCount (int nPoint) {return 0;
 * } ;
 * int LinksOutCount (int nPoint) {return 0;
 * } ;
 * char * GetLinkInput (int nPoint, int nLinkNum) {return NULL;
 * } ;
 * char * GetLinkOutput (int nPoint, int nLinkNum) {return NULL;
 * } ;
 * int LinkTo (int nPoint, int nLinkNum) {return 0;
 * } ;
 * int LinkFrom (int nPoint, int nLinkNum) {return 0;
 * }
 */
void threadProc (cXMLParse* pcXMLParse);

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */

cXMLParse::cXMLParse (cTopology* pcTopology) {
	gValScheme = XercesDOMParser::Val_Auto;
	gpDomParser = NULL;
	gnResultListNum = 0;
	gpcTopology = pcTopology;
	gboLog = FALSE;
	gfPerformanceTime = 0.0f;
	gnRecurseLevel = 0;
	gnCurrentLinkOut = -1;
	gnCurrentLinkIn = -1;
	gboCompleted = true;
	gboResult = true;
	psTopologyMirror = NULL;

	psXMLCompile = new cXMLCompile ();
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
cXMLParse::~cXMLParse () {
	if (psTopologyMirror) {
		delete psTopologyMirror;
		psTopologyMirror = NULL;
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::XMLFileLoaded (void) {
	return (gpDomParser != NULL);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::LoadXMLFile (char* szFilename) {
	XMLPlatformUtils::Initialize ();
	strncpy (gXmlFile, szFilename, sizeof (gXmlFile) - 1);

	/* create a new parser */
	if (gpDomParser) {
		delete gpDomParser;
	}

	gpDomParser = new XercesDOMParser ();
	gpDomParser->setValidationScheme (XercesDOMParser::Val_Auto);
	gpDomParser->setDoNamespaces (true);
	gpDomParser->setDoSchema (true);
	gpDomParser->setValidationSchemaFullChecking (true);
	gpDomParser->setCreateEntityReferenceNodes (true);

	/* parse the document and get the DOM Document interface */
	try
	{
		gpDomParser->parse (gXmlFile);
	}

	catch (...) {
		LogString ("Error");
		MessageBox (NULL, "Error", NULL, MB_OK);
	}

	/*
	 * Clean up ;
	 * XMLPlatformUtils::Terminate();
	 */
	psXMLCompile->LoadFile (szFilename);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::ParseStart (HWND hWnd) {
	cTopologyNode*	psIterator;

	bool boNodesExist;

	psIterator = gpcTopology->GetpList ()->startIterator ();
	if (gpcTopology->GetpList ()->isIteratorValid (psIterator)) {
		boNodesExist = TRUE;
	} else {
		boNodesExist = FALSE;
		gpcTopology->boAnalysisCompleted = FALSE;
	}

	ghWnd = hWnd;
	if ((gboCompleted) && (psXMLCompile->GetCompiled ()) && (boNodesExist)) {
		gboCompleted = FALSE;

		if (psTopologyMirror) {
			delete psTopologyMirror;
			psTopologyMirror = NULL;
		}

		psTopologyMirror = new cTopologyMirror (gpcTopology);

		/*
		 * SendMessage (ghWnd, (UINT)WM_COMMAND, (LPARAM)ID_TOPOLOGY_SHOWRESULTS, 0);
		 */
		ghThreadHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) threadProc, this, 0, NULL);

		/*
		 * SetThreadPriority (ghThreadHandle, THREAD_PRIORITY_BELOW_NORMAL);
		 */
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void threadProc (cXMLParse* pcXMLParse) {
	pcXMLParse->CheckCompositionCompiled ();

	/*
	 * pcXMLParse->LogInitialise ();
	 * pcXMLParse->gboCompleted = FALSE;
	 * pcXMLParse->CheckComposition ();
	 * pcXMLParse->gboCompleted = TRUE;
	 * pcXMLParse->LogDeinitialise ();
	 * SendMessage (pcXMLParse->ghWnd, (UINT)WM_COMMAND,
	 * (LPARAM)ID_TOPOLOGY_SHOWRESULTS, 0);
	 */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::CheckCompositionCompiled (void) {
	gboCompleted = FALSE;
	gpcTopology->boAnalysisCompleted = FALSE;

	/* Instead of starting a new thread we call the new cXMLCompile analysis engine */
	psXMLCompile->CheckComposition (psTopologyMirror);
	if (psTopologyMirror) {
		delete psTopologyMirror;
		psTopologyMirror = NULL;
	}

	gboResult = true;
	for (gnResultListNum = 0; gnResultListNum < psXMLCompile->GetCompiled ()->nSandboxes; gnResultListNum++) {
		if (psXMLCompile->GetCompiled ()->apsSandbox[gnResultListNum]->boTestResult == false) {
			gboResult = false;
		}

		gboResultList[gnResultListNum] = psXMLCompile->GetCompiled ()->apsSandbox[gnResultListNum]->boTestResult;
		strncpy (gszResultList[gnResultListNum],
				 psXMLCompile->GetCompiled ()->apsSandbox[gnResultListNum]->GetID ().c_str (), 32);
		strncpy (gszResultDesc[gnResultListNum],
				 psXMLCompile->GetCompiled ()->apsSandbox[gnResultListNum]->szDescription.c_str (), 64);
	}

	gboCompleted = TRUE;
	gpcTopology->boAnalysisCompleted = TRUE;
	gpcTopology->boAnalysisResult = gboResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::CheckComposition (void) {
	int nCount;
	DOMNode*  pdnSandbox;
	XMLCh const*  xszSandbox;
	XMLCh const*  xszId;
	XMLCh const*  xszConfig;
	XMLCh const*  xszCompose;
	XMLCh const*  xszType;
	XMLCh const*  xszExtended;
	char szSandboxId[1024];
	char szSandboxDesc[1024];
	bool boChecked;
	bool boExtended;
	bool boTestReturn;

	xszSandbox = XMLString::transcode ("sandbox");
	xszId = XMLString::transcode ("id");
	xszConfig = XMLString::transcode ("config");
	xszCompose = XMLString::transcode ("compose");
	xszType = XMLString::transcode ("type");
	xszExtended = XMLString::transcode ("extended");
	boChecked = false;
	boExtended = false;

	gpcTopology->ClearSelected ();
	pOptions = cOptions::create ();

	/*
	 * CheckResults CheckResultsDialog;
	 */
	gnResultListNum = 0;
	gfPerformanceTime = 0.0f;

	LoadXMLFile (gXmlFile);

	if (gpDomParser) {
		DOMNodeList*  pNodeList;

		xercesc_2_7::DOMDocument * doc = gpDomParser->getDocument ();

		/*
		 * We will travel through the tree recursively to discover if ;
		 * the pattern is satisfied at each stage. ;
		 * There are a couple of things to watch out for:- ;
		 * - If more than one pattern is satisfied, we must test them all (recursively) ;
		 * - At each stage we must note that a component has been dealt with, so that ;
		 * cycles can be recognised
		 */
		gpcTopology->CalculateLinkList ();

		pNodeList = doc->getElementsByTagName (xszCompose);
		if (pNodeList->getLength () > 0) {
			LARGE_INTEGER nPerformanceTimeStart;
			LARGE_INTEGER nPerformanceTimeEnd;
			LARGE_INTEGER nPerformanceFrequency;
			QueryPerformanceFrequency (&nPerformanceFrequency);
			QueryPerformanceCounter (&nPerformanceTimeStart);

#if defined PERFORMANCE_TIME
			for (int nRepeat = 0; nRepeat < PERFORMANCE_TIME; nRepeat++)
#endif
			{
#if defined PERFORMANCE_TIME
				_RPT1 (_CRT_WARN, "Check number %d\n", nRepeat);
#endif

				DOMNode*  pParseNode;

				DOMNamedNodeMap*  pnmAttributes;
				DOMNode*  pdnType;

				pParseNode = pNodeList->item (0);
				pnmAttributes = pParseNode->getAttributes ();
				pdnType = pnmAttributes->getNamedItem (xszType);

				if (pdnType) {
					XMLCh const*  xszType;
					xszType = pdnType->getNodeValue ();
					if (XMLString::equals (xszType, xszExtended)) {
						boExtended = true;
					} else {
						boExtended = false;
					}
				}

				for (pdnSandbox = pParseNode->getFirstChild (); pdnSandbox; pdnSandbox = pdnSandbox->getNextSibling ()) {
					if (XMLString::equals (pdnSandbox->getNodeName (), xszSandbox)) {
						DOMNamedNodeMap*  pnmSandBoxAttributes;

						pnmSandBoxAttributes = pdnSandbox->getAttributes ();
						if (pnmSandBoxAttributes) {
							DOMNode*  pdnSandBoxId;
							XMLCh const*  xszSandboxId;

							XMLCh const*  xszSandboxDesc;
							DOMNode*  pdnConfig;
							XMLCh const*  xszTemp;
							DOMElement*	 psElementFind;
							DOMNode*  psElement;
							pdnSandBoxId = pnmSandBoxAttributes->getNamedItem (xszId);
							xszSandboxId = pdnSandBoxId->getNodeValue ();

							strcpy (szSandboxId, "");
							XMLString::transcode (xszSandboxId, szSandboxId, sizeof (szSandboxId) - 1);

							xszSandboxDesc = pdnSandbox->getTextContent ();

							strcpy (szSandboxDesc, "");
							XMLString::transcode (xszSandboxDesc, szSandboxDesc, sizeof (szSandboxDesc) - 1);

							pdnConfig = pnmSandBoxAttributes->getNamedItem (xszConfig);
							xszTemp = pdnConfig->getNodeValue ();
							psElementFind = doc->getElementById (xszTemp);
							psElement = NULL;

							if (psElementFind) {

								/*
								 * psElement = psElementFind->cloneNode (true);
								 */
								psElement = psElementFind;;
							}

							if (psElement) {
								for (nCount = -LINKLIST_OFFSET; nCount < POINTS_MAX; nCount++) {
									gnBoxVisited[nCount + LINKLIST_OFFSET] = 0;
								}

								/* This is where all of the actual checking work is done */
								if (boExtended) {
									gnRecurseLevel = 0;
									gnCurrentLinkOut = -1;
									boTestReturn = TestExtended (psElement, 0);
								} else {
									boTestReturn = Test (psElement, 0);
								}

								if (boTestReturn) {
									if (gnResultListNum < RESULTLIST_LEN) {
										gboResultList[gnResultListNum] = true;
										strncpy (gszResultList[gnResultListNum], szSandboxId, 32);

										/*
										 * _snprintf (gszResultList[gnResultListNum], 64, "%s \t\t satisfied", szSandboxId);
										 */
										strncpy (gszResultDesc[gnResultListNum], szSandboxDesc, 64);
										gnResultListNum++;
									}
								} else {
									if (gnResultListNum < RESULTLIST_LEN) {
										gboResultList[gnResultListNum] = false;
										strncpy (gszResultList[gnResultListNum], szSandboxId, 32);

										/*
										 * _snprintf (gszResultList[gnResultListNum], 64, "%s \t\t not satisfied", szSandboxId);
										 */
										strncpy (gszResultDesc[gnResultListNum], szSandboxDesc, 64);
										gnResultListNum++;
									}
								}

								boChecked = true;
							}

							/*
							 * if (psElement) ;
							 * { ;
							 * psElement->release ();
							 * psElement = NULL;
							 * }
							 */
						}
					}
				}
			}

			QueryPerformanceCounter (&nPerformanceTimeEnd);
			_RPT1 (_CRT_WARN, "Start time %d\n", (LONGLONG) nPerformanceTimeStart.QuadPart);
			_RPT1 (_CRT_WARN, "End time   %d\n", (LONGLONG) nPerformanceTimeEnd.QuadPart);
			_RPT1 (_CRT_WARN, "Difference %d\n",
				   ((LONGLONG) nPerformanceTimeEnd.QuadPart - (LONGLONG) nPerformanceTimeStart.QuadPart));
			_RPT1 (_CRT_WARN, "Frequency  %d\n", (LONGLONG) nPerformanceFrequency.QuadPart);
			_RPT1 (_CRT_WARN, "Amounts to %f seconds\n", (
					   (float) ((LONGLONG) nPerformanceTimeEnd.QuadPart - (LONGLONG) nPerformanceTimeStart.QuadPart) /
					   (float) ((LONGLONG) nPerformanceFrequency.QuadPart)));
			gfPerformanceTime =
				(
					(float) ((LONGLONG) nPerformanceTimeEnd.QuadPart - (LONGLONG) nPerformanceTimeStart.QuadPart) /
					(float) ((LONGLONG) nPerformanceFrequency.QuadPart)
				);
		}
	} else {
		LogString ("No XML document loaded");
		MessageBox (NULL, "No XML document loaded", NULL, MB_OK);
	}

	/*
	 * if (boChecked) ;
	 * { ;
	 * CheckResultsDialog.DoModal ();
	 * }
	 */
	Sleep (pOptions->getSelectSpeed ());
	gpcTopology->ClearSelected ();
	Sleep (pOptions->getColourSpeed ());
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::Test (DOMNode* pdnCurrentNode, int nCurrentComponent) {
	XMLCh const*  xszComponent;
	XMLCh const*  xszInput;
	XMLCh const*  xszOutput;
	XMLCh const*  xszFormat;
	XMLCh const*  xszConfig;
	XMLCh const*  xszCycle;
	XMLCh const*  xszRequired;
	XMLCh const*  xszDisallow;
	XMLCh const*  xszOptional;
	XMLCh const*  xszNull;
	XMLCh const*  xszExternal;

	XMLCh*	xszLinksInput;
	XMLCh*	xszLinksOutput;
	int nLinksIn;
	int nLinksOut;
	int nLinkNum;
	int nStartOfLink;
	int nEndOfLink;
	DOMNode*  pdnComponent;
	DOMNode*  pdnNextNode;
	CYCLE eCycleOut;
	CYCLE eCycleIn;

	xszComponent = XMLString::transcode ("component");
	xszInput = XMLString::transcode ("input");
	xszOutput = XMLString::transcode ("output");
	xszFormat = XMLString::transcode ("format");
	xszConfig = XMLString::transcode ("config");
	xszCycle = XMLString::transcode ("cycle");
	xszRequired = XMLString::transcode ("required");
	xszDisallow = XMLString::transcode ("disallow");
	xszOptional = XMLString::transcode ("optional");
	xszNull = XMLString::transcode ("");
	xszExternal = XMLString::transcode ("external");

	for (pdnComponent = pdnCurrentNode->getFirstChild (); pdnComponent; pdnComponent = pdnComponent->getNextSibling ()) {
		if (XMLString::equals (pdnComponent->getNodeName (), xszComponent)) {

			/*
			 * This is a component, so we need to check it ;
			 * Check the inputs
			 */
			DOMNode*  pdnInput;

			for (pdnInput = pdnComponent->getFirstChild (); pdnInput; pdnInput = pdnInput->getNextSibling ()) {
				if (XMLString::equals (pdnInput->getNodeName (), xszInput)) {

					/*
					 * This is an input, so we need to check it ;
					 * Check the attributes and see if they fit the box
					 */
					DOMNamedNodeMap*  pnmInAttributes;

					/* First check for cycles */
					DOMNode*  pdnInCycle;
					DOMNode*  pdnInFormat;
					pnmInAttributes = pdnInput->getAttributes ();
					pdnInCycle = pnmInAttributes->getNamedItem (xszCycle);

					eCycleIn = CYCLE_OPTIONAL;
					if (pdnInCycle) {
						XMLCh const*  xszCycle;
						xszCycle = pdnInCycle->getNodeValue ();
						if (XMLString::equals (xszCycle, xszRequired)) {
							eCycleIn = CYCLE_REQUIRED;
						}

						if (XMLString::equals (xszCycle, xszDisallow)) {
							eCycleIn = CYCLE_DISALLOW;
						}

						if (XMLString::equals (xszCycle, xszOptional)) {
							eCycleIn = CYCLE_OPTIONAL;
						}
					}

					pdnInFormat = pnmInAttributes->getNamedItem (xszFormat);

					if (pdnInFormat) {

						/* Check the format attribute */
						XMLCh const*  xszInFormat;

						/* Compare the inputs against this */
						XMLStringTokenizer*	 pstInTokenizer;
						bool boInputsMatch;
						XMLCh*	xszToken;
						bool boMultiple;
						int nRepeatCount;
						bool boAny;
						xszInFormat = pdnInFormat->getNodeValue ();
						pstInTokenizer = new XMLStringTokenizer (xszInFormat);
						boInputsMatch = true;

						nLinksIn = gpcTopology->LinksInCount (nCurrentComponent);
						if ((pstInTokenizer->countTokens () != nLinksIn)
						&& (XMLString::findAny (xszInFormat, XMLString::transcode ("*")) == 0)) {
							boInputsMatch = false;
						}

						/*/
						 *  Add code to check for "*"s in the input description Check if there is an asterisk postfixed
						 */
						xszToken = pstInTokenizer->nextToken ();
						boMultiple = false;
						nRepeatCount = 0;
						boAny = false;

						if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

							/* Remove the asterisk */
							XMLString::copyNString (xszToken, xszToken, XMLString::stringLen (xszToken) - 1);
							boMultiple = true;
							if (XMLString::stringLen (xszToken) == 0) {
								boAny = true;
							}
						}

						for (nLinkNum = 0; (boInputsMatch && (nLinkNum < nLinksIn) && (!boAny)); nLinkNum++) {
							xszLinksInput = XMLString::transcode (gpcTopology->GetLinkInput (nCurrentComponent, nLinkNum));

							/* Establish the box at the start of the link */
							nStartOfLink = gpcTopology->LinkFrom (nCurrentComponent, nLinkNum);

							if (((!boMultiple) || (nRepeatCount == 0)) && (!XMLString::equals (xszToken, xszLinksInput))) {
								boInputsMatch = false;
							}

							if ((boMultiple && (nRepeatCount > 0)) && (!XMLString::equals (xszToken, xszLinksInput))) {
								xszToken = pstInTokenizer->nextToken ();
								nRepeatCount = 0;
								boMultiple = false;
								boAny = false;
								if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

									/* Remove the asterisk */
									XMLString::copyNString (xszToken, xszToken, XMLString::stringLen (xszToken) - 1);
									boMultiple = true;
									if (XMLString::stringLen (xszToken) == 0) {
										boAny = true;
									}
								}

								if ((!XMLString::equals (xszToken, xszLinksInput)) && !boAny) {
									boInputsMatch = false;
								}
							}

							if (boInputsMatch && boMultiple) {
								nRepeatCount++;
							}

							if (!boMultiple) {
								xszToken = pstInTokenizer->nextToken ();
								nRepeatCount = 0;
								boMultiple = false;
								boAny = false;
								if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

									/* Remove the asterisk */
									XMLString::copyNString (xszToken, xszToken, XMLString::stringLen (xszToken) - 1);
									boMultiple = true;
									if (XMLString::stringLen (xszToken) == 0) {
										boAny = true;
									}
								}
							}

							if ((gnBoxVisited[nStartOfLink + LINKLIST_OFFSET] > 0) && (((gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] > 0) && (eCycleIn == CYCLE_DISALLOW)) || ((gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] <= 0)
								&& (eCycleIn == CYCLE_REQUIRED)))) {
								boInputsMatch = false;
							}
						}

						if ((pstInTokenizer->hasMoreTokens ()) && (!boAny)) {
							boInputsMatch = false;
						}

						if (((!boMultiple) && (XMLString::stringLen (xszToken) != 0))
						|| ((!boAny) && boMultiple && (nRepeatCount == 0))) {
							boInputsMatch = false;
						}

						delete pstInTokenizer;

						if (boInputsMatch && (gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] > 0)) {

							/* Return backwards */
							return true;
						}

						if (boInputsMatch) {

							/* Check the ouputs */
							DOMNode*  pdnOutput;
							for (pdnOutput = pdnComponent->getFirstChild (); pdnOutput;
								 pdnOutput = pdnOutput->getNextSibling ()) {
								if (XMLString::equals (pdnOutput->getNodeName (), xszOutput)) {

									/*
									 * This is an output, so we need to check it ;
									 * Check the attributes and see if they fit the box
									 */
									DOMNamedNodeMap*  pnmOutAttributes;
									DOMNode*  pdnOutFormat;
									pnmOutAttributes = pdnOutput->getAttributes ();
									pdnOutFormat = pnmOutAttributes->getNamedItem (xszFormat);

									if (pdnOutFormat) {

										/* Check the format attribute */
										XMLCh const*  xszOutFormat;
										bool boOutputsMatch;

										xszOutFormat = pdnOutFormat->getNodeValue ();
										boOutputsMatch = true;

										if (nCurrentComponent == POINT_EXTERNAL) {
											if (!XMLString::equals (xszOutFormat, xszExternal)) {

												/* This shouldn't be an external link */
												boOutputsMatch = false;
											} else {

												/*
												 * This should be an external link;
												 * we don't need to recurse any further
												 */
												return true;
											}
										} else {

											/* Compare the outputs against this */
											XMLStringTokenizer*	 pstOutTokenizer;
											XMLCh*	xszToken;
											bool boMultiple;
											int nRepeatCount;
											bool boAny;
											pstOutTokenizer = new XMLStringTokenizer (xszOutFormat);
											nLinksOut = gpcTopology->LinksOutCount (nCurrentComponent);
											if ((pstOutTokenizer->countTokens () != nLinksOut)
											&& (XMLString::findAny (xszOutFormat, XMLString::transcode ("*")) == 0)) {
												boOutputsMatch = false;
											}

											/* Check if there is an asterisk postfixed */
											xszToken = pstOutTokenizer->nextToken ();
											boMultiple = false;
											nRepeatCount = 0;
											boAny = false;

											if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

												/* Remove the asterisk */
												XMLString::copyNString (xszToken, xszToken,
																		XMLString::stringLen (xszToken) - 1);
												boMultiple = true;
												if (XMLString::stringLen (xszToken) == 0) {
													boAny = true;
												}
											}

											for (nLinkNum = 0; (boOutputsMatch && (nLinkNum < nLinksOut) && (!boAny));
												 nLinkNum++) {
												xszLinksOutput = XMLString::transcode (gpcTopology->GetLinkOutput (nCurrentComponent, nLinkNum));
												if (((!boMultiple) || (nRepeatCount == 0))
												&& (!XMLString::equals (xszToken, xszLinksOutput))) {
													boOutputsMatch = false;
												}

												if ((boMultiple && (nRepeatCount > 0))
												&& (!XMLString::equals (xszToken, xszLinksOutput))) {
													xszToken = pstOutTokenizer->nextToken ();
													nRepeatCount = 0;
													boMultiple = false;
													boAny = false;
													if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

														/* Remove the asterisk */
														XMLString::copyNString (xszToken, xszToken,
																				XMLString::stringLen (xszToken) - 1);
														boMultiple = true;
														if (XMLString::stringLen (xszToken) == 0) {
															boAny = true;
														}
													}

													if ((!XMLString::equals (xszToken, xszLinksOutput)) && !boAny) {
														boOutputsMatch = false;
													}
												}

												if (boOutputsMatch && boMultiple) {
													nRepeatCount++;
												}

												if (!boMultiple) {
													xszToken = pstOutTokenizer->nextToken ();
													nRepeatCount = 0;
													boMultiple = false;
													boAny = false;
													if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

														/* Remove the asterisk */
														XMLString::copyNString (xszToken, xszToken,
																				XMLString::stringLen (xszToken) - 1);
														boMultiple = true;
														if (XMLString::stringLen (xszToken) == 0) {
															boAny = true;
														}
													}
												}
											}

											if ((pstOutTokenizer->hasMoreTokens ()) && (!boAny)) {
												boOutputsMatch = false;
											}

											if (((!boMultiple) && (XMLString::stringLen (xszToken) != 0))
											|| ((!boAny) && boMultiple && (nRepeatCount == 0))) {
												boOutputsMatch = false;
											}

											delete pstOutTokenizer;
										}

										/* Compare the outputs against this */
										if (boOutputsMatch) {
											bool boSuccess;

											/* Get the config atribute if there is one */
											DOMNode*  pdnOutConfig;

											DOMNode*  pdnOutCycle;
											boSuccess = true;
											pdnOutConfig = pnmOutAttributes->getNamedItem (xszConfig);

											if (pdnOutConfig) {
												pdnNextNode = (pdnCurrentNode->getOwnerDocument ())->getElementById (pdnOutConfig->getNodeValue ());

												if (!pdnNextNode) {
													LogString ("Failed to find config id");
													MessageBox (NULL, "Failed to find config id", NULL, MB_OK);
													pdnNextNode = pdnComponent;
												}
											} else {
												pdnNextNode = pdnComponent;
											}

											/* Find out if we're okay to cycle or not */
											pdnOutCycle = pnmOutAttributes->getNamedItem (xszCycle);

											eCycleOut = CYCLE_OPTIONAL;
											if (pdnOutCycle) {
												XMLCh const*  xszCycle;
												xszCycle = pdnOutCycle->getNodeValue ();
												if (XMLString::equals (xszCycle, xszRequired)) {
													eCycleOut = CYCLE_REQUIRED;
												}

												if (XMLString::equals (xszCycle, xszDisallow)) {
													eCycleOut = CYCLE_DISALLOW;
												}

												if (XMLString::equals (xszCycle, xszOptional)) {
													eCycleOut = CYCLE_OPTIONAL;
												}
											}

											gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET]++;

											/* Cycle through each output (the recursive bit) */
											for (nLinkNum = 0; ((nLinkNum < nLinksOut) && boSuccess); nLinkNum++) {

												/* Establish the box at the end of the link */
												nEndOfLink = gpcTopology->LinkTo (nCurrentComponent, nLinkNum);

												/* Find out if this is a cycle */
												if (((gnBoxVisited[nEndOfLink + LINKLIST_OFFSET] > 0)
												&& (eCycleOut == CYCLE_DISALLOW)) || ((gnBoxVisited[nEndOfLink + LINKLIST_OFFSET] <= 0)
												&& (eCycleOut == CYCLE_REQUIRED))) {
													boSuccess = false;
												}

												/* This is the actual recursive bit */
												if (boSuccess && (!Test (pdnNextNode, nEndOfLink))) {
													boSuccess = false;
												}
											}

											if (boSuccess) {
												return true;
											}

											gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] = 0;
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

	return false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLParse::ResultListNum (void) {
	return gnResultListNum;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
char* cXMLParse::ResultListItem (int nItem) {
	char*  szReturn;
	if (nItem < gnResultListNum) {
		szReturn = gszResultList[nItem];
	} else {
		szReturn = NULL;
	}

	return szReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::ResultListItemResult (int nItem) {
	bool boReturn;
	if (nItem < gnResultListNum) {
		boReturn = gboResultList[nItem];
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
char* cXMLParse::ResultListDesc (int nItem) {
	char*  szReturn;
	if (nItem < gnResultListNum) {
		szReturn = gszResultDesc[nItem];
	} else {
		szReturn = NULL;
	}

	return szReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
float cXMLParse::PerformanceTiming (void) {
	return gfPerformanceTime;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::TestExtended (DOMNode* pdnCurrentNode, int nCurrentComponent) {
	XMLCh const*  xszComponent;
	XMLCh const*  xszInput;
	XMLCh const*  xszOutput;
	XMLCh const*  xszFormat;
	XMLCh const*  xszConfig;
	XMLCh const*  xszCycle;
	XMLCh const*  xszRequired;
	XMLCh const*  xszDisallow;
	XMLCh const*  xszOptional;
	XMLCh const*  xszExternal;
	XMLCh const*  xszFollow;
	XMLCh const*  xszYes;
	XMLCh const*  xszNo;
	XMLCh const*  xszFresh;

	XMLCh*	xszLinksInput;
	XMLCh*	xszLinksOutput;
	int nLinksIn;
	int nLinksOut;
	int nLinkNum;
	int nStartOfLink;
	int nEndOfLink;
	DOMNode*  pdnComponent;
	DOMNode*  pdnNextNode;
	CYCLE eCycleOut;
	CYCLE eCycleIn;
	FOLLOW eFollowOut;
	FOLLOW eFollowIn;
	int nChannel;

	xszComponent = XMLString::transcode ("component");
	xszInput = XMLString::transcode ("input");
	xszOutput = XMLString::transcode ("output");
	xszFormat = XMLString::transcode ("format");
	xszConfig = XMLString::transcode ("config");
	xszCycle = XMLString::transcode ("cycle");
	xszRequired = XMLString::transcode ("required");
	xszDisallow = XMLString::transcode ("disallow");
	xszOptional = XMLString::transcode ("optional");
	xszExternal = XMLString::transcode ("external");
	xszFollow = XMLString::transcode ("follow");
	xszYes = XMLString::transcode ("yes");
	xszNo = XMLString::transcode ("no");
	xszFresh = XMLString::transcode ("fresh");

	gnRecurseLevel++;
	LogValue ("\n      ****** Current component is %d ******", nCurrentComponent);
	LogValue ("Recursion level %d", gnRecurseLevel);

	/*
	 * Set the global component variable, as it may be used when evaluating ;
	 * to resolve in[] and out[] variables ;
	 * Establish the global stack variables
	 */
	GlobalStackVariables (nCurrentComponent);

	for (pdnComponent = pdnCurrentNode->getFirstChild (); pdnComponent; pdnComponent = pdnComponent->getNextSibling ()) {
		if (XMLString::equals (pdnComponent->getNodeName (), xszComponent)) {
			LogString ("Node");
			LogNode (pdnComponent);

			/* This is a component, so we need to check it */
			if (EvaluateCond (pdnComponent)) {
				DOMNode*  pdnInput;

				/* Undertake any processes */
				ProcessScan (pdnComponent);

				/* Check the inputs */
				for (pdnInput = pdnComponent->getFirstChild (); pdnInput; pdnInput = pdnInput->getNextSibling ()) {
					if (XMLString::equals (pdnInput->getNodeName (), xszInput)) {
						DOMNode*  pdnInFormat;
						DOMNamedNodeMap*  pnmInAttributes;

						/* First check for cycles */
						DOMNode*  pdnInCycle;
						LogString ("Input");

						/*
						 * This is an input, so we need to check it ;
						 * Check the attributes and see if they fit the box
						 */
						pnmInAttributes = pdnInput->getAttributes ();
						pdnInCycle = pnmInAttributes->getNamedItem (xszCycle);

						eCycleIn = CYCLE_OPTIONAL;
						if (pdnInCycle) {
							XMLCh const*  xszCycle;
							xszCycle = pdnInCycle->getNodeValue ();
							if (XMLString::equals (xszCycle, xszRequired)) {
								eCycleIn = CYCLE_REQUIRED;
								LogString ("Cycle in required");
							}

							if (XMLString::equals (xszCycle, xszDisallow)) {
								eCycleIn = CYCLE_DISALLOW;
								LogString ("Cycle in disallow");
							}

							if (XMLString::equals (xszCycle, xszOptional)) {
								eCycleIn = CYCLE_OPTIONAL;
								LogString ("Cycle in optional");
							}
						}

						pdnInFormat = pnmInAttributes->getNamedItem (xszFormat);

						if (pdnInFormat) {
							DOMNode*  pdnInFollow;
							XMLCh*	xszToken;
							bool boMultiple;
							int nRepeatCount;
							bool boAny;
							XMLStringTokenizer*	 pstInTokenizer;
							bool boInputsMatch;
							XMLCh const*  xszInFormat;
							LogString ("Input format");

							/* Check the format attribute */
							xszInFormat = pdnInFormat->getNodeValue ();
							LogXString (xszInFormat);

							/* Compare the inputs against this */
							pstInTokenizer = new XMLStringTokenizer (xszInFormat);
							boInputsMatch = true;

							nLinksIn = gpcTopology->LinksInCount (nCurrentComponent);
							if ((pstInTokenizer->countTokens () != nLinksIn)
							&& (XMLString::findAny (xszInFormat, XMLString::transcode ("*")) == 0)) {
								boInputsMatch = false;
								LogValue ("Input links don't match (%d link in)", nLinksIn);
							}

							/* Check if there is an asterisk postfixed */
							xszToken = pstInTokenizer->nextToken ();
							boMultiple = false;
							nRepeatCount = 0;
							boAny = false;

							if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

								/* Remove the asterisk */
								XMLString::copyNString (xszToken, xszToken, XMLString::stringLen (xszToken) - 1);
								boMultiple = true;
								if (XMLString::stringLen (xszToken) == 0) {
									boAny = true;
								}
							}

							for (nLinkNum = 0; (boInputsMatch && (nLinkNum < nLinksIn) && (!boAny)); nLinkNum++) {
								xszLinksInput = XMLString::transcode (gpcTopology->GetLinkInput (nCurrentComponent, nLinkNum));

								/* Establish the box at the start of the link */
								nStartOfLink = gpcTopology->LinkFrom (nCurrentComponent, nLinkNum);

								/*
								 * nEndOfLink = gpcTopology->LinkTo (nCurrentComponent, nLinkNum);
								 * if (((!boMultiple) || (nRepeatCount == 0)) && (!XMLString::equals (xszToken,
								 * xszLinksInput))) ;
								 * { ;
								 * boInputsMatch = false;
								 * }
								 */
								if (((!boMultiple) || (nRepeatCount == 0))) {
									nChannel = gpcTopology->GetLinkInputChannelIn (nCurrentComponent, nLinkNum);

									/* Check for the property */
									if (!AnalyseComponent (xszToken, nCurrentComponent, nLinkNum, pdnInput,
										LINKTYPE_INPUT)) {

										/* if (!XMLString::equals (xszToken, xszLinksInput)) */
										boInputsMatch = false;
										LogValue ("Input links (not multiple) don't match on link %d (Analysis failed)",
												  nLinkNum);
									}
								}

								if (boMultiple && (nRepeatCount > 0)) {

									/* if (!XMLString::equals (xszToken, xszLinksInput)) */
									nChannel = gpcTopology->GetLinkInputChannelIn (nCurrentComponent, nLinkNum);
									if (!AnalyseComponent (xszToken, nCurrentComponent, nLinkNum, pdnInput,
										LINKTYPE_INPUT)) {
										xszToken = pstInTokenizer->nextToken ();
										nRepeatCount = 0;
										boMultiple = false;
										boAny = false;
										if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

											/* Remove the asterisk */
											XMLString::copyNString (xszToken, xszToken, XMLString::stringLen (xszToken) - 1);
											boMultiple = true;
											if (XMLString::stringLen (xszToken) == 0) {
												boAny = true;
											}
										}

										if (!boAny) {
											nChannel = gpcTopology->GetLinkInputChannelIn (nCurrentComponent, nLinkNum);
											if (!AnalyseComponent (xszToken, nCurrentComponent, nLinkNum, pdnInput,
												LINKTYPE_INPUT)) {

												/* if (!XMLString::equals (xszToken, xszLinksInput)) */
												boInputsMatch = false;
												LogValue ("Input links (multiple) don't match on link %d (Analysis failed)",
													  nLinkNum);
											}
										}
									}
								}

								if (boInputsMatch && boMultiple) {
									nRepeatCount++;
								}

								if (!boMultiple) {
									xszToken = pstInTokenizer->nextToken ();
									nRepeatCount = 0;
									boMultiple = false;
									boAny = false;
									if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

										/* Remove the asterisk */
										XMLString::copyNString (xszToken, xszToken, XMLString::stringLen (xszToken) - 1);
										boMultiple = true;
										if (XMLString::stringLen (xszToken) == 0) {
											boAny = true;
										}
									}
								}

								if ((gnBoxVisited[nStartOfLink + LINKLIST_OFFSET] > 0) && ((
										(gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] > 0)
								&& (eCycleIn == CYCLE_DISALLOW)) || (
										(gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] <= 0)
								&& (eCycleIn == CYCLE_REQUIRED)))) {
									boInputsMatch = false;
									LogValue ("Input links don't match on link %d (Cycle failed)", nLinkNum);
								}
							}

							if ((pstInTokenizer->hasMoreTokens ()) && (!boAny)) {
								boInputsMatch = false;
								LogValue ("Input links don't match on link %d (More tokens)", nLinkNum);
							}

							if (((!boMultiple) && (XMLString::stringLen (xszToken) != 0))
							|| ((!boAny) && boMultiple && (nRepeatCount == 0))) {
								boInputsMatch = false;
								LogValue ("Input links don't match on link %d (Repeat count failed)", nLinkNum);
							}

							/* Find out if we should follow the link or not */
							pdnInFollow = pnmInAttributes->getNamedItem (xszFollow);

							eFollowIn = FOLLOW_FRESH;
							if (pdnInFollow) {
								XMLCh const*  xszFollow;
								xszFollow = pdnInFollow->getNodeValue ();
								if (XMLString::equals (xszFollow, xszYes)) {
									eFollowIn = FOLLOW_YES;
									LogString ("Follow in yes");
								}

								if (XMLString::equals (xszFollow, xszNo)) {
									eFollowIn = FOLLOW_NO;
									LogString ("Follow in no");
								}

								if (XMLString::equals (xszFollow, xszFresh)) {
									eFollowIn = FOLLOW_FRESH;
									LogString ("Follow in fresh");
								}
							}

							delete pstInTokenizer;

							if (boInputsMatch && ((eFollowIn == FOLLOW_NO) || ((eFollowIn == FOLLOW_FRESH)
							&& (gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] > 0)))) {

								/* Return backwards */
								LogString ("Node already visited; returning");
								gnRecurseLevel--;
								return true;
							}

							/*
							 * if (boInputsMatch && (gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] > 0)) ;
							 * { ;
							 * // Return backwards ;
							 * gnRecurseLevel--;
							 * return true;
							 * }
							 */
							if (boInputsMatch) {

								/*/
								 *  Inputs are okay, check the outputs
								 */
								if (EvaluateCond (pdnInput)) {

									/* Check the outputs */
									DOMNode*  pdnOutput;
									for (pdnOutput = pdnComponent->getFirstChild (); pdnOutput;
										 pdnOutput = pdnOutput->getNextSibling ()) {
										if (XMLString::equals (pdnOutput->getNodeName (), xszOutput)) {
											DOMNamedNodeMap*  pnmOutAttributes;
											DOMNode*  pdnOutFormat;
											LogString ("Output");

											/*
											 * This is an output, so we need to check it ;
											 * Check the attributes and see if they fit the box
											 */
											pnmOutAttributes = pdnOutput->getAttributes ();
											pdnOutFormat = pnmOutAttributes->getNamedItem (xszFormat);

											if (pdnOutFormat) {
												bool boOutputsMatch;

												XMLCh const*  xszOutFormat;
												LogString ("Output format");

												/* Check the format attribute */
												xszOutFormat = pdnOutFormat->getNodeValue ();
												LogXString (xszOutFormat);
												boOutputsMatch = true;

												if (nCurrentComponent == POINT_EXTERNAL) {
													if (!XMLString::equals (xszOutFormat, xszExternal)) {

														/* This shouldn't be an external link */
														boOutputsMatch = false;
														LogString ("Output links don't match (Not external)");
													} else {

														/*
														 * This should be an external link;
														 * we don't need to recurse any further
														 */
														gnRecurseLevel--;
														return true;
													}
												} else {

													/* Compare the outputs against this */
													XMLStringTokenizer*	 pstOutTokenizer;
													XMLCh*	xszToken;
													bool boMultiple;
													int nRepeatCount;
													bool boAny;
													pstOutTokenizer = new XMLStringTokenizer (xszOutFormat);
													nLinksOut = gpcTopology->LinksOutCount (nCurrentComponent);
													if ((pstOutTokenizer->countTokens () != nLinksOut) && (
															XMLString::findAny (xszOutFormat, XMLString::transcode ("*")
														) == 0)) {
														boOutputsMatch = false;
														LogValue ("Output links don't match (%d link out)", nLinksOut);
													}

													/* Check if there is an asterisk postfixed */
													xszToken = pstOutTokenizer->nextToken ();
													boMultiple = false;
													nRepeatCount = 0;
													boAny = false;

													if (XMLString::endsWith (xszToken, XMLString::transcode ("*"))) {

														/* Remove the asterisk */
														XMLString::copyNString (xszToken, xszToken,
																				XMLString::stringLen (xszToken) - 1);
														boMultiple = true;
														if (XMLString::stringLen (xszToken) == 0) {
															boAny = true;
														}
													}

													for (nLinkNum = 0; (boOutputsMatch && (nLinkNum < nLinksOut) && (!boAny));
														 nLinkNum++) {
														xszLinksOutput = XMLString::transcode (gpcTopology->GetLinkOutput (nCurrentComponent, nLinkNum));
														if (((!boMultiple) || (nRepeatCount == 0))) {

															/* Check for the property */
															nChannel = gpcTopology->GetLinkOutputChannelOut (nCurrentComponent,
																											 nLinkNum);
															if (!AnalyseComponent (xszToken, nCurrentComponent,
																nLinkNum, pdnOutput, LINKTYPE_OUTPUT)) {

																/* if (!XMLString::equals (xszToken, xszLinksInput)) */
																boOutputsMatch = false;
																LogValue ("Output links don't match on link %d (Analysis failed)",
																	  nLinkNum);
															}
														}

														if ((boMultiple && (nRepeatCount > 0))) {
															nChannel = gpcTopology->GetLinkOutputChannelOut (nCurrentComponent,
																											 nLinkNum);
															if (!AnalyseComponent (xszToken, nCurrentComponent,
																nLinkNum, pdnOutput, LINKTYPE_OUTPUT)) {

																/* if (!XMLString::equals (xszToken, xszLinksOutput)) */
																xszToken = pstOutTokenizer->nextToken ();
																nRepeatCount = 0;
																boMultiple = false;
																boAny = false;
																if (XMLString::endsWith (xszToken,
																	XMLString::transcode ("*"))) {

																	/* Remove the asterisk */
																	XMLString::copyNString (xszToken, xszToken,
																							XMLString::stringLen (xszToken) - 1);
																	boMultiple = true;
																	if (XMLString::stringLen (xszToken) == 0) {
																		boAny = true;
																	}
																}

																if (!boAny) {
																	nChannel = gpcTopology->GetLinkOutputChannelOut (nCurrentComponent,
																													 nLinkNum);
																	if (!AnalyseComponent (xszToken, nCurrentComponent,
																		nLinkNum, pdnOutput, LINKTYPE_OUTPUT)) {

																		/*
																		 * if (!XMLString::equals (xszToken,
																		 * xszLinksOutput))
																		 */
																		boOutputsMatch = false;
																		LogValue ("Output links (not any) don't match on link %d (Analysis failed)",
																			  nLinkNum);
																	}
																}
															}
														}

														if (boOutputsMatch && boMultiple) {
															nRepeatCount++;
														}

														if (!boMultiple) {
															xszToken = pstOutTokenizer->nextToken ();
															nRepeatCount = 0;
															boMultiple = false;
															boAny = false;
															if (XMLString::endsWith (xszToken, XMLString::transcode ("*")
																)) {

																/* Remove the asterisk */
																XMLString::copyNString (xszToken, xszToken,
																						XMLString::stringLen (xszToken) - 1);
																boMultiple = true;
																if (XMLString::stringLen (xszToken) == 0) {
																	boAny = true;
																}
															}
														}
													}

													if ((pstOutTokenizer->hasMoreTokens ()) && (!boAny)) {
														boOutputsMatch = false;
														LogValue ("Output links don't match on link %d (More tokens)",
																  nLinkNum);
													}

													if (((!boMultiple) && (XMLString::stringLen (xszToken) != 0))
													|| ((!boAny) && boMultiple && (nRepeatCount == 0))) {
														boOutputsMatch = false;
														LogValue ("Output links (not any) don't match on link %d (Repeat count failed)",
															  nLinkNum);
													}

													delete pstOutTokenizer;
												}

												/* Compare the outputs against this */
												if (boOutputsMatch) {
													if (EvaluateCond (pdnOutput)) {
														bool boSuccess;

														/* Get the config atribute if there is one */
														DOMNode*  pdnOutConfig;

														DOMNode*  pdnOutCycle;
														boSuccess = true;
														pdnOutConfig = pnmOutAttributes->getNamedItem (xszConfig);

														if (pdnOutConfig) {
															pdnNextNode = (pdnCurrentNode->getOwnerDocument ())->getElementById (pdnOutConfig->getNodeValue ());

															if (!pdnNextNode) {
																LogString ("Failed to find config id");
																MessageBox (NULL, "Failed to find config id", NULL,
																			MB_OK);
																pdnNextNode = pdnComponent;
															}
														} else {
															pdnNextNode = pdnComponent;
														}

														/* Find out if we're okay to cycle or not */
														pdnOutCycle = pnmOutAttributes->getNamedItem (xszCycle);

														eCycleOut = CYCLE_OPTIONAL;
														if (pdnOutCycle) {
															XMLCh const*  xszCycle;
															xszCycle = pdnOutCycle->getNodeValue ();
															if (XMLString::equals (xszCycle, xszRequired)) {
																eCycleOut = CYCLE_REQUIRED;
																LogString ("Cycle out required");
															}

															if (XMLString::equals (xszCycle, xszDisallow)) {
																eCycleOut = CYCLE_DISALLOW;
																LogString ("Cycle out disallow");
															}

															if (XMLString::equals (xszCycle, xszOptional)) {
																eCycleOut = CYCLE_OPTIONAL;
																LogString ("Cycle out optional");
															}
														}

														gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET]++;

														/* Cycle through each output (the recursive bit) */
														for (nLinkNum = 0; ((nLinkNum < nLinksOut) && boSuccess);
															 nLinkNum++) {
															DOMNode*  pdnOutFollow;

															/* Establish the box at the end of the link */
															nEndOfLink = gpcTopology->LinkTo (nCurrentComponent,
																							  nLinkNum);

															LogValue ("Component at end of link is %d", nEndOfLink);

															/* Find out if this is a cycle */
															if (((gnBoxVisited[nEndOfLink + LINKLIST_OFFSET] > 0)
															&& (eCycleOut == CYCLE_DISALLOW)) || ((
																	gnBoxVisited[nEndOfLink + LINKLIST_OFFSET] <= 0)
															&& (eCycleOut == CYCLE_REQUIRED))) {
																boSuccess = false;
																LogValue ("Output links don't match on link %d (Cycle failed)",
																	  nLinkNum);
															}

															/* Find out if we should follow the link or not */
															pdnOutFollow = pnmOutAttributes->getNamedItem (xszFollow);

															eFollowOut = FOLLOW_FRESH;
															if (pdnOutFollow) {
																XMLCh const*  xszFollow;
																xszFollow = pdnOutFollow->getNodeValue ();
																if (XMLString::equals (xszFollow, xszYes)) {
																	eFollowOut = FOLLOW_YES;
																	LogString ("Follow out yes");
																}

																if (XMLString::equals (xszFollow, xszNo)) {
																	eFollowOut = FOLLOW_NO;
																	LogString ("Follow out no");
																}

																if (XMLString::equals (xszFollow, xszFresh)) {
																	eFollowOut = FOLLOW_FRESH;
																	LogString ("Follow out fresh");
																}
															}

															if (boSuccess) {
																if ((eFollowOut == FOLLOW_YES) || (
																		(eFollowOut == FOLLOW_FRESH) && (gnBoxVisited[nEndOfLink + LINKLIST_OFFSET] <= 0
																	))) {
																	GlobalLinkOut (nLinkNum);

																	/* This is the actual recursive bit */
																	LogString ("Recursively calling TestExtended");
																	if (!TestExtended (pdnNextNode, nEndOfLink)) {
																		boSuccess = false;
																		LogString ("Failure (in recursion)");
																	}

																	/* Establish the global stack variables */
																	GlobalStackVariables (nCurrentComponent);
																	GlobalLinkOut (nLinkNum);
																} else {
																	LogString ("Link success, but not followed");
																}
															}
														}

														if (boSuccess) {
															LogString ("Success");
															gnRecurseLevel--;
															return true;
														}

														gnBoxVisited[nCurrentComponent + LINKLIST_OFFSET] = 0;
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
		}
	}

	LogString ("Failure (final)");
	gnRecurseLevel--;
	return false;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::GlobalStackVariables (int nCurrentComponent) {
	gnCurrentComponent = nCurrentComponent;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::GlobalLinkOut (int nCurrentLinkOut) {
	gnCurrentLinkOut = nCurrentLinkOut;
	gnCurrentLinkIn = gpcTopology->GetLinkOutputInputIndex (gnCurrentComponent, nCurrentLinkOut);
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::ProcessScan (DOMNode* pdnCurrentNode) {
	XMLCh const*  xszProcess;
	XMLCh const*  xszConfig;
	DOMNode*  pdnProcess;
	DOMNode*  pdnNextNode;

	xszProcess = XMLString::transcode ("process");
	xszConfig = XMLString::transcode ("config");

	/* Act on any processing instructions within the current node */
	for (pdnProcess = pdnCurrentNode->getFirstChild (); pdnProcess; pdnProcess = pdnProcess->getNextSibling ()) {
		if (XMLString::equals (pdnProcess->getNodeName (), xszProcess)) {
			LogString ("\nProcess node");
			LogNode (pdnProcess);

			/* This is a process, so we need to check it */
			if (EvaluateCond (pdnProcess)) {
				DOMNamedNodeMap*  pnmAttributes;
				DOMNode*  pdnConfig;
				pnmAttributes = pdnProcess->getAttributes ();
				pdnConfig = pnmAttributes->getNamedItem (xszConfig);

				if (pdnConfig) {
					pdnNextNode = (pdnProcess->getOwnerDocument ())->getElementById (pdnConfig->getNodeValue ());
				} else {
					pdnNextNode = pdnProcess;
				}

				/* This is a recursive bit */
				ProcessScan (pdnNextNode);
			}
		}
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::CheckCondition (DOMNode* pdnCurrentNode, char* szCondition) {
	bool boError;
	bool boReturn;

	gpdnEvalNode = pdnCurrentNode;
	if (evaluate (szCondition, &boError)) {
		boReturn = true;
	} else {
		boReturn = false;
	}

	if (boError) {
		boReturn = false;
		LogString ("Error interpreting condition");
		MessageBox (NULL, "Error interpreting condition", NULL, MB_OK);
	}

	return boReturn;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::ApplyAction (DOMNode* pdnCurrentNode, char* szAction) {
	bool boError;

	gpdnEvalNode = pdnCurrentNode;
	evaluate (szAction, &boError);
	if (boError) {
		LogString ("Error interpreting action");
		MessageBox (NULL, "Error interpreting action", NULL, MB_OK);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::EvaluateCond (DOMNode* pdnCurrentNode) {
	bool boReturn;
	XMLCh const*  xszCond;
	XMLCh const*  xszAction;
	char szTemp[1024];

	DOMNamedNodeMap*  pnmAttributes;
	DOMNode*  pdnCond;

	boReturn = true;
	xszCond = XMLString::transcode ("cond");
	xszAction = XMLString::transcode ("action");
	pnmAttributes = pdnCurrentNode->getAttributes ();
	pdnCond = pnmAttributes->getNamedItem (xszCond);

	if (pdnCond) {
		XMLCh const*  xszCond;
		LogString ("Evaluating condition");
		xszCond = pdnCond->getNodeValue ();
		XMLString::transcode (xszCond, szTemp, (sizeof (szTemp) - 1));
		LogString (szTemp);
		boReturn = CheckCondition (pdnCurrentNode, szTemp);

		if (boReturn) {
			LogString ("Evaluated to TRUE");
		} else {
			LogString ("Evaluated to FALSE");
		}
	}

	if (boReturn) {
		DOMNode*  pdnAction;
		pdnAction = pnmAttributes->getNamedItem (xszAction);

		if (pdnAction) {
			XMLCh const*  xszAction;
			LogString ("Applying action");
			xszAction = pdnAction->getNodeValue ();
			XMLString::transcode (xszAction, szTemp, (sizeof (szTemp) - 1));
			LogString (szTemp);
			ApplyAction (pdnCurrentNode, szTemp);
		}
	}

	return boReturn;
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

int cXMLParse::evaluate (char* szEval, bool* pboError) {
	int nOutput;
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
	}

	if (nTransPos < (int) (sizeof (szClean) - 1)) {
		szClean[nTransPos] = 0;
		gboError = false;
		nOutput = eval (szClean, nTransPos);
	} else {
		char szError[1024];
		sprintf (szError, "Error - expressions cannot be larger than %d\n", (int) (sizeof (szClean)));
		LogString (szError);
		MessageBox (NULL, szError, NULL, MB_OK);
		gboError = true;
		nOutput = 0;
	}

	if (!gboError) {
		if (pboError) {
			*pboError = false;
		}
	} else {
		if (pboError) {
			*pboError = true;
		}

		nOutput = 0;
	}

	return nOutput;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLParse::eval (char* szEval, int nLength) {
	int nBrackets;
	int nPos;
	char cCurrent;
	bool boResult;
	int nResult;
	char szCopy[1024];
	char*  szLHS;
	int nLHSLen;
	char*  szRHS;
	int nRHSLen;
	OPERATOR eOperator;
	int nOpLen;

	if (gboError) {
		return 0;
	}

	if (nLength == 0) {
		LogString ("Empty constant");
		MessageBox (NULL, "Empty constant", NULL, MB_OK);
		gboError = true;
		return 0;
	}

	strncpy (szCopy, szEval, nLength);
	szCopy[nLength] = 0;

#if defined VERBOSE
	printf ("Calculating %s (length %d)\n", szCopy, nLength);
#endif
	boResult = false;

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
					LogString ("Error - too few opening brackets");
					MessageBox (NULL, "Error - too few opening brackets", NULL, MB_OK);
					gboError = true;
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
						nResult = eval (szLHS, nLHSLen) * eval (szRHS, nRHSLen);
						break;
					case OPERATOR_DIVIDE:
						nResult = eval (szLHS, nLHSLen) / eval (szRHS, nRHSLen);
						break;
					case OPERATOR_REMAINDER:
						nResult = eval (szLHS, nLHSLen) % eval (szRHS, nRHSLen);
						break;
					case OPERATOR_ADD:
						nResult = eval (szLHS, nLHSLen) + eval (szRHS, nRHSLen);
						break;
					case OPERATOR_SUB:
						nResult = eval (szLHS, nLHSLen) - eval (szRHS, nRHSLen);
						break;
					case OPERATOR_LESS:
						nResult = eval (szLHS, nLHSLen) < eval (szRHS, nRHSLen);
						break;
					case OPERATOR_GREATER:
						nResult = eval (szLHS, nLHSLen) > eval (szRHS, nRHSLen);
						break;
					case OPERATOR_LESSEQUAL:
						nResult = eval (szLHS, nLHSLen) <= eval (szRHS, nRHSLen);
						break;
					case OPERATOR_GREATEREQUAL:
						nResult = eval (szLHS, nLHSLen) >= eval (szRHS, nRHSLen);
						break;
					case OPERATOR_EQUAL:
						nResult = eval (szLHS, nLHSLen) == eval (szRHS, nRHSLen);
						break;
					case OPERATOR_NOTEQUAL:
						nResult = eval (szLHS, nLHSLen) != eval (szRHS, nRHSLen);
						break;
					case OPERATOR_AND:
						nResult = eval (szLHS, nLHSLen) & eval (szRHS, nRHSLen);
						break;
					case OPERATOR_OR:
						nResult = eval (szLHS, nLHSLen) | eval (szRHS, nRHSLen);
						break;
					case OPERATOR_LAND:
						nResult = eval (szLHS, nLHSLen) && eval (szRHS, nRHSLen);
						break;
					case OPERATOR_LOR:
						nResult = eval (szLHS, nLHSLen) || eval (szRHS, nRHSLen);
						break;
					case OPERATOR_ASSIGN:
						nResult = SetVariableValue (szLHS, nLHSLen, eval (szRHS, nRHSLen));
						break;
					default:
						LogString ("Error - operator failure");
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
			nResult = !eval (szEval + 1, nLength - 1);
			boResult = true;
			break;
		case '~':
			nResult = ~eval (szEval + 1, nLength - 1);
			boResult = true;
			break;
		default:
			break;
		}
	}

	if ((!boResult) && ((szEval[0] == '(') && (szEval[nLength - 1] == ')'))) {
		nResult = eval (szEval + 1, nLength - 2);
		boResult = true;
	}

	if (!boResult) {
		if (nBrackets != 0) {
			char szError[1024];
			sprintf (szError, "Error - brackets do not match (%d)\n", nBrackets);
			LogString (szError);
			MessageBox (NULL, szError, NULL, MB_OK);
			gboError = true;
			return 0;
		}

		nResult = GetVariableValue (szCopy, nLength);
	}

	if (gboError) {
		return 0;
	}

#if defined VERBOSE
	printf (" Calculated %s (length %d) = %d\n", szCopy, nLength, nResult);
#endif
	return nResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLParse::GetVariableValue (char* szEval, int nLength) {
	int nResult;
	int nScan;
	char szVarName[1024];
	char szIndexVal[1024];
	char*  pcBracketOpen[3];
	char*  pcBracketClose;
	int nIndexLen[3];
	int nIndexVal1;
	int nIndexVal2;
	int nVarNameLen;
	XMLCh const*  xszInit;
	int nBracketCount;
	int nVarNum;
	char szError[1024];
	char*  pcPropertyID;
	bool boMore;

	nResult = 0;
	xszInit = XMLString::transcode ("init");

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
				nResult = GetInOutCount (szEval, nLength);
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
						LogString (szError);
						MessageBox (NULL, szError, NULL, MB_OK);
						gboError = true;
						nResult = 0;
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
					LogString ("Error - too many array arguments/indices");
					MessageBox (NULL, "Error - too many array arguments/indices", NULL, MB_OK);
					gboError = true;
					nResult = 0;
				}

				/* Evaluate the indices */
				pcPropertyID = NULL;
				nIndexVal1 = -1;
				if (nVarNum > 0) {
					strncpy (szIndexVal, pcBracketOpen[0] + 1, nIndexLen[0]);
					szIndexVal[nIndexLen[0]] = 0;

					/*
					 * nIndexVal1 = GetVariableValue (szIndexVal, nIndexLen[0]);
					 */
					nIndexVal1 = eval (szIndexVal, nIndexLen[0]);
				}

				nIndexVal2 = -1;
				if (nVarNum > 1) {
					strncpy (szIndexVal, pcBracketOpen[1] + 1, nIndexLen[1]);
					szIndexVal[nIndexLen[1]] = 0;

					/*
					 * nIndexVal2 = GetVariableValue (szIndexVal, nIndexLen[1]);
					 */
					if ((nVarNameLen == 2) && (strncmp (szVarName, "@a", 2) == 0)) {

						/* The second index is actually the property for @a */
						nIndexVal2 = 0;
						pcPropertyID = szIndexVal;
					} else {
						nIndexVal2 = eval (szIndexVal, nIndexLen[1]);
					}
				}

				if (nVarNum > 2) {

					/* This is the property for @ai or @ao */
					strncpy (szIndexVal, pcBracketOpen[2] + 1, nIndexLen[2]);
					szIndexVal[nIndexLen[2]] = 0;
					pcPropertyID = szIndexVal;
				}

				if (!gboError) {
					nResult = GetInitVal (szVarName, nVarNameLen, nIndexVal1, nIndexVal2, pcPropertyID);
				}
			}
		} else {
			nResult = 0;
		}
	} else {

		/*
		 * Parse the value as a number ;
		 * Check to see whether the value is in hex or decimal
		 */
		if (szEval[0] == '#') {

			/* Try hex */
			nScan = sscanf (szEval + 1, "%x", &nResult);
		} else {

			/* Try decimal */
			nScan = sscanf (szEval, "%d", &nResult);
		}

		if (nScan != 1) {

			/* We reckon it must be a variable */
			nResult = GetInitVal (szEval, nLength, -1, -1, NULL);
		}
	}

	return nResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLParse::GetInitVal (char* szEval, int nLength, int nIndex1, int nIndex2, char* szPropertyID) {
	int nResult;
	XMLCh const xszSpace = *XMLString::transcode (" ");
	int nSearchIndex;
	int nIndexStart;
	int nIndexEnd;
	XMLCh const*  xszNodeValue;
	XMLCh*	xszSubString;
	XMLCh const*  xszInit;
	DOMNode*  pdnVarNode;
	bool boFound;
	XMLCh*	xszPropertyID;

	char szVarName[1024];
	char szLog[1024];
	xszInit = XMLString::transcode ("init");

	nResult = 0;
	xszSubString = (XMLCh*) XMLPlatformUtils::fgMemoryManager->allocate (1024 * sizeof (XMLCh));
	pdnVarNode = NULL;
	boFound = FALSE;
	xszPropertyID = (XMLCh*) XMLPlatformUtils::fgMemoryManager->allocate (PROPERTY_SIZE * sizeof (XMLCh));

	/* Find the variable */
	if ((nLength > 1) && (szEval[0] == '@')) {

		/* Special variable */
		if ((nLength == 2) && (strncmp (szEval, "@v", 2) == 0)) {

			/* Get the variable for the current node */
			pdnVarNode = gpdnEvalNode;
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 2) && (strncmp (szEval, "@n", 2) == 0)) {

			/* Current component number */
			pdnVarNode = NULL;
			nResult = gnCurrentComponent;
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "@nv", 3) == 0)) {

			/* Component's number of times visited */
			pdnVarNode = NULL;
			nResult = gnBoxVisited[nIndex1 + LINKLIST_OFFSET];
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "@il", 3) == 0)) {

			/* Current input link index used to arrive at this component */
			pdnVarNode = NULL;
			nResult = gnCurrentLinkIn;
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "ol", 3) == 0)) {

			/* Current ouput link index used to arrive at this component */
			pdnVarNode = NULL;
			nResult = gnCurrentLinkOut;
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 6) && (strncmp (szEval, "@ilnum", 6) == 0)) {

			/* Component's number of input links */
			pdnVarNode = NULL;
			nResult = gpcTopology->LinksInCount (nIndex1);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 6) && (strncmp (szEval, "@olnum", 6) == 0)) {

			/* Component's number of output links */
			pdnVarNode = NULL;
			nResult = gpcTopology->LinksOutCount (nIndex1);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@ilic", 5) == 0)) {

			/* The input link's input channel */
			pdnVarNode = NULL;
			nResult = gpcTopology->GetLinkInputChannelIn (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@iloc", 5) == 0)) {

			/* The input link's output channel */
			pdnVarNode = NULL;
			nResult = gpcTopology->GetLinkInputChannelOut (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@olic", 5) == 0)) {

			/* The output link's input channel */
			pdnVarNode = NULL;
			nResult = gpcTopology->GetLinkOutputChannelIn (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@oloc", 5) == 0)) {

			/* The output link's input channel */
			pdnVarNode = NULL;
			nResult = gpcTopology->GetLinkOutputChannelOut (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 4) && (strncmp (szEval, "@iln", 4) == 0)) {

			/* The node at the start of the input link */
			pdnVarNode = NULL;
			nResult = gpcTopology->LinkFrom (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 4) && (strncmp (szEval, "@oln", 4) == 0)) {

			/* The node at the start of the input link */
			pdnVarNode = NULL;
			nResult = gpcTopology->LinkTo (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "@ai", 3) == 0)) {

			/* Analyse the component on input link */
			pdnVarNode = NULL;
			XMLString::transcode (szPropertyID, xszPropertyID, PROPERTY_SIZE, XMLPlatformUtils::fgMemoryManager);
			nResult = AnalyseComponent (xszPropertyID, nIndex1, nIndex2, gpdnEvalNode, LINKTYPE_INPUT);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 3) && (strncmp (szEval, "@ao", 3) == 0)) {

			/* Analyse the component on output link */
			pdnVarNode = NULL;
			XMLString::transcode (szPropertyID, xszPropertyID, PROPERTY_SIZE, XMLPlatformUtils::fgMemoryManager);
			nResult = AnalyseComponent (xszPropertyID, nIndex1, nIndex2, gpdnEvalNode, LINKTYPE_OUTPUT);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 2) && (strncmp (szEval, "@a", 2) == 0)) {

			/* Analyse the component */
			pdnVarNode = NULL;
			XMLString::transcode (szPropertyID, xszPropertyID, PROPERTY_SIZE, XMLPlatformUtils::fgMemoryManager);
			nResult = AnalyseComponent (xszPropertyID, nIndex1, nIndex2, gpdnEvalNode, LINKTYPE_NONE);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@ilol", 5) == 0)) {

			/* The equivalent output index of the link for the given input index */
			pdnVarNode = NULL;
			nResult = gpcTopology->GetLinkInputOutputIndex (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@olil", 5) == 0)) {

			/* The equivalent input index of the link for the given ouput index */
			pdnVarNode = NULL;
			nResult = gpcTopology->GetLinkOutputInputIndex (nIndex1, nIndex2);
			boFound = TRUE;
		}

		if ((!boFound) && (nLength == 2) && (strncmp (szEval, "@r", 2) == 0)) {

			/* The current depth of recursion within the file */
			pdnVarNode = NULL;
			nResult = gnRecurseLevel;
			boFound = TRUE;
		}

		/* This part added for the demo */
		if ((!boFound) && (nLength == 5) && (strncmp (szEval, "@ncol", 5) == 0)) {

			/* Change the node colour, use 0 to highlight */
			pdnVarNode = NULL;
			nResult = gpcTopology->SetNodeAnalyseColour (nIndex1, nIndex2);
			boFound = TRUE;
			if (nIndex2 == 0) {
				Sleep (pOptions->getSelectSpeed ());
			} else {
				Sleep (pOptions->getColourSpeed ());
			}
		}
	}

	if (!boFound) {
		char szNodeID[1024];

		XMLCh const*  xszVarId;
		strncpy (szNodeID, szEval, nLength);
		szNodeID[nLength] = 0;
		xszVarId = XMLString::transcode (szNodeID);
		pdnVarNode = (gpdnEvalNode->getOwnerDocument ())->getElementById (xszVarId);
	}

	if (pdnVarNode) {
		DOMNamedNodeMap*  pnmAttributes;
		DOMNode*  pdnInit;

		/* Must only use the first index */
		if (nIndex2 >= 0) {
			LogString ("Error - only one dimensional arrays are available");
			MessageBox (NULL, "Error - only one dimensional arrays are available", NULL, MB_OK);
			nIndex2 = -1;
		}

		pnmAttributes = pdnVarNode->getAttributes ();
		pdnInit = pnmAttributes->getNamedItem (xszInit);

		if (pdnInit) {
			xszNodeValue = pdnInit->getNodeValue ();

			/* We've found the correct tag, now we have to parse it */
			if ((XMLString::indexOf (xszNodeValue, xszSpace) == -1)) {
				nResult = XMLString::parseInt (xszNodeValue);
			} else {

				/* We need to find the correct index */
				nIndexStart = 0;
				for (nSearchIndex = 0; ((nSearchIndex < nIndex1) && (nIndexStart != -1)); nSearchIndex++) {
					nIndexStart = XMLString::indexOf (xszNodeValue, xszSpace, nIndexStart, NULL);
					if (nIndexStart != -1) {
						nIndexStart++;
					}
				}

				if (nIndexStart == -1) {
					nIndexStart = 0;
				}

				nIndexEnd = XMLString::indexOf (xszNodeValue, xszSpace, nIndexStart, NULL);
				if (nIndexEnd == -1) {
					nIndexEnd = XMLString::stringLen (xszNodeValue);
				}

				XMLString::subString (xszSubString, xszNodeValue, nIndexStart, nIndexEnd,
									  XMLPlatformUtils::fgMemoryManager);
				nResult = XMLString::parseInt (xszSubString);
			}
		} else {
			LogString ("Error - no init attribute to read variable");
			MessageBox (NULL, "Error - no init attribute to read variable", NULL, MB_OK);
			gboError = true;
			nResult = 0;
		}
	} else {
		if (!boFound) {
			LogString ("Error - variable id not found");
			MessageBox (NULL, "Error - variable id not found", NULL, MB_OK);
			gboError = true;
			nResult = 0;
		}
	}

	XMLPlatformUtils::fgMemoryManager->deallocate (xszSubString);
	XMLPlatformUtils::fgMemoryManager->deallocate (xszPropertyID);

	strncpy (szVarName, szEval, nLength);
	szVarName[nLength] = 0;
	sprintf (szLog, "Resolution: %s = %d", szVarName, nResult);
	LogString (szLog);

	return nResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLParse::SetInitVal (char* szEval, int nLength, int nIndex, int nValue) {
	int nResult;
	XMLCh const xszSpace = *XMLString::transcode (" ");
	int nSearchIndex;
	int nIndexStart;
	int nIndexEnd;
	XMLCh const* xszNodeValue;
	XMLCh* xszSubString;
	XMLCh* xszCatString;
	XMLCh const* xszInit;
	XMLCh const* xszNewElement;
	char szValue[1024];
	int nSubLength;
	DOMNode* pdnVarNode;

	xszInit = XMLString::transcode ("init");
	xszNewElement = XMLString::transcode (" 0");

	nResult = 0;
	xszSubString = (XMLCh*) XMLPlatformUtils::fgMemoryManager->allocate (VARIABLE_SIZE * sizeof (XMLCh));
	xszCatString = (XMLCh*) XMLPlatformUtils::fgMemoryManager->allocate (VARIABLE_SIZE * sizeof (XMLCh));
	pdnVarNode = NULL;

	/* Find the variable */
	if ((nLength == 2) && (strncmp (szEval, "@v", 2) == 0)) {

		/* Get the variable for the current node */
		pdnVarNode = gpdnEvalNode;
	} else {
		char szNodeID[1024];

		XMLCh const*  xszVarId;
		strncpy (szNodeID, szEval, nLength);
		szNodeID[nLength] = 0;
		xszVarId = XMLString::transcode (szNodeID);
		pdnVarNode = (gpdnEvalNode->getOwnerDocument ())->getElementById (xszVarId);
	}

	if (pdnVarNode) {
		DOMNamedNodeMap*  pnmAttributes;
		DOMNode*  pdnInit;
		pnmAttributes = pdnVarNode->getAttributes ();
		pdnInit = pnmAttributes->getNamedItem (xszInit);

		if (pdnInit) {
			xszNodeValue = pdnInit->getNodeValue ();

			/* We've found the correct tag, now we have to parse it */
			if (((XMLString::indexOf (xszNodeValue, xszSpace) == -1) && (nIndex == 0)) || (nIndex < 0)) {
				XMLCh const*  xszValue;

				/* Not an array, so we just replace the whole lot */
				sprintf (szValue, "%d", nValue);
				xszValue = XMLString::transcode (szValue);
				pdnInit->setNodeValue (xszValue);
				nResult = nValue;
			} else {

				/* Find the correct place */
				nIndexStart = 0;
				for (nSearchIndex = 0; ((nSearchIndex < nIndex) && (nIndexStart != -1)); nSearchIndex++) {
					nIndexStart = XMLString::indexOf (xszNodeValue, xszSpace, nIndexStart,
													  XMLPlatformUtils::fgMemoryManager);
					if (nIndexStart != -1) {
						nIndexStart++;
					}
				}

				if (nIndexStart == -1) {

					/* We need to add some empty elements to the end */
					XMLString::copyString (xszCatString, xszNodeValue);
					while (nSearchIndex < nIndex) {
						XMLString::catString (xszCatString, xszNewElement);
						nSearchIndex++;
					}

					/* Now add the new element */
					sprintf (szValue, " %d", nValue);
					XMLString::catString (xszCatString, XMLString::transcode (szValue));
				} else {

					/*
					 * Change an element in the middle ;
					 * nIndexStart = 0;
					 * for (nSearchIndex = 0;
					 * ((nSearchIndex < nIndex) && (nIndexStart != -1));
					 * nSearchIndex++) ;
					 * { ;
					 * nIndexStart = XMLString::indexOf (xszNodeValue, xszSpace, nIndexStart, XMLPlatformUtils::fgMemoryManager);
					 * if (nIndexStart != -1) ;
					 * { ;
					 * nIndexStart++;
					 * } ;
					 * } ;
					 * if (nIndexStart == -1) ;
					 * { ;
					 * nIndexStart = 0;
					 * }
					 */
					nIndexEnd = XMLString::indexOf (xszNodeValue, xszSpace, nIndexStart,
													XMLPlatformUtils::fgMemoryManager);
					if (nIndexEnd == -1) {
						nIndexEnd = XMLString::stringLen (xszNodeValue);
					}

					XMLString::subString (xszCatString, xszNodeValue, 0, nIndexStart, XMLPlatformUtils::fgMemoryManager);

					/* Now add the new element */
					nSubLength = XMLString::stringLen (xszNodeValue);
					if ((nIndexEnd + 1) < nSubLength) {
						sprintf (szValue, "%d ", nValue);
						XMLString::catString (xszCatString, XMLString::transcode (szValue));
						XMLString::subString (xszSubString, xszNodeValue, nIndexEnd + 1, nSubLength,
											  XMLPlatformUtils::fgMemoryManager);
						XMLString::catString (xszCatString, xszSubString);
					} else {
						sprintf (szValue, "%d", nValue);
						XMLString::catString (xszCatString, XMLString::transcode (szValue));
					}
				}

				/* Replace the previous value with the new one */
				pdnInit->setNodeValue (xszCatString);
				nResult = nValue;
			}
		} else {
			LogString ("Error - no init attribute to set variable");
			MessageBox (NULL, "Error - no init attribute to set variable", NULL, MB_OK);
			gboError = true;
			nResult = 0;
		}
	} else {
		LogString ("Error - variable id not found");
		MessageBox (NULL, "Error - variable id not found", NULL, MB_OK);
		gboError = true;
		nResult = 0;
	}

	XMLPlatformUtils::fgMemoryManager->deallocate (xszSubString);
	XMLPlatformUtils::fgMemoryManager->deallocate (xszCatString);

	return nResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLParse::SetVariableValue (char* szEval, int nLength, int nValue) {
	int nResult;
	char szVarName[1024];
	char szIndexVal[1024];
	char*  pcBracket;
	int nIndexLen;
	int nIndexVal;
	int nVarNameLen;
	XMLCh const*  xszInit;

	char szLog[1024];
	nResult = 0;
	xszInit = XMLString::transcode ("init");

	strncpy (szVarName, szEval, nLength);
	szVarName[nLength] = 0;
	sprintf (szLog, "Assignment: %s = %d", szVarName, nValue);
	LogString (szLog);

	/* Find the variable */
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

				/* in[] or out[] special array */
				LogString ("Error - cannot set in[] and out[] variables");
				MessageBox (NULL, "Error - cannot set in[] and out[] variables", NULL, MB_OK);
				nResult = 0;
				gboError = true;
			} else {

				/*
				 * Standard array var[] ;
				 * Evaluate the index ;
				 * nIndexVal = GetVariableValue (szIndexVal, nIndexLen);
				 */
				nIndexVal = eval (szIndexVal, nIndexLen);
				nResult = SetInitVal (szVarName, nVarNameLen, nIndexVal, nValue);
			}
		} else {
			nResult = 0;
		}
	} else {

		/* Just a straightforward variable */
		nResult = SetInitVal (szEval, nLength, -1, nValue);
	}

	return nResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLParse::GetInOutCount (char* szEval, int nLength) {
	char szLink[1024];
	int nLinkNum;
	int nLinksIn;
	int nLinksOut;
	int nResult;

	nResult = 0;

	if (strncmp (szEval, "in[", 3) == 0) {
		strncpy (szLink, szEval + 3, nLength - 4);
		szLink[nLength - 4] = 0;
		nLinksIn = gpcTopology->LinksInCount (gnCurrentComponent);
		for (nLinkNum = 0; nLinkNum < nLinksIn; nLinkNum++) {
			if ((nLength <= 4) || (strcmp (szLink, gpcTopology->GetLinkInput (gnCurrentComponent, nLinkNum)) == 0)) {
				nResult++;
			}
		}
	} else {
		if (strncmp (szEval, "out[", 4) == 0) {
			strncpy (szLink, szEval + 4, nLength - 5);
			szLink[nLength - 5] = 0;
			nLinksOut = gpcTopology->LinksOutCount (gnCurrentComponent);
			for (nLinkNum = 0; nLinkNum < nLinksOut; nLinkNum++) {
				if ((nLength <= 5) || (strcmp (szLink, gpcTopology->GetLinkOutput (gnCurrentComponent, nLinkNum)) == 0)) {
					nResult++;
				}
			}
		} else {
			LogString ("Error - input or output variable badly defined");
			MessageBox (NULL, "Error - input or output variable badly defined", NULL, MB_OK);
			gboError = true;
			return 0;
		}
	}

	return nResult;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::CheckLinkInputProperties (char* szPropertyID, int nCurrentComponent, int nLinkNum, int* pnReturn) {
	bool boCertified;

	/*
	 * First check whether it's one of the predefined properties ;
	 * There currently aren't any for input links ** ;
	 * Check if it's some other certified property
	 */
	boCertified = CheckCertifiedProperties (szPropertyID, gpcTopology->GetLinkInput (nCurrentComponent, nLinkNum),
											pnReturn);

	return boCertified;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::CheckLinkOutputProperties (char* szPropertyID, int nCurrentComponent, int nLinkNum, int* pnReturn) {
	bool boCertified;
	PREPROP ePreProp;
	int nReturn;

	boCertified = FALSE;

	/*
	 * First check whether it's one of the predefined properties ;
	 * There currently aren't any for output links **
	 */
	for (ePreProp = (PREPROP) 0; ((ePreProp < PREPROP_NUM) && (!boCertified)); ePreProp = (PREPROP) ((int) ePreProp + 1)) {
		if (strcmp (szPropertyID, gszPreProp[ePreProp]) == 0) {
			nReturn = gpcTopology->GetPredefinedProperty (nCurrentComponent, nLinkNum, ePreProp);
			if (pnReturn) {
				*pnReturn = nReturn;
			}

			if (nReturn != 0) {
				boCertified = TRUE;
			}
		}
	}

	/* Check if it's some other certified property */
	if (!boCertified) {
		boCertified = CheckCertifiedProperties (szPropertyID, gpcTopology->GetLinkOutput (nCurrentComponent, nLinkNum),
												pnReturn);
	}

	return boCertified;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::CheckNodeProperties (char* szPropertyID, int nCurrentComponent, int* pnReturn) {
	bool boCertified;
	PREPROP ePreProp;
	int nReturn;

	boCertified = FALSE;

	/* First check whether it's one of the predefined properties */
	for (ePreProp = (PREPROP) 0; ((ePreProp < PREPROP_NUM) && (!boCertified)); ePreProp = (PREPROP) ((int) ePreProp + 1)) {
		if (strcmp (szPropertyID, gszPreProp[ePreProp]) == 0) {
			nReturn = gpcTopology->GetPredefinedProperty (nCurrentComponent, ePreProp);
			if (pnReturn) {
				*pnReturn = nReturn;
			}

			if (nReturn != 0) {
				boCertified = TRUE;
			}
		}
	}

	/* Check if it's some other certified property */
	if (!boCertified) {
		boCertified = CheckCertifiedProperties (szPropertyID, gpcTopology->GetComponentProperties (nCurrentComponent),
												pnReturn);
	}

	return boCertified;
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
int cXMLParse::AnalyseComponent (XMLCh*	 xszPropertyID, int nCurrentComponent, int nLinkNum, DOMNode*  pdnCurrentNode,
								 LINKTYPE eLinkType) {
	cAnalyse*  psAnalyse;
	cVM*  psVM;
	int nReturn;
	int nInput;
	bool boReturn;
	char szPrint[64];
	int nChannel;
	bool boCertified;
	char szPropertyID[1024];
	bool boFound;

	psAnalyse = NULL;
	psVM = NULL;
	nReturn = 0;
	boReturn = FALSE;

	gpdnEvalNode = pdnCurrentNode;

	/* Obtain the input from the XML as a variable */
	nInput = GetInitVal ("@v", 2, nLinkNum, -1, NULL);

	psVM = gpcTopology->GetComponentVM (nCurrentComponent);

	XMLString::transcode (xszPropertyID, szPropertyID, sizeof (szPropertyID) - 1, NULL);

	switch (eLinkType) {
	case LINKTYPE_INPUT:
		sprintf (szPrint, "\nComponent %d, input link %d, property ID %s\n", nCurrentComponent, nLinkNum, szPropertyID);
		boCertified = CheckLinkInputProperties (szPropertyID, nCurrentComponent, nLinkNum, &nReturn);
		nChannel = gpcTopology->GetLinkInputChannelIn (nCurrentComponent, nLinkNum);
		break;
	case LINKTYPE_OUTPUT:
		sprintf (szPrint, "\nComponent %d, output link %d, property ID %s\n", nCurrentComponent, nLinkNum, szPropertyID);
		boCertified = CheckLinkOutputProperties (szPropertyID, nCurrentComponent, nLinkNum, &nReturn);
		nChannel = gpcTopology->GetLinkOutputChannelOut (nCurrentComponent, nLinkNum);
		break;
	case LINKTYPE_NONE:
		sprintf (szPrint, "\nComponent %d, property ID %s\n", nCurrentComponent, szPropertyID);
		boCertified = CheckNodeProperties (szPropertyID, nCurrentComponent, &nReturn);
		nChannel = -1;
		break;
	default:
		sprintf (szPrint, "\nComponent %d, (not supported) link %d, property ID %s\n", nCurrentComponent, nLinkNum,
				 szPropertyID);
		boCertified = FALSE;
		nChannel = -1;
		LogString ("Error - link type not supported in AnalyseComponent");
		MessageBox (NULL, "Error - link type not supported in AnalyseComponent", NULL, MB_OK);
		break;
	}

	psVM->ConsolePrint (szPrint);
	sprintf (szPrint, "Input value %d\n", nInput);
	psVM->ConsolePrint (szPrint);

	boFound = FALSE;

	/* Do the analysis */
	if ((!boFound) && (XMLString::equals (xszPropertyID, XMLString::transcode ("id1")))) {
		boFound = TRUE;

		/* Check if the property has been pre-certified */
		if (boCertified) {
			boReturn = (nInput <= nReturn);
		} else {
			psAnalyse = new cAnalyse (psVM);
			psAnalyse->AnalyseCode_id1 (psVM, nChannel, nInput);
			if (psAnalyse->gboCompleted) {
				nReturn = psAnalyse->gnResult;
				boReturn = (nReturn != 0);
			}
		}
	}

	if ((!boFound) && (XMLString::equals (xszPropertyID, XMLString::transcode ("id2")))) {
		boFound = TRUE;

		if (boCertified) {
			boReturn = TRUE;
		} else {
			psAnalyse = new cAnalyse (psVM);
			psAnalyse->AnalyseCode_id2 (psVM, nChannel);
			if (psAnalyse->gboCompleted) {
				nReturn = psAnalyse->gnResult;
				boReturn = TRUE;
			}
		}
	}

	if ((!boFound) && (XMLString::equals (xszPropertyID, XMLString::transcode ("id3")))) {
		boFound = TRUE;

		/* Check if the property has been pre-certified */
		if (boCertified) {
			boReturn = TRUE;
		} else {
			psAnalyse = new cAnalyse (psVM);
			psAnalyse->AnalyseCode_id3 (psVM, nChannel);
			if (psAnalyse->gboCompleted) {
				nReturn = psAnalyse->gnResult;
				boReturn = (psAnalyse->gnResult == TRUE);
			}
		}
	}

	if ((!boFound) && (XMLString::equals (xszPropertyID, XMLString::transcode ("id4")))) {
		boFound = TRUE;

		/* Check if the property has been pre-certified */
		if (boCertified) {
			boReturn = TRUE;
		} else {
			psAnalyse = new cAnalyse (psVM);
			psAnalyse->AnalyseCode_id4 (psVM, nChannel, nInput);
			if (psAnalyse->gboCompleted) {
				nReturn = psAnalyse->gnResult;
				boReturn = (psAnalyse->gnResult == TRUE);
			}
		}
	}

	if (!boFound) {

		/* Catch everything else */
		boReturn = boCertified;
	}

	/* Store the result back into the XML as a variable */
	SetInitVal ("@v", 2, nLinkNum, nReturn);

	sprintf (szPrint, "\nOutput value %d\nResult is %d\n\n", nReturn, boReturn);
	psVM->ConsolePrint (szPrint);

	return nReturn; /* used to be boReturn, may cause problem later */
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
bool cXMLParse::CheckCertifiedProperties (char* szPropertyID, char* szLinkProperties, int* pnReturn) {
	char*  pcFound;
	bool boReturn;
	int nLinkPropsLen;
	int nIDLen;
	int nValue;
	int nFound;
	char*  pcSearch;
	bool boFound;

	boReturn = FALSE;

	/* Search for the property ID within the link properties string */
	nLinkPropsLen = (int) strlen (szLinkProperties);
	nIDLen = (int) strlen (szPropertyID);
	*pnReturn = 0;

	pcSearch = szLinkProperties;
	pcFound = NULL;
	do
	{
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
				*pnReturn = 1;
				if (pcFound[nIDLen] == '=') {
					nFound = sscanf (pcFound + nIDLen + 1, "%d", &nValue);
					if (nFound > 0) {
						*pnReturn = nValue;
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
void cXMLParse::LogInitialise (void) {
	char szLogFile[1024];
	int nPos;

	strncpy (szLogFile, gXmlFile, sizeof (szLogFile) - 1);

	nPos = (int) strlen (szLogFile) - 1;
	while ((nPos > 0) && (szLogFile[nPos] != '/')) {
		nPos--;
	}

	if (szLogFile[nPos] == '/') {
		szLogFile[nPos + 1] = 0;
		strcat (szLogFile, "LogFile.txt");
	} else {
		strcpy (szLogFile, "LogFile.txt");
	}

	if (gboLog) {
		gfhLogFile = fopen (szLogFile, "w");

		strncpy (gszLogFile, szLogFile, sizeof (szLogFile));
		fputs (gszLogFile, gfhLogFile);
		fputs ("\n", gfhLogFile);

		fclose (gfhLogFile);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::LogDeinitialise (void) {
	if (gboLog) {
		gfhLogFile = fopen (gszLogFile, "a");
		fputs ("Log closed", gfhLogFile);
		fputs ("\n", gfhLogFile);
		fclose (gfhLogFile);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::LogString (char* szString) {
	if (gboLog) {
		gfhLogFile = fopen (gszLogFile, "a");
		fputs (szString, gfhLogFile);
		fputs ("\n", gfhLogFile);
		fclose (gfhLogFile);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::LogXString (XMLCh const* xszString) {
	char szString[1024];

	if (gboLog) {
		gfhLogFile = fopen (gszLogFile, "a");
		XMLString::transcode (xszString, szString, sizeof (szString) - 1, NULL);
		fputs (szString, gfhLogFile);
		fputs ("\n", gfhLogFile);
		fclose (gfhLogFile);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::LogValue (char* szString, int nValue) {
	if (gboLog) {
		gfhLogFile = fopen (gszLogFile, "a");
		fprintf (gfhLogFile, szString, nValue);
		fputs ("\n", gfhLogFile);
		fclose (gfhLogFile);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
void cXMLParse::LogNode (DOMNode const* pdnNode) {
	char szString[1024];
	char szTemp[1024];
	DOMNamedNodeMap*  pnmAttributes;
	int nSize;
	int nAttribute;
	DOMNode const*	pdnAttribute;

	if (gboLog) {
		gfhLogFile = fopen (gszLogFile, "a");
		szString[0] = 0;
		strcat (szString, "<");
		szTemp[0] = 0;
		XMLString::transcode (pdnNode->getNodeName (), szTemp, sizeof (szTemp) - 1, NULL);
		strcat (szString, szTemp);

		pnmAttributes = pdnNode->getAttributes ();

		nSize = pnmAttributes->getLength ();
		for (nAttribute = 0; nAttribute < nSize; nAttribute++) {
			pdnAttribute = pnmAttributes->item (nAttribute);
			strcat (szString, " ");
			XMLString::transcode (pdnAttribute->getNodeName (), szTemp, sizeof (szTemp) - 1, NULL);
			strcat (szString, szTemp);
			strcat (szString, "=\"");
			XMLString::transcode (pdnAttribute->getNodeValue (), szTemp, sizeof (szTemp) - 1, NULL);
			strcat (szString, szTemp);
			strcat (szString, "\"");
		}

		strcat (szString, ">");

		fputs (szString, gfhLogFile);
		fputs ("\n", gfhLogFile);
		fclose (gfhLogFile);
	}
}

/*
 =======================================================================================================================
 *  TODO:
 =======================================================================================================================
 */
CompileHandler * cXMLParse::GetFileHandler () {
	return psXMLCompile->GetFileHandler ();
}
