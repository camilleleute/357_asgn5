
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "tree.h"
int aflag = 0, lflag = 0, status = EXIT_SUCCESS;

int integer(const char *string) {
	//printf("hiya\n");
	while (*string) {
		if (!isdigit(*string)) {
			return 0;
		}
		string++;
		}
	return 1;
}

int main(int argc, char * argv[]) {
	
	int opt, numLevels = INT_MAX;
	opterr = 0;
	char* path = ".";
	struct stat buff;
	char linker[PATH_MAX];

	while ((opt = getopt(argc, argv, "adl:")) != -1) {
		switch (opt) {
			case 'a':
				aflag = 1;
				break;
			case 'l':
				lflag = 1;
				break;
			case 'd':
				//printf("hi 1\n");
				if (!integer(optarg)) {
                    			fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");  
                   			 exit(EXIT_FAILURE);
                			}
				//printf("hi\n");
                		numLevels = atoi(optarg);
        			//	printf("after\n");
				
				if (numLevels < 0 ){
					fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");
					exit(EXIT_FAILURE);
				}
				break;
				//} else {
				//	printf("hi\n");
				//	fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");
                                //        exit(EXIT_FAILURE);
				//}
			default:
				fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");
                                exit(EXIT_FAILURE);
		}
	}
	printf("hi\n");
	if (argc != 2) {
		optind--;
	}
	path = argv[optind];
	if (lstat(path, &buff) == -1) {
		fprintf(stdout, "%s: %s\n", path, strerror(errno));
        	status = EXIT_FAILURE;
        	return status;
    	}
	if (aflag) {
        	permissions(&buff);
    	}

	printf("%s", path);

	if (S_ISLNK(buff.st_mode)) {
        	ssize_t length = readlink(path, linker, sizeof(linker) - 1);
        	if (length != -1) {
            		linker[length] = '\0';
            		printf(" -> %s\n", linker);
        	} else {
            		printf("\n");
        	}
        	return status;
    	}

	
	DIR *dir = opendir(path); 
    		if (!dir) {
        		if (errno == EACCES) {
            			printf(": %s\n", strerror(errno));
            			return EXIT_FAILURE;
        		}
    		} else {
        		closedir(dir);
    		}

    	printf("\n");

     	if (numLevels == 0) {
             return status;
        }
        printdir(path, 1, numLevels);                 
        return status;                             
	
}

void printdir(const char* path, int currLevel, int numLevels) {	
	if (currLevel > numLevels) {
		return;
	}
	DIR* dir = opendir(path);
	char **entries = NULL;
    	int count = 0;
    	int size = 0;
	int i = 0;
	if (!dir) {
		fprintf(stderr, "usage: ./tree [-a] [-d N] [-l] [PATH]\n");
                exit(EXIT_FAILURE);
	}	
	struct dirent *entry;
	while ((entry = readdir(dir))!= NULL) {
		if (!aflag && (entry->d_name[0] == '.')) continue;
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		
		if (count == size) {
            		size = size ? size * 2 : 16;
            		entries = realloc(entries, size * sizeof(char *));
        	}
        	entries[count++] = strdup(entry->d_name);
	}
		
	qsort(entries, count, sizeof(char *), compare);
    	processing(path, entries, count, currLevel, numLevels);
	
    	for (i = 0; i < count; i++) {
        	free(entries[i]);
    	}	
    	free(entries);
    	closedir(dir);

}

void processing(const char *path, char **entries, int count, int currLevel, int numLevels) {
	struct stat buff;
    	char new_path[PATH_MAX];
	int i = 0;
    	for (i = 0; i < count; i++) {
        	snprintf(new_path, PATH_MAX, "%s/%s", path, entries[i]);
        	if (lstat(new_path, &buff) == -1) {
            		fprintf(stdout, "%s: %s\n", new_path, strerror(errno));
            		status = EXIT_FAILURE;
            		return;
        }

        handle(new_path, entries[i], &buff,currLevel);

        if (S_ISDIR(buff.st_mode)) {
            printdir(new_path, currLevel + 1, numLevels);
        }
    }
}

int compare(const void *x, const void *y) {
    return strcmp(*(const char **)x, *(const char **)y);
}


void handle(const char *new_path, const char *entry, const struct stat *buff, int currLevel) {
    char link[PATH_MAX];
	int i = 0;
    if (!aflag) {
        for (i = 0; i < currLevel* 4; i++) {
            printf(" ");
        }
    } 
	if (aflag) {
        permissions(buff);

        for (i = 0; i < currLevel * 4; i++) {
            printf(" ");
        }

        if (S_ISLNK(buff->st_mode)) {
            ssize_t length = readlink(new_path, link, sizeof(link) - 1);
            if (length != -1) {
                link[length] = '\0';
                printf("%s -> %s", entry, link);
            } else {
                printf("%s", entry);
            }
        } else {
            printf("%s", entry);
        }
    } else {
        printf("%s", entry);
    }

    DIR *dir = opendir(new_path);
    if (!dir) {
        if (errno == EACCES) {
            printf(": Permission denied");
            status = EXIT_FAILURE;
        }
    } else {
        closedir(dir);
    }

    printf("\n");
}


void permissions(const struct stat *buff) {
	printf("%c", (S_ISDIR(buff->st_mode)) ? 'd' : (S_ISLNK(buff->st_mode)) ? 'l' : '-');
    	printf("%c", (buff->st_mode & S_IRUSR) ? 'r' : '-');
    	printf("%c", (buff->st_mode & S_IWUSR) ? 'w' : '-');
    	printf("%c", (buff->st_mode & S_IXUSR) ? 'x' : '-');
    	printf("%c", (buff->st_mode & S_IRGRP) ? 'r' : '-');
    	printf("%c", (buff->st_mode & S_IWGRP) ? 'w' : '-');
    	printf("%c", (buff->st_mode & S_IXGRP) ? 'x' : '-');
    	printf("%c", (buff->st_mode & S_IROTH) ? 'r' : '-');
    	printf("%c", (buff->st_mode & S_IWOTH) ? 'w' : '-');
    	printf("%c", (buff->st_mode & S_IXOTH) ? 'x' : '-');
    	printf(" ");

}
