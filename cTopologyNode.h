/*$T MATTS/cTopologyNode.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopologyNode.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for Topology Linked List Node ;
 * NOTE: Inherits from cNode ;
 * Members ;
 * nXPos - x-position ;
 * nYPos - y-position ;
 * Methods ;
 */
#ifndef CTOPOLOGYNODE_H
#define CTOPOLOGYNODE_H

#include "cNode.h"
#include "cTopologyLink.h"
#include "cProperties.h"

#define MIN(a, b)			(((a) < (b)) ? (a) : (b))
#define MAX(a, b)			(((a) > (b)) ? (a) : (b))
#define ROT_SCALE			(1.0f)
#define SCALE_SCALE			(100.0f)
#define NODE_SIZE			(0.15f)
#define X_SIZE				(NODE_SIZE)
#define Y_SIZE				(NODE_SIZE)
#define Z_SIZE				(NODE_SIZE)
#define SCREEN_CENTRE_X		(300)
#define SCREEN_CENTRE_Y		(315)
#define SCREEN_CENTRE_Z		(0)
#define SCREEN_WIDTH		(400)
#define SCREEN_HEIGHT		(400)
#define SCREEN_DEPTH		(400)
#define RISK_INFINITY		(999)
#define TOPOLOGYNODE_NAMLEN (256)
#define STAFF_SKILLS		(1024)
#define ENCRYPT_ALGORITHM	(1024)
#define DATA_NAMELEN		(65535)
#define NODECOL_EXTERNAL	(RGB (64, 64, 255))
#define NODECOL_DEFAULT		(RGB (255, 255, 255))

typedef enum
{
	SEC_INVALID = -1,
	SEC_MIN,
	SEC_U		= SEC_MIN,
	SEC_N,
	SEC_C,
	SEC_S,
	SEC_TS,
	SEC_1C,
	SEC_MC,
	SEC_MAX		= SEC_MC,
	SEC_NUM,
} SEC;

typedef enum
{
	SHAPE_INVALID	= -1,
	SHAPE_CUBE,
	SHAPE_SPHERE,
	SHAPE_CONE,
	SHAPE_TORUS,
	SHAPE_TEAPOT,
	SHAPE_NUM
} SHAPE;

class cTopologyNode : public cNode, public cProperties
{
public:
	cTopologyNode (int ID);
	~ cTopologyNode ();

	void NodeSecurityReset ();
	void UpdateCascadeInputTableRecurse (void);
	void UpdateCascadeOutputTableRecurse (void);

	/* The following relate to the Net render */
	float fXPosNetRender;
	float fYPosNetRender;
	float fZPosNetRender;
	float fXPosNet;
	float fYPosNet;
	float fZPosNet;
	float fXVel;
	float fYVel;
	float fZVel;

	/* The following relate to the Geo render */
	float fXPos;
	float fYPos;
	float fZPos;
	float fXRot;
	float fYRot;
	float fZRot;
	float fXScale;
	float fYScale;
	float fZScale;
	int nCol;
	int nAnalyseCol;
	SHAPE nShape;
	int nType;
	bool boAdHoc;
	float fRangeTx;
	float fRangeRx;

	int nControlID;

	cTopologyLink*	pLinksOut;
	cTopologyLink*	pLinksIn;

	cTopologyLink*	findLinkWithChannelOut (int nChannelOut);
	cTopologyLink*	findLinkWithChannelIn (int nChannelIn);

	char*  szCertProps;
	int nSecurityMax;
	int nSecurityMin;
	int nSensitivityLevel;
	char*  szEncryptAlgorithm;
	char*  szStaffSkills;
	bool boFirewall;
	bool boIDS;
	bool boExternal;
	char*  szData;

	char const * GetName ();
	void SetName (char const * szName);
	void SetColour (int nColour);
	void ApplyCharacteristics ();

	/* ganRisk[Min ][Max ] */
	static const int ganRisk[SEC_NUM][SEC_NUM];

	int anRiskIn[SEC_NUM][SEC_NUM];
	int anRiskOut[SEC_NUM][SEC_NUM];

	int nEnumerationTemp;
private:
	bool UpdateCascadeInputTable (void);
	bool UpdateCascadeOutputTable (void);
	void PrintInputTable (void);
	void PrintOutputTable (void);
};
#endif
