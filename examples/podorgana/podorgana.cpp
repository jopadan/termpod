#include <cstdlib>
#include <cstdio>
#include <getopt.h>
#include <termpod/pod.hpp>

bool extract = false;
void parse_args(int argc, char** argv);

int main(int argc, char** argv)
{
	parse_args(argc, argv);
	if(optind < argc)
	{
		tr::pod3::file src(argv[optind]);
		if(extract)
			for(size_t i = 0; i < src.entries.size(); i++)
				src[i].extract();
	}
	exit(EXIT_SUCCESS);
}

void parse_args(int argc, char** argv)
{
	int opt;

	while((opt = getopt(argc, argv, "xa")) != -1) {
		switch(opt) {
			case 'x':
				extract = true;
				break;
			case 'a':
				tr::pod::type::audit::visible = true;
				break;
			default:
				fprintf(stderr, "Usage: %s [-x] [-a] [POD6FILE]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	if(optind >= argc)
	{
		fprintf(stderr, "Usage: %s [-x] [-a] [POD6FILE]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	printf("\n");
}
