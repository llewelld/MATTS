/*$T MATTS/cProperties.cpp GC 1.140 07/01/09 21:12:09 */
/*
 * ;
 * Name: cProperties.cpp ;
 * Last Modified: 17/12/09 ;
 * ;
 * Purpose: Implementation file for node and link property class ;
 */
#include <stdlib.h>
#include "cProperties.h"

/*
 =======================================================================================================================
 *  cProperties constructor
 =======================================================================================================================
 */
cProperties::cProperties () {
	// Create the new Properties map
	pcProperties = new PropertyMap;
}

/*
 =======================================================================================================================
 *  cProperties destructor
 =======================================================================================================================
 */
cProperties::~cProperties () {
	// Free up the property map
	if (pcProperties) {
		// Empty out the map
		PropertyRemoveAll ();

		// Free the map structure
		delete pcProperties;
		pcProperties = NULL;
	}
}

/*
 =======================================================================================================================
 *  Set the specified property with the given value of the given type
 =======================================================================================================================
 */
void cProperties::PropertySet (char const * szName, PROPTYPE eType, void * pValue) {
	Property sPropertySet;

	// Set the type
	sPropertySet.eType = eType;

	// Set the property value (depending on its type)
	switch (eType) {
	case PROPTYPE_INT:
		sPropertySet.nValue = *((int *)pValue);
		break;
	case PROPTYPE_STRING:
		sPropertySet.szValue = (char *)malloc (strlen ((char *)pValue) + 1);
		strcpy (sPropertySet.szValue, (char *)pValue);
		break;
	case PROPTYPE_FLOAT:
		sPropertySet.fValue = *((float *)pValue);
		break;
	case PROPTYPE_BOOL:
		sPropertySet.boValue = *((bool *)pValue);
		break;
	case PROPTYPE_OPTION:
		sPropertySet.szValue = NULL;
		sPropertySet.aszValue = *((list<string> *)pValue);
		break;
	default:
		sPropertySet.pValue = pValue;
		break;
	}

	(*pcProperties)[szName] = sPropertySet;
}

/*
 =======================================================================================================================
 *  Set the specified property with the given ingeter value
 =======================================================================================================================
 */
void cProperties::PropertySetInt (char const * szName, int const nValue) {
	Property sPropertySet;

	// Clear the memory from any existing contents
	PropertyFreeContents (szName);

	// Set the type
	sPropertySet.eType = PROPTYPE_INT;
	// Set the property value
	sPropertySet.nValue = nValue;

	(*pcProperties)[szName] = sPropertySet;
}

/*
 =======================================================================================================================
 *  Set the specified property with the given string value
 =======================================================================================================================
 */
void cProperties::PropertySetString (char const * szName, char const * szValue) {
	Property sPropertySet;

	// Clear the memory from any existing contents
	PropertyFreeContents (szName);

	// Set the type
	sPropertySet.eType = PROPTYPE_STRING;

	// Set the property value
	sPropertySet.szValue = (char *)malloc (strlen (szValue) + 1);
	strcpy (sPropertySet.szValue, szValue);

	(*pcProperties)[szName] = sPropertySet;
}

/*
 =======================================================================================================================
 *  Set the specified property with the given floating point value
 =======================================================================================================================
 */
void cProperties::PropertySetFloat (char const * szName, float fValue) {
	Property sPropertySet;

	// Clear the memory from any existing contents
	PropertyFreeContents (szName);

	// Set the type
	sPropertySet.eType = PROPTYPE_FLOAT;

	// Set the property value
	sPropertySet.fValue = fValue;

	(*pcProperties)[szName] = sPropertySet;
}

/*
 =======================================================================================================================
 *  Set the specified property with the given boolean value
 =======================================================================================================================
 */
void cProperties::PropertySetBool (char const * szName, bool boValue) {
	Property sPropertySet;

	// Clear the memory from any existing contents
	PropertyFreeContents (szName);

	// Set the type
	sPropertySet.eType = PROPTYPE_BOOL;

	// Set the property value
	sPropertySet.boValue = boValue;

	(*pcProperties)[szName] = sPropertySet;
}

/*
 =======================================================================================================================
 *  Add an option string to the specified property
 =======================================================================================================================
 */
