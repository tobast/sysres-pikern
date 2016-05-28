#include "syslib.h"
#include "proglib.h"

void getline(char* buffer, int max_length) {
	for (int i = 0; i < max_length - 1; i++) {
		char c = getchar();
		if (c == '\n' || c == '\r') {
			buffer[i] = '\0';
			return;
		}
		buffer[i] = c;
	}
	buffer[max_length - 1] = '\0';
}

bool str_eq(const char* str1, const char* str2) {
	int i = 0;
	while (true) {
		if (str1[i] != str2[i]) {
			return false;
		} else if (str1[i] == '\0') {
			return true;
		}
		i++;
	}
}

ExpArray<char*> split_string(char* string, char delim) {
	ExpArray<char*> split;
	int starti = 0;
	int i = 0;
	for (; string[i] != '\0'; i++) {
		if (string[i] == delim) {
			if (i == starti) {
				starti++;
				continue;
			}
			char* s = (char*)(malloc(i - starti + 1));
			for (int k = 0; k < i - starti; k++) {
				s[k] = string[starti + k];
			}
			s[i - starti] = '\0';
			split.push_back(s);
			starti = i + 1;
		}
	}
	if (i != starti) {
		char* s = (char*)(malloc(i - starti + 1));
		for (int k = 0; k < i - starti; k++) {
			s[k] = string[starti + k];
		}
		s[i - starti] = '\0';
		split.push_back(s);
	}
	return split;
}

int resolve_path(const char* path) {
	int file = find_file(path);
	if (file != 0) {
		return file;
	}
	const char* prefix = "/bin/";
	int prefix_len = str_len(prefix);
	int len = prefix_len + str_len(path);
	char* cc = (char*)(malloc(len + 1));
	for (int i = 0; i < prefix_len; i++) {
		cc[i] = prefix[i];
	}
	for (int i = prefix_len; i < len; i++) {
		cc[i] = path[i - prefix_len];
	}
	cc[len] = '\0';
	int result = find_file(cc);
	free(cc);
	return result;
}

int main(int /*argc*/, char** /*argv*/) {
	while (true) {
		printf("pi$ ");
		char buffer[1024];
		getline(buffer, 1024);
		ExpArray<char*> split = split_string(buffer, ' ');
		if (split.empty()) continue;
		if (str_eq(split, "cd")) {
			int new_cwd;
			if (split.length() < 2) {
				new_cwd = find_file("/");
			} else {
				new_cwd = find_file(split[1]);
			}
			if (new_cwd != 0) {
				set_cwd(new_cwd);
			}
			continue;
		}
		execution_context ec;
		ec.stdin = get_stdin();
		ec.stdout = get_stdout();
		ec.argc = split.size();
		ec.argv = (char**)(malloc(ec.argc * sizeof(char*)));
		ec.cwd = 0; // Use our cwd.
		for (int i = 0; i < ec.argc; i++) {
			ec.argv[i] = split[i];
		}
		int file = resolve_path(ec.argv[0]);
		int child = execute_file(file, &ec);
		if (child >= 0) {
			int return_value = wait(child);
			printf("Child exited with code %d.\n", return_value);
		} else {
			printf("Failed to run file %s.\n", ec.argv[0]);
		}
	}
}

