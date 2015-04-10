#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

typedef struct{
	int id;
	int arrivalTime;
	int burstTime;//called duration in input
	int startTime;
	int finishTime;
	int timeRemaining;
	}Process;

int quantum;
int cores;

//low to high
int compProcByArrival(const void * elem1, const void * elem2)
{
	int t1 = ((Process*)elem1)->arrivalTime;
	int t2 = ((Process*)elem2)->arrivalTime;
	if(t1>t2) return 1;
	if(t1<t2) return -1;
	return 0;//==
}

//low to high
int compProcById(const void * elem1, const void * elem2)
{
	int t1 = ((Process*)elem1)->id;
	int t2 = ((Process*)elem2)->id;
	if(t1>t2) return 1;
	if(t1<t2) return -1;
	return 0;//==
}

int compRand(const void * elem1, const void * elem2)
{
	int r = rand()%3-1;
	return r;
}

float calcAvgTurnaround(Process* allProceses, int num)
{
	int sum = 0;
	for(int i = 0 ; i < num; i++)
	{
		int wait = allProceses[i].finishTime;
		sum+=wait;
	}
	return ((float)sum)/num;
}

float calcAvgWait(Process* allProceses, int num)
{
	int sum = 0;
	for(int i = 0 ; i < num; i++)
	{
		int wait = allProceses[i].finishTime-allProceses[i].burstTime-allProceses[i].arrivalTime;
		sum+=wait;
	}
	return ((float)sum)/num;
}

//each scheduler needs to sort the processes in the way it wants
int runFCFS_Single(Process* allProceses, int num)
{	
	//sort by arrival time
	qsort(allProceses,num,sizeof(Process),compProcByArrival);
	
	int time = 0;
	
	for(int i = 0 ; i < num; i++)
	{
		//catch up if needed
		if(time < allProceses[i].arrivalTime)
			time = allProceses[i].arrivalTime;
		
		//set start
		allProceses[i].startTime = time;
		
		//increment time
		time += allProceses[i].burstTime;
		
		//set end
		allProceses[i].finishTime = time;
	}
	return time;
}

int runFCFS_Percore(Process* allProceses, int num)
{
	//sort randomly
	qsort(allProceses,num,sizeof(Process),compRand);
	
	int maxTime = -1;
	
	int coreSize = (int)(num/cores);//underestimate
	for(int i = 0; i < cores;i++)
	{
		int startPos = coreSize*i;
		Process* startProcess = allProceses+startPos;
		int thisCoreSize = coreSize;
		if(i==cores-1) 
			thisCoreSize = num - ((i)*coreSize);//get the rest if uneven
		int t = runFCFS_Single(startProcess, thisCoreSize);
		
		printf("running FCFS on %d-%d\n",startPos,startPos+thisCoreSize-1);
		
		if(t>maxTime)maxTime=t;
	}
	return maxTime;
}

