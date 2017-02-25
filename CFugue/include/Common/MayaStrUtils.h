#ifndef __MSTRUTILS_H_AB781278_E118_4246_A3BA_16102A7F2DFD__

#include <atlstr.h>
#define STRUTILS_RETURN_TYPE ATL::CString

#include "StrUtils.h"

#ifdef _MString	// if included Maya/MString.h

// Helper for Maya String convertion to LPCTSTR
inline LPCTSTR MStringToLPCTSTR(const MString& MStr)
{
#if defined(UNICODE) || defined(_UNICODE)
    return MStr.asWChar();
#else
    return MStr.asChar();
#endif
}

// Helper for Maya String convertion to LPCSTR
inline LPCSTR MStringToLPCSTR(const MString& MStr)
{
    return MStr.asChar();
}

// Helper for Maya String convertion to LPCWSTR
inline LPCWSTR MStringToLPCWSTR(const MString& MStr)
{
    return MStr.asWChar();
}

template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MString& MStr)
{
#if defined(UNICODE) || defined(_UNICODE)
    return _T("\"") + OIL::StrUtils_Return_Type(MStr.asWChar()) + _T("\"");
#else
    return _T("\"") + OIL::StrUtils_Return_Type(MStr.asChar()) + _T("\"");
#endif
}
#endif // _MString

#ifdef _MStringArray	// if included Maya/MStringArray.h
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MStringArray& strArr)
{
	OIL::StrUtils_Return_Type strRet(_T("{"));
    
    const unsigned int len = strArr.length();
    
	for(unsigned int i=0; len !=0 && i < len-1; ++i)
	{
		strRet += OIL::ToString(strArr[i]);
		strRet += _T(", ");
	}
    
    if(len != 0) strRet += OIL::ToString(strArr[len-1]);

	return strRet + _T("}");    
}
#endif // _MStringArray

#ifdef _MPoint	// if included maya/MPoint.h
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MPoint& pt)
{
	return OIL::StrUtils_Return_Type(_T("{")) + 
					OIL::ToString(pt.x) + _T(",") + 
					OIL::ToString(pt.y) + _T(",") + 
					OIL::ToString(pt.z) + _T(",") +
					OIL::ToString(pt.w) + _T(" ") +
				_T("}");
}
#endif //_MPoint

#ifdef _MBoundingBox // if included Maya/MBoundingBox.h
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MBoundingBox& bb)
{
	return OIL::StrUtils_Return_Type(_T("{")) + 
					_T("w=") + OIL::ToString(bb.width()) + _T(",") + 
					_T("h=") + OIL::ToString(bb.height()) + _T(",") + 
					_T("d=") + OIL::ToString(bb.depth()) + _T(",") + 
					_T("Center=") + OIL::ToString(bb.center()) + _T("") +
					//_T("Min=") + OIL::ToString(bb.center()) + _T(",") + 
					//_T("Max=") + OIL::ToString(bb.center()) + _T("") + 
				_T("}");
}
#endif // _MBoundingBox

#ifdef _MDagPathArray	// if included Maya/MDagPathArray.h
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MDagPathArray& pathArray)
{
	OIL::StrUtils_Return_Type strRet(_T("{"));

    const unsigned int len = pathArray.length();

	for(unsigned int i=0; len !=0 && i < len-1; ++i)
	{
		strRet += OIL::ToString(pathArray[i].fullPathName());
		strRet += _T(", ");
	}

    if(len != 0) strRet += OIL::ToString(pathArray[len-1].fullPathName());

	return strRet + _T("}");
}
#endif	// _MDagPathArray

#ifdef _MVector	// if included Maya/MVector.h
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MVector& vec)
{
	return OIL::StrUtils_Return_Type(_T("{")) + 
					OIL::ToString(vec.x) + _T(",") + 
					OIL::ToString(vec.y) + _T(",") + 
					OIL::ToString(vec.z) + _T(" ") +
				_T("}");
}
#endif //_MVector

#ifdef _MFloatVector	// if included Maya/MFloatVector.h
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MFloatVector& vec)
{
	return OIL::StrUtils_Return_Type(_T("{")) + 
					OIL::ToString(vec.x) + _T(",") + 
					OIL::ToString(vec.y) + _T(",") + 
					OIL::ToString(vec.z) + _T(" ") +
				_T("}");
}
#endif //_MVector

