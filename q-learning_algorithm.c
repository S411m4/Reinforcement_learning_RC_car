//original Arduino Code written by : Varun2905
//embeded C code written by : S41m4 

#include <util/delay.h>  //to use delay functions
#include "Ultarsonic_driver.h"  



#define ALPHA 0.1     //learning rate , 0: means nothing new is learned and robot depend on past exepriences, 1: means robot always learn and don't depend on past experiences
#define GAMMA 0.5   //discount factor (to decrease rewards by time)
#define episodes 100    //no of iterations
#define states 10   //numbes of states in environment, by default they should be 2 (obstacle , or no obstacle), but we made 10 for more complex environments
#define number_of_actions 4     //total number of actions (forward, backward, stop, left)

uint8_t Obstacle = 0;   //to indicate if there is an obstacle or not
int FLAG;   //to determine how far the robot is in the decision making process, 0: hasn't done anything yet, 1: detected an obstacle and went to next state, and will start to determine whether to explore (by choosing random action) or exploit based on previous Q-table, 2: if exploring is chosen, rewards will be given based on action
int reward;   //reward for performing an action
int state = 0;    // current state of the robot
int action = 0;   //action performed by the robot, number according to index (0: forward, 1: backward, 2: stop, 3: left)
float prob;   //used for epsilon decay, where by time the robot will begin to explore less and exploit more (act based on previous knowledge)
uint8_t action_taken = 0; //to tell whether an action is taken or not
int next_state;   //next state or robot
int actions[4] = {1, 2, 3, 4};    //actions values (1: forward, 2: backward, 3: stop, 4: left)
float EPSILON = 0.90;   //exploration rate


//Q-table, in reality the obstacle avoiding robot has only two states, 1: when it is away from obstacle (no obstacle) , 2: when it near to obstacle (obstacle)
//here we coded 10 states assuming more complex environment, for more accurate learning process (however this makes the learning process slower)

//Q-table has states as rows, and columns as number of actions
// //Q-table will updated with rewards based on the robot actions

float Q[states][number_of_actions] = {{0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}, 
                                      {0.0, 0.0, 0.0, 0.0}};

//reward table based on robot actions (stop when seeing obstacle : 0, backward when seeing obstacle: -5, forward ignoring obstacle: -10, left when seeing obstacle: 10)

int rewards[states][number_of_actions] = {{-10, -5, 0, 10},
                                          {-10, -5, 0, 10},
                                          {-10, -5, 0, 10},
                                          {-10, -5, 0, 10},
                                          {-10, -5, 0, 10},
                                          {-10, -5, 0, 10},
                                          {-10, -5, 0, 10},
                                          {-10, -5, 0, 10},
                                          {-10, -5, 0, 10},
                                          {-10, -5, 0, 10}};

//Q-learning update parameters
float Q_old, Q_new, Q_max;

//function to determine state, if there is an obstacle or not
//you must import the ultrasonice header file, to use the getDistance() function
uint8_t obstacle_avoider()
{
  uint16_t distance = getDistance();
  
  if(distance < 15)
  {
    Obstacle = 1;
  }
  else if
  {
    Obsacle = 0; 
  }
  
  _delay_ms(10);
  
  return Obstacle;
}

//function to reduse epsilon (explorartion parameter) with time. where at the end you get rid of epsilon and the robot learns to aboivd obstacles based on past exeperiences

float decay(float epsilon)
{
  epsilon =  epsilon* 0.98;  
  return epsilon;
}

int get_state()
{
  int state_number;
  state_number = random(0, 10);
  return state_number;
}

//function to find the biggest number in Q_table[next_state]

float MAX(float Q_table[][4], int NEXT_S)
{
  float LIST[4];
  float N1, N2, MAX_value = 0.0, DIFF;
  
 
  for (int b = 0; b <= 3; b++)
  {
 		//make list from given row in table
 		LIST[b] = Q[NEXT_S][b];
 	}

 	for (int j = 0; j <= 2; j++)
 	{
 		//compare MAX_value = 0.0 with each item in list
 		if (MAX_value > LIST[j])
 		{

 			N1 = MAX_value;
 		}
 		else
 		{
 			N1 = LIST[j];
 		}
 		//compare list items with each other to get biggest value of them
 		N2 = LIST[j + 1];
 		DIFF = N1 - N2;

 		if (DIFF > 0)
 		{

 			MAX_value = N1;
 		}
 		else
 		{
 			MAX_value = N2;
 		}
  }
    	return MAX_value;
}


