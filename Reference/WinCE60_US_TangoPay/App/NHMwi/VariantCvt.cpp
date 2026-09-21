// TYPE Conversion FOR IC Chip Data
#include "stdafx.h"
#include ".\NHMWI.h"
#include ".\NHMWICtl.h"
#include ".\NHMWIPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
// Method : Called From AP
///////////////////////////////////////////////////////////////////////////////////////////////////



// [펌.. 출처 : Kalignite/ ]
/**************************************************************************\

  VarArrayToByteArray

  Convert a SAFEARRAY to a byte array.

  The passed variant must be a simple array of variants or a reference to a
  similar array, which can be converted to bytes. Otherwise an MFC Ole 
  exception is thrown.

  If any type errors are found NULL is returned.
  The caller must free the byte array which is returned.

  Input:
    Variant array       The source data.

  Output
    int  Size           The number of elements in the new array

  return 
    BYTE *Array         The same data, converted to bytes and added to a 
                        simple c type array.

\**************************************************************************/    
BYTE* CNHMWICtrl::VarArrayToByteArray(LPCTSTR szToken, const VARIANT *Array, int *Size )
{
    HRESULT     hRc;
    SAFEARRAY   *ArrayPointer;
    long        ArrayIndex[1]; // This is a 1D index, so it's a 1 vertex array
    int         ArrayMin, ArrayMax,ArraySize; // Min and Max in X dimention
    VARIANT     Element;
    VARIANT     ByteElement;
    int         i,j;
	static		BYTE        *NewArray = NULL;					// 2004.05.10
  
	// Initialise our temp variant structure
    VariantInit( &ByteElement );
	VariantInit( &Element );


    ////////////////////////////////////////////////////////////////////////////
    //  Check type info, and get the actual array pointer.

    // It's posible to get a variant with nothing but a variant in it.

	while( Array->vt == (VT_BYREF | VT_VARIANT) )
	{
        Array = Array->pvarVal;
    }
    // Now the variant should be an array or dispatch ptr, if not, we've got a type error.
    if( Array->vt & VT_ARRAY )
    {
        if( Array->vt & VT_BYREF )
            ArrayPointer = *Array->pparray;
        else
            ArrayPointer = Array->parray;
    }
    else
	{
		return NULL;
	}
        

    // Get array bounds.
    ArrayMin = ArrayPointer->rgsabound[0].lLbound;
	ArraySize= ArrayPointer->rgsabound[0].cElements;
    ArrayMax = ArrayMin + ArraySize; // Actualy last index +1 so we can do for( i=ArrayMin; i<ArrayMax;)
//	CString strBuff1("");		
//	strBuff1 = MakeUnPack(&ArrayMax, 1);
	

    // Get the new array to put the data into.
	if (NewArray)												// 2004.05.10
	{
		delete [] NewArray;
		NewArray = NULL;
	}

    NewArray = new BYTE[ ArraySize ];
    if( NewArray == NULL )
	{
		return NULL;
	}

 
	// Loop through the array.

	for( i=ArrayMin, j=0; i< ArrayMax; i++,j++ ){

		// Setup the index of the item we want to access.
		ArrayIndex[0] = i;
//		CString strBuff("");		
//		strBuff	= MakeUnPack(&ArrayIndex[0], 1);


		// Get the item
		hRc = SafeArrayGetElement( ArrayPointer, &ArrayIndex[0], &Element);
		if( FAILED( hRc ) ) 
		{
			return NULL;
		}


		// Coerce to correct type.
		hRc = VariantChangeType( &ByteElement, &Element, (USHORT)0, VT_UI1 );
		if( FAILED( hRc ) )
		{
			VariantClear( &Element );
			return NULL;
		}

		NewArray[j] = ByteElement.bVal;
		VariantClear( &ByteElement );				// [#288] NH KGS 2008.06.12
		VariantClear( &Element );					// [#288] NH KGS 2008.06.12
	}
	// Return data size
	if( Size != NULL )
	{
		*Size = ArraySize;
	}
	ArrayPointer = NULL;

	//SJK_51
    VariantClear( &ByteElement );
	VariantClear( &Element );
	//--
    
	return( NewArray );

}

