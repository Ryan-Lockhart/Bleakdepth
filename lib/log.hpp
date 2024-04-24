#pragma once

#include "typedef.hpp"

#include <format>
#include <list>
#include <string>

namespace Bleakdepth {
	class log_t {
	  private:
		std::list<std::string> messages;
		usize maxMessages;

	  public:
		using iterator = std::list<std::string>::iterator;
		using const_iterator = std::list<std::string>::const_iterator;

		inline log_t(usize maxMessages = 16) : messages(), maxMessages(maxMessages) {}

		inline log_t(cref<log_t> other) noexcept : messages(other.messages), maxMessages(other.maxMessages) {}

		inline log_t(rval<log_t> other) noexcept :
			messages(std::move(other.messages)),
			maxMessages(std::move(other.maxMessages)) {}

		inline ref<log_t> operator=(cref<log_t> other) noexcept {
			messages = other.messages;
			maxMessages = other.maxMessages;

			return *this;
		}

		inline ref<log_t> operator=(rval<log_t> other) noexcept {
			messages = std::move(other.messages);
			maxMessages = std::move(other.maxMessages);

			return *this;
		}

		inline iterator begin() { return messages.begin(); }

		inline iterator end() { return messages.end(); }

		inline const_iterator begin() const { return messages.begin(); }

		inline const_iterator end() const { return messages.end(); }

		inline iterator operator[](usize index) { return std::next(messages.begin(), index); }

		inline const_iterator operator[](usize index) const { return std::next(messages.begin(), index); }

		inline ref<std::string> at(usize index) {
			if (index >= messages.size()) {
				throw(std::out_of_range("Index out of range"));
			}

			return *std::next(messages.begin(), index);
		}

		inline cref<std::string> at(usize index) const {
			if (index >= messages.size()) {
				throw(std::out_of_range("Index out of range"));
			}

			return *std::next(messages.begin(), index);
		}

		inline usize size() const { return messages.size(); }

		inline usize capacity() const { return maxMessages; }

		inline ref<std::string> front() { return messages.front(); }

		inline ref<std::string> back() { return messages.back(); }

		inline cref<std::string> front() const { return messages.front(); }

		inline cref<std::string> back() const { return messages.back(); }

		inline void prune() {
			if (maxMessages <= 0) {
				return;
			}

			while (messages.size() > maxMessages) {
				messages.pop_front();
			}
		}

		inline void prune(usize count) {
			if (count <= 0) {
				return;
			}

			while (messages.size() > count) {
				messages.pop_front();
			}
		}

		inline void clear() { messages.clear(); }

		inline void add(cstr message) { messages.push_back(message); }

		inline void add(std::string message) { messages.push_back(message); }

		inline void add(cstr message, cstr time, cstr file, usize line) {
			messages.push_back(std::format("[{}]: \"{}\" ({}): {}", time, file, line, message));
		}

		inline void add(std::string message, cstr time, cstr file, usize line) {
			messages.push_back(std::format("[{}]: \"{}\" ({}): {}", time, file, line, message));
		}

		template<typename... Args> inline void add(cstr format, Args... args) {
			messages.push_back(std::format(format, { args... }));
		}

		template<typename... Args> inline void add(std::string format, Args... args) {
			messages.push_back(std::format(format, { args... }));
		}

		template<typename... Args> inline void add(cstr format, cstr time, cstr file, usize line, Args... args) {
			messages.push_back(
				std::format(std::format("[{}]: \"{}\" ({}): {}", time, file, line, format).c_str(), { args... })
			);
		}

		template<typename... Args> inline void add(std::string format, cstr time, cstr file, usize line, Args... args) {
			messages.push_back(
				std::format(std::format("[{}]: \"{}\" ({}): {}", time, file, line, format).c_str(), { args... })
			);
		}

		inline operator std::string() const {
			std::string result;

			for (cref<std::string> message : messages) {
				result += message + "\n";
			}

			return result;
		}
	};
} // namespace Bleakdepth