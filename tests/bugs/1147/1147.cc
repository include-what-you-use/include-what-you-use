// IWYU_XFAIL

#include "BaseClass.h"

template <typename TheType, bool Size = sizeof(TheType)>
struct TGetCastType2
{
	static inline constexpr bool SizeValue = Size;
};

void myfunc()
{
	auto v = TGetCastType2<BaseClass>::SizeValue;
	(void)v;
}

/**** IWYU_SUMMARY

(tests/bugs/1147/1147.cc has correct #includes/fwd-decls)

***** IWYU_SUMMARY */
