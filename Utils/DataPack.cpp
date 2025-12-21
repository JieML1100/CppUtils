#pragma once
#include "DataPack.h"
#include <unordered_map>
#include <string>
#include <vector>

enum class DataPachKey : uint8_t {
	FileStart = 0x81,
	FileEnd = 0x98,
	IdStart = 0xC7,
	IdEnd = 0xC8,
	ValueStart = 0x55,
	ValueEnd = 0x56,
	ValueStart_Small = 0x57,
	ValueStart_Small_X = 0x58,
	ChildStart = 0xD4,
	ChildEnd = 0xD5,
	ChildStart_Small = 0xD6,
	ChildStart_Small_X = 0xD7,
};

DataPack& DataPack::operator[](int index) {
	return this->Child[index];
}
DataPack& DataPack::operator[](const std::string& id) {
	for (int i = 0; i < Child.size(); i++) {
		if (this->Child[i].Id == id) {
			return this->Child[i];
		}
	}
	Child.push_back(DataPack(id, 0));
	return this->Child[this->Child.size() - 1];
}

static inline bool is_little_endian() {
	const uint16_t one = 1;
	return *(const uint8_t*)&one == 1;
}

static inline uint16_t bswap16(uint16_t v) {
	return (uint16_t)((v >> 8) | (v << 8));
}

static inline uint32_t bswap32(uint32_t v) {
	return ((v & 0x000000FFu) << 24) |
		((v & 0x0000FF00u) << 8) |
		((v & 0x00FF0000u) >> 8) |
		((v & 0xFF000000u) >> 24);
}

static inline bool read_u16_le(const uint8_t* data, size_t data_len, size_t offset, uint16_t& out) {
	if (offset + sizeof(uint16_t) > data_len)
		return false;
	std::memcpy(&out, data + offset, sizeof(uint16_t));
	if (!is_little_endian())
		out = bswap16(out);
	return true;
}

static inline bool read_u32_le(const uint8_t* data, size_t data_len, size_t offset, uint32_t& out) {
	if (offset + sizeof(uint32_t) > data_len)
		return false;
	std::memcpy(&out, data + offset, sizeof(uint32_t));
	if (!is_little_endian())
		out = bswap32(out);
	return true;
}

static inline void append_u8(std::vector<uint8_t>& out, uint8_t v) {
	out.push_back(v);
}

static inline void append_bytes(std::vector<uint8_t>& out, const void* data, size_t len) {
	if (len == 0)
		return;
	const uint8_t* p = (const uint8_t*)data;
	const size_t oldSize = out.size();
	out.resize(oldSize + len);
	std::memcpy(out.data() + oldSize, p, len);
}

static inline void append_u16_le(std::vector<uint8_t>& out, uint16_t v) {
	if (!is_little_endian())
		v = bswap16(v);
	append_bytes(out, &v, sizeof(v));
}

static inline void append_u32_le(std::vector<uint8_t>& out, uint32_t v) {
	if (!is_little_endian())
		v = bswap32(v);
	append_bytes(out, &v, sizeof(v));
}

static size_t serialized_size(const DataPack& pack) {
	// FileStart (1) + size32 (4) + FileEnd (1)
	size_t total = 6;
	if (!pack.Id.empty()) {
		// IdStart + u16 len + bytes + IdEnd
		total += 1 + 2 + pack.Id.size() + 1;
	}
	if (!pack.Value.empty()) {
		const size_t valueLen = pack.Value.size();
		if (valueLen > UINT32_MAX)
			return 0;
		// ValueStartX + len(1/2/4) + bytes + ValueEnd
		if (valueLen > UINT16_MAX)
			total += 1 + 4 + valueLen + 1;
		else if (valueLen > UINT8_MAX)
			total += 1 + 2 + valueLen + 1;
		else
			total += 1 + 1 + valueLen + 1;
	}
	for (const auto& sub : pack.Child) {
		const size_t childSize = serialized_size(sub);
		if (childSize == 0)
			return 0;
		if (childSize > UINT32_MAX)
			return 0;
		// ChildStartX + len(1/2/4) + childBytes + ChildEnd
		if (childSize > UINT16_MAX)
			total += 1 + 4 + childSize + 1;
		else if (childSize > UINT8_MAX)
			total += 1 + 2 + childSize + 1;
		else
			total += 1 + 1 + childSize + 1;
	}
	return total;
}

