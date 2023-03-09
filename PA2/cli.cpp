//	g++ -pthread -o output new_cli.cpp
//	./output

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <string.h>
using namespace std;

mutex print_lock;
ofstream parse ("parse.txt");
vector<thread> threads;

void * output(int fd0,string command,string inputs,string options,string redirection,string background,string line){
	print_lock.lock();
	if(redirection != ">"){
		cout << "---- "<< this_thread::get_id()<<" "<<line<<endl;
    	char buf[10000]; // Read and print pipe read end
    	read(fd0, buf, 10000);
    	printf("%s", buf);
    	cout << "---- "<< this_thread::get_id()<<endl;
    }
    parse << "----------"			<< endl;
    parse << "Command: "			<< command 		<< endl;
    parse << "Inputs: "				<< inputs 		<< endl;
    parse << "Options: "			<< options 		<< endl;
    parse << "Redirection: "		<< redirection	<< endl;
    parse << "Background Job: "		<< background	<< endl;
    parse << "----------"			<< endl;
    print_lock.unlock();
    return NULL;
}

int main(){
	string line,command;
	ifstream file;
	file.open("commands.txt");
	while (getline(file, line)){
		istringstream words(line);
		words>>command;
		if(command=="wait"){
			for(int i=0;i<threads.size();i++){
				if(threads[i].joinable()){
					threads[i].join();
				}
			}
			print_lock.lock();
			parse << "----------"			<< endl;
    		parse << "Command: "			<< command 		<< endl;
    		parse << "Inputs: "				<< endl;
    		parse << "Options: "			<< endl;
    		parse << "Redirection: "		<< "-" 			<< endl;
    		parse << "Background Job: "		<< "n" 			<< endl;
    		parse << "----------"			<< endl;
    		print_lock.unlock();
		}
		else{
			string word, inputs, options, filename;
			string redirection = "-";
			string background  = "n";
			words>>word;
			int count=2;
			while( word != "<" && word != ">" && word != "&" ){
				if(word.substr(0,1) == "-"){
					options += word + " ";
					count++;
				}
				else{
					inputs += word + " ";
					count++;
				}
				if(words >> word){
				}	
				else{
					break;
				}
			}
			if( word == "<" || word == ">"){
				redirection = word;
				words>>filename>>word;
			}

			if( word == "&"){
				background = "y";
			}
			string execline = command+" "+inputs+options;
			char * myargs[count];
			string argword="";
			int index=0;
			for (int i = 0; i < execline.size(); i++){
				if (execline[i] == ' '){
					myargs[index] = (char*)malloc(argword.length()+1);
					for (int i = 0; i<argword.length(); i++) {
						myargs[index][i] = argword[i];
					}

					myargs[index][argword.length()] = '\0';
					index++;
					argword="";
				}
				else{
					argword+=execline[i];
				}		
			}
			myargs[count-1]=NULL;
			int * fd =new int[2];
			pipe(fd);
			int outputfile,inputfile;

			if(redirection == ">"){ 					// If we need to write on another file
				outputfile =open(filename.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
			}
			else if(redirection == "<"){ 				// If we need to read from another file
				inputfile = open(filename.c_str(), O_RDONLY);
			}
			pid_t c_pid = fork();
			
			if (c_pid == -1) {
				fprintf(stderr, "fork failed\n");
                exit(1);
			}
			else if (c_pid == 0) { // Child process	
				close(fd[0]);
				if(redirection == ">"){ 				// If we need to write on another file
					dup2(outputfile,STDOUT_FILENO);		// Change STDOUT to outputfile
				}
				else if(redirection == "<"){ 			// If we need to read from another file
					dup2(inputfile,STDIN_FILENO); 		// Change STDIN to inputfile
					dup2(fd[1], STDOUT_FILENO); 		// Change STDOUT to pipe
				}
				else { 									// Change output to pipe to read it later
					dup2(fd[1], STDOUT_FILENO); 		// Change STDOUT to pipe
				}
				execvp(myargs[0],myargs);
			}
			else {
				//waitpid(c_pid,NULL,0); // wait child
				close(fd[1]);
				threads.push_back(thread (output, fd[0],command,inputs,options,redirection,background,line));
				if(background != "y"){
					if(threads[threads.size()-1].joinable()){
						threads[threads.size()-1].join();
					}
				}
				if(redirection == ">"){ // Close after output
					close(outputfile);
				}
				else if(redirection == "<"){
					close(inputfile);
				}
		    }
		}
	}
	for(int i=0;i<threads.size();i++){
		if(threads[i].joinable()){
			threads[i].join();
		}
	}
	parse.close();
	file.close();
	return 0;
}