/**************************************************************************\

  ByteArrayToSafeArray

  Convert a simple c type byte array to a VARIANT SAFEARRAY

  If any errors are found an empty Variant is returns. 

  The caller should call DeleteVarArray() to free the array when it is 
  no longer needed.

  Input:
    BYTE *array       The source data.

  Output
    VARIANT SAFEARRAY A Variant structure pointing to a 1D SAFEARRAY of
						variants. The array contains the original data.
					  The dimention boundaries are [0,ArraySize)

\**************************************************************************/    
VARIANT CNHMWICtrl::ByteArrayToSafeArray(BYTE *ByteArray, int ArraySize ) 
{
	static	SAFEARRAY   *m_pSafeArray = NULL;					// 2004.05.10

	VARIANT Result;
	VariantInit(&Result);

    VARIANT *ElemVar;
    long    ArrayIndex[1]; // This is a 1D array, so it's index is a 1 vertex array

    SAFEARRAYBOUND Bounds[1];

	int i;

    HRESULT hRc ;

	// Setup the array boundrys
	Bounds[0].lLbound = 0;
	Bounds[0].cElements = ArraySize;

	// If m_pSafeArray is not NULL, initialize this array 
//	SafeArrayDestroy(m_pSafeArray);								// 20031008 : 최성원차장 디버그 : system Hang
	if (m_pSafeArray)											// 2004.05.10
	{
		SafeArrayDestroy(m_pSafeArray);	
		m_pSafeArray = NULL;
	}

    m_pSafeArray = SafeArrayCreate( VT_VARIANT, 1, Bounds);
    if ( m_pSafeArray == NULL) return Result;

	for( i = 0; i< ArraySize; i++ )
	{
		// Create a new Variant to put his element into
		ElemVar = new VARIANT;
		if( ElemVar == NULL) return Result;

		// Init variant
  		VariantInit( ElemVar );
	
		// Set type and data
		ElemVar->vt = VT_UI1;
		ElemVar->bVal = ByteArray[i];

		// put elemtent into array
		ArrayIndex[0] = i;
		hRc = SafeArrayPutElement( m_pSafeArray, ArrayIndex, ElemVar ); 
		if( FAILED( hRc ) )										// 2004.05.10
		{
			VariantClear( ElemVar ); //SJK_51
			delete ElemVar;
			return Result;
		}

		// The element has now been copied, so we can delete it.
		VariantClear( ElemVar );
		delete ElemVar;
	}

    // Setup the array variant
    Result.vt = VT_ARRAY | VT_VARIANT;
    Result.parray = m_pSafeArray;

	return Result;
}

