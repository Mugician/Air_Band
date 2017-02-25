#ifndef _ERROR_H__FC8EA04F_57D6_4002_81BE_F37E1FA54F47_
#define _ERROR_H__FC8EA04F_57D6_4002_81BE_F37E1FA54F47_

#include <TChar.h>

#ifndef VERBOSE_ERR
#define VERBOSE_ERR	0	//Make this 0 to suppress the default MsgBox()
#endif

namespace Err
{
	/////////////////////////////////////////////////////////////////////
	/// Collects and Formats the Error Message for the given error code.
	/// Returns the input string.
	///
	inline LPCTSTR GetErrorDetails(LPTSTR lpszBuf, DWORD dwBufLen, DWORD dwErrorCode = GetLastError())
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpszBuf, dwBufLen, NULL );
		return lpszBuf;
	}

	class CErrReporter
	{
		enum {ErrMsgBufSize = 1023};

		TCHAR m_szErrBuf[ ErrMsgBufSize + 1 ];

	public:
		enum ErrReportingMode { Mode_MessageBox, Mode_OutputDebugString };

	protected:
		ErrReportingMode m_ReportingMode;

	public:
#if VERBOSE_ERR
		CErrReporter(ErrReportingMode Mode = Mode_MessageBox) : m_ReportingMode(Mode)
		{
			m_szErrBuf[0] = _T('\0');
		}
		CErrReporter(LPCTSTR lpszErrMsg, ErrReportingMode Mode = Mode_MessageBox) : m_ReportingMode(Mode)
		{
			SetErrorMessage(lpszErrMsg);
		}
#else
		CErrReporter(ErrReportingMode Mode = Mode_OutputDebugString) : m_ReportingMode(Mode)
		{
			m_szErrBuf[0] = _T('\0');
		}
		CErrReporter(LPCTSTR lpszErrMsg, ErrReportingMode Mode = Mode_OutputDebugString) : m_ReportingMode(Mode)
		{
			SetErrorMessage(lpszErrMsg);
		}
#endif
		// Copy constructor
		CErrReporter(const CErrReporter& other)
		{
			*this = other;
		}

		/// Gets/Sets the Error Reporting Mode
		inline ErrReportingMode& ReportingMode()
		{
			return m_ReportingMode;
		}

		/// Collects and Formats the Error Message for the last error
		inline LPCTSTR CollectErrorDetails(DWORD dwErrorCode = GetLastError())
		{
			return GetErrorDetails(m_szErrBuf, ErrMsgBufSize, dwErrorCode);
		}

		/// Displays or DebugPrints the Error Message for the last error (Uses GetLastError() API).
		/// Also Sets the Error Message;
		inline void ReportLastError()
		{			
			ReportError(GetLastError());
		}

		/// Displays or DebugPrints the Error Message for the given error code.
		/// Equivalent to ReportLastError() when dwErrorCode == GetLastError();
		inline void ReportError(DWORD dwErrorCode)
		{
			CollectErrorDetails(dwErrorCode);
			ReportError();
		}

		/// Sets and Displays or DebugPrints the supplied Error Message.
		/// Useful for displaying custom messages.
		inline void ReportError(LPCTSTR lpszErrMsg)
		{
			SetErrorMessage(lpszErrMsg);
			ReportError();
		}

		/// Reports the ErrorMessage that was set with SetErrorMessage.
		inline void ReportError() const
		{
			ErrorMessage(m_szErrBuf);
		}

		/// Clears the Error Message Buffer
		inline void Clear()
		{
			ZeroMemory(m_szErrBuf, sizeof(m_szErrBuf));
		}

		/// Sets the Error Message
		inline void SetErrorMessage(LPCTSTR lpszErrMsg)
		{
			_tcsncpy(m_szErrBuf, lpszErrMsg, ErrMsgBufSize);
			m_szErrBuf[ErrMsgBufSize] = _T('\0');
		}

		/// Returns the last Set Error Message
		inline LPCTSTR GetErrorMessage() const
		{
			return m_szErrBuf;
		}

		inline operator LPCTSTR() const
		{
			return GetErrorMessage();
		}

		inline CErrReporter& operator=(const CErrReporter& other)
		{
			m_ReportingMode = other.m_ReportingMode;
			SetErrorMessage(other.m_szErrBuf);
			return *this;
		}

	protected:
		/// Displays the given Error Message with MessageBox
		inline void ErrorMessage(LPCTSTR lpszErrMsg) const
		{
			switch(m_ReportingMode)
			{
			case Mode_MessageBox:	MessageBox(NULL, lpszErrMsg, _T("Error"), MB_OK | MB_ICONERROR); break;
			case Mode_OutputDebugString: OutputDebugString(lpszErrMsg); break;
			default: break;
			}
		}

		friend void ErrorMessage(LPCTSTR lpszErrMsg);
	};

	/// Displays or DebugPrints the given Error Message
	inline void ErrorMessage(LPCTSTR lpszErrMsg)
	{
		static CErrReporter ErrRep;
		ErrRep.ErrorMessage(lpszErrMsg);
	}
}			// namespace Err
#endif		// #ifndef _ERROR_H__FC8EA04F_57D6_4002_81BE_F37E1FA54F47_

