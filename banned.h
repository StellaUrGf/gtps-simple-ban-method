``cpp
#ifndef _BANNED_DIRECTORY
// make sure to change your banned directory
#define _BANNED_DIRECTORY "banned.bin"
#endif // !_BANNED_DIRECTORY

#include <iostream>
#include <chrono>
#include <string>
#include <stdint.h>
#include <cstring>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <fstream>

namespace utility
{
	class banned_s {
	public:
		void add_ban(const std::string& id, std::chrono::seconds time) {
			if (id.size() <= 0x00 || id.empty())
				throw std::invalid_argument("id cannot be empty");
			auto now = std::chrono::system_clock::now();
			m_banned[id] = now + time;
		}
		void remove_ban(const std::string& id) {
			m_banned.erase(id);
		}
		bool is_banned(const std::string& id) {
			if (auto pair = m_banned.find(id); pair != m_banned.end()) {
				auto now = std::chrono::system_clock::now();
				if (pair->second < now)
					return true;
				else {
					m_banned.erase(pair);
					return false;
				}
			} return false;
		}
		const int64_t get_duration(std::chrono::system_clock::time_point time) noexcept {
			return std::chrono::duration_cast<
				std::chrono::seconds>(std::chrono::system_clock::time_point(time).time_since_epoch()).count();
		}
		void serialize(const std::string& path = _BANNED_DIRECTORY) {
			uintmax_t size = sizeof(int64_t) + sizeof(uint16_t) * m_banned.size();
			for (const auto [name, duration] : m_banned)
				size += name.size();
			std::unique_ptr<uint8_t> data(new uint8_t[size]);
			size_t mempos = 0x000;
			std::memcpy(data.get() + mempos, &size, 0x8);
			mempos += 0x8;
			for (const auto& [name, duration] : m_banned) {
				uint16_t lens = (uint16_t)name.size();
				std::memcpy(data.get() + mempos, &lens, sizeof(uint16_t));
				mempos += sizeof(uint16_t);
				std::memcpy(data.get() + mempos, name.c_str(), lens);
				mempos += lens;
				const int64_t time = get_duration(duration);
				std::memcpy(data.get() + mempos, &time, 0x8);
				mempos += 0x8;
			}
			std::ofstream ofs(path, std::ios::binary);
			if (ofs.fail())
				throw std::exception("failed to open directory");
			ofs.write(reinterpret_cast<char*>(data.get()), mempos);
			ofs.flush();
			ofs.close();
		}
		void deserialize(const std::string& path = _BANNED_DIRECTORY) {
			std::ifstream ifs(path, std::ios::binary | std::ios::ate);
			if (!ifs.is_open())
				throw std::exception("failed to open file directory");
			uintmax_t size = static_cast<uintmax_t>(ifs.tellg());
			ifs.seekg(0x0, std::ios::beg);
			std::unique_ptr<uint8_t> buffer(new uint8_t[size]);
			ifs.read((char*)buffer.get(), size);
			ifs.close();
			size_t mempos = 0x0, count;
			count = *reinterpret_cast<size_t*>(buffer.get() + mempos);
			mempos += sizeof(size_t);
			for (size_t i = 0; i < count; i++) {
				uint16_t lens = *(uint16_t*)&buffer.get()[mempos];
				mempos += sizeof(uint16_t);
				std::string name((char*)buffer.get() + mempos, lens);
				mempos += lens;
				const int64_t time = *reinterpret_cast<int64_t*>(buffer.get() + mempos);
				mempos += 0x8;
				m_banned[name] = std::chrono::system_clock::time_point(std::chrono::seconds(time));
			}
		}
		const std::unordered_map<std::string,
			std::chrono::system_clock::time_point> get() {
			return m_banned;
		}
	private:
		std::unordered_map<std::string, 
			std::chrono::system_clock::time_point> m_banned;
	};
}
#endif // !_BANNED_METHOD_H_
``
