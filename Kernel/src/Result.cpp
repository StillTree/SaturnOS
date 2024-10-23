#include "Result.hpp"

namespace SaturnKernel
{
	template <typename T> template <typename... Args> auto Result<T>::MakeOk(Args&&... args) -> Result<T>
	{
		return Result(OK, T(Forward<Args>(args)...));
	}

	template <typename T> auto Result<T>::MakeErr(ErrorCode e) -> Result<T>
	{
		return Result(ERR, e);
	}

	template <typename T> Result<T>::Result(Result<T>&& other) noexcept
		: m_isOk(other.m_isOk)
	{
		if(other.m_isOk)
			new(&Value) T(Move(other.Value));
		else
			Error = other.Error;
	}

	template <typename T> auto Result<T>::operator=(Result<T>&& other) noexcept -> Result&
	{
		if(m_isOk)
			Value.~T();

		m_isOk = other.m_isOk;
		if(m_isOk)
			new(&Value) T(Move(other.Value));
		else
			Error = other.Error;

		return *this;
	}

	template <typename T> Result<T>::Result(OkType /*unused*/, T&& v)
		: Value(Move(v)),
		  m_isOk(true)
	{
	}

	template <typename T> Result<T>::Result(ErrType /*unused*/, ErrorCode e)
		: Error(e),
		  m_isOk(false)
	{
	}

	template <typename T> [[nodiscard]] auto Result<T>::IsError() const noexcept -> bool
	{
		return !m_isOk;
	}

	template <typename T> Result<T>::~Result()
	{
		if(m_isOk)
			Value.~T();
	}

	// Result's void specialization

	auto Result<void>::MakeOk() -> Result<void>
	{
		return Result(OK);
	}

	auto Result<void>::MakeErr(ErrorCode e) -> Result<void>
	{
		return Result(ERR, e);
	}

	Result<void>::Result(OkType /*unused*/)
		: Error(ErrorCode::Success)
	{
	}

	Result<void>::Result(ErrType /*unused*/, ErrorCode e)
		: Error(e)
	{
	}

	[[nodiscard]] auto Result<void>::IsError() const noexcept -> bool
	{
		return Error != ErrorCode::Success;
	}
}
