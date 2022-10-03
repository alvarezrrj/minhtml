// HTML file minifier - removes newlines, tabs and comments.

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include <stdlib.h>
#include <stdbool.h>

char *fslash(char *fpath);
char *ofname(char *path, char *infilename, bool *f);

int main(int argc, char *argv[])
{
	// Check for correct amount of args
	if (argc != 2 && argc != 3)
	{
		printf("Usage: minhtml <input> [<out dir>[/<out file>]]\n");
		printf("Minfy html file into stdout or <out dir>\n");
		return 1;
	}

	char *infilename;
	FILE *outptr;
	bool isheap = false; 	// Keeps track of weather malloc was used in ofname()

	// work out input filename from file path
	char *slash = fslash(argv[1]);
	// if argv[1] contains '/'
	if (slash != NULL)
	{
		infilename = slash + 1;
	}
	else 
	{
		infilename = argv[1];
	}

	// Open input file
	FILE *inptr = fopen(argv[1], "r");
	if (inptr == NULL)
	{
		printf("Could not open %s\n", infilename);
		return 2;
	}
	
	// define output file pointer
	if (argc == 2)
	{
		outptr = stdout;
	}
	else
	{
		char *ofn = ofname(argv[2], infilename, &isheap);
		// Open output file
		outptr = fopen(ofn, "w");
		// If malloc was used in ofname, free ofn
		if (isheap) free(ofn);
		if (outptr == NULL)
		{
			printf("Could not create output file: %s\n", ofn);
			return 3;
		}
	}

	char buff;
	char bigbuff[3];
	int csize = sizeof(char);
	// Read input file
	while (fread(&buff, csize, 1, inptr))
	{
		// If char is newline or tab, do nothing
		if (buff == '\n' || buff == '\t') continue;
		else if (buff == '<')
		{
			fread(&bigbuff[0], csize, 3, inptr);
			if (bigbuff[0] == '!' && bigbuff[1] == '-' && bigbuff[2] == '-')
			{
				do 
				{
					// Read three chars
					fread(&bigbuff[0], csize, 3, inptr);
					// Move pointer back two chars
					fseek(inptr, -2 * csize, SEEK_CUR);
				}
				// While read chars not equal "-->"
				while (!(bigbuff[0] == '-' && bigbuff[1] == '-' && bigbuff[2] == '>'));
				
				// Return pointer end of closing comment
				fseek(inptr, 2 * csize, SEEK_CUR);
			}
			else 
			{
				// If those 4 chars != "<!--", write them.
				fwrite(&buff, csize, 1, outptr);
				fwrite(&bigbuff, csize, 3, outptr);
			}
		}
		else
		{
			fwrite(&buff, csize, 1, outptr);
		}			
	}
	
	fclose(inptr);
	fclose(outptr);
}




// Slash finder function - returns pointer to last slash on a string
char *fslash(char *fpath)
{
	// Find a slash
	char *sptr = memmem(fpath, strlen(fpath), "/", sizeof(char));
	// If no slash, return null
	if (sptr == NULL) return NULL;
	// If no more slashes, return this slash
	else if (memmem(sptr + 1, strlen(sptr + 1), "/", sizeof(char)) == NULL) return sptr;
	// Else, keep looking recursively
	else return fslash(sptr + 1);
}

// Out file name composer
char *ofname(char *path, char *infilename, bool *f)
{
	// If slashes are found in string
	char *outslash = fslash(path);
	if (outslash != NULL)
	{
		// And there is a string after the last slash
		if (*(outslash + 1) != '\0')
		{
			return path;
		}
		else
		{
			// Concat out file path and in file name
			char *outfilename = malloc(strlen(path) + strlen(infilename) + 1);
			*f = true;
			strcat(outfilename, path);
			return strcat(outfilename, infilename);
		}
	}
	else
	{
		char *outfilename = malloc(strlen(path) + strlen(infilename) + 2);
		*f = true;
		// Add a slash into the mix
		strcat(outfilename, path);
		strcat(outfilename, "/");
		return strcat(outfilename, infilename);
	}
}
