#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

static bool is_whitespace(char ch)
{
	const char *whitespace_chars = " \t\n\r";
	return (strchr(whitespace_chars, ch) != NULL);
}

static bool is_special(char ch)
{
	const char *special_chars = "{};.[]<>*,()#+=";
	return (strchr(special_chars, ch) != NULL);
}

enum {
	TOKEN_FLAG_double_quote = 1 << 0,
	TOKEN_FLAG_single_quote = 1 << 1,
	TOKEN_FLAG_forward_slash_escape = 1 << 2,
	TOKEN_FLAG_double_slash_comment = 1 << 3,
};

int get_token(FILE *input, char *token, int max_len, uint32_t flags)
{
	int in_string = 0;
	int pos = 0;
	bool escape_char = false;

	// TODO: Support comments: '//', or '/*', or '#'...

	if (feof(input))
		return EOF;

	while (1) {
		// TODO: Support utf8
		int ch = fgetc(input);

		if (ch < 0)
			break;
		//printf("Got 0x%x. escape: %d in_string: %d pos: %d\n", ch, escape_char, in_string, pos);

		if (!escape_char) {
			if (is_whitespace(ch) && pos > 0 && !in_string)
				break;
			if (is_whitespace(ch) && pos == 0)
				continue;

			if (pos > 0 && is_special(ch) && !in_string) {
				ungetc(ch, input);
				break;
			}

			if ((flags & TOKEN_FLAG_double_quote) && ch == '"' && in_string == 0)
				in_string = ch;
			else if ((flags & TOKEN_FLAG_single_quote) && ch == '\'' && in_string == 0)
				in_string = ch;
			else if (in_string == ch)
				in_string = 0;
		}

		if (pos < max_len)
			token[pos++] = ch;
		if (is_special(ch) && !in_string)
			break;
		if (!escape_char)
			escape_char = (flags & TOKEN_FLAG_forward_slash_escape) && (ch == '\\');
		else
			escape_char = false;
	}

	token[pos] = '\0';
	return pos;
}


int main(int argc, const char *argv[])
{
	FILE *fp;

	if (argc < 2)
		return -1;

	fp = fopen(argv[1], "rb");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	int count = 0;
	while (1) {
		char token[256];
		int e = get_token(fp, token, sizeof(token), TOKEN_FLAG_double_quote | TOKEN_FLAG_single_quote | TOKEN_FLAG_forward_slash_escape);
		if (e == 0)
			break;
		if (e < 0) {
			fprintf(stderr, "Failure: %d\n", e);
			break;
		}
		printf("Token %d: '%s'[%d]\n", count++, token, e);
	}

	fclose(fp);
	return 0;
}

