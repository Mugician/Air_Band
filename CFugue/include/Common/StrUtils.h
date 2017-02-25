#ifndef __STRUTILS_H__91CD1AE9_5C86_4261_AB5D_BDDAFBDA8DC5
#define __STRUTILS_H__91CD1AE9_5C86_4261_AB5D_BDDAFBDA8DC5

#include "_TChar.h" 	// On Non win32 platforms we use a local TChar.h

///
/// Before including this file #Define STRUTILS_RETURN_TYPE to use that type as the default return type.
/// For example, to use _bstr_t as the return type, #define STRUTILS_RETURN_TYPE _bstr_t
///
#ifndef STRUTILS_RETURN_TYPE
#include "MString.h"
#define STRUTILS_RETURN_TYPE CFugue::MString
#endif

#include <stdio.h>

#pragma warning(push)
#pragma warning(disable:4996)	// sprintf deprecated warning: disable;

namespace OIL
{

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#if defined(_WIN64)
    typedef __int64 INT_PTR, *PINT_PTR;
    typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

    typedef __int64 LONG_PTR, *PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;

    #define __int3264   __int64

#else
    typedef _W64 int INT_PTR, *PINT_PTR;
    typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;

    typedef _W64 long LONG_PTR, *PLONG_PTR;
    typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;

    #define __int3264   __int32

#endif

