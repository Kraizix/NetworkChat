#pragma once
#include <unordered_map>
#include "Common/Command.h"
#include "ser/Buffer.h"

namespace ser
{
	class Serializer
	{

	public:
		/// <summary>
		/// Copy the object into the buffer.
		/// </summary>
		template<typename T>
		void MemcpyObj(Buffer& buffer, T& obj, size_t size) {
			std::memcpy(buffer.m_data, &obj, size);
			buffer.m_data += size;
			buffer.m_remaining -= size;
		}

		/// <summary>
		/// Copy the buffer into the object.
		/// </summary>
		template<typename T>
		void MemcpyBuffer(T& obj, Buffer& buffer, size_t size) {
			std::memcpy(&obj, buffer.m_data, size);
			buffer.m_data += size;
			buffer.m_remaining -= size;
		}

		/// <summary>
		/// Read the passed buffer and write the binary data into the object.
		/// </summary>
		/// <typeparam name="T">The type of object to write back to</typeparam>
		/// <param name="buffer"> : The buffer with serialized data</param>
		/// <param name="obj"> : The object to write back to</param>
		template <typename T>
		bool Read(Buffer& buffer, T& obj) {
			buffer.m_data = 0;
			return T();
		}

	#pragma region readSpecialization
		template <>
		bool Read<CommandType>(Buffer& buffer, CommandType& obj)
		{
			uint8_t type;
			MemcpyBuffer<uint8_t>(type, buffer, sizeof(uint8_t));
			obj = static_cast<CommandType>(type);
			return true;
		}

		template <>
		bool Read<std::string>(Buffer& buffer, std::string& obj)
		{
			size_t objLength;
			size_t lengthSize = sizeof(size_t);
			MemcpyBuffer<size_t>(objLength, buffer, lengthSize);
			obj.resize(objLength);

			obj.assign(reinterpret_cast<char*>(buffer.m_data), objLength);
			buffer.m_data += objLength;
			buffer.m_remaining -= objLength;

			return true;
		}

		template <>
		bool Read<uint16_t>(Buffer& buffer, uint16_t& obj)
		{
			MemcpyBuffer<uint16_t>(obj, buffer, sizeof(uint16_t));

			return true;
		}

		template <>
		bool Read<int>(Buffer& buffer, int& obj)
		{
			size_t size = sizeof(int);

			MemcpyBuffer<int>(obj, buffer, size);

			return true;
		}

		template <>
		bool Read<std::vector<int>>(Buffer& buffer, std::vector<int>& obj)
		{
			size_t objLength;
			size_t lengthSize = sizeof(size_t);
			MemcpyBuffer<size_t>(objLength, buffer, lengthSize);

			obj.reserve(objLength);
			for (int i = 0; i < objLength; ++i)
			{
				int item;
				Read<int>(buffer, item);
				obj.push_back(item);
			}
			return true;
		}

		template <>
		bool Read<std::vector<std::string>>(Buffer& buffer, std::vector<std::string>& obj) {
			
			size_t objLength;
			size_t lengthSize = sizeof(size_t);
			MemcpyBuffer<size_t>(objLength, buffer, lengthSize);

			obj.reserve(objLength);
			for (int i = 0; i < objLength; ++i)
			{
				std::string item = "";
				Read<std::string>(buffer, item);
				obj.emplace_back(std::move(item));
			}
			return true;
		}

		template<>
		bool Read<Command>(Buffer& buffer, Command& obj)
		{
			Read(buffer, obj.type);
			Read(buffer, obj.data);
			return true;
		}
	#pragma endregion

		/// <summary>
		/// Write the passed object binary data into the buffer.
		/// </summary>
		/// <typeparam name="T">The type of passed object to get data from</typeparam>
		/// <param name="buffer"> : The buffer to write data into</param>
		/// <param name="obj"> : The object to get the data from</param>
		/// <returns>Wether or not the data has been successfuly wroten</returns>
		
		template <typename T>
		[[nodiscard]] bool Write(Buffer& buffer, T& obj) {
			buffer.m_data = 0;
			obj = nullptr;
			return false;
		};

	#pragma region writeSpecialization
		template <>
		bool Write<std::string>(Buffer& buffer, std::string& obj)
		{
			size_t objLength = obj.length();
			if (!(objLength > 0))
				return false;

			size_t lengthSize = sizeof(size_t);
			//Space for characters + Space for string length
			size_t totalSize = objLength + lengthSize;

			if (buffer.m_remaining < totalSize)
				return false;

			MemcpyObj(buffer, objLength, lengthSize);

			std::memcpy(buffer.m_data, obj.data(), objLength);
			buffer.m_data += objLength;
			buffer.m_remaining -= objLength;

			return true;
		}

		template<>
		bool Write<uint16_t>(Buffer& buffer, uint16_t& obj)
		{
			size_t size = sizeof(uint16_t);
			if (buffer.m_remaining < size)
				return false;

			MemcpyObj<uint16_t>(buffer, obj, size);
			return true;
		}

		template <>
		bool Write<int>(Buffer& buffer, int& obj)
		{
			size_t size = sizeof(int);
			if (buffer.m_remaining < size)
				return false;

			MemcpyObj<int>(buffer, obj, size);

			return true;
		}
		
		bool VectorWriteSize(Buffer& buffer, size_t vectorSize, size_t vectorTypeSize) {
			size_t lengthSize = sizeof(size_t);
			//Check if buffer has enough space for holding the vector size and the raw data
			size_t totalSize = vectorSize * vectorTypeSize + lengthSize;
			if (buffer.m_remaining < totalSize)
				return false;

			MemcpyObj<size_t>(buffer, vectorSize, lengthSize);
			return true;
		}

		template <>
		bool Write<std::vector<int>>(Buffer& buffer, std::vector<int>& obj)
		{
			//Writes the size of the vector in the buffer if able to
			if (!VectorWriteSize(buffer, obj.size(), sizeof(int)))
				return false;

			for (auto& item : obj)
			{
				if (!Write<int>(buffer, item))
					return false;
			}
			return true;
		}

		template <>
		bool Write<std::vector<std::string>>(Buffer& buffer, std::vector<std::string>& obj) {
			//Writes the size of the vector in the buffer if able to
			//if (!VectorWriteSize(buffer, obj.size(), sizeof(char)))
				//return false;
			size_t lengthSize = sizeof(size_t);
			size_t vectorSize = obj.size();
			MemcpyObj<size_t>(buffer, vectorSize, lengthSize);

			for (auto& item : obj)
			{
				if (!Write<std::string>(buffer, item))
					return false;
			}
			return true;
		}

		template<>
		bool Write<Command>(Buffer& buffer, Command& obj)
		{
			if (!Write<uint16_t>(buffer, obj.total_length))
				return false;
			{
				uint8_t type = static_cast<uint8_t>(obj.type);
				MemcpyObj<uint8_t>(buffer, type, sizeof(uint8_t));
			}
			if (!Write<std::string>(buffer, obj.data))
				return false;
			return true;
		}
	#pragma endregion
	};
}