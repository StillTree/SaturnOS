#pragma once

#include "Core.hpp"
#include "STD.hpp"

namespace SaturnKernel {

enum class ErrorCode : u8 {
	Success = 0,
	NotEnoughMemoryPages,
	NotEnoughMemoryFrames,
	SerialOutputUnavailabe,
	OutOfMemory,
	FrameAlreadyDeallocated,
	PageAlreadyMapped,
	PageAlreadyUnmapped,
	HeapBlockTooSmall,
	HeapBlockIncorrectAlignment,
	HeapBlockIncorrectSplitSize,
	InvalidSDTSignature,
	XSDTCorrupted,
	X2APICUnsupported,
	IOAPICNotPresent,
	InvalidBARIndex,
	InvalidMSIXVector,
	PCICapabilitiesNotSupported,
};

struct OkType { };
struct ErrType { };

inline constexpr OkType OK;
inline constexpr ErrType ERR;

/// This type will very likely change in the future, possibly being replced by something completely different.
template <typename T> class [[nodiscard]] Result {
public:
	Result(const Result<T>& other) = delete;
	auto operator=(const Result<T>& other) -> Result& = delete;

	Result(Result<T>&& other) noexcept
		: m_isOk(other.m_isOk)
	{
		if (other.m_isOk)
			Value = T(Move(other.Value));
		else
			Error = other.Error;
	}

	auto operator=(Result<T>&& other) noexcept -> Result&
	{
		if (m_isOk)
			Value.~T();

		m_isOk = other.m_isOk;
		if (m_isOk)
			Value = T(Move(other.Value));
		else
			Error = other.Error;

		return *this;
	}

	~Result()
	{
		if (m_isOk)
			Value.~T();
	}

	template <typename... Args> static auto MakeOk(Args&&... args) -> Result<T> { return Result(OK, T(Forward<Args>(args)...)); }

	static auto MakeErr(ErrorCode e) -> Result<T> { return Result(ERR, e); }

	[[nodiscard]] auto IsError() const noexcept -> bool { return !m_isOk; }

	[[nodiscard]] auto IsOk() const noexcept -> bool { return m_isOk; }

private:
	/// Constructor for success case, moves.
	Result(OkType /*unused*/, T&& v)
		: Value(Move(v))
		, m_isOk(true)
	{
	}

	/// Constructor for error case.
	Result(ErrType /*unused*/, ErrorCode e)
		: Error(e)
		, m_isOk(false)
	{
	}

public:
	union {
		ErrorCode Error;
		T Value;
	};

private:
	bool m_isOk;
};

/// This type will very likely change in the future, possibly being replced by something completely different.
/// This is a `void` specialisation for the Result type.
template <> class [[nodiscard]] Result<void> {
public:
	Result(const Result<void>& other) = delete;
	auto operator=(const Result<void>& other) -> Result& = delete;

	Result(Result<void>&& other) noexcept = default;
	auto operator=(Result<void>&& other) noexcept -> Result& = default;

	~Result() = default;

	static auto MakeOk() -> Result<void> { return Result(OK); }

	static auto MakeErr(ErrorCode e) -> Result<void> { return { ERR, e }; }

	[[nodiscard]] auto IsError() const noexcept -> bool { return Error != ErrorCode::Success; }

	[[nodiscard]] auto IsOk() const noexcept -> bool { return Error == ErrorCode::Success; }

private:
	/// Constructor for success case.
	explicit Result(OkType /*unused*/)
		: Error(ErrorCode::Success)
	{
	}

	/// Constructor for error case.
	Result(ErrType /*unused*/, ErrorCode e)
		: Error(e)
	{
	}

public:
	ErrorCode Error;
};

}
