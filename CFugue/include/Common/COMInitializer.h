#ifndef _COMINITIALIZER_H_8B5ACB47_D183_469b_AE67_B8785F9177EF
#define _COMINITIALIZER_H_8B5ACB47_D183_469b_AE67_B8785F9177EF

#include "ErrorReporter.h"

#ifndef _WIN32_DCOM
#define _WIN32_DCOM		// Required for CoInitializeEx()
#endif

#include <ObjBase.h>
#include <Map>

//
// use #define COM_THREADING_MODEL COINIT_MULTITHREADED before
// including this header file to change the COM Threading model
//
#ifndef COM_THREADING_MODEL 
#define COM_THREADING_MODEL COINIT_APARTMENTTHREADED
#endif

//
// To Use Automatic COM Initialization facility for any class, just 
// make it a derived class of CCOMInitializer class;
//
// The CCOMInitializer is a typedefed class that automatically assumes
// a multithreaded com initialization or single threaded com initialization
// based on the presence/absence of preprocessor directive MULTITHREADED
//		(This is different from THREAD_APARTMENT_MODEL)

// IMPORTANT:
// If a COM based class is used across multiple threads, then use
//		#define MULTITHREADED 
// before including this header file; That would guarantee that COM 
// is initialized properly once per each thread; However that would
// come at the cost of checking the ThreadID and COM initialization
// state per each object created !!
//
// Applications that use COM based classes only in a single thread can
// avoid that cost by not using the #define MULTITHREADED; (This is the
// default setting, just include the header and everything is fine);
//
// Note that an application might have many threads, out of which only 
// single thread might be requiring COM access; In that case it should be 
// treated as a single threaded COM app only; It is not the total thread count
// that matters - it is the number of threads that actually use the 
// COM based classes that matters;
//
//
// More than One class can use this facility; That is, more than one class can
// have the CCOMInitializer as its base class - and CCOMInitializer guarantes that
// COM initialization is not duplicated unnecessarily;
//
// However, it should be noted that when more than one class are derived from 
// CCOMInitializer and if all of them are from single thread, then it should be treated
// as single thread app; But, when more than one class are derived from CCOMInitializer
// and each class is from different thread, then it should be treated as MULTITHREADED (even
// though the class itself is confined to (used in) a single thread) and #define MULTITHREADED 
// should be used before including this header file;
//
// The Single threaded and Multithreaded should be calculated based on the clients of
// CCOMInitializer; and not based on the application threading model or the individual classes;
//
// To Specifty the Application Threading Model for the CoInitialize() modify the settings in the
// below code (perhaps by using CoInitializeEx() instead of the present CoInitialize());
//

//
// CMultiThreadCOMInitializer: Guaranteed Call for CoInitialize(), CoUninitialize() Once Per Thread;
// To Make Sure COM is initialized before any class is used, make it a derived class of CCOMInitializer;
//
// Every time an instance is created for a CCOMInitializer derived object, the thread id is checked and
// COM is initialized for that thread, if not already done; A std::map is used to keep track of
// <ThreadID, InitializationState> pair;
//
// All CoUninitialize() calls (One for each CoInitialize()) are done at the time of application exit;
//
class CMultiThreadCOMInitializer
{
	//
	// Initializer: Takes care of parining a CoUninitialize() for each successful CoInitialize();
	//
	class Initializer
	{
		Initializer& operator=(const Initializer&);
	public:
		enum INIT_STATE {NOTYET = -1, INITFAILED = 0, INITSUCCEEDED = 1};
		
		INIT_STATE m_nInitState;

		inline Initializer(const Initializer& other)
		{
			m_nInitState = NOTYET;
		}
	
		inline Initializer()
		{
			m_nInitState = NOTYET;
		}
		inline void InitializeCOM(DWORD dwThreadID)
		{
			HRESULT hr;			
			if(INITFAILED == (m_nInitState = SUCCEEDED(hr = CoInitializeEx(NULL, COM_THREADING_MODEL)) ? INITSUCCEEDED : INITFAILED))
			{
				if(hr == E_OUTOFMEMORY)
					Err::ErrorMessage(_T("Out of Memory Error Occured While Initializing COM !!"));
				else
					Err::ErrorMessage(_T("Unable to Initialize COM !!"));
			}			
		}
		inline ~Initializer()
		{
			if(m_nInitState == INITSUCCEEDED)
			{
				CoUninitialize();
				m_nInitState = NOTYET;
			}
		}
		inline INIT_STATE InitState() const	{	return m_nInitState;	}
	};

protected:

	CMultiThreadCOMInitializer(void)	// The Constructor is Protected; So Objects cannot directly be created;
	{
		typedef std::map<DWORD, Initializer> INITOBJECTMAP;

		static INITOBJECTMAP m_InitObjectMap;	// Static guarantess that Only One Map is created for the whole app;

		DWORD dwCurrentThreadID = GetCurrentThreadId();

        Initializer& Init = m_InitObjectMap[dwCurrentThreadID];	// Automatically creates a new object if not already exists;
		
		if(Init.InitState() == Initializer::NOTYET)	// If COM has not been initialized for this thread, do it now;
			Init.InitializeCOM(dwCurrentThreadID);
	}

	virtual ~CMultiThreadCOMInitializer(void)
	{
	}
};

//
// CSingleThreadCOMInitializer: Guaranteed Call for CoInitialize(), CoUninitialize() Once Per Application;
// To Make Sure COM is initialized before any class is used, make it a derived class of CCOMInitializer;
//
// COM is initialized automatically when the first time an object is created for any CCOMInitializer 
// derived class; Since this is single threading, any class object (irrespective of who triggered the COM)
// can use the COM facilities once COM is initialized;
//
// The CoUninitialize() is called automatically when the application is about to exit;
//
class CSingleThreadCOMInitializer
{
	class Initializer
	{
		bool m_bInitialized;
	public:
		Initializer()
		{
			HRESULT hr;

			m_bInitialized = SUCCEEDED(hr = CoInitializeEx(NULL, COM_THREADING_MODEL));

			if(m_bInitialized == false)
			{
				if(hr == E_OUTOFMEMORY)
					Err::ErrorMessage(_T("Out of Memory Error Occured While Initializing COM !!"));
				else
					Err::ErrorMessage(_T("Unable to Initialize COM !!"));
			}
		}
		~Initializer()
		{
			if(m_bInitialized)
			{
				CoUninitialize();
				m_bInitialized = false;
			}
		}
	};
protected:
	CSingleThreadCOMInitializer()
	{
		static Initializer Init;
	}
	virtual ~CSingleThreadCOMInitializer(){}
};


#ifdef MULTITHREADED
typedef CMultiThreadCOMInitializer	CCOMInitializer;
#else 
typedef CSingleThreadCOMInitializer	CCOMInitializer;
#endif

#endif