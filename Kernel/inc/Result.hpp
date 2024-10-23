#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	enum class ErrorCode : U8
	{
		Success = 0,
		NotEnoughMemoryPages,
		SerialOutputUnavailabe,
	};

	struct OkType
	{
	};
	struct ErrType
	{
	};

	inline constexpr OkType OK;
	inline constexpr ErrType ERR;

	/// This type will very likely change in the future, possibly being replced by something completely different.
	template <typename T> class [[nodiscard]] Result
	{
	public:
		Result(const Result<T>& other)					  = delete;
		auto operator=(const Result<T>& other) -> Result& = delete;

		Result(Result<T>&& other) noexcept;
		auto operator=(Result<T>&& other) noexcept -> Result&;

		~Result();

		template <typename... Args> static auto MakeOk(Args&&... args) -> Result<T>;
		static auto MakeErr(ErrorCode e) -> Result<T>;

		[[nodiscard]] auto IsError() const noexcept -> bool;

	private:
		/// Constructor for success case, moves.
		explicit Result(OkType, T&& v);
		/// Constructor for error case.
		explicit Result(ErrType, ErrorCode e);

	public:
		union
		{
			ErrorCode Error;
			T Value;
		};

	private:
		bool m_isOk;
	};

	/// This type will very likely change in the future, possibly being replced by something completely different.
	/// This is a `void` specialisation for the Result type.
	template <> class [[nodiscard]] Result<void>
	{
	public:
		Result(const Result<void>& other)					 = delete;
		auto operator=(const Result<void>& other) -> Result& = delete;

		Result(Result<void>&& other) noexcept = default;
		auto operator=(Result<void>&& other) noexcept -> Result& = default;

		~Result() = default;

		static auto MakeOk() -> Result<void>;

		static auto MakeErr(ErrorCode e) -> Result<void>;

		[[nodiscard]] auto IsError() const noexcept -> bool;

	private:
		/// Constructor for success case.
		explicit Result(OkType);
		/// Constructor for error case.
		explicit Result(ErrType, ErrorCode e);

	public:
		ErrorCode Error;
	};
}
