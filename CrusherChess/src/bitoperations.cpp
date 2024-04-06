#include <cassert>
#include <bit>
#include "bitoperations.h"

/****************************************\
 ========================================
			  Bit operations
 ========================================
\****************************************/
#pragma region



//count bitboard bits
 int count_bitsv1(U64 num)//slow
{
	int cnt = 0;

	while (num)
	{
		num &= num - 1;
		++cnt;
	}
	return cnt;
}
 int count_bits(U64 num)//fast
{
	return std::popcount(num);
}


 int get_lsb_index(U64 num)
{
	//-num = (0 - num) = (~num + 1ULL) -> flip bits & add one

	assert(num != 0ULL);

	return count_bits((num & (0 - num)) - 1);
}
#pragma endregion