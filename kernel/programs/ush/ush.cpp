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

int main(int /*argc*/, char** /*argv*/) {
	while (true) {
		printf("pi$ ");
		char buffer[1024];
		getline(buffer, 1024);
		ExpArray<char*> split = split_string(buffer, ' ');
		if (split.empty()) continue;
		execution_context ec;
		ec.stdin = get_stdin();
		ec.stdout = get_stdout();
		ec.argc = split.size();
		ec.argv = (char**)(malloc(ec.argc * sizeof(char*)));
		for (int i = 0; i < ec.argc; i++) {
			ec.argv[i] = split[i];
		}
		int file = find_file(ec.argv[0]);
		int child = execute_file(file, &ec);
		if (child >= 0) {
			int return_value = wait(child);
			printf("Child exited with code %d.\n", return_value);
		} else {
			printf("Failed to run file %s.", ec.argv[0]);
		}
	}
}
