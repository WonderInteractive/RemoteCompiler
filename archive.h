#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <stdexcept>
#include <cstring>
#if 1
#include "parallel_hashmap/phmap.h"
#include "parallel_hashmap/phmap_dump.h"
using phmap::flat_hash_map;
using phmap::parallel_flat_hash_map;
using phmap::parallel_flat_hash_set;
#define EXTRAARGS , phmap::priv::hash_default_hash<K>, \
                    phmap::priv::hash_default_eq<K>, \
                    std::allocator<std::pair<K, V>>, 4, std::mutex
#define EXTRAARGS2 , phmap::priv::hash_default_hash<K>, \
                    phmap::priv::hash_default_eq<K>, \
                    std::allocator<K>, 4, std::mutex
template <class K, class V>
using HashT = parallel_flat_hash_map<K, V EXTRAARGS>;
template <class K>
using SetT = parallel_flat_hash_set<K EXTRAARGS2>;
#else
template <class K, class V>
using HashT = std::unordered_map<K, V>;
template <class K>
using SetT = std::set<K>;
#endif
#include <filesystem>
namespace fs = std::filesystem;
using namespace std::chrono_literals;
using i8 = int8_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
extern struct FS _fs;
struct AssetInfo {
	std::string data = "";
	std::string uc_data = "";
	u32 size = 0, crc = 0;
};
struct AssetDiff {
	u32 old_crc = 0, new_crc = 0;
	bool is_patch;
	std::string data = "";
	size_t size() const {
		return sizeof(u32) * 2 + data.size() + sizeof(bool);
	}
};
using fnhash = u64;
using datahash = u64;
#define MAKE_HASH32(str) XXH32(str.data(), str.size(), 0)
#define MAKE_HASH64(str) XXH64(str.data(), str.size(), 0)
#define MAKE_FNHASH(str) XXH32(str.data(), str.size(), 0)
struct AssetHash {
	std::vector<datahash> dep_map;
	u32 size = 0;
};
struct GameInfoDebug {
	std::string GameID = "";
	std::string GameName = "";
	u32 GameRuntime = 0;
	HashT<fnhash, std::string> fnhash_to_fn;
	HashT<datahash, std::vector<fnhash>> datahash_to_fnhash;
};
struct GameInfo {
	std::string GameID = "";
	HashT<fnhash, datahash> fnhash_to_datahash;
	HashT<datahash, AssetHash> datahash_to_assethash;
	HashT<fnhash, std::vector<std::pair<bool, std::string>>> directories;
	std::vector<datahash> loadOnStart;
	std::vector<fnhash> blackList;
};

