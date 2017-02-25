#ifndef __EVENTEDPROPERTY_H__221ADEB0_5C31_4bfe_872F_73B16AB08F75
#define __EVENTEDPROPERTY_H__221ADEB0_5C31_4bfe_872F_73B16AB08F75

#include "EventHandler.h"

template<typename DATATYPE>
class CPropertyChangedEventArgs: public OIL::CEventHandlerArgs
{
protected:
	DATATYPE m_OldValue;
	DATATYPE m_NewValue;
public:
	template<typename OLDVALTYPE, typename NEWVALTYPE>
	CPropertyChangedEventArgs(const OLDVALTYPE& oldVal, const NEWVALTYPE& NewVal)
		: m_OldValue(oldVal), m_NewValue(NewVal)
	{
	}
	inline const DATATYPE& OldValue() const	{	return m_OldValue;	}
	inline const DATATYPE& NewValue() const	{	return m_NewValue;	}
};

template<typename DATATYPE>
class CPropertyChangingEventArgs: public CPropertyChangedEventArgs<DATATYPE>
{
	bool	 m_bCancel;
public:
	template<typename OLDVALTYPE, typename NEWVALTYPE>
	CPropertyChangingEventArgs(const OLDVALTYPE& oldVal, const NEWVALTYPE& NewVal)
		: CPropertyChangedEventArgs<DATATYPE>(oldVal, NewVal), m_bCancel(false)
	{		
	}
	inline bool Cancel() const	{	return m_bCancel;	}
	inline void SetCancel()		{	m_bCancel = true;	}
};

template<typename DATATYPE>
class CEventedProperty : public OIL::CEventSource
{
	DATATYPE m_Data;
public:
	typedef CEventedProperty<DATATYPE> thisClass;

	typedef OIL::CEventT<CEventedProperty<DATATYPE>,  CPropertyChangingEventArgs<DATATYPE> > PropertyChangingEvent;
	typedef OIL::CEventT<CEventedProperty<DATATYPE>,  CPropertyChangedEventArgs<DATATYPE> > PropertyChangedEvent;

	PropertyChangingEvent ValueChanging;
	PropertyChangedEvent ValueChanged;

	CEventedProperty()	{}

	template<typename T>
	CEventedProperty(const T& InitVal): m_Data(InitVal)	{}

	inline operator DATATYPE() const	{	return m_Data;	}

	template<typename T>
	inline CEventedProperty& operator =(const T& other)	
	{	
		CPropertyChangingEventArgs<DATATYPE> evArgs(m_Data, other);
		RaiseEvent(&ValueChanging, &evArgs);

		if(evArgs.Cancel() == false)
		{
			CPropertyChangedEventArgs<DATATYPE> ChangedEvArgs(m_Data,other); //Not to Loose Old Data we first create the EvArgs obj
			m_Data = other;
			RaiseEvent(&ValueChanged, &ChangedEvArgs);
		}

		return *this;
	}
};

#endif