/**************************************************************************\
Q167668
\**************************************************************************/    
int CNHMWICtrl::VarArrayToStringArray(const VARIANT *Array, CStringArray& SArray)
{
    HRESULT     hRc;
    SAFEARRAY   *ArrayPointer;
    long        ArrayIndex[1];
    int         ArrayMin, ArrayMax,ArraySize; // Min and Max in X dimention
    VARIANT     Element;
    VARIANT     ByteElement;
    int         i,j;
  
	// Initialise our temp variant structure
    VariantInit( &ByteElement );
	VariantInit( &Element );
	SArray.RemoveAll();

    ////////////////////////////////////////////////////////////////////////////
    //  Check type info, and get the actual array pointer.
    // It's posible to get a variant with nothing but a variant in it.
	while( Array->vt == (VT_BYREF | VT_VARIANT) )
	{
        Array = Array->pvarVal;
    }
    // Now the variant should be an array or dispatch ptr, if not, we've got a type error.
    if( Array->vt & VT_ARRAY )
    {
        if( Array->vt & VT_BYREF )
            ArrayPointer = *Array->pparray;
        else
            ArrayPointer = Array->parray;
    }
    else
	{
		return 0;
	}
        
    // Get array bounds.
    ArrayMin = ArrayPointer->rgsabound[0].lLbound;
	ArraySize= ArrayPointer->rgsabound[0].cElements;
    ArrayMax = ArrayMin + ArraySize; // Actualy last index +1 so we can do for( i=ArrayMin; i<ArrayMax;)

	// Loop through the array.
	for( i=ArrayMin, j=0; i< ArrayMax; i++,j++ )
	{
		ArrayIndex[0] = i;

		hRc = SafeArrayGetElement( ArrayPointer, &ArrayIndex[0], &Element);
		if( FAILED( hRc ) ) 
			break;

		hRc = VariantChangeType( &ByteElement, &Element, (USHORT)0, VT_BSTR );
		if( FAILED( hRc ) )
			break;

		SArray.Add(CString(ByteElement.bstrVal));

		VariantClear( &ByteElement );				// [#288] NH KGS 2008.06.12
		VariantClear( &Element );					// [#288] NH KGS 2008.06.12
	}
	ArrayPointer = NULL;

	//SJK_51
    VariantClear( &ByteElement );
	VariantClear( &Element );

    return( SArray.GetSize() );

}

/**************************************************************************\
\**************************************************************************/    
VARIANT CNHMWICtrl::StringArrayToSafeArray(CStringArray& SArray) 
{
	static	SAFEARRAY   *m_pSafeArray = NULL;					// 2004.05.10

	VARIANT Result;
	VariantInit(&Result);

    VARIANT *ElemVar;
    long    ArrayIndex[1]; // This is a 1D array, so it's index is a 1 vertex array

    SAFEARRAYBOUND Bounds[1];

	int i;

    HRESULT hRc ;

	// Setup the array boundrys
	Bounds[0].lLbound = 0;
	Bounds[0].cElements = SArray.GetSize();

	// If m_pSafeArray is not NULL, initialize this array 
	if (m_pSafeArray)											// 2004.05.10
	{
		SafeArrayDestroy(m_pSafeArray);	
		m_pSafeArray = NULL;
	}

    m_pSafeArray = SafeArrayCreate( VT_VARIANT, 1, Bounds);
    if ( m_pSafeArray == NULL) return Result;

	for( i = 0; i< SArray.GetSize(); i++ )
	{
		// Create a new Variant to put his element into
		ElemVar = new VARIANT;
		if( ElemVar == NULL) return Result;

		// Init variant
  		VariantInit( ElemVar );
	
		// Set type and data
		ElemVar->vt = VT_BSTR;

		ElemVar->bstrVal = SArray.GetAt(i).AllocSysString();	// 2007.12.21

		// put elemtent into array
		ArrayIndex[0] = i;
		hRc = SafeArrayPutElement( m_pSafeArray, ArrayIndex, ElemVar ); 
		if( FAILED( hRc ) )										// 2004.05.10
		{
			VariantClear( ElemVar );	//SJK_51
			delete ElemVar;
			return Result;
		}

		// The element has now been copied, so we can delete it.
		VariantClear( ElemVar );
		delete ElemVar;
	}

    // Setup the array variant
    Result.vt = VT_ARRAY | VT_VARIANT;
    Result.parray = m_pSafeArray;

	return Result;
}