void cProperties::PropertySetOptionAdd (char const * szName, char const * szOption) {
	Property sPropertySet;
	string szNewOption;
	PropertyMap::iterator psFind;

	szNewOption.assign (szOption);

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		sPropertySet.eType = PROPTYPE_OPTION;

		// Set the property value for a new entry
		sPropertySet.aszValue.clear ();
		sPropertySet.aszValue.push_back (szNewOption);
		sPropertySet.szValue = NULL;
		(*pcProperties)[szName] = sPropertySet;
	}
	else {
		// We found the entry, so need to update its value
		if (psFind->second.eType == PROPTYPE_OPTION) {
			// This is already an option variable
			psFind->second.aszValue.push_back (szNewOption);
		}
		else {
			// It's not an option variable, so we need to reassign it

			// Remove the existing value
			PropertyRemove (szName);

			// Set the new value to be an option
			sPropertySet.eType = PROPTYPE_OPTION;

			// Set the property value for a new entry
			sPropertySet.aszValue.clear ();
			sPropertySet.aszValue.push_back (szNewOption);
			sPropertySet.szValue = NULL;
			(*pcProperties)[szName] = sPropertySet;
		}
	}
}

/*
 =======================================================================================================================
 *  Set the specified property with the given option value
 =======================================================================================================================
 */
void cProperties::PropertySetOption (char const * szName, char const * szValue) {
	Property sPropertySet;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		sPropertySet.eType = PROPTYPE_OPTION;

		// Set the property value for a new entry
		sPropertySet.aszValue.clear ();
		// Set the property value
		sPropertySet.szValue = (char *)malloc (strlen (szValue) + 1);
		strcpy (sPropertySet.szValue, szValue);

		(*pcProperties)[szName] = sPropertySet;
	}
	else {
		// We found the entry, so need to update its value
		if (psFind->second.eType == PROPTYPE_OPTION) {
			// This is already an option variable
			if (psFind->second.szValue) {
				free (psFind->second.szValue);
			}
			psFind->second.szValue = (char *)malloc (strlen (szValue) + 1);
			strcpy (psFind->second.szValue, szValue);
		}
		else {
			// It's not an option variable, so we need to reassign it

			// Remove the existing value
			PropertyRemove (szName);

			// Set the new value to be an option
			sPropertySet.eType = PROPTYPE_OPTION;

			// Set the property value for a new entry
			sPropertySet.aszValue.clear ();
			// Set the property value
			sPropertySet.szValue = (char *)malloc (strlen (szValue) + 1);
			strcpy (sPropertySet.szValue, szValue);
			(*pcProperties)[szName] = sPropertySet;
		}
	}
}

/*
 =======================================================================================================================
 *  Set the enumeration value as an integer of the specified option property
 =======================================================================================================================
 */
void cProperties::PropertySetOptionEnum (char const * szName, int const nValue) {
	int nCount;
	PropertyMap::iterator psFind;
	list<string>::iterator iOption;
	string szValue;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind != pcProperties->end ()) {
		// We found the entry, so need to return the enum value
		iOption = psFind->second.aszValue.begin ();
		nCount = 0;
		while ((nCount < nValue) && (iOption != psFind->second.aszValue.end ())) {
			++nCount;
			++iOption;
		}
		if (iOption != psFind->second.aszValue.end ()) {
			// We found the entry, so need to update its value
			if (psFind->second.eType == PROPTYPE_OPTION) {
				szValue.assign (iOption->c_str ());
				// This is already an option variable
				if (psFind->second.szValue) {
					free (psFind->second.szValue);
				}
				psFind->second.szValue = (char *)malloc (szValue.length () + 1);
				strcpy (psFind->second.szValue, szValue.c_str());
			}
		}
	}
}

/*
 =======================================================================================================================
 *  Get the specified property, independent of its type
 =======================================================================================================================
 */
void * cProperties::PropertyGet (char const * szName) {
	void * pReturn;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		pReturn = NULL;
	}
	else {
		// We found the entry, so need to return its value
		// The value returned will depend on the type
		switch (psFind->second.eType) {
		case PROPTYPE_INT:
			pReturn = & psFind->second.nValue;
			break;
		case PROPTYPE_STRING:
			pReturn = psFind->second.szValue;
			break;
		case PROPTYPE_FLOAT:
			pReturn = & psFind->second.fValue;
			break;
		case PROPTYPE_BOOL:
			pReturn = & psFind->second.boValue;
			break;
		case PROPTYPE_OPTION:
			pReturn = psFind->second.szValue;
			break;
		default:
			pReturn = & psFind->second.pValue;
			break;
		}
	}

	return pReturn;
}

/*
 =======================================================================================================================
 *  Returns the type of the specified property
 =======================================================================================================================
 */
