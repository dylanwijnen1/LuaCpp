#pragma once

namespace lpp
{
	class RefCounter
	{
		size_t m_refCount;

	public:
		RefCounter() : m_refCount(0) {}
		RefCounter(size_t count) : m_refCount(count) {}
		~RefCounter() = default;

		RefCounter(const RefCounter&) = default;
		RefCounter(RefCounter&&) = default;
		RefCounter& operator=(const RefCounter&) = default;
		RefCounter& operator=(RefCounter&&) = default;

		void Increment()
		{
			++m_refCount;
		}

		size_t Decrement()
		{
			return --m_refCount;
		}

		size_t GetCount() const
		{
			return m_refCount;
		}

		bool IsValid() const { return m_refCount != 0; }

	};
};