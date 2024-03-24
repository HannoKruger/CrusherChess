#pragma once
#include <cassert>
#include "typedefs.h"

//psuedo random number state
//unsigned int random_state = 1804289383;


//static  unsigned int randu32();
//
//static  U64 randu64();
//
//U64 generate_magic_number();


class PRNG {
    U64 s;

    U64 rand64() {
        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }

public:
    explicit PRNG(U64 seed) :
        s(seed) {
        assert(seed);
    }

    template<typename T>
    T rand() {
        return T(rand64());
    }

    // Special generator used to fast init magic numbers.
    // Output values only have 1/8th of their bits set on average.
    template<typename T>
    T sparse_rand() {
        return T(rand64() & rand64() & rand64());
    }
};

//U64 generate_magic_number()
//{
//	return randu64() & randu64() & randu64();
//}


//generate 32bit psuedo legal moves
//constexpr unsigned int randu32()
//{
//	//fast method
//	//seed = (214013U * seed + 2531011U);
//	//return (seed >> 16) & 0x7FFFU;
//
//	random_state ^= random_state << 13;
//	random_state ^= random_state >> 17;
//	random_state ^= random_state << 5;
//
//	return random_state;
//}

//constexpr  U64 randu64()
//{
//	U64 n1, n2, n3, n4;
//
//	//only keep top 16 bits
//	n1 = (U64)randu32() & 0xFFFF;
//	n2 = (U64)randu32() & 0xFFFF;
//	n3 = (U64)randu32() & 0xFFFF;
//	n4 = (U64)randu32() & 0xFFFF;
//
//	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
//}
