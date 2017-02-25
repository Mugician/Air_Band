#ifndef __bitcache_h_11A6BC64_8CC3_447B_AD83_56BCF4FF0083__
#define __bitcache_h_11A6BC64_8CC3_447B_AD83_56BCF4FF0083__

#include <vector>
#include <memory>

#ifndef _ASSERTE
#define _ASSERTE(x) (0)
#endif

#if !defined(_NO_TRANSFORM_SEQUENCES) || (!_NO_TRANSFORM_SEQUENCES)
#define _USE_TRANSFORM_SEQUENCES 1
#else
#define _USE_TRANSFORM_SEQUENCES 0
#endif

#if _USE_TRANSFORM_SEQUENCES
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/next_prior.hpp>
#endif

//#warning "INFO: some message for GCC Compilation"
namespace bc
{
    typedef unsigned char BYTE;
    typedef unsigned long long bitunit;
    typedef std::vector<bitunit> ADVEC;

	typedef std::vector<bitunit> SetBitVec;

	template<typename T> struct LoadValVec { typedef std::vector<T> type; };
	template<typename T> struct LoadValMap { typedef typename LoadValVec<typename LoadValVec<T>::type >::type type; };


    // Returns the 0-based position of the highest bit set. For example, HiBitPosition<64>==6 and HiBitPosition<63>==5;
    template<unsigned int x>
    struct HiBitPosition
    {
        static const unsigned int Pos = HiBitPosition<(x >> 1)>::Pos + 1;
    };
    template<>
    struct HiBitPosition<1>
    {
        static const unsigned int Pos = 0;
    };

	// Creates a bitunit with all the bits in it set to 1
	template<unsigned int x>
	struct TurnOnAllBits
	{
		static const bitunit Value = (TurnOnAllBits<(x-1)>::Value | ((bitunit)1 << (x-1)));
	};
	template<>
	struct TurnOnAllBits<1>
	{
		static const bitunit Value = (bitunit)1;
	};

    struct mTraits
    {
        enum : size_t
        {
            nByteSizeInBits     = sizeof(BYTE)*8,
            nBytesPerBitUnit    = sizeof(bitunit),
            nBitsPerBitUnit     = nBytesPerBitUnit * 8,
            nBitUnitModuloMask  = nBitsPerBitUnit - 1,  // useful for computing the relative offset within a bitunit from the overall index
            nBitUnitDivShiftBits= HiBitPosition<nBitsPerBitUnit>::Pos, // useful for dividing the overall index for arriving at bitunit index
        };
		enum : bitunit
		{
			nBitUnitAllBitsOn	= TurnOnAllBits<nBitsPerBitUnit>::Value, // useful for setting and unsetting a range of bits
		};
    };

	template<unsigned int size> inline size_t _bitsum(const BYTE* pb) { _ASSERTE("Not Implemented: _bitsum(BYTE*)" == NULL); return -1; }

	template<typename T> inline size_t bitsum(T bu) { return _bitsum<sizeof(T)>((const BYTE*)&bu); }

    namespace Tr // Transform Methods
    {
        struct Null  { template<typename T> inline static T tr(T b) { return  b; } };		  // Null Transform
        struct Invert{ template<typename T> inline static T tr(T b) { return ~b; } };		  // Inverts the given byte/int
        struct And   { template<typename T> inline static T tr(T a, T b) { return a & b; } }; // Boolean And
        struct Or    { template<typename T> inline static T tr(T a, T b) { return a | b; } }; // Boolean Or
    }
    namespace Pr // Predicate Methods
    {
        struct Equal { template<typename T1, typename T2> inline static bool pr(T1 t1, T2 t2) { return t1 == t2; } };
        struct Less  { template<typename T1, typename T2> inline static bool pr(T1 t1, T2 t2) { return t1 < t2;  } };
        struct Great { template<typename T1, typename T2> inline static bool pr(T1 t1, T2 t2) { return t1 > t2;  } };
    }
    namespace Red // Reduction Methods
    {
        struct toBitCount { template<typename T> inline static size_t red(size_t res, T bu) { return res + bitsum(bu); }  };
		struct toMinVal	{ template<typename T> inline static T red(T res, T bu) { return bu < res ? bu : res; } };
		struct toMaxVal	{ template<typename T> inline static T red(T res, T bu) { return bu > res ? bu : res; } };	
		struct toSum	{ template<typename T> inline static T red(T res, T val){ return res + val; } };
    }

#if _USE_TRANSFORM_SEQUENCES ///// Executes a transformation sequence over the given data ///////
    namespace _detail
    {
        template<typename _tBegin, typename _tEnd>
        struct _s
        {
            template<typename outType, typename inType>
            static inline outType _apply(inType data)  { return _s< typename boost::mpl::next< _tBegin >::type, _tEnd>::template _apply<outType>( _tBegin::type::tr(data) ); }

			template<typename outType, typename inType1, typename inType2>
			static inline outType _apply(inType1 in1, inType2 in2) { return _s< typename boost::mpl::next< _tBegin >::type, _tEnd>::template _apply<outType>( _tBegin::type::tr(in1, in2) ); }
        };

        template<typename _tEnd>
        struct _s<_tEnd, _tEnd>
        {
            template<typename outType, typename inType>
            static inline outType _apply(inType data) { return data; }
        };

		template<typename TrSeq, typename outType, typename inType>
		inline outType _exec(inType data) { return _s< typename boost::mpl::begin< TrSeq >::type, typename boost::mpl::end< TrSeq >::type>::template _apply<outType>(data); }

		template<typename TrSeq, typename outType, typename inType1, typename inType2>
		inline outType _exec(inType1 in1, inType2 in2) { return _s< typename boost::mpl::begin< TrSeq >::type, typename boost::mpl::end< TrSeq >::type>::template _apply<outType>(in1, in2); }

		struct _unspecified_type { }; // useful as default type

		// type selection based on condition
		template<typename T> struct isTypeSpecified { enum { Result = true }; };
		template<> struct isTypeSpecified<_unspecified_type> { enum { Result = false }; };

		template<typename T1, typename T2> struct areTypesSame { enum { Result = false }; };
		template<typename T> struct areTypesSame<T, T> { enum { Result = true }; };

		template<typename yesType, typename noType, bool cond=true> struct select {	typedef yesType ResultType;		};
		template<typename yesType, typename noType>	struct select<yesType, noType, false> { typedef noType ResultType; };
		template<typename outType, typename inType>
		struct selectValidType
		{
			typedef typename select<outType, inType, isTypeSpecified<outType>::Result >::ResultType ResultType; // if outType is unspecified, set the inType as outType
		};
		template<typename outType, typename inType1, typename inType2>
		struct selectValidType2
		{
			typedef typename select<outType,
									typename select<inType1, _unspecified_type, areTypesSame<inType1, inType2>::Result >::ResultType,
									isTypeSpecified<outType>::Result
									>::ResultType ResultType; // if outType is unspecified, set the inType1 as outType only if inType1 == inType2; otherwise leave it as unspecified
		};
	}
	namespace Tr // Transform Sequence
	{
		template<typename Seq, typename outType = _detail::_unspecified_type> struct TrSeq
		{
			template<typename inType>
			inline static typename _detail::selectValidType<outType, inType>::ResultType tr(inType b) { return  _detail::_exec<Seq, typename _detail::selectValidType<outType, inType>::ResultType>(b); }

			template<typename inType1, typename inType2>
			inline static typename _detail::selectValidType2<outType, inType1, inType2>::ResultType tr(inType1 in1, inType2 in2) { return  _detail::_exec<Seq, typename _detail::selectValidType2<outType, inType1, inType2>::ResultType>(in1, in2); }
		};
	}
	//// Transformation Sequence Usage Example:
	//
	// #include <boost/mpl/vector.hpp>
	//
	// std::vector<unsigned char> in(256);
	// std::vector<size_t> out(256);
	//
	// bc::gen_seq(in, 0); //<-- generate the sequence data
	//
	// typedef boost::mpl::vector2<bc::Tr::Invert, TrBitSum<size_t> > invertedBitSum; //<-- define the transform sequence
	//
	// transform<Tr::TrSeq<invertedBitSum, size_t> >(in, out); //<-- apply the sequence over the input
	//
#endif // _USE_TRANSFORM_SEQUENCES

