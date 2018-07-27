/*$T MATTS/cTopology.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cTopolgy.h ;
 * Last Modified: 20/01/05 ;
 * ;
 * Purpose: Header file for cTopolgy ;
 * Members ;
 * hWindow - Handle to main window ;
 * pList - Pointer to the list of Virtual Machines ;
 * Methods ;
 * plotMain() - Draws the console text and input ;
 * setVMList() - Sets up a list of Virtual Machines to deal with ;
 * TopologyInitialise() - Initialise ;
 * newNode() - Call when a new VM node is created ;
 */
#ifndef CTOPOLOGY_H
#define CTOPOLOGY_H

#define WIN32_LEAN_AND_MEAN 1

#include "cVM.h"
#include "cTopologyList.h"
#include "cTimer.h"
#include "cModel.h"
#include "cTexture.h"
#include "cRenderGeo.h"
#include "cRenderNet.h"

#define POINTS_MAX			(1024)
#define LINKLIST_OFFSET		(1)
#define DELTA_ROT			(0.1f)
#define DELTA_ROT_SCROLL	(0.5f)
#define NEW_NODE_ZPOS		(0.7f)
#define BGCOL_RED			(0.2f)
#define BGCOL_GREEN			(0.2f)
#define BGCOL_BLUE			(0.4f)
#define POINTTO_HALFLIFE	(0.1f)
#define POINTTO_MINMOVE		(0.001f)
#define MAX_NODENUM			(50)
#define MAX_LINKNUM			(50)
#define MOUSE_ROTATE_SCALE	(400.0f)
#define MOUSE_TRANSLATE_SCALE	(100.0f)

typedef enum
{
	DRAGTYPE_INVALID= -1,
	DRAGTYPE_BOXMOVE,
	DRAGTYPE_LINKSET,
	DRAGTYPE_ROTATE,
	DRAGTYPE_TRANSLATE,
	DRAGTYPE_NUM
} DRAGTYPE;

typedef enum
{
	DRAGEND_INVALID		= -1,
	DRAGEND_NONE,
	DRAGEND_BOXMOVE,
	DRAGEND_LINKSET,
	DRAGEND_LINKUNSET,
	DRAGEND_ROTATE,
	DRAGEND_NUM
} DRAGEND;

typedef enum
{
	OBJECTTYPE_INVALID	= -1,
	OBJECTTYPE_NODE,
	OBJECTTYPE_LINK,
	OBJECTTYPE_MAP,
	OBJECTTYPE_NUM
} OBJECTTYPE;

typedef enum
{
	PREPROP_INVALID				= -1,
	PREPROP_SENSITIVITYLEVEL,
	PREPROP_ENCRYPTIONSTRENGTH,
	PREPROP_STAFFSKILLS,
	PREPROP_FIREWALL,
	PREPROP_IDS,
	PREPROP_EXTERNAL,
	PREPROP_CHANNELOUT,
	PREPROP_NUM
} PREPROP;

static char const *const gszPreProp[] =
{
	"SensitivityLevel",
	"EncryptionStrength",
	"StaffSkills",
	"Firewall",
	"IDS",
	"External",
	"ChannelOut"
};

typedef enum _BUTTON
{
	BUTTON_INVALID				= -1,
	BUTTON_SELECT,
	BUTTON_LINKS,
	BUTTON_SPIN,
	BUTTON_NET,
	BUTTON_DETAILS,
	BUTTON_LEFT,
	BUTTON_NODES,
	BUTTON_RIGHT,
	BUTTON_NUM
} BUTTON;

class cWindow;

class cTopology
{
private:
	cTopologyList*	pList;
	cTopologyNode*	psPointToNode;
	cWindow * psWindow;
	HWND hWindow;
	int nHeight;
	int nWidth;
	cTimer*	 psTimer;
	unsigned long uTime;
	unsigned long uTimePrev;
	float fTimeStep;
	float fNodeButtonRotate;
	unsigned int uNodeButtonLeftTimeClicked;
	unsigned int uNodeButtonRightTimeClicked;
	int gnBoxLinkInCount[POINTS_MAX];
	int gnBoxLinkOutCount[POINTS_MAX];
	int gnBoxLinkBoxIn[POINTS_MAX][20];
	int gnBoxLinkBoxOut[POINTS_MAX][20];
	cTopologyLink*	gpsBoxLinkBoxIn[POINTS_MAX][20];
	cTopologyLink*	gpsBoxLinkBoxOut[POINTS_MAX][20];
	cTopologyNode*	gpsBox[POINTS_MAX];
	int nNextLinkID;
	bool boScreenShotTemp;
	float fAnalysisRed;
	float fAnalysisGreen;
	cFileHub * psFileHub;
	int nNodeTypes;

	void Normalise (float* pfX, float* pfY, float* pfZ);
	void CreateTimer (void);
	void DeleteTimer (void);

