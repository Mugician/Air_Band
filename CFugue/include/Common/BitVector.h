#ifndef __BITVECTOR_H__D40A115A_BC2F_4125_A068_D151EFFA2677
#define __BITVECTOR_H__D40A115A_BC2F_4125_A068_D151EFFA2677

//#ifdef _DEBUG
#include "StrUtils.h"
//#endif

#include <vector>

class CBitVector
{
	typedef unsigned long BASEUNIT;

	typedef std::vector<BASEUNIT> BASEUNITVECTOR;

	BASEUNITVECTOR	m_vecBaseUnits;

	enum { BitsPerUnit = sizeof(BASEUNIT) * 8 };

public:
	CBitVector(){}	

	~CBitVector(){}

    CBitVector(CBitVector&& that) : m_vecBaseUnits(std::move(that.m_vecBaseUnits))
    {
    }

	CBitVector(const CBitVector& that)
	{
		*this = that;
	}

	inline CBitVector& operator=(const CBitVector& that)
	{
		this->m_vecBaseUnits = that.m_vecBaseUnits ;
		return *this;
	}

    inline CBitVector& operator=(CBitVector&& that)
    {
        m_vecBaseUnits = std::move(that.m_vecBaseUnits);
        return *this;
    }

    // IsBitSet() is same as operator[], but returns 0 for out-of-bound values
	inline bool IsBitSet(const int nBitIndex) const
	{
		return nBitIndex >=0 && 
			nBitIndex < BitsPerUnit *(int)this->m_vecBaseUnits.size() && 
			(this->m_vecBaseUnits[nBitIndex/BitsPerUnit] & ((BASEUNIT)1 << (nBitIndex % BitsPerUnit)));
	}

	inline bool operator[](int nBitIndex) const
	{
        _ASSERTE(nBitIndex >=0 && nBitIndex < BitsPerUnit *(int)this->m_vecBaseUnits.size());
        // This is a faster version of IsBitSet(), 
        // but should be used with care in Release mode since it doesn't validate the boundaries.
		return (this->m_vecBaseUnits[nBitIndex/BitsPerUnit] & ((BASEUNIT)1 << (nBitIndex % BitsPerUnit))) ? true : false;
	}

	inline void SetTrue(int nBitIndex) 
	{
		*this += nBitIndex;
	}

	inline void SetFalse(int nBitIndex)
	{
		*this -= nBitIndex;
	}

    // Sets all the true bit positions from the given vector to true in this vector.
    // If a bit is not set in the given vector it would not be altered in this vector.
    inline void SetTrue(const CBitVector& other)
    {
        size_t nMax = other.m_vecBaseUnits.size();

        // This operation results in a larger vector when sizes are not equal.
        // So Add Space for any Extra Units.
		while(m_vecBaseUnits.size() < nMax)
			m_vecBaseUnits.push_back(0);

		for(size_t nBaseUnitIndex=0 ; nBaseUnitIndex < nMax ; ++nBaseUnitIndex)
			m_vecBaseUnits[nBaseUnitIndex] |= other.m_vecBaseUnits[nBaseUnitIndex];
    }

    // Sets all the true bit positions from the given vector to false in this vector.
    // If a bit is not set in the given vector it would not be altered in this vector
    inline void SetFalse(const CBitVector& other)
    {
        size_t nMax = min(m_vecBaseUnits.size(), other.m_vecBaseUnits.size());

        for(size_t nBaseUnitIndex=0; nBaseUnitIndex < nMax; ++nBaseUnitIndex)
        {
            m_vecBaseUnits[nBaseUnitIndex] &= (~other.m_vecBaseUnits[nBaseUnitIndex]);
        }     
    }

    //Returns a BitVector with the Given BitIndex set to True along with existing ones
	inline CBitVector operator+(int nBitIndex) const		
	{
		CBitVector tempVector(*this);
		return tempVector += nBitIndex;
	}

    //Modifies the BitVector by setting the Given BitIndex to True
	inline CBitVector& operator+=(int nBitIndex)				
	{
        while(nBitIndex >= BitsPerUnit * (int)this->m_vecBaseUnits.size())
			this->m_vecBaseUnits.push_back(0);

		this->m_vecBaseUnits[nBitIndex/BitsPerUnit] |= ((BASEUNIT)1 << (nBitIndex % BitsPerUnit));
		return *this;
	}

    //Returns a BitVector with the Given BitIndex set to False along with existing ones
	inline CBitVector operator-(int nBitIndex) const		
	{
		CBitVector tempVector(*this);
		return tempVector -= nBitIndex;
	}

    //Modifies the BitVector by setting the Given BitIndex to False
	inline CBitVector& operator-=(int nBitIndex)				
	{
        while(nBitIndex >= BitsPerUnit * (int)this->m_vecBaseUnits.size())
			this->m_vecBaseUnits.push_back(0);

		this->m_vecBaseUnits[nBitIndex/BitsPerUnit] &= ~(((BASEUNIT)1 << (nBitIndex % BitsPerUnit)));
		return *this;
	}

    //Returns the First Index Whose Bit Value is Set to True; 
    //  Else returns -1 if none found
	inline int GetFirstTrueIndex()	const						
	{
		return GetNextIndex(0, true);
	}

    //Returns any Index after nCurrentIndex Whose Bit Value is Set to True; 
    //  Else returns -1 if none found
	inline int GetNextTrueIndex(int nCurrentIndex)	const		
	{
		return GetNextIndex(nCurrentIndex, true);
	}

	inline int GetFirstFalseIndex()	const
	{
		return GetNextIndex(0, false);
	}

	inline int GetNextFalseIndex(int nCurrentIndex)	const
	{
		return GetNextIndex(nCurrentIndex, false);
	}

