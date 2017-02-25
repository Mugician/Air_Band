#ifndef __PATH_HELPER_H__516BEAA4_9BA9_47f8_A9B1_DF4AC1B98D7A
#define __PATH_HELPER_H__516BEAA4_9BA9_47f8_A9B1_DF4AC1B98D7A

#include <AtlStr.h>

inline bool PathIsDots(const CString& strFilePath)
{
	return strFilePath == _T(".") || strFilePath == _T("..");
}

inline CString& PathAddBackSlash(CString& strFilePath)
{
	if(strFilePath.Right(1) != _T('\\'))
		strFilePath += _T("\\");
	return strFilePath;
}

inline CString PathGetFileName(const CString& strFilePath)
{
	int nIndex = strFilePath.ReverseFind(_T('\\'));
	if(nIndex == -1)
		return strFilePath;
	return strFilePath.Mid(nIndex+1);
}

inline CString PathGetFileExtension(const CString& strFilePath)
{
	int nIndex = strFilePath.ReverseFind(_T('.'));
	if(nIndex == -1)
		return _T("");
	return strFilePath.Mid(nIndex+1);
}

inline CString PathGetFileTitle(const CString& strFilePath)
{
	CString strFileName(PathGetFileName(strFilePath));
	if(false == strFileName.IsEmpty())
	{
		int nIndex = strFileName.ReverseFind(_T('.'));
		if(nIndex == -1)
			return strFileName;
		return strFileName.Mid(0, nIndex);
	}
	else
		return strFileName;
}

inline void PathGetFileTitleAndExtension(const CString& strFilePath, CString& strFileTitle, CString& strFileExtension)
{
	strFileTitle = _T("");
	strFileExtension = _T("");

	CString strFileName(PathGetFileName(strFilePath));

	if(false == strFileName.IsEmpty())
	{
		int nIndex = strFileName.ReverseFind(_T('.'));
		if(nIndex != -1)
		{
			strFileTitle = strFileName.Mid(0, nIndex);
			strFileExtension = strFileName.Mid(nIndex+1);
		}
		else		
			strFileTitle = strFileName;
	}
}

inline CString PathGetFolder(const CString& strFilePath)	// If strFilePath points to a dir then it should have a \ in the end;
{
	int nIndex = strFilePath.ReverseFind(_T('\\'));
	if(nIndex == -1)
		return _T(".\\");
	return strFilePath.Mid(0, nIndex+1);	// We Have to include terminating \\ also
}

inline void PathGetFolderAndFileName(const CString& strFilePath, CString& strFolder, CString& strFileName)
{
	int nIndex = strFilePath.ReverseFind(_T('\\'));
	if(nIndex == -1)	
	{
		strFileName = strFilePath;
		strFolder = _T(".\\");
		return;
	}
	strFolder = strFilePath.Mid(0, nIndex+1);	// We Have to include terminating \\ also
	strFileName = strFilePath.Mid(nIndex+1);
}

//
// PathGetRootDir: Returns the First Dir on the path;
// if strItemPath is a filename or empty, then returns empty string;
// e.g. c:\ returns "c:\\" ; c:\text\one\ returns "c:\\" , one.txt returns ""
//
inline CString PathGetRootDir(const CString& strItemPath)
{
	int nIndex = strItemPath.Find(_T('\\'));
	if(nIndex == -1)	return _T("");
	return strItemPath.Mid(0, nIndex+1);	// We Have to include terminating \\ also
}

//
// Replicates the non-strSourceRoot portion of strTargetItemPath at strDestination Root;
// strTargetItemPath could be a File Path rooted at strSourceRoot;
//
bool CreateDirectoryStructure(CString& strDestinationRoot, const CString& strSourceRoot, const CString& strTargetItemPath)
{
	CString strRelativeTargetDir;

	if(strTargetItemPath.Find(strSourceRoot) != 0)
		strRelativeTargetDir = strTargetItemPath;
	else
		strRelativeTargetDir = strTargetItemPath.Mid(strSourceRoot.GetLength());

    CString FirstDir = PathGetRootDir(strRelativeTargetDir);
	while(FirstDir.IsEmpty() == false)
	{
        strDestinationRoot += FirstDir;
		if(PathFileExists(strDestinationRoot) == false)
			if(!CreateDirectory(strDestinationRoot, NULL))
				return false;
		strRelativeTargetDir = strRelativeTargetDir.Mid(FirstDir.GetLength());
		FirstDir = PathGetRootDir(strRelativeTargetDir);;
	}

	return true;
}

#endif
