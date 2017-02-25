#ifndef __WINSTRUTILS_H__64B78566_C82A_438c_BA3E_50C763964D7B__
#define __WINSTRUTILS_H__64B78566_C82A_438c_BA3E_50C763964D7B__

#include "StrUtils.h"

//////////////////////////////////////////////////////////////////////////
/// WinStrUtils.h is a specialization of Strutils.h for Windows datatypes
///

namespace OIL
{
#ifdef _INC_COMUTIL	// if included Comutil.h
	//template<>
	//inline StrUtils_Return_Type	ToString(const variant_t& varVal)
	//{
	//	return varVal;
	//}
#endif	// _INC_COMUTIL

#ifdef GUID_DEFINED	// if included GuidDef.h
	template<>
	inline StrUtils_Return_Type	ToString(const GUID& guid)
	{
#pragma warning(push)
#pragma warning(disable:4244)	// Disable the nasty "LPVOID to Unsigned long" conversion warning
		StrUtils_Return_Type strData1 = UnSignedNumber2Str(guid.Data1, sizeof(guid.Data1) * 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData2 = UnSignedNumber2Str(guid.Data2, sizeof(guid.Data2) * 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData3 = UnSignedNumber2Str(guid.Data3, sizeof(guid.Data3) * 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData40 = UnSignedNumber2Str((unsigned int)(guid.Data4[0]), 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData41 = UnSignedNumber2Str((unsigned int)(guid.Data4[1]), 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData42 = UnSignedNumber2Str((unsigned int)(guid.Data4[2]), 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData43 = UnSignedNumber2Str((unsigned int)(guid.Data4[3]), 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData44 = UnSignedNumber2Str((unsigned int)(guid.Data4[4]), 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData45 = UnSignedNumber2Str((unsigned int)(guid.Data4[5]), 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData46 = UnSignedNumber2Str((unsigned int)(guid.Data4[6]), 2, DEFAULT_FILLER, 16, false);
		StrUtils_Return_Type strData47 = UnSignedNumber2Str((unsigned int)(guid.Data4[7]), 2, DEFAULT_FILLER, 16, false);
	return strData1 + _T("-") + strData2 + _T("-") + strData3 + _T("-") +
			strData40 + strData41 + _T("-") + strData42 + strData43 + strData44 + strData45 + strData46 + strData47;
#pragma warning(pop)
	}
#endif // GUID_DEFINED


 #ifdef _OBJBASE_H_	// if included ObjBase.h
	template<>
	inline StrUtils_Return_Type	ToString(const COMSD& comsd)
	{
		const TCHAR* strComSD[] = { _T("SD_LAUNCHPERMISSIONS"), _T("SD_ACCESSPERMISSIONS"), _T("SD_LAUNCHRESTRICTIONS"), _T("SD_ACCESSRESTRICTIONS")};
		return strComSD[comsd];
	}
#endif // _OBJBASE_H_

#ifdef WINVER   // if included Windows.h

#define DECLARE_HANDLE_TYPE_TO_STRING(name)	\
	template<>\
	inline StrUtils_Return_Type ToString(const name& _obj)\
	{	\
		return ToString((void*)_obj);		\
	}

	DECLARE_HANDLE_TYPE_TO_STRING(HBITMAP)
	DECLARE_HANDLE_TYPE_TO_STRING(HBRUSH)
	DECLARE_HANDLE_TYPE_TO_STRING(HCOLORSPACE)
	DECLARE_HANDLE_TYPE_TO_STRING(HDESK)
	DECLARE_HANDLE_TYPE_TO_STRING(HENHMETAFILE)
#ifdef WINABLE
	DECLARE_HANDLE_TYPE_TO_STRING(HEVENT)
#endif
	DECLARE_HANDLE_TYPE_TO_STRING(HFONT)
	DECLARE_HANDLE_TYPE_TO_STRING(HGLRC)
	DECLARE_HANDLE_TYPE_TO_STRING(HHOOK)
	DECLARE_HANDLE_TYPE_TO_STRING(HICON)
	DECLARE_HANDLE_TYPE_TO_STRING(HINSTANCE)
	DECLARE_HANDLE_TYPE_TO_STRING(HKL)
	DECLARE_HANDLE_TYPE_TO_STRING(HKEY)
	DECLARE_HANDLE_TYPE_TO_STRING(HMENU)
	DECLARE_HANDLE_TYPE_TO_STRING(HMETAFILE)
	DECLARE_HANDLE_TYPE_TO_STRING(HMONITOR)
	DECLARE_HANDLE_TYPE_TO_STRING(HRGN)
	DECLARE_HANDLE_TYPE_TO_STRING(HRSRC)
	DECLARE_HANDLE_TYPE_TO_STRING(HSPRITE)
	DECLARE_HANDLE_TYPE_TO_STRING(HSTR)
	DECLARE_HANDLE_TYPE_TO_STRING(HTASK)
	DECLARE_HANDLE_TYPE_TO_STRING(HUMPD)
	DECLARE_HANDLE_TYPE_TO_STRING(HWINSTA)
	DECLARE_HANDLE_TYPE_TO_STRING(HWND)

	template<>
	inline StrUtils_Return_Type ToString<FARPROC>(const FARPROC& fProc)
	{
		return ToString((void*)fProc);
	}

	//template<>
	//inline StrUtils_Return_Type ToString(const HINSTANCE& hInstance)
	//{
	//	return ToString((void*)hInstance);
	//}

	//template<>
	//inline StrUtils_Return_Type ToString(const HWND& hWnd)
	//{
	//	return ToString((void*)hWnd);
	//}
	//#if defined __GNUC__
	//#if __WORDSIZE == 64
	//#define __int64 long long
	//#define __uint64 unsigned long long
	//#else // wordsize is 32
	//#include <inttypes.h>
	//#define __int64  int64_t
	//#define __uint64 uint64_t
	//#endif //wordsize
	//#elif defined _MSC_VER
	//#define __uint64 unsigned __int64
	//#else // some unknown compiler
	//#define __int64 long long
	//#define __uint64 unsigned long long
	//#endif // if defined __GNUC__

	template<>
	inline StrUtils_Return_Type	ToString(const __int64& nNumber)
	{
		return SignedNumber2Str(nNumber);
	}

	template<>
	inline StrUtils_Return_Type ToString(const LARGE_INTEGER& liNumber)
	{
		return ToString(liNumber.QuadPart);
	}

	template<>
	inline StrUtils_Return_Type	ToString(const POINT& Point)
	{
		return StrUtils_Return_Type(_T("{")) + ToString(Point.x) + _T(",") + ToString(Point.y) + _T("}");
	}

	template<>
	inline StrUtils_Return_Type	ToString(const RECT& rect)
	{
		return StrUtils_Return_Type(_T("{")) + ToString(rect.left) + _T(",") + ToString(rect.top) + _T(",") + ToString(rect.right) + _T(",") + ToString(rect.bottom) + _T("}");;
	}

	template<>
	inline StrUtils_Return_Type	ToString(const SIZE& Size)
	{
		return StrUtils_Return_Type(_T("{")) + ToString(Size.cx) + _T(",") + ToString(Size.cy) + _T("}");
	}


	template<>
	inline StrUtils_Return_Type ToString(const ULARGE_INTEGER& uliNumber)
	{
		return ToString(uliNumber.QuadPart);
	}
#endif  // ifdef WINVER

#ifdef DIRECT3D_VERSION // if included DirectX headers
	template<>
	inline StrUtils_Return_Type	ToString(const D3DXVECTOR3& Vec)
	{
		return StrUtils_Return_Type(_T("{")) + ToString(Vec.x) + _T(",") + ToString(Vec.y) + _T(",") + ToString(Vec.z) + _T("}");
	}
#endif	// ifdef DIRECT3D_VERSION
} // namespace OIL
#endif // __WINSTRUTILS_H__64B78566_C82A_438c_BA3E_50C763964D7B__