int runRR_Single(Process* allProceses, int num)
{
	int time = 0;
	if(quantum > 0)
	{
		
		for(int i = 0 ; i < num; i++)
		{
			//make sure time remaining is accurate
			allProceses[i].timeRemaining = allProceses[i].burstTime;
			
			//make sure time remaining is invalid
			allProceses[i].startTime=-1;
			
		}
		
		int ranSomething = 1;	
		int timeToEnd = 0;
		
		while(!timeToEnd)
		{
			ranSomething=0;
			for(int i = 0 ; i < num; i++)
			{
				//if the process still needs time
				if(allProceses[i].timeRemaining && time>=allProceses[i].arrivalTime)
				{
					ranSomething=1;
					
					if(allProceses[i].startTime < 0)
						allProceses[i].startTime=time;
					
					//reduce time remaining
					if(allProceses[i].timeRemaining >= quantum)
					{
						allProceses[i].timeRemaining-=quantum;
						time+=quantum;
						printf("Proc#:%d runs for:%d\n",allProceses[i].id,quantum);
					}
					else if(allProceses[i].timeRemaining < quantum)
					{
						time+=allProceses[i].timeRemaining;
						printf("Proc#:%d runs for:%d\n",allProceses[i].id,allProceses[i].timeRemaining);
						allProceses[i].timeRemaining=0;
					}
					
					//if we ended
					if(!allProceses[i].timeRemaining)
					{
						allProceses[i].finishTime=time;
						printf("Proc#:%d finised at:%d\n",allProceses[i].id,allProceses[i].finishTime);
					}
					
				}
			}
			
			//if we didnt run anything that loop, nothing has arrived
			if(!ranSomething)
			{
				int minArrivalTimePastCurrentTime = INT_MAX;
				for(int i = 0 ; i < num; i++)
				{
					if(allProceses[i].arrivalTime > time)
					{
						if(minArrivalTimePastCurrentTime>allProceses[i].arrivalTime)
						{
							minArrivalTimePastCurrentTime=allProceses[i].arrivalTime;
						}
					}
				}
				if(minArrivalTimePastCurrentTime < INT_MAX)
					time = minArrivalTimePastCurrentTime;
				else
					timeToEnd = 1;
			}
		}
		
	}
	else
		exit(1);
	printf("----overall Time:%d\n",time);
	return time;
}

int runRR_Percore(Process* allProceses, int num)
{
	//sort randomly
	qsort(allProceses,num,sizeof(Process),compRand);
	
	int maxTime = -1;
	
	int coreSize = (int)(num/cores);//underestimate
	for(int i = 0; i < cores;i++)
	{
		int startPos = coreSize*i;
		Process* startProcess = allProceses+startPos;
		int thisCoreSize = coreSize;
		if(i==cores-1) 
			thisCoreSize = num - ((i)*coreSize);//get the rest if uneven
		int t = runRR_Single(startProcess, thisCoreSize);
		
		printf("running RR on %d-%d\n",startPos,startPos+thisCoreSize-1);
		
		if(t>maxTime)maxTime=t;
	}
	return maxTime;
}

int runRR_SingleFromUntil(Process** allProceses, int num, int time, int endTime)
{
	
	if(quantum > 0)
	{
		int ranSomething = 1;	
		int timeToEnd = 0;
		while(!timeToEnd && time<endTime)
		{
			ranSomething=0;
			for(int i = 0 ; i < num; i++)
			{
				if(allProceses[i]==NULL)
					continue;
				
				//if the process still needs time
				if(allProceses[i]->timeRemaining && time>=allProceses[i]->arrivalTime)
				{
					ranSomething=1;
					
					if(allProceses[i]->startTime < 0)
						allProceses[i]->startTime=time;
					
					//reduce time remaining
					if(allProceses[i]->timeRemaining >= quantum)
					{
						allProceses[i]->timeRemaining-=quantum;
						time+=quantum;
						printf("Proc#:%d runs for:%d\n",allProceses[i]->id,quantum);
					}
					else if(allProceses[i]->timeRemaining < quantum)
					{
						time+=allProceses[i]->timeRemaining;
						printf("Proc#:%d runs for:%d\n",allProceses[i]->id,allProceses[i]->timeRemaining);
						allProceses[i]->timeRemaining=0;
					}
					
					//if we ended
					if(!allProceses[i]->timeRemaining)
					{
						allProceses[i]->finishTime=time;
						printf("Proc#:%d finised at:%d\n",allProceses[i]->id,allProceses[i]->finishTime);
					}
					
				}
			}
			
			//if we didnt run anything that loop, nothing has arrived
			if(!ranSomething)
			{
				int minArrivalTimePastCurrentTime = INT_MAX;
				for(int i = 0 ; i < num; i++)
				{
					if(allProceses[i]==NULL)
						continue;
					if(allProceses[i]->arrivalTime > time)
					{
						if(minArrivalTimePastCurrentTime>allProceses[i]->arrivalTime)
						{
							minArrivalTimePastCurrentTime=allProceses[i]->arrivalTime;
						}
					}
				}
				if(minArrivalTimePastCurrentTime < INT_MAX)
					time = minArrivalTimePastCurrentTime;
				else
					timeToEnd = 1;
			}
		}
		
	}
	else
		exit(1);
		
		
	int numRunning = 0;
	for(int i = 0 ; i < num; i++)
	{
		if(allProceses[i]!=NULL&&allProceses[i]->timeRemaining>0)
		numRunning++;
	}
		
	return numRunning;
}


