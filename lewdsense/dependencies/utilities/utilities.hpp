#pragma once
#include <cstdint>
#include "../../source-sdk/math/vector3d.hpp"
#define M_PI 3.14159265358979323846

using ServerRankRevealAll2 = char(__cdecl*)(int*);
namespace utilities {
	template<unsigned int IIdx, typename TRet, typename ... TArgs>
	static auto callvfunc(void* thisptr, TArgs ... argList) -> TRet
	{
		using Fn = TRet(__thiscall*)(void*, decltype(argList)...);
		return (*static_cast<Fn**>(thisptr))[IIdx](thisptr, argList...);
	}
	template<typename FuncType>
	__forceinline static FuncType call_virtual(void* ppClass, int index) {
		int* pVTable = *(int**)ppClass;
		int dwAddress = pVTable[index];
		return (FuncType)(dwAddress);
	}
	namespace math {
		template <typename t> t clamp_value(t value, t min, t max) {
			if (value > max) {
				return max;
			}
			if (value < min) {
				return min;
			}
			return value;
		}
	}
	extern ServerRankRevealAll2 ServerRankRevealAllEx;
	std::uint8_t* pattern_scan(void* module, const char* signature);
	extern void ServerRankRevealAll();
}