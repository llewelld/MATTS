/*$T MATTS/cProperties.h GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cProperties.h ;
 * Last Modified: 17/12/09 ;
 * ;
 * Purpose: Header file for cProperities node and link property utility class ;
 * Members ;
 * Methods ;
 */
#ifndef CPROPERTIES_H
#define CPROPERTIES_H

#include <map>
#include <list>
#include <string>

using namespace std;

typedef enum {
	PROPTYPE_INVALID = -1,

	PROPTYPE_INT,
	PROPTYPE_STRING,
	PROPTYPE_FLOAT,
	PROPTYPE_BOOL,
	PROPTYPE_OPTION,
	PROPTYPE_POINTER,
	
	PROPTYPE_NUM
} PROPTYPE;

/*
 =======================================================================================================================
 *  The properties
 *  TODO: This should really be done using classes and inheritance
 =======================================================================================================================
 */
typedef struct _Property {
	PROPTYPE eType;
	union {
		int nValue;
		char * szValue;
		float fValue;
		bool boValue;
		void * pValue;
	};
	list<string> aszValue;
	union {
		int nMin;
		float fMin;
	};
	union {
		int nMin;
		float fMax;
	};
} Property;

/*
 =======================================================================================================================
 *  The property comparision operator
 =======================================================================================================================
 */
struct PropertyKeyCompare {
	bool operator () (const char* s1, const char* s2) const {
		return (strcmp (s1, s2) < 0);
	}
};

/*
 =======================================================================================================================
 *  The map container
 =======================================================================================================================
 */
typedef std::map<const string, Property> PropertyMap;

class cProperties
{
public:
	cProperties ();
	~cProperties ();

	void PropertySet (char const * szName, PROPTYPE eType, void * pValue);
	void PropertySetInt (char const * szName, int const nValue);
	void PropertySetString (char const * szName, char const * szValue);
	void PropertySetFloat (char const * szName, float fValue);
	void PropertySetBool (char const * szName, bool boValue);
	void PropertySetOptionAdd (char const * szName, char const * szOption);
	void PropertySetOption (char const * szName, char const * szOption);
	void PropertySetOptionEnum (char const * szName, int const nValue);

	void * PropertyGet (char const * szName);
	PROPTYPE PropertyGetType (char const * szName);
	int PropertyGetInt (char const * szName);
	char * PropertyGetString (char const * szName);
	float PropertyGetFloat (char const * szName);
	bool PropertyGetBool (char const * szName);
	list<string> PropertyGetOptionList (char const * szName);
	char * PropertyGetOption (char const * szName);
	int PropertyGetOptionEnum (char const * szName);

	bool PropertyExists (char const * szName);
	void PropertyRemove (char const * szName);
	void PropertyRemoveAll ();
	void PropertyCopy (cProperties const * const psFrom);
	void PropertyTransfer (cProperties const * psFrom);

	PropertyMap const * GetPropertyMap ();

private:
	PropertyMap::iterator PropertyFreeContents (char const * szName);

	PropertyMap * pcProperties;
};
#endif /* ifndef CPROPERTIES_H */
