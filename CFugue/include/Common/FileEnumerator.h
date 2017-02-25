#ifndef __FILE_ENUMERATOR_H_EEF8FBA5_B0F1_4682_9614_2027C16002F4
#define __FILE_ENUMERATOR_H_EEF8FBA5_B0F1_4682_9614_2027C16002F4

#include "PathHelper.h"

typedef bool (*PFN_EnumeratorCallback)(LPCTSTR lpszItemName, LPCTSTR lpszItemDir, LPWIN32_FIND_DATA lpszItemFindData, LPVOID lpUserData);

// Recursively enumerates the given directory for the specified file types. Invokes the supplied callback for each file found.
// Can be used to enumerate sub-directories too.
// (Does not support modifying the search properties while enumeration is in progress).
class CFileEnumerator
{
	CString	m_strSearchDir;		// The search directory.

	CString	m_strFileToSearch;	// Pattern of the file that should be searched for. Valid only for file enumeration

	bool	m_bRecurseSubDirectories;	// Indicates if sub-directories should be recursed during an enumeration

	bool	m_bIgnoreDots;	// Valid only for directory listing.

	bool	m_bReportMatchingDirsAlso;	// Valid only for File Listing. Indicates if the directories should be included in the reporting while searching for files

	PFN_EnumeratorCallback m_pfnEnumCallback;	// Callback function for the enumeration

	inline static bool EnumCallback(LPCTSTR, LPCTSTR, LPWIN32_FIND_DATA, LPVOID)
	{
		return false;	// returning false stops the enumeration at that point
	}
public:

	CFileEnumerator(PFN_EnumeratorCallback pfnEnumCallback = CFileEnumerator::EnumCallback,	// Callback function for the enumeration
					LPCTSTR lpszSearchDir = _T("."),	// Should be a directory path (excluding the file name and wild cards) eg. "c:\\" or "C:\\Dir1"
					LPCTSTR lpszFileToSearch = _T("*.*"),// Should be a file name or wild card eg. "*.*"
					bool bRecurseSubDirectories = true,// Should the sub-directories be recursed?
					bool bReportMatchingDirsAlso = true,// Should the matching directories also be reported while searching for files? - Valid only for file enumeration
					bool bIgnoreDots = true	// Should . and .. be informed ?? - Valid Only when Listing Directories
					)
	{
		m_pfnEnumCallback = pfnEnumCallback;
		m_strSearchDir = lpszSearchDir;
		m_strFileToSearch = lpszFileToSearch;
		m_bRecurseSubDirectories = bRecurseSubDirectories;
		m_bReportMatchingDirsAlso = bReportMatchingDirsAlso;
		m_bIgnoreDots = bIgnoreDots;
	}

	~CFileEnumerator(void)
	{
	}

	PFN_EnumeratorCallback& FileEnumeratorCallback()
	{
		return m_pfnEnumCallback;
	}

	bool& IgnoreDots()				{	return m_bIgnoreDots;	}	// Indicates if Dots will be processed for the Callback (only in sub-directory listing mode)

	bool& RecurseSubDirectories()	{	return m_bRecurseSubDirectories;	}	// Indicates if sub-directories will be recursively searched or not

	CString& FileToSearch()			{	return m_strFileToSearch;	}	// The Files being searched for

	LPCTSTR FileToSearch() const	{	return m_strFileToSearch;	}	// The Files being searched for

	CString& SearchDir()			{	return m_strSearchDir;	}	// The directory being searched

	LPCTSTR SearchDir() const		{	return m_strSearchDir;	}	// The directory being searched

	bool& ReportMatchingDirsAlso()	{	return m_bReportMatchingDirsAlso;	}	// Indicates if Directories should also be reported while searching for files if the pattern matches

