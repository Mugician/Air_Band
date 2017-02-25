/*
	This is part of CFugue, the C++ Runtime for MIDI Score Programming
	Copyright (C) 2009 Gopalakrishna Palem

	For links to further information, or to contact the author,
	see <http://cfugue.sourceforge.net/>.

    $LastChangedDate: 2014-05-21 07:29:42 +0530 (Wed, 21 May 2014) $
    $Rev: 597 $
    $LastChangedBy: krishnapg $
*/

#ifndef __MSTRING_H__2B50AFA1_EFB9_428a_A397_3FFEA175FA33__
#define __MSTRING_H__2B50AFA1_EFB9_428a_A397_3FFEA175FA33__

#include "_TChar.h"	// On Non win32 platforms we use a local TChar.h
#include <string>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <sstream>

namespace CFugue
{
#if defined _UNICODE || defined UNICODE
	typedef std::wstringstream _stringstreamBase;
	typedef std::wstring	_stringBase;
	typedef wchar_t	_baseChar;
	typedef char	_otherChar;
#else
	typedef std::stringstream _stringstreamBase;
	typedef std::string		_stringBase;
	typedef char	_baseChar;
	typedef wchar_t	_otherChar;
#endif
	/// <Summary> Helper class for string stream manipulations, useful for tracing </Summary>
	/// It is advised to use always std::wstringstream as the base, since it is capable of
	/// dealing with both unicode and non-unicode (i.e., it correctly promotes the char* to wchar_t*).
	/// On the otherhand, when std::stringstream is used as the base, any unicode input supplied to it
	/// will be just read as unsigned int* and not as string.
	class MStringStream : public _stringstreamBase
	{
	public:
		typedef _stringstreamBase _Mybase;
		inline MStringStream(){}
		inline MStringStream(const wchar_t* arg) { 	*this << arg; 	}
		inline MStringStream(const char* arg) { *this << arg;	}
	};

	/// <Summary> Helper class for simple string manipulations </Summary>
	class MString : public _stringBase
	{
		typedef const _baseChar* CHAR_PTR;
		typedef const _otherChar* OCHAR_PTR;
	public:
		typedef _stringBase _Mybase;
		inline MString() {}
		//inline MString(MString&& other) : _stringBase(std::move(other)) { }
		inline MString(CHAR_PTR arg) : _stringBase(arg) { }
		inline MString(OCHAR_PTR arg) : _stringBase(MStringStream(arg).str()) { }
		inline MString(const MStringStream& arg) : _stringBase(arg.str()) { }
		template<typename T> inline MString& operator<<(T arg) 
		{ 
			MStringStream _s; _s << arg; this->append(_s.str()); return *this; 
		}
		//inline MString& operator=(MString&& other) 
		//{ 
		//	_Mybase::operator=(std::move(other)); return *this; 
		//}
		inline operator CHAR_PTR() const { return c_str(); }
	};

//	/// <Summary> Helper class for simple string manipulations </Summary>
//	class MString : public
//#ifdef UNICODE
//		std::wstring
//#else
//		std::string
//#endif // UNICODE
//	{
//#ifdef UNICODE
//		typedef std::wstring base;
//#else
//		typedef std::string base;
//#endif // UNICODE
//	public:
//		inline MString() : base()
//        { }
//		inline MString(const TCHAR* sz) : base(sz)
//        { }
//		inline MString(const base& sz) : base(sz)
//        { }
//		inline MString& operator += (const TCHAR* sz)
//		{
//			base::operator += (sz);
//			return *this;
//		}
//		inline MString operator + (const TCHAR* sz) const
//		{
//			return (base&)(*this) + sz;
//		}
//		inline MString operator + (const TCHAR ch) const
//		{
//			return (base&)(*this) + ch;
//		}
//		inline MString operator + (const MString& other) const
//		{
//			return (*this) + (const TCHAR*)other;
//		}
//		inline friend MString operator + ( const TCHAR* sz, const MString& obj)
//		{
//			return MString(sz) + obj;
//		}
//		inline operator const TCHAR* () const {	return c_str(); }
//
//#if defined UNICODE || defined _UNICODE
//		inline MString(const char* arg)
//        {
//            size_t nLen = strlen(arg);
//            wchar_t *pSz = new wchar_t[nLen +2];
//            mbstowcs(pSz, arg, nLen+1);
//            *this = pSz;
//            delete[] pSz;
//        }
//		inline MString operator + (const char* sz) const
//		{
//			return std::move((base&)(*this) + MString(sz));
//		}
//#else
//		inline MString(const wchar_t* argw)
//        {
//            size_t nLen = wcslen(argw);
//            char* pSz = new char[nLen + 2];
//            wcstombs(pSz, argw, nLen+1);
//            *this = pSz;
//            delete[] pSz;
//        }
//		inline MString operator + (const wchar_t* sz) const
//		{
//			return std::move((base&)(*this) + MString(sz));
//		}
//#endif  // Cross Unicode and normal initialization
//
//#if defined QSTRING_H
//    #if defined UNICODE || defined _UNICODE
//        inline QString toQString() const
//        {
//            return QString::fromStdWString((base&)*this); //QString((QChar*)stdWString.c_str(), (int) stdWString.length());
//        }
//        inline static QString toQString(const MString& stdWString)
//        {
//            return stdWString.toQString();
//        }
//        inline static MString::base fromQString(const QString& qString)
//        {
//            return qString.toStdWString();  //std::wstring((wchar_t*)qString.unicode());
//        }
//    #else
//        #error "no QString to MString conversions in non-unicode mode"
//    #endif // unicode, normal QString
//#endif  // if QString is available
//
//	}; // class MString

} // namespace CFugue

#endif // __MSTRING_H__2B50AFA1_EFB9_428a_A397_3FFEA175FA33__
