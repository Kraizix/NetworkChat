#pragma once
#include <concepts>
#include <unordered_map>
#include <functional>
#include <stdexcept>

namespace ser
{
	using SerializableType = uint16_t;

	struct Buffer
	{
		/// <summary> Buffer raw data </summary>
		uint8_t* m_data;
		/// <summary> The remaining size of the buffer </summary>
		size_t m_remaining;
		Buffer(uint8_t* d, size_t size) : m_data(d), m_remaining(size) {}
	};

	class Serializer
	{
		const SerializableType test = 1;

		std::unordered_map<SerializableType, std::function<bool(Buffer&)>> m_readFunctionMap;

		/// <summary>
		/// Read the passed buffer and write the binary data into the object.
		/// </summary>
		/// <typeparam name="T">The type of object to write back to</typeparam>
		/// <param name="buffer"> : The buffer with serialized data</param>
		/// <param name="obj"> : The object to write back to</param>
		template <typename T>
		T Read(Buffer& buffer) {
			buffer.m_data = 0;
			return T();
		}

		#pragma region readSpecialization
		template <>
		std::string Read<std::string>(Buffer& buffer)
		{
			size_t objLength;
			size_t lengthSize = sizeof(size_t);
			memcpy(&objLength, buffer.m_data, lengthSize);
			buffer.m_data += lengthSize;
			buffer.m_remaining -= lengthSize;

			std::string obj(reinterpret_cast<char*>(buffer.m_data), objLength);

			buffer.m_data += objLength;
			buffer.m_remaining -= objLength;

			return obj;
		}

		template <>
		int Read<int>(Buffer& buffer)
		{
			int obj = 0;
			size_t size = sizeof(int);

			memcpy(&obj, buffer.m_data, size);
			buffer.m_data += size;
			buffer.m_remaining -= size;

			return obj;
		}
		#pragma endregion

	public:

		/// <summary>
		/// Write the passed object binary data into the buffer.
		/// </summary>
		/// <typeparam name="T">The type of passed object to get data from</typeparam>
		/// <param name="buffer"> : The buffer to write data into</param>
		/// <param name="obj"> : The object to get the data from</param>
		/// <returns>Wether or not the data has been successfuly wroten</returns>
		template <typename T>
		bool Write(Buffer& buffer, T* obj) {
			buffer.m_data = 0;
			obj = nullptr;
			return false;
		};

		#pragma region writeSpecialization
		template <>
		bool Write<const char*>(Buffer& buffer, const char** obj)
		{
			size_t objLength = std::strlen(*obj);
			if (!(objLength > 0))
				return false;

			size_t lengthSize = sizeof(size_t);
			//Space for characters + Space for string length
			size_t totalSize = objLength + lengthSize;
			
			if (buffer.m_remaining < totalSize)
				return false;

			memcpy(buffer.m_data, &objLength, lengthSize);
			buffer.m_data += lengthSize;
			buffer.m_remaining -= lengthSize;

			memcpy(buffer.m_data, *obj, objLength);
			buffer.m_data += objLength;
			buffer.m_remaining -= objLength;

			return true;
		}

		template <>
		bool Write<int>(Buffer& buffer, int* obj)
		{
			size_t size = sizeof(int);
			if (buffer.m_remaining < size) 
				return false;

			memcpy(buffer.m_data, obj, size);
			buffer.m_data += size;
			buffer.m_remaining -= size;

			return true;
		}
		#pragma endregion
	};
}