using SizeMap = std::unordered_map<const DataPack*, size_t>;

static size_t compute_sizes(const DataPack& pack, SizeMap& sizes) {
	size_t total = 6;
	if (!pack.Id.empty())
		total += 1 + 2 + pack.Id.size() + 1;
	if (!pack.Value.empty()) {
		const size_t valueLen = pack.Value.size();
		if (valueLen > std::numeric_limits<uint32_t>::max())
			return 0;
		if (valueLen > UINT16_MAX)
			total += 1 + 4 + valueLen + 1;
		else if (valueLen > UINT8_MAX)
			total += 1 + 2 + valueLen + 1;
		else
			total += 1 + 1 + valueLen + 1;
	}
	for (const auto& sub : pack.Child) {
		const size_t childSize = compute_sizes(sub, sizes);
		if (childSize == 0)
			return 0;
		if (childSize > std::numeric_limits<uint32_t>::max())
			return 0;
		if (childSize > UINT16_MAX)
			total += 1 + 4 + childSize + 1;
		else if (childSize > UINT8_MAX)
			total += 1 + 2 + childSize + 1;
		else
			total += 1 + 1 + childSize + 1;
	}
	sizes[&pack] = total;
	return total;
}

void DataPack::operator=(const std::initializer_list<uint8_t> data) {
	this->Value.resize(data.size());
	if (data.size() > 0)
		std::memcpy(this->Value.data(), data.begin(), data.size());
}
void DataPack::operator=(const std::initializer_list<uint8_t>* data) {
	this->Value.resize(data->size());
	if (data->size() > 0)
		std::memcpy(this->Value.data(), data->begin(), data->size());
}
void DataPack::operator=(const char* data) {
	std::string str = data;
	this->Value.resize(str.size());
	if (!str.empty())
		std::memcpy(this->Value.data(), str.c_str(), str.size());
}
void DataPack::operator=(const wchar_t* data) {
	std::wstring str = data;
	this->Value.resize(str.size() * 2);
	if (!str.empty())
		std::memcpy(this->Value.data(), str.c_str(), str.size() * 2);
}
void DataPack::operator=(char* data) {
	std::string str = data;
	this->Value.resize(str.size());
	if (!str.empty())
		std::memcpy(this->Value.data(), str.c_str(), str.size());
}
void DataPack::operator=(wchar_t* data) {
	std::wstring str = data;
	this->Value.resize(str.size() * 2);
	if (!str.empty())
		std::memcpy(this->Value.data(), str.c_str(), str.size() * 2);
}
void DataPack::operator=(std::string data) {
	this->Value.resize(data.size());
	if (!data.empty())
		std::memcpy(this->Value.data(), data.c_str(), data.size());
}
void DataPack::operator=(std::wstring data) {
	this->Value.resize(data.size() * 2);
	if (!data.empty())
		std::memcpy(this->Value.data(), data.c_str(), data.size() * 2);
}
void DataPack::RemoveAt(int index) {
	this->Child.erase(this->Child.begin() + index);
}
DataPack::DataPack() :Id({}), Value({}) {}
DataPack::DataPack(const uint8_t* data, int data_len) {
	if (data == nullptr)
		return;
	if (data_len < 6)
		return;
	if (data[0] != (uint8_t)DataPachKey::FileStart) {
		return;
	}
	uint32_t bufferSizeU32 = 0;
	if (!read_u32_le(data, (size_t)data_len, 1, bufferSizeU32))
		return;
	const size_t bufferSize = (size_t)bufferSizeU32;
	if (bufferSize < 6 || bufferSize >(size_t)data_len) {
		return;
	}
	if (data[bufferSize - 1] != (uint8_t)DataPachKey::FileEnd) {
		return;
	}
	size_t index = 5;
	while (index < bufferSize - 1) {
		DataPachKey key = (DataPachKey)data[index];
		switch (key) {
		case DataPachKey::IdStart: {
			index += 1;
			uint16_t idLen = 0;
			if (!read_u16_le(data, bufferSize, index, idLen))
				return;
			index += 2;
			if (index + (size_t)idLen >= bufferSize)
				return;
			if (data[index + idLen] != (uint8_t)DataPachKey::IdEnd)
				return;
			this->Id.assign((char*)&data[index], idLen);
			index += (size_t)idLen + 1;
			break;
		}
		case DataPachKey::ValueStart: {
			index += 1;
			uint32_t valueLenU32 = 0;
			if (!read_u32_le(data, bufferSize, index, valueLenU32))
				return;
			index += 4;
			const size_t valueLen = (size_t)valueLenU32;
			if (index + valueLen >= bufferSize)
				return;
			if (data[index + valueLen] != (uint8_t)DataPachKey::ValueEnd)
				return;
			this->Value.assign(&data[index], &data[index + valueLen]);
			index += valueLen + 1;
			break;
		}
		case DataPachKey::ValueStart_Small: {
			index += 1;
			uint16_t valueLenU16 = 0;
			if (!read_u16_le(data, bufferSize, index, valueLenU16))
				return;
			index += 2;
			const size_t valueLen = (size_t)valueLenU16;
			if (index + valueLen >= bufferSize)
				return;
			if (data[index + valueLen] != (uint8_t)DataPachKey::ValueEnd)
				return;
			this->Value.assign(&data[index], &data[index + valueLen]);
			index += valueLen + 1;
			break;
		}
		case DataPachKey::ValueStart_Small_X: {
			index += 1;
			if (index + 1 > bufferSize)
				return;
			const uint8_t valueLenU8 = data[index];
			index += 1;
			const size_t valueLen = (size_t)valueLenU8;
			if (index + valueLen >= bufferSize)
				return;
			if (data[index + valueLen] != (uint8_t)DataPachKey::ValueEnd)
				return;
			this->Value.assign(&data[index], &data[index + valueLen]);
			index += valueLen + 1;
			break;
		}
		case DataPachKey::ChildStart: {
			index += 1;
			uint32_t childLenU32 = 0;
			if (!read_u32_le(data, bufferSize, index, childLenU32))
				return;
			index += 4;
			const size_t childLen = (size_t)childLenU32;
			if (childLen < 6)
				return;
			if (index + childLen >= bufferSize)
				return;
			if (data[index + childLen] != (uint8_t)DataPachKey::ChildEnd)
				return;
			this->Child.emplace_back(&data[index], (int)childLen);
			index += childLen + 1;
			break;
		}
		case DataPachKey::ChildStart_Small: {
			index += 1;
			uint16_t childLenU16 = 0;
			if (!read_u16_le(data, bufferSize, index, childLenU16))
				return;
			index += 2;
			const size_t childLen = (size_t)childLenU16;
			if (childLen < 6)
				return;
			if (index + childLen >= bufferSize)
				return;
			if (data[index + childLen] != (uint8_t)DataPachKey::ChildEnd)
				return;
			this->Child.emplace_back(&data[index], (int)childLen);
			index += childLen + 1;
			break;
		}
		case DataPachKey::ChildStart_Small_X: {
			index += 1;
			if (index + 1 > bufferSize)
				return;
			const uint8_t childLenU8 = data[index];
			index += 1;
			const size_t childLen = (size_t)childLenU8;
			if (childLen < 6)
				return;
			if (index + childLen >= bufferSize)
				return;
			if (data[index + childLen] != (uint8_t)DataPachKey::ChildEnd)
				return;
			this->Child.emplace_back(&data[index], (int)childLen);
			index += childLen + 1;
			break;
		}
		default: {
			return;
		}
		}
	}
}

