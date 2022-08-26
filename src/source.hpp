#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>

struct Source {
private:
	struct Error {
		int line;
		int col;
		int len;
		std::string error;
		bool operator==(const Error& other) {
			return line == other.line
			       && col == other.col
			       && len == other.len
			       && error == other.error;
		}
	};

public:
	std::vector<std::string> lines;

	Source(std::istream& is, std::string filepath = "") : filepath(std::move(filepath)) {
		std::string line;
		while (std::getline(is, line)) {
			lines.push_back(std::move(line));
		}
		if (lines.empty()) {
			throw std::runtime_error("Expected non-empty input");
		}
	}

	bool has_errors() {
		return !errors.empty();
	}

	void report_error(int line, int col, int len, std::string error) const {
		errors.push_back({line, col, len, std::move(error)});
	}

	void emit_errors(std::ostream& os) const {
		// sort errors
		std::sort(errors.begin(), errors.end(), [](const Error& a, const Error& b) -> bool {
			if (a.line == b.line) {
				if (a.col == b.col) {
					if (a.len == b.len) {
						return a.error < b.error;
					}
					return a.len < b.len;
				}
				return a.col < b.col;
			}
			return a.line < b.line;
		});

		// remove duplicates
		errors.erase(std::unique(errors.begin(), errors.end()), errors.end());

		// emit errors
		for (const Error& error : errors) {
			if (error.line >= lines.size()) {
				throw std::runtime_error("Invalid position: line=" + std::to_string(error.line));
			}
			if (filepath != "") {
				os << filepath << ":";
			}
			os << (error.line + 1) << ":" << error.col << ": error: " << error.error << '\n';

			// print source in output stream
			std::string line = lstrip(lines[error.line]);
			int wspaceDiff = (int)lines[error.line].size() - (int)line.size();
			os << left_pad(std::to_string(error.line + 1), 5)
			   << " |  " << line << '\n';
			os << std::string(5, ' ')
			   << " |  " << std::string(error.col - wspaceDiff, ' ') << "^";
			if (error.len > 1) {
				os << std::string(error.len - 1, '~');
			}

			os << std::endl;
		}
	}

private:
	mutable std::vector<Error> errors;
	std::string filepath;

	static std::string left_pad(const std::string& str, int len, char pad = ' ') {
		return std::string(std::max(0, len - (int)str.size()), pad) + str;
	}

	static std::string lstrip(const std::string& str) {
		// strip leading whitespace
		int i = 0;
		while (i < str.size() && isspace(str[i])) {
			++i;
		}
		return str.substr(i);
	}
};

struct Location {
	const Source* source = nullptr;
	int line;
	int colStart;
	int colEnd;
};