	inline CBitVector operator|(const CBitVector& that) const
	{
		CBitVector tempVector(*this);
		return tempVector |= that;
	}

    // Perform Boolean OR of bitVectors
	CBitVector& operator|=(const CBitVector& that)
	{
        // Boolean OR results in a larger vector when sizes are not equal.
        // So Add Space for any Extra Units.
		while(m_vecBaseUnits.size() < that.m_vecBaseUnits.size())			
			m_vecBaseUnits.push_back(0);

		BASEUNITVECTOR::const_iterator iterBegin = that.m_vecBaseUnits.begin();
		BASEUNITVECTOR::const_iterator iterEnd = that.m_vecBaseUnits.end();
		BASEUNITVECTOR::const_iterator iter = iterBegin;

		for(int nBaseUnitIndex=0 ; iter != iterEnd; ++iter, ++nBaseUnitIndex)		
			m_vecBaseUnits[nBaseUnitIndex] |= *iter;

		return *this;
	}

	inline CBitVector operator&(const CBitVector& that) const
	{
		CBitVector tempVector(*this);
		return tempVector &= that;
	}

    // Perform Boolean AND of bitVectors
	CBitVector& operator&=(const CBitVector& that)
	{
        // Boolean AND results in smaller vector when sizes are not equal. 
        // So we can just remove any Extra Units we have without bothering to compare the sizes.
		while(m_vecBaseUnits.size() > that.m_vecBaseUnits.size())			
			m_vecBaseUnits.pop_back();

		BASEUNITVECTOR::iterator iterBegin = m_vecBaseUnits.begin();
		BASEUNITVECTOR::iterator iterEnd = m_vecBaseUnits.end();
		BASEUNITVECTOR::iterator iter = iterBegin;

		for(int nBaseUnitIndex=0 ; iter != iterEnd; ++iter, ++nBaseUnitIndex)		
			*iter &= that.m_vecBaseUnits[nBaseUnitIndex];

		return *this;
	}

    //Returns True if Any bit is Set to True
	inline bool Any() const 
	{
		for(int i=0, nMax = (int) this->m_vecBaseUnits.size(); i < nMax; ++i)
			if(this->m_vecBaseUnits[i])	
				return true;
		return false;
	}

    // Returns True if all bits are Set to False
	inline bool None() const
	{
		return !Any();
	}

    // Clears all the bits to False
	inline void Clear()
	{
		for(int i=0, nMax = (int) this->m_vecBaseUnits.size(); i < nMax; ++i)
			this->m_vecBaseUnits[i] = 0;
	}

    // Returns the number of bits set
	inline int BitCount() const	
	{
		BASEUNITVECTOR::const_iterator iter = m_vecBaseUnits.begin();
		BASEUNITVECTOR::const_iterator iterEnd = m_vecBaseUnits.end();

		int nBitCount = 0;

		for(int nBaseUnitIndex=0 ; iter != iterEnd; ++iter, ++nBaseUnitIndex)
		{
			BASEUNIT nUnit = m_vecBaseUnits[nBaseUnitIndex];	//TODO: Improve performance using char lookup for bitcount
			while(nUnit)
			{
				if(nUnit & 1)	// if bit set
					nBitCount ++;
				nUnit = nUnit >> 1;
			}
		}

		return nBitCount;
	}

    // Compares the bitVectors bit by bit
	inline bool operator==(const CBitVector& other) const
	{
		int nMyTrueIndex =  GetFirstTrueIndex();
		int nOtherTrueIndex = other.GetFirstTrueIndex();

		while((nMyTrueIndex == nOtherTrueIndex) && (nMyTrueIndex!=-1) && (nOtherTrueIndex!=-1))
		{
			nMyTrueIndex = GetNextTrueIndex(nMyTrueIndex);
			nOtherTrueIndex = other.GetNextTrueIndex(nOtherTrueIndex);
		}
		
		return nMyTrueIndex == nOtherTrueIndex;
	}

	inline bool operator!=(const CBitVector& other) const
	{
		return !(*this == other);
	}

protected:
	int GetNextIndex(int nCurrentIndex, bool bVal) const
	{
		++nCurrentIndex;		//Run Past the Current Index (Start Searching From Next Index Onwards)

		if(nCurrentIndex <0 || nCurrentIndex >= BitsPerUnit * (int) m_vecBaseUnits.size())	return -1;

		unsigned int nBaseUnitIndex  = (nCurrentIndex / BitsPerUnit);
		unsigned int nBaseUnitOffset = (nCurrentIndex % BitsPerUnit);	

		BASEUNITVECTOR::const_iterator iterEnd = m_vecBaseUnits.end();
		BASEUNITVECTOR::const_iterator iter = m_vecBaseUnits.begin() + nBaseUnitIndex;

		for( ;iter != iterEnd; ++iter, ++nBaseUnitIndex, nBaseUnitOffset = 0)
		{
			BASEUNIT nVal = *iter;

			unsigned int nOffset =  nBaseUnitOffset;	

			nVal >>= nOffset;

			while(nVal)
			{
				if((nVal & 1) == bVal)
					return nBaseUnitIndex * BitsPerUnit + nOffset;
				nVal >>= 1;
				nOffset++;
			}
		}

		return -1;	
	}

//#ifdef _DEBUG
public:
	OIL::StrUtils_Return_Type ToString() const
	{
		OIL::StrUtils_Return_Type str(_T("{"));	

		for(int i=0, nMax = BitsPerUnit * (int) this->m_vecBaseUnits.size(); i < nMax; ++i)
			if(this->IsBitSet(i))
				str += OIL::ToString(i) + _T(",");

		return str + "}";
	}	
//#endif		//End of operator LPCTSTR() Debug Only Version
};

#endif