	template<typename _Tr, typename TOut, typename _TIn1, typename _TIn2>
	struct TRVEC
	{
		size_t _size;
		struct const_pointer
		{
			typename _TIn1::const_pointer ptr1;
			typename _TIn2::const_pointer ptr2;
			inline TOut				operator*()		{ return _Tr::tr(*ptr1, *ptr2);			}
			inline const_pointer&	operator++()	{ ++ptr1; ++ptr2; return *this;			}
			inline const_pointer	operator++(int)	{ return const_pointer(ptr1++, ptr2++); }	// postfix
			inline const_pointer(const typename _TIn1::const_pointer &p1, const typename _TIn2::const_pointer &p2) : ptr1(p1), ptr2(p2) { }
		} _start ;
	public:
		// typedef typename const_pointer const_pointer;

		inline const_pointer data() const { return _start; }

		inline TRVEC(const _TIn1& in1, const _TIn1& in2) : _start(in1.data(), in2.data()), _size(in1.size()) {	}

		inline size_t size() const { return _size; }
	};
	template<typename _Tr, typename TOut, typename TIn1, typename TIn2>
	inline TRVEC<_Tr, TOut, TIn1, TIn2> trvec(const TIn1& in1, const TIn2& in2) { return TRVEC<_Tr, TOut, TIn1, TIn2>(in1, in2); }

	////////////// Transform Methods //////////

    template<class _Transform, typename _VecIn, typename _VecOut>
    inline void transform(const _VecIn& in, _VecOut& out) // copies transformed content to out
    {
        _ASSERTE(in.size() <= out.size());

		typename _VecOut::pointer	pbOut = out.data();
		typename _VecIn::const_pointer	pbIn  = in.data();
		typename _VecIn::const_pointer const pbInLast = pbIn + in.size();

        while(pbIn != pbInLast)
        {
            *pbOut++ = _Transform::tr(*pbIn++);
        }
    }

	template<class _Transform, typename _vecIn1, typename _vecIn2, typename _vecOut>
	inline void transform(const _vecIn1& in1, const _vecIn2& in2, _vecOut& out)
	{
        _ASSERTE(in1.size() <= in2.size());
		_ASSERTE(in1.size() <= out.size());

		typename _vecOut::pointer	pbOut = out.data();
        typename _vecIn1::const_pointer pbIn1 = in1.data();
        typename _vecIn2::const_pointer pbIn2 = in2.data();
		typename _vecIn1::const_pointer const pbIn1Last = pbIn1 + in1.size();

        while(pbIn1 != pbIn1Last)
        {
            *pbOut++ = _Transform::tr(*pbIn1++, *pbIn2++);
        }
	}

    template<class _Tr1, class _Tr2, class _BinTr, typename _vecT1, typename _vecT2, typename _vecOut>
    inline void zip(const _vecT1& in1, const _vecT2& in2, _vecOut& out) // out = BinTr(Tr1(in1), Tr2(in2))
    {
        _ASSERTE(in1.size() == in2.size() == out.size());

		typename _vecOut::pointer pbOut = out.data();
        typename _vecT1::const_pointer pbIn1 = in1.data();
        typename _vecT2::const_pointer pbIn2 = in2.data();

        for(size_t i=0, nMax = in1.size(); i < nMax; ++i)
        {
            *pbOut++ = _BinTr::tr(_Tr1::tr(*pbIn1++), _Tr2::tr(*pbIn2++));
        }
    }

    template<class _Red, class _Tr, typename Result, typename _vecT>
    inline Result reduce(const _vecT& in, const Result& init = 0)
    {
        Result res(init);
        typename _vecT::const_pointer pbIn = in.data();
        typename _vecT::const_pointer const pbInLast = pbIn + in.size();

        while(pbIn != pbInLast)
            res = _Red::red(res, _Tr::tr(*pbIn++));
        return res;
    }

    template<class _Red, class _Tr1, class _Tr2, typename Result, typename _vecT1, typename _vecT2>
    inline Result reduce(const _vecT1& vec1, const _vecT2& vec2, const Result& init=0)
    {
        _ASSERTE(vec1.size() == vec2.size());

        Result res(init);
        typename _vecT1::const_pointer pbVec1 = vec1.data();
        typename _vecT2::const_pointer pbVec2 = vec2.data();

        for(size_t i=0, nMax = vec1.size(); i < nMax; ++i)
            res = _Red::red(res, _Tr1::tr(*pbVec1++), _Tr2::tr(*pbVec2++));
        return res;
    }

    template<class _Transform, class _Red, typename Result, typename _vecT1, typename _vecT2>
    inline Result transform_reduce(const _vecT1& vec1, const _vecT2& vec2, const Result& init=0)
    {
        Result res(init);
        typename _vecT1::const_pointer pbVec1 = vec1.data();
        typename _vecT2::const_pointer pbVec2 = vec2.data();

        for(size_t i=0, nMax = vec1.size(); i < nMax; ++i)
            res = _Red::red(res, _Transform::tr(*pbVec1++, *pbVec2++));
        return res;
    }

    template<class _Transform, class _Red, typename Result, typename _vecT1, typename _vecT2, typename _vecOut>
    inline Result transform_reduce(const _vecT1& vec1, const _vecT2& vec2, _vecOut& vecOut, const Result& init=0) // also stores the output of transform apart from applying the reduction
    {
        Result res(init);
        typename _vecT1::const_pointer pbVec1 = vec1.data();
        typename _vecT2::const_pointer pbVec2 = vec2.data();
		typename _vecOut::pointer pvecOut = vecOut.data();

        for(size_t i=0, nMax = vec1.size(); i < nMax; ++i)
            res = _Red::red(res, *pvecOut++ = _Transform::tr(*pbVec1++, *pbVec2++));
        return res;
    }

    template<class _Tr1, class _Tr2, class _BinTr, class _Red, typename Result, typename _vecT1, typename _vecT2, typename _vecOut>
    inline Result zip_reduce(const _vecT1& in1, const _vecT2& in2, _vecOut& out, const Result& init=0) // Result = Red(Result, BinTr(Tr1(in1), Tr2(in2)))
    {
        _ASSERTE(in1.size() == in2.size() == out.size());

		Result res(init);
		typename _vecOut::pointer pbOut = out.data();
        typename _vecT2::const_pointer pbIn1 = in1.data();
        typename _vecT1::const_pointer pbIn2 = in2.data();

        for(size_t i=0, nMax = in1.size(); i < nMax; ++i)
            res = _Red::red(res, *pbOut++ = _BinTr::tr(_Tr1::tr(*pbIn1++), _Tr2::tr(*pbIn2++)));
		return res;
    }