DataPack::DataPack(const char* key) {
	this->Id = key;
	this->Value.resize(0);
}
DataPack::DataPack(std::string id, uint8_t* data, int len) {
	this->Id = id;
	this->Value.resize(len);
	if (len > 0)
		std::memcpy(this->Value.data(), data, len);
}
DataPack::DataPack(std::vector<uint8_t> data) : DataPack(data.data(), data.size()) {}
DataPack::DataPack(std::initializer_list<uint8_t> data) : DataPack((uint8_t*)data.begin(), data.size()) {}
DataPack::DataPack(std::string id, std::string data) {
	this->Id = id;
	this->Value.resize(data.size());
	if (!data.empty())
		std::memcpy(this->Value.data(), data.c_str(), data.size());
}
DataPack::DataPack(std::string id, std::wstring data) {
	this->Id = id;
	this->Value.resize(data.size() * 2);
	if (!data.empty())
		std::memcpy(this->Value.data(), data.c_str(), data.size() * 2);
}
DataPack::DataPack(std::string id, char* data) {
	this->Id = id;
	std::string str = data;
	this->Value.resize(str.size());
	if (!str.empty())
		std::memcpy(this->Value.data(), str.c_str(), str.size());
}
DataPack::DataPack(std::string id, const char* data) {
	this->Id = id;
	std::string str = data;
	this->Value.resize(str.size());
	if (!str.empty())
		std::memcpy(this->Value.data(), str.c_str(), str.size());
}
DataPack::DataPack(std::string id, wchar_t* data) {
	this->Id = id;
	std::wstring str = data;
	this->Value.resize(str.size() * 2);
	if (!str.empty())
		std::memcpy(this->Value.data(), str.c_str(), str.size() * 2);
}
DataPack::DataPack(std::string id, const wchar_t* data) {
	this->Id = id;
	std::wstring str = data;
	this->Value.resize(str.size() * 2);
	if (!str.empty())
		std::memcpy(this->Value.data(), str.c_str(), str.size() * 2);
}
void DataPack::Add(const DataPack& val) {
	this->Child.push_back(val);
}
void DataPack::clear() {
	this->Child.clear();
}
size_t DataPack::size() const {
	return this->Child.size();
}
void DataPack::resize(size_t value) {
	this->Child.resize(value);
}
void DataPack::WriteTo(std::vector<uint8_t>& out) const {
	append_u8(out, (uint8_t)DataPachKey::FileStart);
	const size_t sizePos = out.size();
	append_u32_le(out, 0u);

	if (!this->Id.empty()) {
		append_u8(out, (uint8_t)DataPachKey::IdStart);
		append_u16_le(out, (uint16_t)this->Id.size());
		append_bytes(out, this->Id.data(), this->Id.size());
		append_u8(out, (uint8_t)DataPachKey::IdEnd);
	}

	if (!this->Value.empty()) {
		const size_t valueLen = this->Value.size();
		if (valueLen > UINT16_MAX) {
			append_u8(out, (uint8_t)DataPachKey::ValueStart);
			append_u32_le(out, (uint32_t)valueLen);
		}
		else if (valueLen > UINT8_MAX) {
			append_u8(out, (uint8_t)DataPachKey::ValueStart_Small);
			append_u16_le(out, (uint16_t)valueLen);
		}
		else {
			append_u8(out, (uint8_t)DataPachKey::ValueStart_Small_X);
			append_u8(out, (uint8_t)valueLen);
		}
		append_bytes(out, this->Value.data(), this->Value.size());
		append_u8(out, (uint8_t)DataPachKey::ValueEnd);
	}

	for (const auto& sub : this->Child) {
		const size_t childLen = serialized_size(sub);
		if (childLen == 0)
			continue;
		if (childLen > UINT16_MAX) {
			append_u8(out, (uint8_t)DataPachKey::ChildStart);
			append_u32_le(out, (uint32_t)childLen);
		}
		else if (childLen > UINT8_MAX) {
			append_u8(out, (uint8_t)DataPachKey::ChildStart_Small);
			append_u16_le(out, (uint16_t)childLen);
		}
		else {
			append_u8(out, (uint8_t)DataPachKey::ChildStart_Small_X);
			append_u8(out, (uint8_t)childLen);
		}
		sub.WriteTo(out);
		append_u8(out, (uint8_t)DataPachKey::ChildEnd);
	}

	append_u8(out, (uint8_t)DataPachKey::FileEnd);

	const size_t packStart = sizePos - 1;
	const size_t totalSize = out.size() - packStart;
	uint32_t totalSizeU32 = (uint32_t)totalSize;
	if (!is_little_endian())
		totalSizeU32 = bswap32(totalSizeU32);
	std::memcpy(out.data() + sizePos, &totalSizeU32, sizeof(totalSizeU32));
}

