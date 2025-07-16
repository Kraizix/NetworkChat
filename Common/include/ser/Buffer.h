#pragma once

namespace ser
{
	struct Buffer
	{
		/// <summary> Buffer raw data </summary>
		uint8_t* m_data;
		/// <summary> Base pointer of the buffer </summary>
		uint8_t* m_base;
		/// <summary> The remaining size of the buffer </summary>
		size_t m_remaining;

		size_t m_size;
		Buffer(uint8_t* d, size_t size) : m_data(d), m_base(d), m_remaining(size), m_size(size) {}

		~Buffer()
		{
			delete[] m_base;
			m_data = nullptr;
			m_base = nullptr;
		}

		Buffer(const Buffer& other)
			: m_data(new uint8_t[other.m_size]), m_base(m_data), m_remaining(other.m_remaining), m_size(other.m_size)
		{
			std::copy(other.m_base, other.m_base + other.m_size, m_base);
		}

		Buffer& operator=(const Buffer& other)
		{
			if (this != &other)
			{
				delete[] m_base;
				m_size = other.m_size;
				m_remaining = other.m_remaining;
				m_base = new uint8_t[m_size];
				std::copy(other.m_base, other.m_base + m_size, m_base);
				m_data = m_base;
			}
			return *this;
		}

		/// <summary>
		/// Back up the current position in the buffer
		/// </summary>
		void Reset()
		{
			m_data = m_base;
			m_remaining = m_size;
		}
	};
}