PROPTYPE cProperties::PropertyGetType (char const * szName) {
	PROPTYPE eType;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		eType = PROPTYPE_INVALID;
	}
	else {
		// We found the entry, so need to return its type
		eType = psFind->second.eType;
	}

	return eType;
}

/*
 =======================================================================================================================
 *  Get the value of the specified integer property
 =======================================================================================================================
 */
int cProperties::PropertyGetInt (char const * szName) {
	int nReturn;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		nReturn = 0;
	}
	else {
		// We found the entry, so need to return its value
		nReturn = psFind->second.nValue;
	}

	return nReturn;
}

/*
 =======================================================================================================================
 *  Get the value of the specified string property
 =======================================================================================================================
 */
char * cProperties::PropertyGetString (char const * szName) {
	char * szReturn;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		szReturn = NULL;
	}
	else {
		// We found the entry, so need to return its value
		szReturn = psFind->second.szValue;
	}

	return szReturn;
}

/*
 =======================================================================================================================
 *  Get the value of the specified floating point property
 =======================================================================================================================
 */
float cProperties::PropertyGetFloat (char const * szName) {
	float fReturn;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		fReturn = 0.0f;
	}
	else {
		// We found the entry, so need to return its value
		fReturn = psFind->second.fValue;
	}

	return fReturn;
}

/*
 =======================================================================================================================
 *  Get the value of the specified boolean property
 =======================================================================================================================
 */
bool cProperties::PropertyGetBool (char const * szName) {
	bool boReturn;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		boReturn = false;
	}
	else {
		// We found the entry, so need to return its value
		boReturn = psFind->second.boValue;
	}

	return boReturn;
}

/*
 =======================================================================================================================
 *  Get the value of the specified option property
 =======================================================================================================================
 */
list<string> cProperties::PropertyGetOptionList (char const * szName) {
	list<string> aszReturn;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		aszReturn.clear ();
	}
	else {
		// We found the entry, so need to return its value
		aszReturn = psFind->second.aszValue;
	}

	return aszReturn;
}

/*
 =======================================================================================================================
 *  Get the value of the specified string property
 =======================================================================================================================
 */
char * cProperties::PropertyGetOption (char const * szName) {
	char * szReturn;
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		szReturn = NULL;
	}
	else {
		// We found the entry, so need to return its value
		szReturn = psFind->second.szValue;
	}

	return szReturn;
}

/*
 =======================================================================================================================
 *  Get the enumeration value of the specified string property
 =======================================================================================================================
 */
int cProperties::PropertyGetOptionEnum (char const * szName) {
	int nReturn;
	int nCount;
	PropertyMap::iterator psFind;
	list<string>::iterator iOption;

	// Find the entry
	psFind = pcProperties->find (szName);
	if (psFind == pcProperties->end ()) {
		// Entry couldn't be found
		nReturn = -1;
	}
	else {
		// We found the entry, so need to return the enum value
		iOption = psFind->second.aszValue.begin ();
		nReturn = -1;
		nCount = 0;
		while ((nReturn < 0) && (iOption != psFind->second.aszValue.end ())) {
			if (iOption->compare (psFind->second.szValue) == 0) {
				nReturn = nCount;
			}
			++nCount;
			++iOption;
		}
	}

	return nReturn;
}

/*
 =======================================================================================================================
 *  Find out if the specified property currently exists
 =======================================================================================================================
 */

bool cProperties::PropertyExists (char const * szName) {
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);

	// Return a boolean depending on whether the entry was found
	return (psFind != pcProperties->end ());
}

/*
 =======================================================================================================================
 *  Free any memory allocated to the specified property
 =======================================================================================================================
 */
PropertyMap::iterator cProperties::PropertyFreeContents (char const * szName) {
	PropertyMap::iterator psFind;

	// Find the entry
	psFind = pcProperties->find (szName);

	// Empty out the contents of the value if necessary
	if (psFind != pcProperties->end ()) {
		switch (psFind->second.eType) {
		case PROPTYPE_STRING:
			// Free the string value
			if (psFind->second.szValue) {
				free (psFind->second.szValue);
				psFind->second.szValue = NULL;
			}
			break;
		case PROPTYPE_OPTION:
			// Free the option value
			if (psFind->second.szValue) {
				free (psFind->second.szValue);
				psFind->second.szValue = NULL;
			}
			// Clear the option list
			psFind->second.aszValue.clear ();
			break;
		default:
			// Do nothing
			break;
		}

		// This entry is no longer valid
		psFind->second.eType = PROPTYPE_INVALID;
	}
	return psFind;
}

