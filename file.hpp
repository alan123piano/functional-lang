#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>

struct File {
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

	File(std::istream& is, const std::string& filepath = "") : filepath(filepath) {
		std::string line;
		while (std::getline(is, line)) {
			lines.push_back(std::move(line));
		}
	}

	void report_error(int line, int col, int len, const std::string& error) const {
		errors.push_back({line, col, len, error});
	}

	void report_error_at_eof(const std::string& error) const {
		errors.push_back({(int)lines.size()-1, (int)lines[lines.size()-1].size(), 1, error});
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
				os << error.line << " " << error.col << " " << error.len << " " << error.error << std::endl;
				throw std::runtime_error("Invalid position: line=" + std::to_string(error.line));
			}
			if (filepath != "") {
				os << filepath << ":";
			}
			os << (error.line + 1) << ":" << error.col << ": error: " << error.error << '\n';
			os << " " << lines[error.line] << '\n';
			os << " " << std::string(error.col, ' ') << "^";
			if (error.len > 1) {
				os << std::string(error.len - 1, '~');
			}
			os << std::endl;
		}
	}

private:
	mutable std::vector<Error> errors;
	std::string filepath;
};
