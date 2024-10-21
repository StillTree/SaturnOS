#pragma once

// This header contains all the stuff stolen from the C++'s standard library

namespace SaturnKernel
{
	template <typename T> struct RemoveReference
	{
		using type = T;
	};

	template <typename T> struct RemoveReference<T&>
	{
		using type = T;
	};

	template <typename T> struct RemoveReference<T&&>
	{
		using type = T;
	};

	template <typename T> [[nodiscard]] constexpr auto Forward(typename RemoveReference<T>::type&& t) noexcept -> T&&
	{
		return static_cast<T&&>(t);
	}

	template <typename T> [[nodiscard]] constexpr auto Forward(typename RemoveReference<T>::type& t) noexcept -> T&&
	{
		return static_cast<T&&>(t);
	}

	template <typename T> [[nodiscard]] constexpr auto Move(T&& t) noexcept -> typename RemoveReference<T>::type&&
	{
		return static_cast<typename RemoveReference<T>::type&&>(t);
	}
}
