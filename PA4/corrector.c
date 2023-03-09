#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct Person {
	char * gender;char * firstname;char * surname;
} Person;
Person **people;
int PersonCount = 1;

int CountLines(FILE *fptr) {
	int count = 0;
	char line[1024];  // buffer to hold each line of the file
	while (fgets(line, sizeof(line), fptr)) {
		count++;
	}
	return count;
}

void getDatabase(){
	FILE * fptr;
	char * line = NULL;
	size_t len = 0;
  	size_t read;
	fptr = fopen("database.txt", "r");
	if (fptr == NULL){printf("Could not open database.txt. Exiting the program."); exit(0);}
	char ch;
	PersonCount = CountLines(fptr);
	fclose(fptr);
	fptr = fopen("database.txt", "r");
	people = malloc(sizeof(Person) * (PersonCount+1));  
	int i = 0;
	while ((read = getline(&line, &len, fptr)) != -1) {
		char copy[strlen(line) + 1];
		strcpy(copy, line);
		people[i] = (Person*)malloc(sizeof(Person)); 
		char* gender =  strtok(copy, " ");
		char* firstname = strtok(NULL, " ");
		char* surname = strtok(NULL, " ");
		people[i]->gender = (char*)malloc(5);
		if (strcmp(gender, "m") == 0){
			people[i]->gender = "Mr.";
		}
		else{
			people[i]->gender = "Ms.";
		}
		people[i]->firstname = (char*)malloc(strlen(firstname) + 1 );
		strcpy(people[i]->firstname, firstname);
		people[i]->surname = (char*)malloc(strlen(surname) + 1);
		if (surname[strlen(surname)-1] == '\n'){
			surname[strlen(surname)-1] = '\0';
		}            
		strcpy(people[i]->surname, surname);
		i++;
	}
	fclose(fptr);
}

char *extension(const char *filename){
	char *ext = malloc(128 * sizeof(char)); 
	char copy[strlen(filename) + 1];
	strcpy(copy, filename);
	char *tok = strtok(copy, ".");
	while (tok != NULL) {
		strcpy(ext, tok);
		tok = strtok(NULL, ".");
	}
	return ext;
}

char* concatenate(const char *str1, const char *str2){
	char * str = malloc(strlen(str1) + strlen(str2) + 1);
	strcpy(str, str1);
	strcat(str, str2);
	return str;
}

void corrector(char * path){
	DIR * currdir = opendir(path);
	struct dirent * d;
	char word[100];
	while ((d = readdir(currdir)) != NULL){
		char * filename = concatenate(path,d->d_name);
		if((!(strcmp(d->d_name, ".") == 0 ||
			strcmp(d->d_name, "..") == 0 ||
			strcmp(filename, "./database.txt") == 0)) &&
			strcmp(extension(d->d_name), "txt") == 0){// Check if we should read the file
			FILE * fd = fopen(filename, "r+");
			if (fd == NULL){ printf("Could not open this file: %s\n", filename); return;}
			else{
				while(fscanf(fd, "%s", word) > 0){
					char* temp;
					long filePos = ftell(fd);  // Get the current position in the file
					for (int i = 0; i < PersonCount; i++) {
						if (strcmp(people[i]->firstname, word) == 0){
							int wordPos = ftell(fd) - strlen(word);
							fseek(fd, wordPos - 4, SEEK_SET); // Move the file pointer to the position before the first name
							fputs(people[i]->gender, fd); // Write the person's gender to the file before their first name
							fseek(fd, wordPos + strlen(word) + 1, SEEK_SET); // Move the file pointer to the position after the first name
							fputs(people[i]->surname, fd); // Write the surname to the file
						}
					}
					filePos = ftell(fd);  // Update the current position in the file
				}
			}
			fclose(fd);
		}
		else if (strcmp(d->d_name, "..") != 0 && strcmp(d->d_name, ".") != 0 && strcmp(filename, "./database.txt") != 0){ // If the current entry is a directory (but not '.' or '..'), call this function recursively on that directory
			char * dir;
			if (path[strlen(path)-2] == '/'){
				dir = concatenate(path,d->d_name);
			}  
			else{
				dir = concatenate(concatenate(path,d->d_name),"/");
			}
			DIR * checkDir = opendir(dir);
			if (checkDir != 0){
				corrector(dir);
			}
			closedir(checkDir);
		}
	}
	closedir(currdir);
}

int main(int argc, char* argv[]){
	getDatabase();
	corrector("./");
	return 0;
}