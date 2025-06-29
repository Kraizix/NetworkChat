#pragma once

namespace ser
{
	struct Buffer
	{
		/// <summary> Buffer raw data </summary>
		uint8_t* m_data;
		/// <summary> The remaining size of the buffer </summary>
		size_t m_remaining;
		Buffer(uint8_t* d, size_t size) : m_data(d), m_remaining(size) {}
	};
}