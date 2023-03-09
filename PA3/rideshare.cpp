#include <iostream>
#include <string>
#include <vector>
#include <semaphore.h>
#include <pthread.h>

using namespace std;

int* wait_A;             // Stores the waiting A fans
int* wait_B;             // Stores the waiting B fans
sem_t full;              // Wakes/Awakes fans that come after car is formed
sem_t* sem_A;            // Wakes/Awakes A fans
sem_t* sem_B;            // Wakes/Awakes B fans
pthread_mutex_t print;   // Mutex for printing
pthread_mutex_t exec;    // Mutex for executing
pthread_barrier_t wait;  // Barrier fro waiting all of the threads join the car so that captain can print


void* func(void* arg){
	char team = *((char*)arg); // Get which fan given
	bool captain = false;
	bool should_post = true;
	int* currentTeamAmount;
	int* otherTeamAmount;
	sem_t* currentTeamSem;
	sem_t* otherTeamSem;

	sem_wait(&full);           // Wait if there is enough fans

	pthread_mutex_lock(&print);
	cout<<"Thread ID: "<<pthread_self()<<", Team: "<<team<<", I am looking for a car"<<endl;
	pthread_mutex_unlock(&print);

	if(team == 'A'){           // Get the corresponding semaphores and counts
		currentTeamAmount = wait_A;
		otherTeamAmount   = wait_B;
		currentTeamSem    = sem_A;
		otherTeamSem      = sem_B;
	}
	else{
		currentTeamAmount = wait_B;
		otherTeamAmount   = wait_A;
		currentTeamSem    = sem_B;
		otherTeamSem      = sem_A;
	}
	pthread_mutex_lock(&exec);
	*currentTeamAmount=(*currentTeamAmount+1);  // Increase current team count by 1
	if(*currentTeamAmount+*otherTeamAmount==5){ // If there are 5 fans this thread should not post
		should_post=false;
	}

	if(*currentTeamAmount == 4){ // If current team reached 4 
		captain = true;
		*currentTeamAmount = (*currentTeamAmount-4);
		// Get 3 current team fans to car by waking them
		sem_post(currentTeamSem);
		sem_post(currentTeamSem);
		sem_post(currentTeamSem);
		pthread_mutex_unlock(&exec);
	}
	else if(*currentTeamAmount == 2 && *otherTeamAmount >= 2){ // If current team reached 2 and other team is also 2 
		captain = true;
		*currentTeamAmount = (*currentTeamAmount-2);
		*otherTeamAmount   = (*otherTeamAmount-2);
		// Get 1 current team fan and 2 other team fans to car by waking them
		sem_post(currentTeamSem);
		sem_post(otherTeamSem);
		sem_post(otherTeamSem);
		pthread_mutex_unlock(&exec);
	}
	else if(*currentTeamAmount+*otherTeamAmount==4){ // If car is not full but reached 4 people
		sem_post(&full);
		pthread_mutex_unlock(&exec);
		sem_wait(currentTeamSem);
	}
	else{
		pthread_mutex_unlock(&exec); // If car is not full and not reached 4 people
		sem_wait(currentTeamSem);
	}

	pthread_mutex_lock(&print);
	cout<<"Thread ID: "<<pthread_self()<<", Team: "<<team<<", I have found a spot in a car"<<endl;
	pthread_mutex_unlock(&print);

	pthread_barrier_wait(&wait); // Wait all threads to get in the car before captain prints
	if(captain){
		cout<<"Thread ID: "<<pthread_self()<<", Team: "<<team<<", I am the captain and driving the car"<<endl;
	}

	pthread_barrier_wait(&wait); // Wait captain to print then get new fans
	if(should_post){
		sem_post(&full);
	}
	return NULL;
}

int main(int argc, char* argv[]){
	int countA = atoi(argv[1]);
	int countB = atoi(argv[2]);
	int countTotal = countA + countB;
	if (countTotal % 4 != 0 || countA % 2 != 0 || countB % 2 != 0){ // Check if the given inputs are correct
		cout <<"The main terminates"<<endl;
		return 0;
	}
	vector<char> RandTotalFans; // Randomly generate a vector with corresponding counts
	for (int i = 0; i < countTotal; i++){
		if(countA==0){
			RandTotalFans.push_back('B');
		}
		else if(countB==0){
			RandTotalFans.push_back('A');
		}
		else{
			int random=rand() % 2;
			if(random==0){
				RandTotalFans.push_back('A');
				countA--;
			}
			else {
				RandTotalFans.push_back('B');
				countB--;
			}
		}
   	}
   	//Initialize semaphores/counts/barrier
   	wait_A = (int*)malloc(sizeof(int));
	*wait_A=0;
	wait_B = (int*)malloc(sizeof(int));
	*wait_B=0;
   	sem_A = (sem_t*)malloc(sizeof(sem_t));
	sem_init(sem_A, 0,0);
   	sem_B = (sem_t*)malloc(sizeof(sem_t));
	sem_init(sem_B, 0,0);

	sem_init(&full, 0,4);
	pthread_barrier_init(&wait,NULL,4);
	pthread_t* threads = (pthread_t*) malloc(countTotal * sizeof(pthread_t));
	for (int i = 0; i < countTotal; i++){
		pthread_create(&(threads[i]), NULL, func, &RandTotalFans[i]);
	}
    for(int i=0;i<countTotal;i++){
		if(threads[i] != 0){
			pthread_join(threads[i], NULL);
		}
	}
    pthread_barrier_destroy(&wait); 
    cout <<"The main terminates"<<endl;

    return 0;
}