struct Asset {
	u32 uc_size = 0;
	u32 crc = 0;
	std::string data;
	size_t size() const {
		return sizeof(u32) * 2 + data.size();
	}
};
enum MESSAGE_TYPE : u8 {
	UNKNOWN_ASSET, ASSET, GAME_INFO, GAME_DEBUG, TRACE
};
enum struct CompressType { NONE, ZSTD, FLZMA };
struct Archive {
	std::string const_str = "";
	enum class ARCHIVE_TYPE {
		VECTOR, MAP, UNORDERED_MAP, STRING, U8, U16, U32, U64, I8, I16, I32, I64, F32, F64
	};
	std::string& test;
	//std::string ctest;
	uint32_t pos = 0;
	//uint32_t cpos = 0;
	uint32_t size = 0;
	//uint32_t csize = 0; //uncompressed_size_for_compressed_data
	//uint32_t ccsize = 0; //compressed_size_for_compressed_data
	bool compressed = false;
	bool owns = false;
	bool corrupt = false;
	const char* data = nullptr;
	//const char* cdata = nullptr;
	void SetCorrupt() {
		corrupt = true;
		exit(1);
	}
	Archive(std::string& str) : test(str) {
		size = (uint32_t)test.size();
		data = test.data();
		owns = true;
		pos = 0;
	};
	void Set(std::string& str) {
		test = str;
		size = (uint32_t)test.size();
		data = test.data();
		owns = true;
		pos = 0;
	}
	Archive(const char* str, const uint32_t _size = 100000000) : data(str), size(_size), test(const_str) {
		size = _size;
		//string has the following layout
		//compressed_size
		/*memcpy(&ccsize, (void*)(str), 4);
		if (ccsize > 0) {
			ctest.resize(ccsize);
			memcpy((void*)(ctest.data()), (void*)(data), ccsize);
			cdata = ctest.data();
			//decompress here

		}*/
	};
	~Archive() {
		//if (owns)
			//free((void*)test);
	}
	void reset() { pos = 0; }
	void read(char* data2, const uint32_t size2, const bool m_free = false) {
		memcpy(data2, this->data + pos, size2);
		pos += size2;
		/*if (m_free) {
			//test.erase(0, size);
			//test.shrink_to_fit();
		} else*/
	}
	/*void read_compressed(char* data, const uint32_t size, const bool m_free = false) {
		memcpy(data, this->cdata + cpos, size);
		cpos += size;

	}
	void write_compressed(const char* data, const uint32_t size) {
		if (cpos + size > this->csize) {
			this->csize += size > 10000 ? size : 10000;
			ctest.resize(this->csize);
			this->cdata = ctest.data();
		}
		memcpy((void*)(ctest.data() + cpos), data, size);
		cpos += size;
	}*/
	void write(const char* data2, const uint32_t size2) {
		if (pos + size2 > this->size) {
			//this->size += size > 10000 ? size : 10000;
			this->size += size2;
			test.resize(this->size);
			this->data = test.data();
		}
		memcpy((void*)(test.data() + pos), data2, size2);
		pos += size2;
	}
	std::string& consume() {
		if (test.size() > pos)
			test.resize(pos);
		return test;
	}
	Archive& operator>>(int& v) {
		read((char*)&v, sizeof(int));
		return *this;
	}
	Archive& operator<<(int v) {
		write((const char*)&v, sizeof(int));
		return *this;
	}
	Archive& operator>>(MESSAGE_TYPE& v) {
		read((char*)&v, sizeof(MESSAGE_TYPE));
		return *this;
	}
	Archive& operator<<(MESSAGE_TYPE v) {
		write((const char*)&v, sizeof(MESSAGE_TYPE));
		return *this;
	}
	Archive& operator>>(CompressType& v) {
		read((char*)&v, sizeof(CompressType));
		return *this;
	}
	Archive& operator<<(CompressType v) {
		write((const char*)&v, sizeof(CompressType));
		return *this;
	}
	Archive& operator>>(AssetInfo& v) {
		*this >> v.crc >> v.data >> v.uc_data >> v.size;
		return *this;
	}
	Archive& operator<<(AssetInfo v) {
		*this << v.crc << v.data << v.uc_data << v.size;
		return *this;
	}
	Archive& operator>>(AssetDiff& v) {
		*this >> v.old_crc >> v.new_crc >> v.is_patch >> v.data;
		return *this;
	}
	Archive& operator<<(AssetDiff v) {
		*this << v.old_crc << v.new_crc << v.is_patch << v.data;
		return *this;
	}
	Archive& operator>>(AssetHash& v) {
		*this >> v.dep_map >> v.size;
		return *this;
	}
	Archive& operator<<(const AssetHash& v) {
		*this << v.dep_map << v.size;
		return *this;
	}
	Archive& operator>>(GameInfo& v) {
		*this >> v.GameID >> v.directories >> v.fnhash_to_datahash >> v.datahash_to_assethash >> v.loadOnStart >> v.blackList;
		return *this;
	}
	Archive& operator<<(const GameInfo& v) {
		*this << v.GameID << v.directories << v.fnhash_to_datahash << v.datahash_to_assethash << v.loadOnStart << v.blackList;
		return *this;
	}
	Archive& operator>>(GameInfoDebug& v) {
		*this >> v.GameID >> v.GameName >> v.GameRuntime >> v.fnhash_to_fn >> v.datahash_to_fnhash;
		return *this;
	}
	Archive& operator<<(const GameInfoDebug& v) {
		*this << v.GameID << v.GameName << v.GameRuntime << v.fnhash_to_fn << v.datahash_to_fnhash;
		return *this;
	}
	Archive& operator>>(Asset& v) {
		*this >> v.uc_size >> v.crc >> v.data;
		return *this;
	}
	Archive& operator<<(Asset v) {
		*this << v.uc_size << v.crc << v.data;
		return *this;
	}
	Archive& operator>>(bool& v) {
		read((char*)&v, sizeof(bool));
		return *this;
	}
	Archive& operator<<(bool v) {
		write((const char*)&v, sizeof(bool));
		return *this;
	}
	Archive& operator>>(uint8_t& v) {
		read((char*)&v, sizeof(uint8_t));
		return *this;
	}
	Archive& operator<<(uint8_t v) {
		write((const char*)&v, sizeof(uint8_t));
		return *this;
	}
	Archive& operator>>(uint16_t& v) {
		read((char*)&v, sizeof(uint16_t));
		return *this;
	}
	Archive& operator<<(uint16_t v) {
		write((const char*)&v, sizeof(uint16_t));
		return *this;
	}
	Archive& operator>>(uint32_t& v) {
		read((char*)&v, sizeof(uint32_t));
		return *this;
	}
	Archive& operator<<(uint32_t v) {
		write((const char*)&v, sizeof(uint32_t));
		return *this;
	}
	Archive& operator>>(uint64_t& v) {
		read((char*)&v, sizeof(uint64_t));
		return *this;
	}
	Archive& operator<<(uint64_t v) {
		write((const char*)&v, sizeof(uint64_t));
		return *this;
	}
	Archive& operator>>(float& v) {
		read((char*)&v, sizeof(float));
		return *this;
	}
	Archive& operator<<(float v) {
		write((const char*)&v, sizeof(float));
		return *this;
	}
	Archive& operator>>(double& v) {
		read((char*)&v, sizeof(double));
		return *this;
	}
	Archive& operator<<(double v) {
		write((const char*)&v, sizeof(double));
		return *this;
	}
	/*Archive& operator>(uint32_t& v) {
		read_compressed((char*)&v, sizeof(uint32_t));
		return *this;
	}
	const Archive& operator<(uint32_t v) {
		write_compressed((const char*)&v, sizeof(uint32_t));
		return *this;
	}*/
	Archive& operator>>(std::string& v) {
		if (corrupt) return *this;
		//VerifyType(ARCHIVE_STRING);
		uint32_t len;
		*this >> len;
		if (len > (1100 << 20)) { SetCorrupt(); return *this; }
		v.resize(len);
		read((char*)v.data(), len);
		/*char buffer[4096];
		uint32_t toRead = len;
		while (toRead != 0) {
			uint32_t l = std::min(toRead, (uint32_t)sizeof(buffer));
			read(buffer, l);
			v += std::string(buffer, l);
			toRead -= l;
		}*/
		return *this;
	}
	Archive& operator<<(const std::string& v) {
		uint32_t size2 = (uint32_t)v.size();
		*this << size2;
		if (size2 > (200 << 20)) { SetCorrupt(); return *this; }
		write((const char*)v.data(), size2);
		return *this;
	}
	/*Archive& operator>(std::string& v) {
		if (corrupt) return *this;
		//VerifyType(ARCHIVE_STRING);
		uint32_t len;
		*this > len;
		if (len > 100000000) { SetCorrupt(); return *this; }
		v.resize(len);
		read_compressed((char*)v.data(), len);
		return *this;
	}
	const Archive& operator<(const std::string& v) {
		uint32_t size2 = v.size();
		*this < size2;
		write_compressed((const char*)v.data(), size2);
		return *this;
	}*/
	template <class T>
	Archive& operator>>(std::vector<T>& v) {
		if (corrupt) return *this;
		uint32_t size2 = 0;
		*this >> size2;
		if (size2 > 1000000) { SetCorrupt(); return *this; }
		v.resize(size2);
		for (auto& _v : v)
			*this >> _v;
		return *this;
	}
	template <class T>
	Archive& operator<<(const std::vector<T>& v) {
		uint32_t size2 = v.size();
		*this << size2;
		for (auto& _v : v)
			*this << _v;
		return *this;
	}
	template <class T, class T2>
	Archive& operator>>(HashT<T, T2>& v) {
		if (corrupt) return *this;
		uint32_t size2 = 0;
		*this >> size2;
		for (uint32_t i = 0; i < size2; ++i) {
			std::pair<T, T2> value;
			*this >> value;
			v.insert(v.end(), value);
		}
		return *this;
	}
	template <class T, class T2>
	Archive& operator<<(const HashT<T, T2>& v) {
		uint32_t size2 = v.size();
		*this << size2;
		for (auto& _v : v)
			*this << _v;
		return *this;
	}
	template <class T>
	Archive& operator>>(SetT<T>& v) {
		if (corrupt) return *this;
		uint32_t size2 = 0;
		*this >> size2;
		for (uint32_t i = 0; i < size2; ++i) {
			T value;
			*this >> value;
			v.insert(v.end(), value);
		}
		return *this;
	}
	template <class T>
	Archive& operator<<(const SetT<T>& v) {
		uint32_t size2 = v.size();
		*this << size2;
		for (auto& _v : v)
			*this << _v;
		return *this;
	}
	template <class T, class T2>
	Archive& operator>>(std::pair<T, T2>& v) {
		*this >> v.first;
		*this >> v.second;
		return *this;
	}
	template <class T, class T2>
	Archive& operator<<(const std::pair<T, T2>& v) {
		*this << v.first;
		*this << v.second;
		return *this;
	}
};