/*
 =======================================================================================================================
 *  Remove the specified property
 =======================================================================================================================
 */
void cProperties::PropertyRemove (char const * szName) {
	PropertyMap::iterator psFind;

	psFind = PropertyFreeContents (szName);

	if (psFind != pcProperties->end()) {
		pcProperties->erase (psFind);
	}
}

/*
 =======================================================================================================================
 *  Remove all of the properties
 =======================================================================================================================
 */
void cProperties::PropertyRemoveAll () {
	PropertyMap::iterator psFind;

	// Empty out the contents of the values if necessary
	for (psFind = pcProperties->begin (); psFind != pcProperties->end (); psFind++) {
		switch (psFind->second.eType) {
		case PROPTYPE_STRING:
			if (psFind->second.szValue) {
				free (psFind->second.szValue);
			}
			break;
		default:
			// Do nothing
			break;
		}
	}

	// Now clear the entire map
	pcProperties->clear ();
}

/*
 =======================================================================================================================
 *  Copy all properties from one object to another
 =======================================================================================================================
 */
void cProperties::PropertyCopy (cProperties const * psFrom) {
	PropertyMap::iterator psFind;
	Property sPropertySet;

	// Empty out the old map
	PropertyRemoveAll ();

	// Create the new map by copying each element
	for (psFind = psFrom->pcProperties->begin (); psFind != psFrom->pcProperties->end (); psFind++) {
		// Set the type
		sPropertySet.eType = psFind->second.eType;

		// Set the property value (depending on its type)
		switch (sPropertySet.eType) {
		case PROPTYPE_INT:
			sPropertySet.nValue = psFind->second.nValue;
			break;
		case PROPTYPE_STRING:
			sPropertySet.szValue = (char *)malloc (strlen (psFind->second.szValue) + 1);
			strcpy (sPropertySet.szValue, psFind->second.szValue);
			break;
		case PROPTYPE_FLOAT:
			sPropertySet.fValue = psFind->second.fValue;
			break;
		case PROPTYPE_BOOL:
			sPropertySet.boValue = psFind->second.boValue;
			break;
		case PROPTYPE_OPTION:
			if (psFind->second.szValue) {
				sPropertySet.szValue = (char *)malloc (strlen (psFind->second.szValue) + 1);
				strcpy (sPropertySet.szValue, psFind->second.szValue);
			}
			else {
				sPropertySet.szValue = NULL;
			}
			sPropertySet.aszValue = psFind->second.aszValue;
			break;
		default:
			sPropertySet.pValue = psFind->second.pValue;
			break;
		}

		(*pcProperties)[psFind->first] = sPropertySet;
	}
}

/*
 =======================================================================================================================
 *  Copy all properties from one object to another
 =======================================================================================================================
 */
void cProperties::PropertyTransfer (cProperties const * psFrom) {
	PropertyMap::iterator psFind;
	Property sPropertySet;

	// Create the new map by copying each element
	for (psFind = psFrom->pcProperties->begin (); psFind != psFrom->pcProperties->end (); psFind++) {
		// Set the type
		sPropertySet.eType = psFind->second.eType;

		// Set the property value (depending on its type)
		switch (sPropertySet.eType) {
		case PROPTYPE_INT:
			sPropertySet.nValue = psFind->second.nValue;
			break;
		case PROPTYPE_STRING:
			sPropertySet.szValue = (char *)malloc (strlen (psFind->second.szValue) + 1);
			strcpy (sPropertySet.szValue, psFind->second.szValue);
			break;
		case PROPTYPE_FLOAT:
			sPropertySet.fValue = psFind->second.fValue;
			break;
		case PROPTYPE_BOOL:
			sPropertySet.boValue = psFind->second.boValue;
			break;
		case PROPTYPE_OPTION:
			if (psFind->second.szValue) {
				sPropertySet.szValue = (char *)malloc (strlen (psFind->second.szValue) + 1);
				strcpy (sPropertySet.szValue, psFind->second.szValue);
			}
			else {
				sPropertySet.szValue = NULL;
			}
			sPropertySet.aszValue = psFind->second.aszValue;
			break;
		default:
			sPropertySet.pValue = psFind->second.pValue;
			break;
		}

		(*pcProperties)[psFind->first] = sPropertySet;
	}
}

/*
 =======================================================================================================================
 *  Return the properties map
 =======================================================================================================================
 */
PropertyMap const * cProperties::GetPropertyMap () {
	return pcProperties;
}