//function to find the index of biggest Q value in Q table[state]

 int ARGMAX(float Q_table[][4], int NEXT_S)
 {

 	float array[4];
 	float N1, N2, MAX_value = 0.0, DIFF, NUMBER;
 	int MAX_index;

 	for (int u = 0; u <= 3; u++)
 	{

 		array[u] = Q_table[NEXT_S][u];
 	}

 	for (int p = 0; p <= 2; p++)
 	{
 		if (MAX_value > array[p])
 		{
 			N1 = MAX_value;
 		}
 		else
 		{
 			N1 = array[p];
 		}

 		N2 = array[p + 1];
 		DIFF = N1 - N2;

 		if (DIFF > 0)
 		{
 			MAX_value = N1;
 		}

 		else
 		{
 			MAX_value = N2;
 		}
 	}

 	for (int r = 0; r <= 3; r++)
 	{
 		NUMBER = array[r];
 		if (NUMBER == MAX_value)
 		{
 			MAX_index = r;
 			break;
 		}
 	}
 	return MAX_index;
 }

 //function to update Q_table and Q_value, based on BellMan equation using temporal difference learning approach
 void update(float Q_table[][4], int S, int NEXT_S, int A, int actions[], int R, float learning_rate, float discount_factor)
 {

 	Q_old = Q_table[S][A];
 	Q_max = MAX(Q_table, NEXT_S);
 	Q_new = (1 - learning_rate) * Q_old + learning_rate * (R + discount_factor * Q_max);
 	Q_table[S][A] = Q_new;
 }

 //choosing random number to determine whether to explore or exploit
 float random_no(float EXPLORATION_PARAMETER)
 {
 	float RANDOM_VARIABLE;
 	float PROBABILITY;

 	RANDOM_VARIABLE = rand(0, 100);
 	PROBABILITY = RANDOM_VARIABLE / 100;

 	return PROBABILITY;
 }

 int main()
 {
 	//infinite loop
 	while (1)
 	{
 		for (int I = 0; I < episodes; I++)
 		{

 			action_taken = 0;
 			FLAG = 0;

 			while (1)
 			{
 				forward();
 				Obstacle = obstacle_avoider();
 				if (Obstacle == 1)
 				{
 					next_state = state + 1;

 					if (next_state == 10)
 					{
 						next_state = 0;
 					}
 					else if (next_state < 0)
 					{
 						next_state = 0;
 					}
 					FLAG = 1;
 					break;
 				}
 			}

 			if (FLAG == 1)
 			{

 				prob = random(EPSILON); //choose a random value within exploration rate, to determine whether to explore or exploit
 				if (prob <= EPSILON)	//explore the actions
 				{
 					action = random(0, 4);
 					FLAG = 2;
 				}
 				else //exploit the actions from Q_table
 				{
 					action = ARGMAX(Q, state);
 					FLAG = 2;
 				}
 			}

 			if (FLAG == 2)
 			{
 				if (action == 0)
 				{
 					forward();
 					_delay_ms(1500);
 					stop();
 					reward = rewards[state][action];
 				}

 				if (action == 1)
 				{
 					backward();
 					_delay_ms(2500);
 					stop();
 					reward = rewards[state][action];
 				}

 				if (action == 2)
 				{
 					stop();
 					reward = rewards[state][action];
 				}
 				if (action == 3)
 				{
 					turn_left();
 					_delay_ms(2000);
 					stop();
 					reward = rewards[state][action];
 				}

 				action_taken = 1;
 				_delay_ms(500);
 			}

 			if (action_taken == 1)
 			{
 				//if an action is taken, update Q-table with new values based on action taken
 				update(Q, state, next_state, action, actions, reward, ALPHA, GAMMA);
 				state = next_state;
 				EPSILON = decay(EPSILON); //decay epsilon to decrease exploration rate by time
 				if (EPSILON < 0.5)
 				{

 					EPSILON == 0.9;
 				}
 				_delay_ms(7000);
 			}
 		}

 		//////////////////////////////////////End of training////////////////////////////

 		////////////////////////////////////Evaluation//////////////////////////////////
 		//check whether Q values are right or wrong, if all Q values are right/accurate comment this section
 		//write the evaluation code

 		///////////////////////////////////end of evaluation/////////////////////////

 		//////////////////////////////////Testing//////////////////////////////////

 		//testing by letting the robot act on its own based on past experiences without learning new actions
 		
    /*
    while (1)
 		{
 			forward();
 			Obstacle = obstacle_avoider();
 			if (Obstacle == 1)
 			{

 				state = get_state();
 				action = ARGMAX(Q, state);

 				if (action == 0)
 				{
 					forward();
 					_delay_ms(1500);
 					stop();
 				}

 				if (action == 1)
 				{
 					backward();
					_delay_ms(1500);
 					stop();
 				}

 				if (action == 2)
 				{
 					stop();
 				}
 				if (action == 3)
 				{
 					turn_left();
 					_delay_ms(2000);
 					stop();
 				}
 			}
 		}
 	}
 }
*/