    typedef STRUTILS_RETURN_TYPE StrUtils_Return_Type;

#if !defined(_countof)
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

#if defined __GNUC__ && defined __STRICT_ANSI__
    #define PRINTNUM(BUF,SPEC,NUM)	_stprintf(BUF,_countof(BUF),SPEC,NUM) // swprint is by default a secure version !!
#elif defined __GNUC__ && !defined __STRICT_ANSI__
    #define PRINTNUM(BUF,SPEC,NUM)	_stprintf(BUF,SPEC,NUM)    // Gcc does not have a secure version of sprintf
#elif  defined _MSC_VER
    #define PRINTNUM(BUF,SPEC,NUM)	_stprintf_s(BUF,_countof(BUF),SPEC,NUM) 	//MSVC has different Secure version
#endif // __GNUC__

#define BUFExtra					4
#define GetBufSizeForType(xType) ((sizeof(xType)*8) + BUFExtra)	// We need extra for space and any other chars (such as 0x)

#define DEFAULT_WIDTH		-1
#define DEFAULT_FILLER		_T('0')
#define DEFAULT_RADIX		10
#define DEFAULT_PREFIXUSAGE	true

// Converts the given number to its positive form
template<typename TNumberType>
inline TNumberType _abs(TNumberType	number) { return( number>=0 ? number : -number ); }

// Converts the given unsigned number to String. Only integral types are supported.
template<typename TNumberType>
StrUtils_Return_Type UnSignedNumber2Str(const TNumberType TNumber,		// Number that should be converted to string
								int Width = DEFAULT_WIDTH,				// Width of the output
								TCHAR FillWith = DEFAULT_FILLER,		// The character that should be used to fill the width, if any
								int Radix = DEFAULT_RADIX,				// Base of the representation
								bool bPrefixRadix=DEFAULT_PREFIXUSAGE	// Should the output contain "0x" "0b" prefixes
								)
{
	TCHAR buf[GetBufSizeForType(TNumberType)];

	TCHAR* pText = buf + _countof(buf) - 1;	/* we want to start the last digit at the end of the buffer */

	*pText-- = _T('\0');

	if(Width >= (_countof(buf) - 2))
		Width = DEFAULT_WIDTH;

	TNumberType nNumber = TNumber;

	do{
        int digit = (int)(nNumber % Radix);

        nNumber /= Radix;   /* reduce number */

        if (digit >= 10)/* if hex digit, make it a letter */
            digit = (digit - 10 + (int)_T('A'));
		else
			digit = digit + (int)_T('0');

        *pText-- = (TCHAR)digit;  /* store the digit */

		if(Width != DEFAULT_WIDTH)
			if(--Width == 0) break;

    } while (nNumber != 0);

	if(Width != DEFAULT_WIDTH)
		while(Width-- != 0 && pText != buf)
			*pText-- = FillWith;	// Fill the Text with the Fill Character

	if(bPrefixRadix && Radix == 16)
	{
		*pText-- = _T('x');
		*pText-- = _T('0');
	}
	if(bPrefixRadix && Radix == 2)
	{
		*pText-- = _T('b');
		*pText-- = _T('0');
	}

	++pText;   /* text points to first digit now */

	return pText;
}

// Converts the given signed number to String. Only integral types are supported.
template<typename TNumberType>
StrUtils_Return_Type SignedNumber2Str(const TNumberType TNumber,		// Number that should be converted to string
								int Width = DEFAULT_WIDTH,				// Width of the output
								TCHAR FillWith = DEFAULT_FILLER,		// The character that should be used to fill the width, if any
								int Radix = DEFAULT_RADIX,				// Base of the representation
								bool bPrefixRadix=DEFAULT_PREFIXUSAGE,	// Should the output contain "0x" "0b" prefixes
								bool bSpaceForSign = true				// Should empty space be inserted for Sign when number is positive
								)
{
	TCHAR buf[GetBufSizeForType(TNumberType)];

	TCHAR* pText = buf + _countof(buf) - 1;	/* we want start the last digit at end of buffer */

	*pText-- = _T('\0');

	if(Width >= (_countof(buf) - 2))
		Width = DEFAULT_WIDTH;

	TNumberType nNumber = _abs<TNumberType>(TNumber);

	do{
        int digit = _abs<int>((int)(nNumber % Radix));

        nNumber /= Radix;   /* reduce number */

        if (digit >= 10)/* if hex digit, make it a letter */
            digit = (digit - 10 + (int)_T('A'));
		else
			digit = digit + (int)_T('0');

        *pText-- = (TCHAR)digit;  /* store the digit */

		if(Width != DEFAULT_WIDTH)
			if(--Width == 0) break;

    } while (nNumber != 0);

	if(Width != DEFAULT_WIDTH)
		while(Width-- != 0 && pText != buf)
			*pText-- = FillWith;	// Fill the Text with the Fill Character

	if(TNumber < (TNumberType)0)	// Take care of the sign
	{
		*pText-- = _T('-');
	}
	else
	{
		if(bSpaceForSign) *pText-- = _T(' ');
	}

	if(bPrefixRadix && Radix == 16)
	{
		*pText-- = _T('x');
		*pText-- = _T('0');
	}
	if(bPrefixRadix && Radix == 2)
	{
		*pText-- = _T('b');
		*pText-- = _T('0');
	}

	++pText;   /* text points to first digit now */

	return pText;
}




// This default method is the base for all specializations that later follow.
// In an ideal situation, this should never be called. If
// this is being called, then it indicates that we have a type
// that doesn't have a correct specialized form. In such case
// we can either choose to specialize ToString() for that type, or
// let this defualt method return an empty string.
template<typename T>
inline StrUtils_Return_Type ToString(const T& Arg)
{
	return _T("");
}

// This form of ToString() acts as base for all custom pointer types.
// While this looks simple, it is very powerful, as it can take care of
// pointer-to-pointer types, pointer-to-pointer-to-pointer types and so on.
// As a Plus, this can takes care of expanding the 'content' also.
template<typename T>
inline StrUtils_Return_Type ToString(const T* pArg)
{
// Disable warning: "A qualifier, such as const, is applied to a function type defined by typedef"
#pragma warning(push)
#pragma warning(disable:4180)

	// We have got a pointer to be converted to a string.
	// Lets see if we can represent the content also as string.
	if(pArg != NULL)
	{
		//StrUtils_Return_Type strArgContent = ToString((const T&)*pArg);

		//// Lets check if we got valid content (to add braces around it)
		//LPCTSTR lpszContent = (LPCTSTR)strArgContent;

		//if(lpszContent != NULL && *lpszContent != _T('\0'))
		//{
		//	// Add braces around the content only if they are not already present.
		//	// For simplicity we check only for start brace in the content. If we
		//	// see a start brace we assume that the end brace is also present at
		//	// the end. This need not be a invalid assumption - but we ignore it for now.
		//	LPCTSTR szStartBrace = (*lpszContent != _T('{') ? _T("{") : _T(""));
		//	LPCTSTR szEndBrace = (*lpszContent != _T('{') ? _T("}") : _T(""));

		//	return ToString((const void*)pArg) + szStartBrace + strArgContent + szEndBrace;
		//}
	}

	// Seems we have got a NULL pointer. We can simply return "0x00000000" directly. But
	// we choose to use ToString() to ensure the correct number of zeroes in the output.
	return ToString((const void*)pArg);

#pragma warning(pop) // disable:4180
}

template<typename T>
inline StrUtils_Return_Type ToString(T* pArg)
{
	return ToString((const T*) pArg);
}

template<>
inline StrUtils_Return_Type ToString(const bool& bVal)
{
	return bVal ? _T("True") : _T("False");
}

template<>
inline StrUtils_Return_Type	ToString(const unsigned short& nNumber)
{
	return UnSignedNumber2Str(nNumber);
}

template<>
inline StrUtils_Return_Type	ToString(const signed short& nNumber)
{
	return SignedNumber2Str(nNumber);
}

template<>
inline StrUtils_Return_Type	ToString(const unsigned int& nNumber)
{
	return UnSignedNumber2Str(nNumber);
}

template<>
inline StrUtils_Return_Type	ToString(const signed int& nNumber)
{
	return SignedNumber2Str(nNumber);
}

template<>
inline StrUtils_Return_Type	ToString(const unsigned long& lNumber)
{
	return UnSignedNumber2Str(lNumber);
}

template<>
inline StrUtils_Return_Type	ToString(const signed long& lNumber)
{
	return SignedNumber2Str(lNumber);
}

template<>
inline StrUtils_Return_Type	ToString(const float& fNumber)
{
//#if defined _UNICODE || defined UNICODE
//	return std::to_wstring(fNumber);
//#else
//    return std::to_string(fNumber);
//#endif
	TCHAR	sz[GetBufSizeForType(fNumber)];
	PRINTNUM(sz,_T("% 0.2f"),fNumber);
	return sz;
}

template<>
inline StrUtils_Return_Type ToString(const double& fNumber)
{
	return ToString((const float) fNumber);
}

template<>
inline StrUtils_Return_Type	ToString(const char& ch)
{
	StrUtils_Return_Type str;
	//const char str[2] = {(char)ch, '\0'};
	//return ToString((const int)ch) + _T("'") + StrUtils_Return_Type(str) + _T("'");
	str << (const int)ch << _T("'") << StrUtils_Return_Type(str) << _T("'");
	return str;
}

template<>
inline StrUtils_Return_Type	ToString(const unsigned char& ch)
{
	StrUtils_Return_Type str;
	//const char str[2] = {(char)ch, '\0'};
	//return ToString((const unsigned int)ch) + _T("'") + StrUtils_Return_Type(str) + _T("'");
	str << (const unsigned int)ch + _T("'") + StrUtils_Return_Type(str) + _T("'");
	return str;
}

#ifdef _NATIVE_WCHAR_T_DEFINED
template<>
inline StrUtils_Return_Type	ToString(const wchar_t& wch)
{
	const wchar_t wstr[2] = {wch, L'\0'};
	return ToString((const int)wch) + _T("'") + StrUtils_Return_Type(wstr) + _T("'");
}
#endif

template<>
inline StrUtils_Return_Type	ToString(const char* sz)
{
	StrUtils_Return_Type str;
	str << _T("\"") << StrUtils_Return_Type(sz) << _T("\"");
	return str;
}
template<>
inline StrUtils_Return_Type	ToString(char* sz)
{
	return ToString((const char*)sz);
}

template<>
inline StrUtils_Return_Type	ToString(const wchar_t* wsz)
{
	StrUtils_Return_Type str;
	str << _T("\"") << StrUtils_Return_Type(wsz) << _T("\"");
	return str;
}
template<>
inline StrUtils_Return_Type	ToString(wchar_t* wsz)
{
	return ToString((const wchar_t*)wsz);
}

template<>
inline StrUtils_Return_Type	ToString(const void* pVoid)
{
#pragma warning(push)
#pragma warning(disable:4244)	// Disable the nasty "LPVOID to Unsigned long" conversion warning
	return UnSignedNumber2Str(reinterpret_cast<ULONG_PTR>(pVoid), sizeof(ULONG_PTR) * 2, DEFAULT_FILLER, 16);
#pragma warning(pop)
}
template<>
inline StrUtils_Return_Type	ToString(void* pVoid)
{
	return ToString((const void*)pVoid);
}

#if defined(_VECTOR_) // if included <vector>
template<typename T>
inline StrUtils_Return_Type ToString(const std::vector<T>& Container, const TCHAR* lpszSeperator = _T(","), const TCHAR* lpszBegin=_T("{"), const TCHAR* lpszEnd=_T("}"))
{
    StrUtils_Return_Type Str(lpszBegin);

    std::vector<T>::const_iterator iter = Container.begin();
    std::vector<T>::const_iterator iterEnd = Container.end();

    if(iter != iterEnd)
        Str += ToString(*iter++);

    while(iter != iterEnd)
        Str += lpszSeperator + ToString(*iter++);

    return Str + lpszEnd;
}
#endif  // if defined(_VECTOR_)

#if defined(_LIST_) // if included <list>
template<typename T>
inline StrUtils_Return_Type ToString(const std::list<T>& Container, const TCHAR* lpszSeperator = _T(","), const TCHAR* lpszBegin=_T("{"), const TCHAR* lpszEnd=_T("}"))
{
    StrUtils_Return_Type Str(lpszBegin);

    std::list<T>::const_iterator iter = Container.begin();
    std::list<T>::const_iterator iterEnd = Container.end();

    if(iter != iterEnd)
        Str += ToString(*iter++);

    while(iter != iterEnd)
        Str += lpszSeperator + ToString(*iter++);

    return Str + lpszEnd;
}
#endif  // if defined(_LIST_)

#if defined(_MAP_)  // if included <map>
template<typename K, typename T>
inline StrUtils_Return_Type ToString(const std::map<K, T>& Container, const TCHAR* lpszSeperator = _T(","), const TCHAR* lpszBegin=_T("{"), const TCHAR* lpszEnd=_T("}"))
{
    StrUtils_Return_Type Str(lpszBegin);

    std::map<K, T>::const_iterator iter = Container.begin();
    std::map<K, T>::const_iterator iterEnd = Container.end();

    if(iter != iterEnd)
    {
        Str += _T("(") + ToString(iter->first) + lpszSeperator + ToString(iter->second) + _T(")");
        ++iter;
    }

    while(iter != iterEnd)
    {
        Str += lpszSeperator + _T("(") + ToString(iter->first) + lpszSeperator + ToString(iter->second) + _T(")");
        ++iter;
    }

    return Str + lpszEnd;
}
#endif  // if defined(_MAP_)

}	// namespace OIL

#pragma warning(pop)

#endif	// ifdef __STRUTILS_H__91CD1AE9_5C86_4261_AB5D_BDDAFBDA8DC5