    template<class _Tr1, class _Tr2, class _Pred, typename _vecT1, typename _vecT2>
    inline bool comp(const _vecT1& vec1, const _vecT2& vec2) // returns true if _Pred::pr(_Tr1(vec1), _Tr2(vec2)) is true for all
    {
        typename _vecT1::const_pointer pbVec1 = vec1.data();
        typename _vecT2::const_pointer pbVec2 = vec2.data();
        const size_t   nMax = vec1.size();
        volatile size_t i=0;
        while(i < nMax && _Pred::pr(_Tr1::tr(*pbVec1++), _Tr2::tr(*pbVec2++))) ++i;
        return i >= nMax;
    }

    template<class _Tr1, class _Tr2, class _Pred, typename _vecT1, typename _vecT2, typename TOut>
    inline bool comp_set(const _vecT1& vec1, const _vecT2& vec2, TOut& out) // returns true if _Pred::pr(_Tr1(vec1), _Tr2(vec2)) is true for all, with optional value set in out
    {
        typename _vecT1::const_pointer pbVec1 = vec1.data();
        typename _vecT2::const_pointer pbVec2 = vec2.data();
        const size_t   nMax = vec1.size();
        volatile size_t i=0;
        while(i < nMax && _Pred::pr(_Tr1::tr(*pbVec1++), _Tr2::tr(*pbVec2++), out)) ++i;
        return i >= nMax;
    }

	template<typename vecT, typename Result, typename _FindFn>
	inline bool scan(const vecT& in, Result& res, _FindFn _found) // scan the input array till the search condition is met. Returns false if search fails.
	{
		typename vecT::const_pointer pVec = in.data();
		for(size_t i=0, nMax = in.size(); i < nMax; ++i)
		{
			if(_found(res = *pVec++)) { return true; }
		}
		return false;
	}

	template<typename _Red, typename vecT, typename Result, typename _FindFn>
	inline Result scan_reduce(const vecT& in, Result& res, _FindFn _found) // scan the input array till the search condition is met. Returns false if search fails.
	{
		typename vecT::const_pointer pVec = in.data();
		for(size_t i=0, nMax = in.size(); i < nMax; ++i, ++pVec)
		{
			const typename vecT::value_type val = *pVec;
			if(_found(val)) { return res; }
			res = _Red::red(res, val);
		}
		return res;
	}