	HANDLE FindFirstSubDir(LPCTSTR lpszDir, LPWIN32_FIND_DATA pFindData) const
	{
        HANDLE hFind = FindFirstFile(CString(lpszDir) + _T("*.*"), pFindData);
		if(hFind == INVALID_HANDLE_VALUE)	return hFind;

		do
		{
			if(PathIsDots(pFindData->cFileName))
			{
				if(m_bIgnoreDots == false)
					return hFind;
				continue;
			}

			if(!(pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;

			return hFind;

		}while(FindNextFile(hFind, pFindData));

		return INVALID_HANDLE_VALUE;
	}

	BOOL FindNextSubDir(HANDLE hFind, LPWIN32_FIND_DATA pFindData) const
	{
		while(FindNextFile(hFind, pFindData))
		{
			if(PathIsDots(pFindData->cFileName))	//!_tcscmp(pFindData->cFileName, _T(".")) || !_tcscmp(pFindData->cFileName, _T("..")))
			{
				if(m_bIgnoreDots == false)
					return true;
				continue;
			}

			if(!(pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;

			return true;
		}
		return false;
	}

	//
	// ListAllDirectories: Enumerates all directories in the SearchDir folder; Doesnt take FileToSearch() into account;
	// The optional lpUserData parameter would be sent back to the caller in the EnumeratorCallback.
	// (Does not support modifying the search properties while listing is in progress)
	// Returns true if search is complete. False if the callback requested to stop the search.
	bool ListAllDirectories(LPVOID lpUserData = NULL) const
	{
		WIN32_FIND_DATA ffData;

		CString strSearchDir((LPCTSTR)SearchDir());
		PathAddBackSlash(strSearchDir);

		HANDLE hFind = FindFirstSubDir(strSearchDir, &ffData);
		if(hFind == INVALID_HANDLE_VALUE) return true;

		bool bContinue = true;

		do
		{
			if(false == (*m_pfnEnumCallback)(ffData.cFileName, strSearchDir, &ffData, lpUserData))
				return false;

			if(m_bRecurseSubDirectories)
			{
				if(PathIsDots(ffData.cFileName)) continue;

				CFileEnumerator feDir(m_pfnEnumCallback,
									strSearchDir + ffData.cFileName, 
									_T("*.*"), 
									m_bRecurseSubDirectories,
									m_bReportMatchingDirsAlso,
									m_bIgnoreDots);
				bContinue = feDir.ListAllDirectories(lpUserData);
			}
		}while(bContinue && FindNextSubDir(hFind, &ffData));

		FindClose(hFind);

		return true;
	}

	//
	// EnumerateFiles: Enumerates all files of type FileToSearch() in the SearchDir() folder;
	// Subdirectories will be considered for enumeration based on the RecurseSubDirectories() property.
	// The optional lpUserData parameter would be sent back to the caller in the EnumeratorCallback.
	// (Does not support modifying the search properties while enumeration is in progress)
	// Returns true if search is complete. False if the callback requested to stop the search.
	bool EnumerateFiles(LPVOID lpUserData = NULL) const
	{
		bool bContinue = true;

		CString strSearchDir((LPCTSTR)SearchDir());

		PathAddBackSlash(strSearchDir);

		WIN32_FIND_DATA ffData;

		HANDLE hFind = FindFirstFile(strSearchDir + (LPCTSTR)FileToSearch(), &ffData);
		if(hFind == INVALID_HANDLE_VALUE)
		{
			if(m_bRecurseSubDirectories)	// If this dir doesn't contain the wild card files, check in the subdirs
			{
				CFileEnumerator feDir(EnumCallback, 
										strSearchDir, 
										_T("*.*"), 
										m_bRecurseSubDirectories,
										m_bReportMatchingDirsAlso,
										true);	// important to ignore the dots for subdir enumeration
				hFind = feDir.FindFirstSubDir(strSearchDir, &ffData);
				if(hFind == INVALID_HANDLE_VALUE)	return true;
				do
				{
					bContinue = CFileEnumerator( m_pfnEnumCallback, 
												strSearchDir + ffData.cFileName,
												FileToSearch(),
												m_bRecurseSubDirectories,
												m_bReportMatchingDirsAlso,
												m_bIgnoreDots).EnumerateFiles(lpUserData);
				}while(bContinue && feDir.FindNextSubDir(hFind, &ffData));

				FindClose(hFind);

				return bContinue;
			}
			else
				return true;
		}

		do		// this directory contains some files that match the wild card..
		{
			if(PathIsDots(ffData.cFileName))
			{
				if(m_bIgnoreDots == false)
					bContinue = (*m_pfnEnumCallback)(ffData.cFileName, strSearchDir, &ffData, lpUserData);
				continue;
			}

			if((ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if(m_bReportMatchingDirsAlso)	// Do we need to report the matching directories too?
					bContinue = (*m_pfnEnumCallback)(ffData.cFileName, strSearchDir, &ffData, lpUserData);
			}
			else
			{
				bContinue = (*m_pfnEnumCallback)(ffData.cFileName, strSearchDir, &ffData, lpUserData);
			}

		}while(bContinue && FindNextFile(hFind, &ffData));

		FindClose(hFind);

		if(bContinue && m_bRecurseSubDirectories) //List the files from sub-directories, if required
		{
			CFileEnumerator feDir(EnumCallback, 
									strSearchDir, 
									_T("*.*"), 
									m_bRecurseSubDirectories, 
									m_bReportMatchingDirsAlso,
									true);	// important to ignore the dots for subdir enumeration
			hFind = feDir.FindFirstSubDir(strSearchDir, &ffData);
			if(hFind == INVALID_HANDLE_VALUE)	return true;
			do
			{
				bContinue = CFileEnumerator( m_pfnEnumCallback, 
											strSearchDir + ffData.cFileName,
											FileToSearch(),
											m_bRecurseSubDirectories,
											m_bReportMatchingDirsAlso,
											m_bIgnoreDots).EnumerateFiles(lpUserData);
			}while(bContinue && feDir.FindNextSubDir(hFind, &ffData));

			FindClose(hFind);
		}

		return bContinue;
	}
};

#endif