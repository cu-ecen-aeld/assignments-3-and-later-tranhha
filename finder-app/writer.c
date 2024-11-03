#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
int main(int argc, char *argv[])
{
	/*Initiate syslog*/
	openlog(NULL, 0, LOG_USER);
	
	/*Check if 2 arguments are provided*/	
	if (argc != 3)
	{
		fprintf(stderr, "Error: Please enter 2 arguments <writefile> and <writestr>\n");
		syslog(LOG_DEBUG, "Error: 2 arguments <writefile> and <writestr> were not provided\n");
		return 1;
	}

	//Get the arguments for path and message to be wrriten
	char* path = argv[1];
	char* text = argv[2];
	FILE *file = fopen(path, "w");
	if (file == NULL)
	{
		fprintf(stderr, "Error opening file\n");
		syslog(LOG_ERR, "Error opening file %s\n", path);
		return EXIT_FAILURE;
	}

	
	//Write message 
	fprintf(file, "%s\n", text);
	fclose(file);

	//syslog
	syslog(LOG_DEBUG, "Writing %s to %s.\n", text, path);
	printf("Write successfully\n");
	return 0;
}
