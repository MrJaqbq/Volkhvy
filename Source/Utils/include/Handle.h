//
// Created by mrjaqbq on 06.03.16.
//

#ifndef VOLKHVY_HANDLE_H
#define VOLKHVY_HANDLE_H

namespace Utils
{
	typedef uint32_t handle_t;

	union RawHandle
	{

	public:
		handle_t Key;

		struct {
			uint8_t LiveId;
			uint8_t Type;
			uint16_t Index;
		};

		RawHandle() : Key(0) {}
	};

	template<typename Traits>
	class Handle
	{
		using ptr_t = typename Traits::ptr_t;

		ptr_t value;

		auto close() noexcept -> void
		{
			if(*this)
			{
				Traits::close(value);
			}
		}

		Handle(Handle const &) = delete;
		auto operator=(Handle const&) -> Handle& = delete;

	public:

		explicit Handle(ptr_t raw_value = Traits::invalid()) noexcept :
			value { raw_value } { }

		Handle(Handle && other) noexcept : value { other.release() } { }

		auto operator=(Handle && other) noexcept -> Handle&
		{
			if(this != &other)
			{
				reset(other.release());
			}

			return *this;
		}

		~Handle() noexcept
		{
			close();
		}

		explicit operator bool() const noexcept
		{
			return value != Traits::invalid();
		}

		auto get() const noexcept -> ptr_t
		{
			return value;
		}

		auto release() noexcept -> ptr_t
		{
			auto raw_value = value;
			value = Traits::invalid();
			return raw_value;
		}

		auto reset(ptr_t raw_value = Traits::invalid()) noexcept -> bool
		{
			if(value != raw_value)
			{
				close();
				value = raw_value;
			}

			return static_cast<bool>(*this);
		}

		auto swap(Handle<Traits>& other) noexcept -> void
		{
			std::swap(value, other.value);
		}
	};

	template<typename Traits>
	auto swap(Handle<Traits>& left, Handle<Traits>& right) noexcept -> void
	{
		left.swap(right);
	}
}

#endif //VOLKHVY_HANDLE_H