#ifdef _MFloatMatrix // if included Maya/MFloatMatrix.h
template<> 
inline OIL::StrUtils_Return_Type OIL::ToString(const MFloatMatrix& fm)
{
	return OIL::StrUtils_Return_Type(_T("{")) +
		OIL::StrUtils_Return_Type(_T("{")) + OIL::ToString(fm.matrix[0][0]) + _T(",") + OIL::ToString(fm.matrix[0][1]) + _T(",") + OIL::ToString(fm.matrix[0][2]) + _T(",") + OIL::ToString(fm.matrix[0][3]) + _T(" }") +
		OIL::StrUtils_Return_Type(_T("{")) + OIL::ToString(fm.matrix[1][0]) + _T(",") + OIL::ToString(fm.matrix[1][1]) + _T(",") + OIL::ToString(fm.matrix[1][2]) + _T(",") + OIL::ToString(fm.matrix[1][3]) + _T(" }") +
		OIL::StrUtils_Return_Type(_T("{")) + OIL::ToString(fm.matrix[2][0]) + _T(",") + OIL::ToString(fm.matrix[2][1]) + _T(",") + OIL::ToString(fm.matrix[2][2]) + _T(",") + OIL::ToString(fm.matrix[2][3]) + _T(" }") +
		OIL::StrUtils_Return_Type(_T("{")) + OIL::ToString(fm.matrix[3][0]) + _T(",") + OIL::ToString(fm.matrix[3][1]) + _T(",") + OIL::ToString(fm.matrix[3][2]) + _T(",") + OIL::ToString(fm.matrix[3][3]) + _T(" }") +
				_T("}");
}
#endif // _MFloatMatrix

#ifdef _MMatrix	//  if included Maya/MMatrix.h
template<> 
inline OIL::StrUtils_Return_Type OIL::ToString(const MMatrix& mat)
{
	return OIL::StrUtils_Return_Type(_T("{")) +
		OIL::StrUtils_Return_Type(_T("{")) + OIL::ToString(mat.matrix[0][0]) + _T(",") + OIL::ToString(mat.matrix[0][1]) + _T(",") + OIL::ToString(mat.matrix[0][2]) + _T(",") + OIL::ToString(mat.matrix[0][3]) + _T(" }") +
		OIL::StrUtils_Return_Type(_T("{")) + OIL::ToString(mat.matrix[1][0]) + _T(",") + OIL::ToString(mat.matrix[1][1]) + _T(",") + OIL::ToString(mat.matrix[1][2]) + _T(",") + OIL::ToString(mat.matrix[1][3]) + _T(" }") +
		OIL::StrUtils_Return_Type(_T("{")) + OIL::ToString(mat.matrix[2][0]) + _T(",") + OIL::ToString(mat.matrix[2][1]) + _T(",") + OIL::ToString(mat.matrix[2][2]) + _T(",") + OIL::ToString(mat.matrix[2][3]) + _T(" }") +
		OIL::StrUtils_Return_Type(_T("{")) + OIL::ToString(mat.matrix[3][0]) + _T(",") + OIL::ToString(mat.matrix[3][1]) + _T(",") + OIL::ToString(mat.matrix[3][2]) + _T(",") + OIL::ToString(mat.matrix[3][3]) + _T(" }") +
				_T("}");
}
#endif // _MMatrix

#ifdef _MFnCamera	// if included Maya/MFnCamera.h
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MFnCamera::FilmFit& ff)
{
	const TCHAR* FilmFitStr[] = { _T("kFillFilmFit"), _T("kHorizontalFilmFit"), _T("kVerticalFilmFit"), _T("kOverscanFilmFit"), _T("kInvalid") };
	return OIL::StrUtils_Return_Type(FilmFitStr[ff]);
}

template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MFnCamera::RollOrder& ro)
{
	const TCHAR* RollOrderStr[] = { _T("kRotateTranslate"), _T("kTranslateRotate") };
	return OIL::StrUtils_Return_Type(RollOrderStr[ro]);
}
#endif // _MFnCamera

#ifdef _MColor // if included Maya/MColor.h
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MColor& Color)
{
	return OIL::StrUtils_Return_Type(_T("{")) +
				OIL::ToString(Color.r)	+ _T(",") +
				OIL::ToString(Color.g)	+ _T(",") +
				OIL::ToString(Color.b)	+ _T(",") +
				OIL::ToString(Color.a)	+ _T(" ") +
			_T("}");
}
#endif //_MColor

#ifdef _MFnNurbsCurve	// if included <Maya/MFnNurbsCurve.h>
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MFnNurbsCurve::Form& form)
{
	const TCHAR* FormStr[] = { _T("kInvalid"), _T("kOpen"), _T("kClosed"), _T("kPeriodic"), _T("kLast")};
	return OIL::StrUtils_Return_Type(FormStr[form]);
}
#endif //_MFnNurbsCurve

#ifdef _MFnNurbsSurface	// if included <Maya/MFnNurbsSurface.h>
template<>
inline OIL::StrUtils_Return_Type OIL::ToString(const MFnNurbsSurface::Form& form)
{
	const TCHAR* FormStr[] = { _T("kInvalid"), _T("kOpen"), _T("kClosed"), _T("kPeriodic"), _T("kLast")};
	return OIL::StrUtils_Return_Type(FormStr[form]);
}
#endif //_MFnNurbsSurface

#endif //__MSTRUTILS_H_AB781278_E118_4246_A3BA_16102A7F2DFD__