/**************************************************************************\
REMARK	: 2004.03.31
		  Q167668참조
\**************************************************************************/    
int CNHMWICtrl::VarArrayToIntArray(const VARIANT *Array, CArray<int, int>& IArray)
{
    HRESULT     hRc;
    SAFEARRAY   *ArrayPointer;
    long        ArrayIndex[1];
    int         ArrayMin, ArrayMax,ArraySize; // Min and Max in X dimention
    VARIANT     Element;
    VARIANT     ByteElement;
    int         i,j;
  
	// Initialise our temp variant structure
    VariantInit( &ByteElement );
	VariantInit( &Element );
	IArray.RemoveAll();

    ////////////////////////////////////////////////////////////////////////////
    //  Check type info, and get the actual array pointer.
    // It's posible to get a variant with nothing but a variant in it.
	while( Array->vt == (VT_BYREF | VT_VARIANT) )
	{
        Array = Array->pvarVal;
    }
    // Now the variant should be an array or dispatch ptr, if not, we've got a type error.
    if( Array->vt & VT_ARRAY )
    {
        if( Array->vt & VT_BYREF )
            ArrayPointer = *Array->pparray;
        else
            ArrayPointer = Array->parray;
    }
    else
	{
		return 0;
	}
        
    // Get array bounds.
    ArrayMin = ArrayPointer->rgsabound[0].lLbound;
	ArraySize= ArrayPointer->rgsabound[0].cElements;
    ArrayMax = ArrayMin + ArraySize; // Actualy last index +1 so we can do for( i=ArrayMin; i<ArrayMax;)

	// Loop through the array.
	for( i=ArrayMin, j=0; i< ArrayMax; i++,j++ )
	{
		ArrayIndex[0] = i;

		hRc = SafeArrayGetElement( ArrayPointer, &ArrayIndex[0], &Element);
		if( FAILED( hRc ) ) 
			break;

		hRc = VariantChangeType( &ByteElement, &Element, (USHORT)0, VT_I4 );
		if( FAILED( hRc ) )
			break;

		IArray.Add(ByteElement.lVal);

		VariantClear( &ByteElement );				// [#288] NH KGS 2008.06.12
		VariantClear( &Element );					// [#288] NH KGS 2008.06.12
	}
	ArrayPointer = NULL;

	//SJK_51
    VariantClear( &ByteElement );
	VariantClear( &Element );

    return( IArray.GetSize() );

}

/**************************************************************************\
REMARK	: 2004.03.31
\**************************************************************************/    
VARIANT CNHMWICtrl::IntArrayToSafeArray(CArray<int, int>& IArray) 
{
	static	SAFEARRAY   *m_pSafeArray = NULL;					// 2004.05.10

	VARIANT Result;
	VariantInit(&Result);

    VARIANT *ElemVar;
    long    ArrayIndex[1]; // This is a 1D array, so it's index is a 1 vertex array

    SAFEARRAYBOUND Bounds[1];

	int i;

    HRESULT hRc ;

	// Setup the array boundrys
	Bounds[0].lLbound = 0;
	Bounds[0].cElements = IArray.GetSize();

	// If m_pSafeArray is not NULL, initialize this array 
	if (m_pSafeArray)											// 2004.05.10
	{
		SafeArrayDestroy(m_pSafeArray);	
		m_pSafeArray = NULL;
	}

    m_pSafeArray = SafeArrayCreate( VT_VARIANT, 1, Bounds);
    if ( m_pSafeArray == NULL) return Result;

	for( i = 0; i< IArray.GetSize(); i++ )
	{
		// Create a new Variant to put his element into
		ElemVar = new VARIANT;
		if( ElemVar == NULL) return Result;

		// Init variant
  		VariantInit( ElemVar );
	
		// Set type and data
		ElemVar->vt = VT_I4;
		ElemVar->lVal = (long)(IArray.GetAt(i));

		// put elemtent into array
		ArrayIndex[0] = i;
		hRc = SafeArrayPutElement( m_pSafeArray, ArrayIndex, ElemVar ); 
		if( FAILED( hRc ) )										// 2004.05.10
		{
			VariantClear( ElemVar );	//SJK_51
			delete ElemVar;
			return Result;
		}

		// The element has now been copied, so we can delete it.
		VariantClear( ElemVar );
		delete ElemVar;
	}

    // Setup the array variant
    Result.vt = VT_ARRAY | VT_VARIANT;
    Result.parray = m_pSafeArray;

	return Result;
}