	/*
	 * void TransformBasis (float fXScreen, float fYScreen, float fZScreen, float pfX,
	 * float * pfY, float * pfZ);
	 */
	void RenderHUD ();
	void PlotButton (int nXPos, int nYPos, unsigned int uTexture, unsigned int uTextureDown, bool boDown);
	void SendNodeButtonNodeType ();
public:
	cRenderGeo*	 psRenderGeo;
	cRenderNet*	 psRenderNet;
	cRender*  psRenderCurrent;
	RENDERTYPE eRenderType;
	cTexture*  psTexture;
	cObject*  psObject;
	cTopologyNode*	psSelectedNode;
	cTopologyLink*	psSelectedLink;
	bool boAnalysisCompleted;
	bool boAnalysisResult;
	bool aboButtonPressed[BUTTON_NUM];
	int nNodeButtonNodeType;

	cTopology (cTopologyList* pTopologyList, HWND hwnd, cFileHub * psFileHub, cWindow * psWindow);
	~ cTopology ();
	void SetNodeTextureInfo (int nNodeTextureStart, int nNodeTypes);
	void NextRenderType ();
	void PlotMain (HDC sHDC);
	static GLuint LoadTextureRaw (char const *const szFilename, int const nWidth, int const nHeight, bool boAlpha);
	static GLuint LoadTextureJPEG (char const *const szFilename);
	static GLuint LoadTexturePNG (char const *const szFilename);
	bool ClickButton (int nXPos, int nYPos);
	static float DotProdAngle (float fX1, float fY1, float fX2, float fY2);
	SelectObj SelectMain (int nXPos, int nYPos);
	SelectObj SelectMap (int nXPos, int nYPos);
	void Resize (RECT const* psNewWinRect);
	cTopologyLink*	GetSelectedLink (void);
	cTopologyNode*	GetSelectedNode (void);
	cTopologyList*	GetpList (void);
	cTopologyNode*	GetNodeToDelete (void);
	void SetSelectedLink (cTopologyLink* psSetSelectedLink);
	void SetSelectedNode (cTopologyNode* psSetSelectedNode);
	void ConvertCoords (int nXMousePos, int nYMousePos, float fZPos, double* pfX, double* pfY, double* pfZ);
	void RedrawTopology (void);
	void CalculateLinkList (void);
	int LinksInCount (int nPoint);
	int LinksOutCount (int nPoint);
	char*  GetLinkInput (int nPoint, int nLinkNum);
	char*  GetLinkOutput (int nPoint, int nLinkNum);
	int LinkTo (int nPoint, int nLinkNum);
	int LinkFrom (int nPoint, int nLinkNum);
	cVM*  GetComponentVM (int nPoint);
	int GetLinkInputChannelIn (int nPoint, int nLinkNum);
	int GetLinkInputChannelOut (int nPoint, int nLinkNum);
	int GetLinkOutputChannelIn (int nPoint, int nLinkNum);
	int GetLinkOutputChannelOut (int nPoint, int nLinkNum);
	int GetLinkInputOutputIndex (int nPoint, int nLinkNum);
	int GetLinkOutputInputIndex (int nPoint, int nLinkNum);
	char*  GetComponentProperties (int nPoint);
	int GetPredefinedProperty (int nPoint, PREPROP ePreProp);
	int GetPredefinedProperty (int nPoint, int nLinkNum, PREPROP ePreProp);
	cTopologyLink*	FindCommandLink (int nCommandIDFrom, int nCommandIDTo);
	int GetNextLinkID (void);
	void ResetNextID (void);
	bool GetOverlayStatus (void);
	void SetOverlayStatus (bool boStatus);
	void ToggleOverlayStatus (void);
	static SEC ConvertSecurityToInt (char cSecurity);
	static char ConvertIntToSecurity (SEC eSecurity);
	void MoveCentre (float fXDelta, float fYDelta, float fZDelta);
	void GetCentre (float* pfX, float* pfY, float* pfZ);
	void SetCentre (float fX, float fY, float fZ);
	void ChangeView (float fTheta, float fPhi, float fPsi, float fRadius);
	bool PointTowards (float nXPos, float nYPos, float nZPos, float fRadius);
	void ViewOffset (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius);
	void CentreOffset (float* pfX, float* pfY, float* pfZ);
	void GetView (float* pfTheta, float* pfPhi, float* pfPsi, float* pfRadius);
	bool boLinkCancelled;
	void Update (unsigned long uTime);
	void Point (void);
	void PointToSelected (void);
	void ResetAnalyseView (void);
	void SelectNextNode (void);
	void SelectPreviousNode (void);
	void SelectNextLink (void);
	void SelectPreviousLink (void);
	void ClearSelected (void);
	int SetNodeAnalyseColour (int nPoint, int nColour);
	cTopologyNode* DecodeNodeName (char const * szName);
	cTopologyNode* DecodeNodeNumber (int nNode);
	cTopologyLink* DecodeLinkName (char const * szName);
	cTopologyLink* DecodeLinkNumber (int nLink);
	bool CreateLink (int nFrom, int nTo, int nLinkID);
	void DeleteLink (cTopologyLink* psLink);
	void MoveNode (cTopologyNode* psNode, float fXPos, float fYPos, float fZPos);
	void UpdateAdHocLinks (cTopologyNode* psNode);
	cTimer * GetTimer (void);
	void ClearTimerEvents (void);
	bool PrepareForNextRenderType ();
};

#endif /* CTOPOLOGY_H */