    typedef signed char BITID;
	inline const BITID* GetOnBitPositions(BYTE b)
	{
        static const BITID OnBitPositions[256][9] = {
		        {-1,-1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [0]
 		        {0, -1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [1]
 		        {1, -1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [2]
 		        {0, 1, -1,-1,-1,-1,-1,-1,-1,}, 	 // [3]
 		        {2, -1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [4]
 		        {0, 2, -1,-1,-1,-1,-1,-1,-1,}, 	 // [5]
 		        {1, 2, -1,-1,-1,-1,-1,-1,-1,}, 	 // [6]
 		        {0, 1, 2, -1,-1,-1,-1,-1,-1,}, 	 // [7]
 		        {3, -1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [8]
 		        {0, 3, -1,-1,-1,-1,-1,-1,-1,}, 	 // [9]
 		        {1, 3, -1,-1,-1,-1,-1,-1,-1,}, 	 // [10]
 		        {0, 1, 3, -1,-1,-1,-1,-1,-1,}, 	 // [11]
 		        {2, 3, -1,-1,-1,-1,-1,-1,-1,}, 	 // [12]
 		        {0, 2, 3, -1,-1,-1,-1,-1,-1,}, 	 // [13]
 		        {1, 2, 3, -1,-1,-1,-1,-1,-1,}, 	 // [14]
 		        {0, 1, 2, 3, -1,-1,-1,-1,-1,}, 	 // [15]
 		        {4, -1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [16]
 		        {0, 4, -1,-1,-1,-1,-1,-1,-1,}, 	 // [17]
 		        {1, 4, -1,-1,-1,-1,-1,-1,-1,}, 	 // [18]
 		        {0, 1, 4, -1,-1,-1,-1,-1,-1,}, 	 // [19]
 		        {2, 4, -1,-1,-1,-1,-1,-1,-1,}, 	 // [20]
 		        {0, 2, 4, -1,-1,-1,-1,-1,-1,}, 	 // [21]
 		        {1, 2, 4, -1,-1,-1,-1,-1,-1,}, 	 // [22]
 		        {0, 1, 2, 4, -1,-1,-1,-1,-1,}, 	 // [23]
 		        {3, 4, -1,-1,-1,-1,-1,-1,-1,}, 	 // [24]
 		        {0, 3, 4, -1,-1,-1,-1,-1,-1,}, 	 // [25]
 		        {1, 3, 4, -1,-1,-1,-1,-1,-1,}, 	 // [26]
 		        {0, 1, 3, 4, -1,-1,-1,-1,-1,}, 	 // [27]
 		        {2, 3, 4, -1,-1,-1,-1,-1,-1,}, 	 // [28]
 		        {0, 2, 3, 4, -1,-1,-1,-1,-1,}, 	 // [29]
 		        {1, 2, 3, 4, -1,-1,-1,-1,-1,}, 	 // [30]
 		        {0, 1, 2, 3, 4, -1,-1,-1,-1,}, 	 // [31]
 		        {5, -1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [32]
 		        {0, 5, -1,-1,-1,-1,-1,-1,-1,}, 	 // [33]
 		        {1, 5, -1,-1,-1,-1,-1,-1,-1,}, 	 // [34]
 		        {0, 1, 5, -1,-1,-1,-1,-1,-1,}, 	 // [35]
 		        {2, 5, -1,-1,-1,-1,-1,-1,-1,}, 	 // [36]
 		        {0, 2, 5, -1,-1,-1,-1,-1,-1,}, 	 // [37]
 		        {1, 2, 5, -1,-1,-1,-1,-1,-1,}, 	 // [38]
 		        {0, 1, 2, 5, -1,-1,-1,-1,-1,}, 	 // [39]
 		        {3, 5, -1,-1,-1,-1,-1,-1,-1,}, 	 // [40]
 		        {0, 3, 5, -1,-1,-1,-1,-1,-1,}, 	 // [41]
 		        {1, 3, 5, -1,-1,-1,-1,-1,-1,}, 	 // [42]
 		        {0, 1, 3, 5, -1,-1,-1,-1,-1,}, 	 // [43]
 		        {2, 3, 5, -1,-1,-1,-1,-1,-1,}, 	 // [44]
 		        {0, 2, 3, 5, -1,-1,-1,-1,-1,}, 	 // [45]
 		        {1, 2, 3, 5, -1,-1,-1,-1,-1,}, 	 // [46]
 		        {0, 1, 2, 3, 5, -1,-1,-1,-1,}, 	 // [47]
 		        {4, 5, -1,-1,-1,-1,-1,-1,-1,}, 	 // [48]
 		        {0, 4, 5, -1,-1,-1,-1,-1,-1,}, 	 // [49]
 		        {1, 4, 5, -1,-1,-1,-1,-1,-1,}, 	 // [50]
 		        {0, 1, 4, 5, -1,-1,-1,-1,-1,}, 	 // [51]
 		        {2, 4, 5, -1,-1,-1,-1,-1,-1,}, 	 // [52]
 		        {0, 2, 4, 5, -1,-1,-1,-1,-1,}, 	 // [53]
 		        {1, 2, 4, 5, -1,-1,-1,-1,-1,}, 	 // [54]
 		        {0, 1, 2, 4, 5, -1,-1,-1,-1,}, 	 // [55]
 		        {3, 4, 5, -1,-1,-1,-1,-1,-1,}, 	 // [56]
 		        {0, 3, 4, 5, -1,-1,-1,-1,-1,}, 	 // [57]
 		        {1, 3, 4, 5, -1,-1,-1,-1,-1,}, 	 // [58]
 		        {0, 1, 3, 4, 5, -1,-1,-1,-1,}, 	 // [59]
 		        {2, 3, 4, 5, -1,-1,-1,-1,-1,}, 	 // [60]
 		        {0, 2, 3, 4, 5, -1,-1,-1,-1,}, 	 // [61]
 		        {1, 2, 3, 4, 5, -1,-1,-1,-1,}, 	 // [62]
 		        {0, 1, 2, 3, 4, 5, -1,-1,-1,}, 	 // [63]
 		        {6, -1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [64]
 		        {0, 6, -1,-1,-1,-1,-1,-1,-1,}, 	 // [65]
 		        {1, 6, -1,-1,-1,-1,-1,-1,-1,}, 	 // [66]
 		        {0, 1, 6, -1,-1,-1,-1,-1,-1,}, 	 // [67]
 		        {2, 6, -1,-1,-1,-1,-1,-1,-1,}, 	 // [68]
 		        {0, 2, 6, -1,-1,-1,-1,-1,-1,}, 	 // [69]
 		        {1, 2, 6, -1,-1,-1,-1,-1,-1,}, 	 // [70]
 		        {0, 1, 2, 6, -1,-1,-1,-1,-1,}, 	 // [71]
 		        {3, 6, -1,-1,-1,-1,-1,-1,-1,}, 	 // [72]
 		        {0, 3, 6, -1,-1,-1,-1,-1,-1,}, 	 // [73]
 		        {1, 3, 6, -1,-1,-1,-1,-1,-1,}, 	 // [74]
 		        {0, 1, 3, 6, -1,-1,-1,-1,-1,}, 	 // [75]
 		        {2, 3, 6, -1,-1,-1,-1,-1,-1,}, 	 // [76]
 		        {0, 2, 3, 6, -1,-1,-1,-1,-1,}, 	 // [77]
 		        {1, 2, 3, 6, -1,-1,-1,-1,-1,}, 	 // [78]
 		        {0, 1, 2, 3, 6, -1,-1,-1,-1,}, 	 // [79]
 		        {4, 6, -1,-1,-1,-1,-1,-1,-1,}, 	 // [80]
 		        {0, 4, 6, -1,-1,-1,-1,-1,-1,}, 	 // [81]
 		        {1, 4, 6, -1,-1,-1,-1,-1,-1,}, 	 // [82]
 		        {0, 1, 4, 6, -1,-1,-1,-1,-1,}, 	 // [83]
 		        {2, 4, 6, -1,-1,-1,-1,-1,-1,}, 	 // [84]
 		        {0, 2, 4, 6, -1,-1,-1,-1,-1,}, 	 // [85]
 		        {1, 2, 4, 6, -1,-1,-1,-1,-1,}, 	 // [86]
 		        {0, 1, 2, 4, 6, -1,-1,-1,-1,}, 	 // [87]
 		        {3, 4, 6, -1,-1,-1,-1,-1,-1,}, 	 // [88]
 		        {0, 3, 4, 6, -1,-1,-1,-1,-1,}, 	 // [89]
 		        {1, 3, 4, 6, -1,-1,-1,-1,-1,}, 	 // [90]
 		        {0, 1, 3, 4, 6, -1,-1,-1,-1,}, 	 // [91]
 		        {2, 3, 4, 6, -1,-1,-1,-1,-1,}, 	 // [92]
 		        {0, 2, 3, 4, 6, -1,-1,-1,-1,}, 	 // [93]
 		        {1, 2, 3, 4, 6, -1,-1,-1,-1,}, 	 // [94]
 		        {0, 1, 2, 3, 4, 6, -1,-1,-1,}, 	 // [95]
 		        {5, 6, -1,-1,-1,-1,-1,-1,-1,}, 	 // [96]
 		        {0, 5, 6, -1,-1,-1,-1,-1,-1,}, 	 // [97]
 		        {1, 5, 6, -1,-1,-1,-1,-1,-1,}, 	 // [98]
 		        {0, 1, 5, 6, -1,-1,-1,-1,-1,}, 	 // [99]
 		        {2, 5, 6, -1,-1,-1,-1,-1,-1,}, 	 // [100]
 		        {0, 2, 5, 6, -1,-1,-1,-1,-1,}, 	 // [101]
 		        {1, 2, 5, 6, -1,-1,-1,-1,-1,}, 	 // [102]
 		        {0, 1, 2, 5, 6, -1,-1,-1,-1,}, 	 // [103]
 		        {3, 5, 6, -1,-1,-1,-1,-1,-1,}, 	 // [104]
 		        {0, 3, 5, 6, -1,-1,-1,-1,-1,}, 	 // [105]
 		        {1, 3, 5, 6, -1,-1,-1,-1,-1,}, 	 // [106]
 		        {0, 1, 3, 5, 6, -1,-1,-1,-1,}, 	 // [107]
 		        {2, 3, 5, 6, -1,-1,-1,-1,-1,}, 	 // [108]
 		        {0, 2, 3, 5, 6, -1,-1,-1,-1,}, 	 // [109]
 		        {1, 2, 3, 5, 6, -1,-1,-1,-1,}, 	 // [110]
 		        {0, 1, 2, 3, 5, 6, -1,-1,-1,}, 	 // [111]
 		        {4, 5, 6, -1,-1,-1,-1,-1,-1,}, 	 // [112]
 		        {0, 4, 5, 6, -1,-1,-1,-1,-1,}, 	 // [113]
 		        {1, 4, 5, 6, -1,-1,-1,-1,-1,}, 	 // [114]
 		        {0, 1, 4, 5, 6, -1,-1,-1,-1,}, 	 // [115]
 		        {2, 4, 5, 6, -1,-1,-1,-1,-1,}, 	 // [116]
 		        {0, 2, 4, 5, 6, -1,-1,-1,-1,}, 	 // [117]
 		        {1, 2, 4, 5, 6, -1,-1,-1,-1,}, 	 // [118]
 		        {0, 1, 2, 4, 5, 6, -1,-1,-1,}, 	 // [119]
 		        {3, 4, 5, 6, -1,-1,-1,-1,-1,}, 	 // [120]
 		        {0, 3, 4, 5, 6, -1,-1,-1,-1,}, 	 // [121]
 		        {1, 3, 4, 5, 6, -1,-1,-1,-1,}, 	 // [122]
 		        {0, 1, 3, 4, 5, 6, -1,-1,-1,}, 	 // [123]
 		        {2, 3, 4, 5, 6, -1,-1,-1,-1,}, 	 // [124]
 		        {0, 2, 3, 4, 5, 6, -1,-1,-1,}, 	 // [125]
 		        {1, 2, 3, 4, 5, 6, -1,-1,-1,}, 	 // [126]
 		        {0, 1, 2, 3, 4, 5, 6, -1,-1,}, 	 // [127]
 		        {7, -1,-1,-1,-1,-1,-1,-1,-1,}, 	 // [128]
 		        {0, 7, -1,-1,-1,-1,-1,-1,-1,}, 	 // [129]
 		        {1, 7, -1,-1,-1,-1,-1,-1,-1,}, 	 // [130]
 		        {0, 1, 7, -1,-1,-1,-1,-1,-1,}, 	 // [131]
 		        {2, 7, -1,-1,-1,-1,-1,-1,-1,}, 	 // [132]
 		        {0, 2, 7, -1,-1,-1,-1,-1,-1,}, 	 // [133]
 		        {1, 2, 7, -1,-1,-1,-1,-1,-1,}, 	 // [134]
 		        {0, 1, 2, 7, -1,-1,-1,-1,-1,}, 	 // [135]
 		        {3, 7, -1,-1,-1,-1,-1,-1,-1,}, 	 // [136]
 		        {0, 3, 7, -1,-1,-1,-1,-1,-1,}, 	 // [137]
 		        {1, 3, 7, -1,-1,-1,-1,-1,-1,}, 	 // [138]
 		        {0, 1, 3, 7, -1,-1,-1,-1,-1,}, 	 // [139]
 		        {2, 3, 7, -1,-1,-1,-1,-1,-1,}, 	 // [140]
 		        {0, 2, 3, 7, -1,-1,-1,-1,-1,}, 	 // [141]
 		        {1, 2, 3, 7, -1,-1,-1,-1,-1,}, 	 // [142]
 		        {0, 1, 2, 3, 7, -1,-1,-1,-1,}, 	 // [143]
 		        {4, 7, -1,-1,-1,-1,-1,-1,-1,}, 	 // [144]
 		        {0, 4, 7, -1,-1,-1,-1,-1,-1,}, 	 // [145]
 		        {1, 4, 7, -1,-1,-1,-1,-1,-1,}, 	 // [146]
 		        {0, 1, 4, 7, -1,-1,-1,-1,-1,}, 	 // [147]
 		        {2, 4, 7, -1,-1,-1,-1,-1,-1,}, 	 // [148]
 		        {0, 2, 4, 7, -1,-1,-1,-1,-1,}, 	 // [149]
 		        {1, 2, 4, 7, -1,-1,-1,-1,-1,}, 	 // [150]
 		        {0, 1, 2, 4, 7, -1,-1,-1,-1,}, 	 // [151]
 		        {3, 4, 7, -1,-1,-1,-1,-1,-1,}, 	 // [152]
 		        {0, 3, 4, 7, -1,-1,-1,-1,-1,}, 	 // [153]
 		        {1, 3, 4, 7, -1,-1,-1,-1,-1,}, 	 // [154]
 		        {0, 1, 3, 4, 7, -1,-1,-1,-1,}, 	 // [155]
 		        {2, 3, 4, 7, -1,-1,-1,-1,-1,}, 	 // [156]
 		        {0, 2, 3, 4, 7, -1,-1,-1,-1,}, 	 // [157]
 		        {1, 2, 3, 4, 7, -1,-1,-1,-1,}, 	 // [158]
 		        {0, 1, 2, 3, 4, 7, -1,-1,-1,}, 	 // [159]
 		        {5, 7, -1,-1,-1,-1,-1,-1,-1,}, 	 // [160]
 		        {0, 5, 7, -1,-1,-1,-1,-1,-1,}, 	 // [161]
 		        {1, 5, 7, -1,-1,-1,-1,-1,-1,}, 	 // [162]
 		        {0, 1, 5, 7, -1,-1,-1,-1,-1,}, 	 // [163]
 		        {2, 5, 7, -1,-1,-1,-1,-1,-1,}, 	 // [164]
 		        {0, 2, 5, 7, -1,-1,-1,-1,-1,}, 	 // [165]
 		        {1, 2, 5, 7, -1,-1,-1,-1,-1,}, 	 // [166]
 		        {0, 1, 2, 5, 7, -1,-1,-1,-1,}, 	 // [167]
 		        {3, 5, 7, -1,-1,-1,-1,-1,-1,}, 	 // [168]
 		        {0, 3, 5, 7, -1,-1,-1,-1,-1,}, 	 // [169]
 		        {1, 3, 5, 7, -1,-1,-1,-1,-1,}, 	 // [170]
 		        {0, 1, 3, 5, 7, -1,-1,-1,-1,}, 	 // [171]
 		        {2, 3, 5, 7, -1,-1,-1,-1,-1,}, 	 // [172]
 		        {0, 2, 3, 5, 7, -1,-1,-1,-1,}, 	 // [173]
 		        {1, 2, 3, 5, 7, -1,-1,-1,-1,}, 	 // [174]
 		        {0, 1, 2, 3, 5, 7, -1,-1,-1,}, 	 // [175]
 		        {4, 5, 7, -1,-1,-1,-1,-1,-1,}, 	 // [176]
 		        {0, 4, 5, 7, -1,-1,-1,-1,-1,}, 	 // [177]
 		        {1, 4, 5, 7, -1,-1,-1,-1,-1,}, 	 // [178]
 		        {0, 1, 4, 5, 7, -1,-1,-1,-1,}, 	 // [179]
 		        {2, 4, 5, 7, -1,-1,-1,-1,-1,}, 	 // [180]
 		        {0, 2, 4, 5, 7, -1,-1,-1,-1,}, 	 // [181]
 		        {1, 2, 4, 5, 7, -1,-1,-1,-1,}, 	 // [182]
 		        {0, 1, 2, 4, 5, 7, -1,-1,-1,}, 	 // [183]
 		        {3, 4, 5, 7, -1,-1,-1,-1,-1,}, 	 // [184]
 		        {0, 3, 4, 5, 7, -1,-1,-1,-1,}, 	 // [185]
 		        {1, 3, 4, 5, 7, -1,-1,-1,-1,}, 	 // [186]
 		        {0, 1, 3, 4, 5, 7, -1,-1,-1,}, 	 // [187]
 		        {2, 3, 4, 5, 7, -1,-1,-1,-1,}, 	 // [188]
 		        {0, 2, 3, 4, 5, 7, -1,-1,-1,}, 	 // [189]
 		        {1, 2, 3, 4, 5, 7, -1,-1,-1,}, 	 // [190]
 		        {0, 1, 2, 3, 4, 5, 7, -1,-1,}, 	 // [191]
 		        {6, 7, -1,-1,-1,-1,-1,-1,-1,}, 	 // [192]
 		        {0, 6, 7, -1,-1,-1,-1,-1,-1,}, 	 // [193]
 		        {1, 6, 7, -1,-1,-1,-1,-1,-1,}, 	 // [194]
 		        {0, 1, 6, 7, -1,-1,-1,-1,-1,}, 	 // [195]
 		        {2, 6, 7, -1,-1,-1,-1,-1,-1,}, 	 // [196]
 		        {0, 2, 6, 7, -1,-1,-1,-1,-1,}, 	 // [197]
 		        {1, 2, 6, 7, -1,-1,-1,-1,-1,}, 	 // [198]
 		        {0, 1, 2, 6, 7, -1,-1,-1,-1,}, 	 // [199]
 		        {3, 6, 7, -1,-1,-1,-1,-1,-1,}, 	 // [200]
 		        {0, 3, 6, 7, -1,-1,-1,-1,-1,}, 	 // [201]
 		        {1, 3, 6, 7, -1,-1,-1,-1,-1,}, 	 // [202]
 		        {0, 1, 3, 6, 7, -1,-1,-1,-1,}, 	 // [203]
 		        {2, 3, 6, 7, -1,-1,-1,-1,-1,}, 	 // [204]
 		        {0, 2, 3, 6, 7, -1,-1,-1,-1,}, 	 // [205]
 		        {1, 2, 3, 6, 7, -1,-1,-1,-1,}, 	 // [206]
 		        {0, 1, 2, 3, 6, 7, -1,-1,-1,}, 	 // [207]
 		        {4, 6, 7, -1,-1,-1,-1,-1,-1,}, 	 // [208]
 		        {0, 4, 6, 7, -1,-1,-1,-1,-1,}, 	 // [209]
 		        {1, 4, 6, 7, -1,-1,-1,-1,-1,}, 	 // [210]
 		        {0, 1, 4, 6, 7, -1,-1,-1,-1,}, 	 // [211]
 		        {2, 4, 6, 7, -1,-1,-1,-1,-1,}, 	 // [212]
 		        {0, 2, 4, 6, 7, -1,-1,-1,-1,}, 	 // [213]
 		        {1, 2, 4, 6, 7, -1,-1,-1,-1,}, 	 // [214]
 		        {0, 1, 2, 4, 6, 7, -1,-1,-1,}, 	 // [215]
 		        {3, 4, 6, 7, -1,-1,-1,-1,-1,}, 	 // [216]
 		        {0, 3, 4, 6, 7, -1,-1,-1,-1,}, 	 // [217]
 		        {1, 3, 4, 6, 7, -1,-1,-1,-1,}, 	 // [218]
 		        {0, 1, 3, 4, 6, 7, -1,-1,-1,}, 	 // [219]
 		        {2, 3, 4, 6, 7, -1,-1,-1,-1,}, 	 // [220]
 		        {0, 2, 3, 4, 6, 7, -1,-1,-1,}, 	 // [221]
 		        {1, 2, 3, 4, 6, 7, -1,-1,-1,}, 	 // [222]
 		        {0, 1, 2, 3, 4, 6, 7, -1,-1,}, 	 // [223]
 		        {5, 6, 7, -1,-1,-1,-1,-1,-1,}, 	 // [224]
 		        {0, 5, 6, 7, -1,-1,-1,-1,-1,}, 	 // [225]
 		        {1, 5, 6, 7, -1,-1,-1,-1,-1,}, 	 // [226]
 		        {0, 1, 5, 6, 7, -1,-1,-1,-1,}, 	 // [227]
 		        {2, 5, 6, 7, -1,-1,-1,-1,-1,}, 	 // [228]
 		        {0, 2, 5, 6, 7, -1,-1,-1,-1,}, 	 // [229]
 		        {1, 2, 5, 6, 7, -1,-1,-1,-1,}, 	 // [230]
 		        {0, 1, 2, 5, 6, 7, -1,-1,-1,}, 	 // [231]
 		        {3, 5, 6, 7, -1,-1,-1,-1,-1,}, 	 // [232]
 		        {0, 3, 5, 6, 7, -1,-1,-1,-1,}, 	 // [233]
 		        {1, 3, 5, 6, 7, -1,-1,-1,-1,}, 	 // [234]
 		        {0, 1, 3, 5, 6, 7, -1,-1,-1,}, 	 // [235]
 		        {2, 3, 5, 6, 7, -1,-1,-1,-1,}, 	 // [236]
 		        {0, 2, 3, 5, 6, 7, -1,-1,-1,}, 	 // [237]
 		        {1, 2, 3, 5, 6, 7, -1,-1,-1,}, 	 // [238]
 		        {0, 1, 2, 3, 5, 6, 7, -1,-1,}, 	 // [239]
 		        {4, 5, 6, 7, -1,-1,-1,-1,-1,}, 	 // [240]
 		        {0, 4, 5, 6, 7, -1,-1,-1,-1,}, 	 // [241]
 		        {1, 4, 5, 6, 7, -1,-1,-1,-1,}, 	 // [242]
 		        {0, 1, 4, 5, 6, 7, -1,-1,-1,}, 	 // [243]
 		        {2, 4, 5, 6, 7, -1,-1,-1,-1,}, 	 // [244]
 		        {0, 2, 4, 5, 6, 7, -1,-1,-1,}, 	 // [245]
 		        {1, 2, 4, 5, 6, 7, -1,-1,-1,}, 	 // [246]
 		        {0, 1, 2, 4, 5, 6, 7, -1,-1,}, 	 // [247]
 		        {3, 4, 5, 6, 7, -1,-1,-1,-1,}, 	 // [248]
 		        {0, 3, 4, 5, 6, 7, -1,-1,-1,}, 	 // [249]
 		        {1, 3, 4, 5, 6, 7, -1,-1,-1,}, 	 // [250]
 		        {0, 1, 3, 4, 5, 6, 7, -1,-1,}, 	 // [251]
 		        {2, 3, 4, 5, 6, 7, -1,-1,-1,}, 	 // [252]
 		        {0, 2, 3, 4, 5, 6, 7, -1,-1,}, 	 // [253]
 		        {1, 2, 3, 4, 5, 6, 7, -1,-1,}, 	 // [254]
 		        {0, 1, 2, 3, 4, 5, 6, 7, -1,}, 	 // [255]
 		        };
		return OnBitPositions[b];
	}

    template<class _Transform, class _Fn>
    inline void for_each_bit_uncond(const ADVEC& advec, _Fn _op) // applies _Fn for each ON bit thats a result of _Transform
    {
        const bitunit* pBitUnit = &advec[0];
        const BYTE* pByte = (BYTE*)pBitUnit;
        for(size_t i=0, nBitOffset=0, nByteCount = mTraits::nBytesPerBitUnit * advec.size(); i < nByteCount; ++i, ++pByte, nBitOffset += mTraits::nByteSizeInBits)
        {
            const BITID* pBitID = GetOnBitPositions(_Transform::tr(*pByte));
            while(*pBitID >= 0)
            {
                _op((size_t)*pBitID + nBitOffset);
                ++pBitID;
            }
        }
    }

    template<class _Transform, class _Fn>
    inline void for_each_bit_uncond(const ADVEC& advec1, const ADVEC& advec2, _Fn _op) // applies _Fn for each ON bit thats a result of _Transform
    {
		_ASSERTE(advec1.size() <= advec2.size());
        const bitunit* pBitUnit1 = &advec1[0]; const BYTE* pByte1 = (BYTE*)pBitUnit1;
		const bitunit* pBitUnit2 = &advec2[0]; const BYTE* pByte2 = (BYTE*)pBitUnit2;
        for(size_t i=0, nBitOffset=0, nByteCount = mTraits::nBytesPerBitUnit * advec1.size(); i < nByteCount; ++i, ++pByte1, ++pByte2, nBitOffset += mTraits::nByteSizeInBits)
        {
            const BITID* pBitID = GetOnBitPositions(_Transform::tr(*pByte1, *pByte2));
            while(*pBitID >= 0)
            {
                _op((size_t)*pBitID + nBitOffset);
                ++pBitID;
            }
        }
    }

	template<class _Transform, class _Fn>
	inline bool for_each_setbit_while(const ADVEC& advec, _Fn _op) // applies _Fn for each ON bit while the _op returns true; 
	{
        const bitunit* pBitUnit = &advec[0];
        const BYTE* pByte = (BYTE*)pBitUnit;
        for(size_t i=0, nBitOffset=0, nByteCount = mTraits::nBytesPerBitUnit * advec.size(); i < nByteCount; ++i, ++pByte, nBitOffset += mTraits::nByteSizeInBits)
        {
            const BITID* pBitID = GetOnBitPositions(_Transform::tr(*pByte));
            while(*pBitID >= 0)
            {
                if(_op((size_t)*pBitID + nBitOffset) == false) return false;
                ++pBitID;
            }
        }
		return true;
	}

    template<typename vecType, class _Fn>
    inline void for_each_unit(vecType& advec, _Fn _op)
    {
        typename vecType::value_type* pbu = &advec[0];
        for(size_t i=0, nMax = advec.size(); i < nMax; ++i)
            _op(*pbu++);
    }

	template<typename _vecIn, typename _vecOut, class _Fn>
	inline void for_each_unit_output(const _vecIn& in, _vecOut& out, _Fn _op)
	{
			typename _vecOut::value_type* pbuOut = &out[0];
		const typename _vecIn::value_type* pbuIn = &in[0];
		for(size_t i=0, nMax=in.size(); i < nMax; ++i)
			*pbuOut++ = _op(*pbuIn++);
	}

	template<typename TLoadValVec, class _Fn>
	inline void for_each_pair(const TLoadValVec& vec, _Fn _op) // invoke _Fn on each pair of values in vec.
	{
		const typename TLoadValVec::value_type* pVali = &vec[0];
		for(size_t i=0, nMax = vec.size(), iMax = nMax-1; i < iMax; ++i, ++pVali)
		{
			const typename TLoadValVec::value_type* pValj = pVali+1;
			for(size_t j=i+1; j < nMax; ++j)
				_op(*pVali, *pValj++);
		}
	}

	template<typename TLoadValVec, typename TLoadValMap, class _Fn>
	inline void for_each_pair_output(const TLoadValVec& vecIn, TLoadValMap& mapOut ,_Fn _op) // invoke _Fn on each pair of values in vec. and output it to mapOut
	{
		const typename TLoadValVec::value_type* pVali = &vecIn[0];
		for(size_t i=0, nMax = vecIn.size(), iMax = nMax-1; i < iMax; ++i, ++pVali)
		{
			typename TLoadValMap::value_type::value_type* pOut  = &mapOut[i][i+1];
			const typename TLoadValVec::value_type* pValj = pVali+1;
			for(size_t j=i+1; j < nMax; ++j)
				*pOut++ = _op(*pVali, *pValj++);
		}
	}

	template<typename TLoadValVec1, typename TLoadValVec2, class _Fn>
	inline void for_each_pair(const TLoadValVec1& vec1, const TLoadValVec2& vec2, _Fn _op) // invoke _Fn for each pair of values from (vec1 x vec2)
	{
		const typename TLoadValVec1::value_type* pVal1 = &vec1[0];
		for(size_t i=0, iMax = vec1.size(); i < iMax; ++i, ++pVal1)
		{
			const typename TLoadValVec2::value_type* pVal2 = &vec2[0];
			for(size_t j=0, jMax = vec2.size(); j < jMax; ++j)
				_op(*pVal1, *pVal2++);
		}
	}

    ///////////////////// utility wrappers //////////////////////////
    template<typename _vecIn, typename _vecOut>
    inline void make_copy(const _vecIn& in, _vecOut& out) // copies as-is.
    {
        transform<Tr::Null>(in, out);
    }

    template<typename _vecIn, typename _vecOut>
    inline void make_inverted_copy(const _vecIn& in, _vecOut& out) // copies inverted content to out
    {
        transform<Tr::Invert>(in, out);
    }

    template<typename _vecT>
    inline void invert(_vecT& in) // inverts the content in-place
    {
        make_inverted_copy(in, in);
    }

    template<typename _vecT1, typename _vecT2>
    bool is_equal(const _vecT1& vec1, const _vecT2& vec2)
    {
        return comp<Tr::Null, Tr::Null, Pr::Equal>(vec1, vec2);
    }

    template<class _Fn, typename _vecT>
    inline void for_each_setbit(const _vecT& advec, _Fn _op)
    {
       for_each_bit_uncond<Tr::Null>(advec, _op);
    }

    template<class _Fn, typename _vecT>
    inline void for_each_unsetbit(const _vecT& advec, _Fn _op)
    {
        for_each_bit_uncond<Tr::Invert>(advec, _op);
    }

    // Sets the bit to true at the given 0-based nBitPos index in the advec
    inline void setbit(ADVEC& advec, size_t nBitPos)
    {
        const size_t nBitUnitIndex  = nBitPos >> mTraits::nBitUnitDivShiftBits; // nBitUnitIndex = nBitPos/nBitsPerBitUnit;
        const size_t nBitOffset     = nBitPos & mTraits::nBitUnitModuloMask;    // nBitOffset = nBitPos % nBitsPerBitUnit;

        _ASSERTE((nBitPos % mTraits::nBitsPerBitUnit) == nBitOffset);
        _ASSERTE((nBitPos / mTraits::nBitsPerBitUnit) == nBitUnitIndex);

        advec[nBitUnitIndex] |= ((bitunit)1 << nBitOffset);
    }

    // Sets the bit to false at the given 0-based nBitPos index in the advec
    inline void unsetbit(ADVEC& advec, size_t nBitPos)
    {
        const size_t nBitUnitIndex  = nBitPos >> mTraits::nBitUnitDivShiftBits; // nBitUnitIndex = nBitPos/nBitsPerBitUnit;
        const size_t nBitOffset     = nBitPos & mTraits::nBitUnitModuloMask;    // nBitOffset = nBitPos % nBitsPerBitUnit;

        _ASSERTE((nBitPos % mTraits::nBitsPerBitUnit) == nBitOffset);
        _ASSERTE((nBitPos / mTraits::nBitsPerBitUnit) == nBitUnitIndex);

        advec[nBitUnitIndex] &= ~((bitunit)1 << nBitOffset);
    }

	inline void setbits(ADVEC& advec, size_t nBitPosStart, size_t nBitPosEnd) // Sets all bits in the range [nBitPosStart, nBitPosEnd] to 1
	{
        const size_t nBitUnitIndexEnd = nBitPosEnd  >> mTraits::nBitUnitDivShiftBits;
        const size_t nBitOffsetEnd    = nBitPosEnd   & mTraits::nBitUnitModuloMask;
        const size_t nBitOffset		= nBitPosStart & mTraits::nBitUnitModuloMask;
        size_t nBitUnitIndex	= nBitPosStart >> mTraits::nBitUnitDivShiftBits;

		if(nBitUnitIndex == nBitUnitIndexEnd)
		{
			advec[nBitUnitIndex] |= (((bitunit)mTraits::nBitUnitAllBitsOn << nBitOffset) & ((bitunit)mTraits::nBitUnitAllBitsOn >> (bitunit)(mTraits::nBitsPerBitUnit-(nBitOffsetEnd+1))));
			return;
		}

        advec[nBitUnitIndex] |= ((bitunit)mTraits::nBitUnitAllBitsOn << nBitOffset);

		while(++nBitUnitIndex < nBitUnitIndexEnd)
			advec[nBitUnitIndex] |= ((bitunit)mTraits::nBitUnitAllBitsOn);

		advec[nBitUnitIndex] |= ((bitunit)mTraits::nBitUnitAllBitsOn >> (bitunit)(mTraits::nBitsPerBitUnit-(nBitOffsetEnd+1)));
	}

	inline void unsetbits(ADVEC& advec, size_t nBitPosStart, size_t nBitPosEnd) // Sets all bits in the range [nBitPosStart, nBitPosEnd] to 0
	{
        const size_t nBitUnitIndexEnd  = nBitPosEnd >> mTraits::nBitUnitDivShiftBits;
        const size_t nBitOffsetEnd     = nBitPosEnd & mTraits::nBitUnitModuloMask;
        const size_t nBitOffset     = nBitPosStart & mTraits::nBitUnitModuloMask;
		size_t nBitUnitIndex  = nBitPosStart >> mTraits::nBitUnitDivShiftBits;

		if(nBitUnitIndex == nBitUnitIndexEnd)
		{
			advec[nBitUnitIndex] &= ~(((bitunit)mTraits::nBitUnitAllBitsOn << nBitOffset) & ((bitunit)mTraits::nBitUnitAllBitsOn >> (bitunit)(mTraits::nBitsPerBitUnit-(nBitOffsetEnd+1))));
			return;
		}

		advec[nBitUnitIndex] &= ~((bitunit)mTraits::nBitUnitAllBitsOn << nBitOffset);

		while(++nBitUnitIndex < nBitUnitIndexEnd)
			advec[nBitUnitIndex] &= ~((bitunit)mTraits::nBitUnitAllBitsOn);

		advec[nBitUnitIndex] &= ~((bitunit)mTraits::nBitUnitAllBitsOn >> (bitunit)(mTraits::nBitsPerBitUnit-(nBitOffsetEnd+1)));
	}

	inline void unsetbits(ADVEC& advec, size_t nBitPosStart) // Sets all bits in the advec to 0 starting from position nBitPosStart
	{
		unsetbits(advec, nBitPosStart, (advec.size() * mTraits::nBitsPerBitUnit) - 1);
	}

	inline void setallbits(ADVEC& advec)
	{
		bitunit* pbu = &advec[0];
		const bitunit* const pbuLast = pbu + advec.size();
		while(pbu != pbuLast) *pbu++ = (bitunit) mTraits::nBitUnitAllBitsOn;
	}

	inline void unsetallbits(ADVEC& advec)
	{
		bitunit* pbu = &advec[0];
		const bitunit* const pbuLast = pbu + advec.size();
		while(pbu != pbuLast) *pbu++ = (bitunit) 0;
	}

    inline bool isbitset(const ADVEC& advec, size_t nBitPos)
    {
        const size_t nBitUnitIndex  = nBitPos >> mTraits::nBitUnitDivShiftBits; // nBitUnitIndex = nBitPos/nBitsPerBitUnit;
        const size_t nBitOffset     = nBitPos & mTraits::nBitUnitModuloMask;    // nBitOffset = nBitPos % nBitsPerBitUnit;

        _ASSERTE((nBitPos % mTraits::nBitsPerBitUnit) == nBitOffset);
        _ASSERTE((nBitPos / mTraits::nBitsPerBitUnit) == nBitUnitIndex);

        return (advec[nBitUnitIndex] & ((bitunit)1 << nBitOffset)) != 0 ;
    }

	inline size_t getfirstsetbit(const ADVEC& advec)
	{
        const bitunit* pBitUnit = &advec[0];
		
		size_t nBitOffset=0, byteIndex =0;

		while(*pBitUnit == 0)
		{
			++pBitUnit, nBitOffset += mTraits::nBitsPerBitUnit;
#if _DEBUG			
			_ASSERTE(byteIndex < mTraits::nBytesPerBitUnit * advec.size()); // if this assertion has hit, then the input vector has no set bits !! Its empty
			byteIndex += mTraits::nBytesPerBitUnit;
#endif
		}
        
		const BYTE* pByte = (BYTE*)pBitUnit;
		while(*pByte == 0) { ++pByte; nBitOffset += mTraits::nByteSizeInBits; }

        const BITID* pBitID = GetOnBitPositions(*pByte); _ASSERTE(isbitset(advec, *pBitID + nBitOffset) == true);
		return *pBitID + nBitOffset;
	}

    /////////// bit count utilities /////////////////////

    inline size_t bitsum(BYTE b) { static const char *const _Bitsperbyte =
		                                        "\0\1\1\2\1\2\2\3\1\2\2\3\2\3\3\4"
		                                        "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
		                                        "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
		                                        "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		                                        "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
		                                        "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		                                        "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		                                        "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
		                                        "\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
		                                        "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		                                        "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		                                        "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
		                                        "\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		                                        "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
		                                        "\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
		                                        "\4\5\5\6\5\6\6\7\5\6\6\7\6\7\7\x8";  return _Bitsperbyte[b]; }
    template<>
    inline size_t _bitsum<1>(const BYTE* pb) { return bitsum(pb[0]); }
    template<>
    inline size_t _bitsum<2>(const BYTE* pb) { return bitsum(pb[0]) + bitsum(pb[1]); }
    template<>
    inline size_t _bitsum<4>(const BYTE* pb) { return bitsum(pb[0]) + bitsum(pb[1]) + bitsum(pb[2]) + bitsum(pb[3]); }
    template<>
    inline size_t _bitsum<8>(const BYTE* pb) { return bitsum(pb[0]) + bitsum(pb[1]) + bitsum(pb[2]) + bitsum(pb[3]) + bitsum(pb[4]) + bitsum(pb[5]) + bitsum(pb[6]) + bitsum(pb[7]); }
    template<>
    inline size_t _bitsum<16>(const BYTE* pb){ return bitsum(pb[0]) + bitsum(pb[1]) + bitsum(pb[2]) + bitsum(pb[3]) + bitsum(pb[4]) + bitsum(pb[5]) + bitsum(pb[6]) + bitsum(pb[7]) + bitsum(pb[8]) + bitsum(pb[9]) + bitsum(pb[10]) + bitsum(pb[11]) + bitsum(pb[12]) + bitsum(pb[13]) + bitsum(pb[14]) + bitsum(pb[15]); }

	template<typename T>
    inline size_t bitsum(const std::vector<T>& advec)
    {
        return reduce<Red::toBitCount, Tr::Null> (advec, (size_t)0);
    }

    //////////// Sequence generator utilities //////////////////////

    template<typename T>
    inline ADVEC gen_seq(T first, T end) // generates sequence of values in the range [first, end)
    {
        ADVEC seq(end - first);
        bitunit* pbu = &seq[0];
        while(first < end)
            *pbu++ = first ++;
        return seq;
    }
    template<typename _vecType, typename T>
    inline void gen_seq(_vecType& vec, T first, T step=1) // generates sequence of values in the range [first, end)
    {		
        typename _vecType::iterator iter = vec.begin();
		typename _vecType::iterator iterEnd = vec.end();
        for(; iter != iterEnd; ++iter, first += step)
        {
            *iter = first;
        }
    }

	////////////// Debug Helpers /////////////////////////////////////
#if _DEBUG
#include <string>
#include <stdlib.h>
	inline std::string toString(const SetBitVec& vec)
	{
		char numBuf[mTraits::nBitsPerBitUnit+1];
		std::string str("{");
		for_each_setbit(vec, [&](size_t id) {  sprintf(numBuf, "%llu,", (uintptr_t)id); str += numBuf; } );
		str += "}";
		return str;
	}
	template<typename T>
	inline std::string toString(const T& obj)
	{
		char numBuf[mTraits::nBitsPerBitUnit+1];
		std::string str("{");
		std::for_each(obj.cbegin(), obj.cend(), [&](T::value_type val) { sprintf(numBuf, "%llu,", (uintptr_t)val); str += numBuf; });
		str += "}";
		return str;
	}
#endif

} // namespace bc

/////////////////////////////////////////////////////////////
//
/** -------- OnBitPositions Generator code -------------
 **
    void PrintOnBits(unsigned char ch)
    {
        printf("{");
        int nSetCount=0;
        for(int i=0; i < 8; ++i, ch = (ch >> 1))
            if(ch & 1) { printf("%d, ", i); nSetCount++; }
        while(nSetCount++ < 9) printf("-1,");
        printf("}");
    }
    main()
    {
        printf("int OnBits[256][9] = {\n\t\t");
        for(int i=0; i < 256; ++i)
        {
            PrintOnBits((unsigned char)i); printf(", \t // [%d] \n \t\t",i);
        }
        printf("};");
    }
 **
 **/
/////////////////////////////////////////////////////////////

#endif // __bitcache_h_11A6BC64_8CC3_447B_AD83_56BCF4FF0083__
