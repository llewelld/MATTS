/*$T MATTS/cTopologyMirror.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopologyMirror.h ;
 * Last Modified: 12/09/08 ;
 * ;
 * Purpose: Provide a mirrored version of the topology including all properties ;
 * needed for an analysis. Ensures that the analysis is safe even if ;
 * the real topology changes. ;
 */
#ifndef CTOPOLOGYMIRROR_H
#define CTOPOLOGYMIRROR_H

#include "cTopology.h"
#include "cVM.h"
#include "cProperties.h"

/**
 *  cTopologyMirrorNode  */
class cTopologyMirrorNode : public cProperties
{
	friend class cTopologyMirror;
private:
	int nEnumerationTemp;
	cVM*  virtualMachine;
	int nSensitivityLevel;
	char*  szEncryptAlgorithm;
	char*  szStaffSkills;
	bool boFirewall;
	bool boIDS;
	bool boExternal;
	char*  szCertProps;
public:
	cTopologyMirrorNode (cTopologyNode const *const psNode, int nEnumeration);
	~ cTopologyMirrorNode ();
};

/**
 *  cTopologyMirrorLink  */
class cTopologyMirrorLink : public cProperties
{
	friend class cTopologyMirror;
private:
	int nLinkOutIndexTemp;
	int nLinkInIndexTemp;
	cTopologyMirrorNode*  psLinkFrom;
	int nChannelIn;
	int nChannelOut;
	cTopologyMirrorNode*  psLinkTo;
	char*  szLinkInput;
	char*  szLinkOutput;
	int nSensitivityLevel;
public:
	cTopologyMirrorLink (cTopologyLink const *const psLink, int nLinkOutIndex, int nLinkInIndex, cTopologyMirrorNode * const *const apsBox);
	~cTopologyMirrorLink ();
};

/**
 *  cTopologyMirror  */
class cTopologyMirror
{
private:
	int nPoints;
	int nLinks;

	int*  anBoxLinkInCount;
	int*  anBoxLinkOutCount;
	cTopologyMirrorNode **	apsBox;
	cTopologyMirrorLink ***	 appsBoxLinkBoxOut;
	cTopologyMirrorLink ***	 appsBoxLinkBoxIn;
	cTopologyMirrorLink **	apsLink;
public:
	cTopologyMirror (cTopology* psTopology);
	~ cTopologyMirror ();

	int LinksInCount (int nPoint);
	int LinkFrom (int nPoint, int nLinkNum);
	int GetLinkInputChannelIn (int nPoint, int nLinkNum);
	int LinksOutCount (int nPoint);
	int GetLinkOutputChannelOut (int nPoint, int nLinkNum);
	int LinkTo (int nPoint, int nLinkNum);
	int GetLinkOutputInputIndex (int nPoint, int nLinkNum);
	cVM*  GetComponentVM (int nPoint);
	int GetLinkInputChannelOut (int nPoint, int nLinkNum);
	int GetLinkOutputChannelIn (int nPoint, int nLinkNum);
	int GetLinkInputOutputIndex (int nPoint, int nLinkNum);
	char*  GetLinkInput (int nPoint, int nLinkNum);
	char*  GetLinkOutput (int nPoint, int nLinkNum);
	int GetPredefinedProperty (int nPoint, PREPROP ePreProp);
	int GetPredefinedProperty (int nPoint, int nLinkNum, PREPROP ePreProp);
	char*  GetComponentProperties (int nPoint);
	cTopologyMirrorNode * GetNode (int nPoint);
	cTopologyMirrorLink * GetLinkIn (int nPoint, int nLinkNum);
	cTopologyMirrorLink * GetLinkOut (int nPoint, int nLinkNum);
};
#endif