static void write_to_sized(const DataPack& pack, std::vector<uint8_t>& out, const SizeMap& sizes) {
	append_u8(out, (uint8_t)DataPachKey::FileStart);
	const size_t sizePos = out.size();
	append_u32_le(out, 0u);

	if (!pack.Id.empty()) {
		append_u8(out, (uint8_t)DataPachKey::IdStart);
		append_u16_le(out, (uint16_t)pack.Id.size());
		append_bytes(out, pack.Id.data(), pack.Id.size());
		append_u8(out, (uint8_t)DataPachKey::IdEnd);
	}

	if (!pack.Value.empty()) {
		const size_t valueLen = pack.Value.size();
		if (valueLen > UINT16_MAX) {
			append_u8(out, (uint8_t)DataPachKey::ValueStart);
			append_u32_le(out, (uint32_t)valueLen);
		}
		else if (valueLen > UINT8_MAX) {
			append_u8(out, (uint8_t)DataPachKey::ValueStart_Small);
			append_u16_le(out, (uint16_t)valueLen);
		}
		else {
			append_u8(out, (uint8_t)DataPachKey::ValueStart_Small_X);
			append_u8(out, (uint8_t)valueLen);
		}
		append_bytes(out, pack.Value.data(), pack.Value.size());
		append_u8(out, (uint8_t)DataPachKey::ValueEnd);
	}

	for (const auto& sub : pack.Child) {
		auto it = sizes.find(&sub);
		if (it == sizes.end())
			continue;
		const size_t childLen = it->second;
		if (childLen > UINT16_MAX) {
			append_u8(out, (uint8_t)DataPachKey::ChildStart);
			append_u32_le(out, (uint32_t)childLen);
		}
		else if (childLen > UINT8_MAX) {
			append_u8(out, (uint8_t)DataPachKey::ChildStart_Small);
			append_u16_le(out, (uint16_t)childLen);
		}
		else {
			append_u8(out, (uint8_t)DataPachKey::ChildStart_Small_X);
			append_u8(out, (uint8_t)childLen);
		}
		write_to_sized(sub, out, sizes);
		append_u8(out, (uint8_t)DataPachKey::ChildEnd);
	}

	append_u8(out, (uint8_t)DataPachKey::FileEnd);

	const size_t packStart = sizePos - 1;
	const size_t totalSize = out.size() - packStart;
	uint32_t totalSizeU32 = (uint32_t)totalSize;
	if (!is_little_endian())
		totalSizeU32 = bswap32(totalSizeU32);
	std::memcpy(out.data() + sizePos, &totalSizeU32, sizeof(totalSizeU32));
}

std::vector<uint8_t> DataPack::GetBytes() const {
	std::vector<uint8_t> out;
	SizeMap sizes;
	const size_t total = compute_sizes(*this, sizes);
	if (total > 0)
		out.reserve(total);
	write_to_sized(*this, out, sizes);
	return out;
}