void runRR_Load(Process* allProceses, int num)
{
			
	for(int i = 0 ; i < num; i++)
	{
		//make sure time remaining is accurate
		allProceses[i].timeRemaining = allProceses[i].burstTime;
		
		//make sure time remaining is invalid
		allProceses[i].startTime=-1;
	}
	
	//sort by arrival
	qsort(allProceses,num,sizeof(Process),compProcByArrival);
		
	//make one queue per core
	Process*** arrOfQueuesOfProcesses = (Process***)malloc(sizeof(Process**)*cores);
	
	//each queue needs to be big enough
	for(int i = 0 ; i < cores; i++)
		arrOfQueuesOfProcesses[i] = (Process**)malloc(sizeof(Process*)*num);
		
	for(int i = 0; i<cores; i++)
		for(int j = 0 ; j < num; j++)
			arrOfQueuesOfProcesses[i][j]=NULL;
		
	int currentTime;
	int endTime;
	int index = 0;
	
	for(int currProc = 0; currProc<num;currProc++)
	{		
		currentTime = allProceses[currProc].arrivalTime;

		//if the last process, run until done
		if(currProc == num-1)
			endTime=INT_MAX;
		else
			endTime=allProceses[currProc+1].arrivalTime;
			
			
		printf("Assigning proc#%d to core#%d\n",allProceses[currProc].id,index);
		
		for(int i = 0 ; i < num; i++)
		{
			if(arrOfQueuesOfProcesses[index][i]==NULL)
			{
				arrOfQueuesOfProcesses[index][i]=&allProceses[currProc];
				break;
			}
		}
		
		printf("------------------------\n");
		printf("Time:%d\n",currentTime);
		for(int c = 0; c < cores; c++)
		{
			printf("Core:%d\n\t",c);
			for(int i = 0 ; i < num; i++)
			{
				//if currently running on this core
				if(arrOfQueuesOfProcesses[c][i]!=NULL
					&&arrOfQueuesOfProcesses[c][i]->timeRemaining>0)
						printf("%d\t",arrOfQueuesOfProcesses[c][i]->id);
			}
			printf("\n");
		}
		printf("------------------------\n");
		
		int minRunning=INT_MAX;

		for(int i = 0 ; i < cores; i++)
		{
			int numRunning = runRR_SingleFromUntil(arrOfQueuesOfProcesses[i],num,currentTime,endTime);
			if(minRunning>numRunning){
				minRunning=numRunning;
				index = i;
			}
		}
		
		currentTime=endTime;
	}
	
	
}

int main()
{
	srand((int)time(NULL));
	
	//testing
	cores = 4;
	quantum = 4;
	int numProcesses = 10;
	Process* processes = (Process*)malloc(sizeof(Process)*numProcesses);
	
	for(int i = 0; i < numProcesses; i++)
	{
		processes[i].id = i+1;
		processes[i].arrivalTime = i;//rand()%10;
		processes[i].timeRemaining = processes[i].burstTime = i+1;//rand()%10;
		processes[i].startTime = -1;
		processes[i].finishTime = -1;
	}
	
	runRR_Load(processes,numProcesses);
	
	printf("Avg Turnaround:%f\n",calcAvgTurnaround(processes,numProcesses));
	printf("Avg Wait:%f\n",calcAvgWait(processes,numProcesses));
	
	qsort(processes,numProcesses,sizeof(Process),compProcById);
	
	for(int i = 0; i < numProcesses; i++)
	{
		printf("ID:%d EndTime:%d\n",processes[i].id,processes[i].finishTime);
	}
	
	
	return 0;
}
