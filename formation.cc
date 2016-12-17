
/**
* g++ -c communicate.c
*
*g++ -I/usr/local/include -L/usr/local/lib -o formation `pkg-config --cflags playerc++` formation.cc `pkg-config --libs playerc++` -lnsl communicate.o
*
*
**/


#include "formation.h"



int port;
int proxyport;
int d_sense;
int dist;
double dfc, ndfc;

#define RAYS 32
using namespace PlayerCc;
using namespace std;


double safewalkTimer(){

	clock_t startTime = clock();
	double firstPass = 0;
	
	return firstPass = ((250*clock()) - startTime) / CLOCKS_PER_SEC;
}


void send_cmd(int bfd, double x, double y, double yaw, char form)
{

	// create a message
	int nbytes = 0;
	char *msg = NULL;
	char *char_from = NULL;
	char *char_x = NULL;
	char *char_y = NULL;
	char *char_yaw = NULL;
	char *char_form = NULL;

	msg = (char *) malloc(60*sizeof(char));
	char_from = (char *) malloc(10*sizeof(char));
	char_x = (char*) malloc(sizeof(float));
	char_y = (char*) malloc(sizeof(float));
	char_yaw = (char*) malloc(sizeof(float));
	char_form = (char*) malloc(sizeof(char));

	itoa((int)port, char_from);
	sprintf(char_x, "%f",  x);
	sprintf(char_y, "%f", y);
	sprintf(char_yaw, "%f" , yaw);
	sprintf(char_form, "%c", form);

	strcpy(msg, "L");
	strcat(msg, char_from);
	strcat(msg, "$");
	strcat(msg, char_x);
	strcat(msg, "&");
	strcat(msg, char_y);
	strcat(msg, "Y");
	strcat(msg, char_yaw);
	strcat(msg, "#");
	strcat(msg, char_form);
	strcat(msg, "!");
	printf("sending message1: %s\n", msg);
	nbytes = talk_to_all(bfd, msg, H);
}

void send_cmd2(int bfd, double x, double y, char leaderForm)
{

	// create a message
	int nbytes = 0;
	char *msg = NULL;
	char *char_from = NULL;
	char *char_x = NULL;
	char *char_y = NULL;
	char *char_form = NULL;

	msg = (char *) malloc(60*sizeof(char));
	char_from = (char *) malloc(10*sizeof(char));
	char_x = (char*) malloc(sizeof(float));
	char_y = (char*) malloc(sizeof(float));
	char_form = (char*) malloc(sizeof(float));

	itoa((int)port, char_from);
	sprintf(char_x, "%f",  x);
	sprintf(char_y, "%f", y);
	sprintf(char_form, "%c", leaderForm);

	strcpy(msg, "T");
	strcat(msg, "$");
	strcat(msg, char_x);
	strcat(msg, "&");
	strcat(msg, char_y);
	strcat(msg, "#");
	strcat(msg, char_form);
	strcat(msg, "!");
	printf("sending message: %s\n", msg);
	nbytes = talk_to_all(bfd, msg, H);
}

int randomNumber(int radius){

	int isecret, iguess;
	isecret = rand() & ((radius)*2) + (-radius);
	return isecret;
}


void parse_msg(char *msg, float *x, float *y, float *ryaw, char *f)
{
	//passes by reference to set position, speed, and yaw to objects in main using the parse method
  char *ptr, *token;

  token = strtok(msg, "!");
  while (token != NULL) {
    if (token[0] == 'C') {
    }
    ptr = strstr(token, "C");
    ptr++;
    ptr = strstr(token, "$");
    ptr++;
    *x = atof(ptr);
    ptr = strstr(token, "&");
    ptr++;
    *y = atof(ptr);
    ptr = strstr(token, "Y");
    ptr++;
    *ryaw = atof(ptr);
    ptr = strstr(token, "#");
    ptr++;
    *f = *ptr;
    token = strtok(NULL, "!");
  }
}
void parse_msg2(char *msg, float *x, float *y, char *f, char *t)
{
	//passes by reference to set position, speed, and yaw to objects in main using the parse method
  char *ptr, *token;

  token = strtok(msg, "!");
  while (token != NULL) {
  	*t = token[0];
    if (*t == 'L') {
//    	printf("This is a leader message.\n");
    }
    if(*t == 'T'){
 //   	printf("This is a task manager message.\n");
    }
    ptr = strstr(token, "$");
    ptr++;
    *x = atof(ptr);
    ptr = strstr(token, "&");
    ptr++;
    *y = atof(ptr);
    ptr = strstr(token, "#");
    ptr++;
    *f = *ptr;
    token = strtok(NULL, "!");
  }
}

void safewalk(double *newspeed, double *newturnrate, LaserProxy &lp, Position2dProxy &pp, double x, double y, char formation)
{

	double minR = lp.GetMinRight();
	double minL = lp.GetMinLeft();

	double l = (1e5*minR)/500-100;
	double r = (1e5*minL)/500-100;

	if (l > 100)
		l = 100;
	if (r > 100)
		r = 100;
	if(port == 6665){
			*newspeed = (r+l)/1e3;
			if(formation == 'l'){
				*newspeed = 0.13;
			}
	}else {

		if(r<10 || l <10){
			*newturnrate = (r-l);
			*newturnrate = limit(*newturnrate, -40.0, 40.0);	//newturnrate has to be inside the limit (-40 to 40)
			*newturnrate = dtor(*newturnrate);
		}else{
			pp.GoTo(x,y,0);
		}
	}

	
}

double calcDistance(float x1, float x2, float y1, float y2){

	double distance;

	distance = sqrt( ((x2-x1) * (x2-x1)) + ((y2-y1) * (y2-y1)) );
	return distance;
}

/* main function */
int main(int argc, char** argv)
{
	parse_args(argc,argv);
 	int nbytes = 0;
  	char msg[MAXBUF];
  	char msg2[MAXBUF];
	double pointX, pointY;
	char rform;
	double endtime;
	double formX[3];
	double formY[3];
	int counter = 0;
	bool complete = true;
	bool starting = true;
	queue<double> waypointX;
	queue<double> waypointY;

//if executed w/o port number
	port = atoi(argv[1]);
	if (argc < 2) {
		printf("mytest port\n");
    	exit(1);
  	}

/*********************************************************************************************************************************/

  
 //proxyport to avoid address already in use
  switch(port)
  {
  	case 6665: proxyport = 7777; break;
  	case 6666: proxyport = 7778; break;
  	case 6667: proxyport = 7779; break;
  	case 6668: proxyport = 7780; break;
  	case 6669: proxyport = 7781; break;	//task manager
  	case 6670: proxyport = 7782; break;
  	default:   proxyport = 7783; break;
  }

   
// 6665 	7777

	int leaderport = 7777;
	int taskmanagerPort = 7781;

	 char input[100];
	 char *ptr;
	 ptr = strtok(input, " ");
	 if(proxyport == taskmanagerPort){
	  	while(1){
			printf("\nEnter points in form of x y (f to exit):");
	  		cin.getline(input, 100);
	  		if(strcmp(input , "f") == 0)break;
			ptr = strtok(input, " ");
	  		waypointX.push(atof(ptr));
	  		ptr = strtok(NULL, " ");
	  		waypointY.push(atof(ptr));
	  		counter++;
		}
	}
  	try
 	{
	 	PlayerClient robot(gHostname, port);
	    Position2dProxy pp(&robot, gIndex);
	    LaserProxy lp(&robot, gIndex);

		double newspeed = 0.0, newturnrate = 0.0;

		int br, lr;
		int tmB, tmL;
		int fb, fl;

		/* Create broadcast and listen objects for the different robots */
		if(port == 6665){
			br = create_broadcast(PORT_R, H);
			lr = create_listen(PORT_H, H);
		}
		else if(port == 6669){
			tmB = create_broadcast(PORT_H,H);
			tmL = create_listen(PORT_R,H);
		}
		else{
			fb = create_broadcast(PORT_H,H);
			fl = create_listen(PORT_R,H);
		}

		char currFormation;
		float wayX = 0.0, wayY = 0.0;
		double moe = 0.2;
		double currx, curry;
		float leaderX, leaderY, leaderYaw;
		char leaderForm = 'd';
		char messageType;


		while(1){
			safewalkTimer();
			currx = pp.GetXPos(); curry = pp.GetYPos();
			robot.Read();
			pp.SetSpeed(newspeed, newturnrate);

			switch (proxyport){

				case 7781:
					//task manager sending tasks
					nbytes = listen_to_robot(tmL,msg2);
					parse_msg2(msg2, &leaderX, &leaderY, &currFormation, &messageType);
					printf("\nMSG: %s\n", msg2);

					if(leaderX >= (wayX-moe) && leaderX <= (wayX + (moe + 0.5)) && leaderY >= (wayY-moe) && leaderY <= (wayY+moe)){
							complete = true;
							counter--;

					}
					else{
						complete = false;
					}
					printf("leader Position X: %f leader Position Y: %f  Formation: %c\n", leaderX, leaderY, currFormation);
					printf("Waypoint X: %f Waypoint Y: %f  Formation: %c\n", wayX, wayY, currFormation);
					if(complete || starting) {
						printf("COMPLETE");
						if(!waypointX.empty() || !waypointY.empty()){
							wayX = waypointX.front();
							wayY = waypointY.front();
							waypointX.pop();
							waypointY.pop();
							
							//alternating formations							
							if(leaderForm == 'l')leaderForm = 'd';
							else leaderForm = 'l';
							send_cmd2(tmB, wayX, wayY, leaderForm);
						}
						if(waypointX.empty())printf("IM EMPTY!\n");

						if(counter == 0){
							send_cmd2(tmB,0.0,0.0,'f');
							exit(1);
						}

					}
					if(counter <= 0) counter = 0;
					starting = false;
				break;


			/**********LEADER ROBOT ************/

				case 7777:
				send_cmd(br, pp.GetXPos(), pp.GetYPos(), pp.GetYaw(), currFormation);
				//Leader reaches the waypoint	
				if(complete){

					printf("LISTENING TO MANAGER\n");
					nbytes = listen_to_robot(lr, msg2);
					if(nbytes == 0) continue;
					printf("\nMSG: %s\n", msg2);
					parse_msg2(msg2, &leaderX, &leaderY, &currFormation, &messageType);
					printf("%f, %f, %c\n", leaderX, leaderY, messageType);

					printf("leaderX: %f leaderY: %f  Formation: %c\n", leaderX, leaderY, currFormation);
					if(nbytes == 0) continue;
					if(currFormation == 'f'){
						pp.SetSpeed(0.0,0.0);
						exit(1);
					}
					complete = false;
				}

				//Leader moving to waypoint
				if(!complete){
					printf("NOT COMPLETE");
					safewalk(&newspeed, &newturnrate, lp, pp, leaderX, leaderY, currFormation);
					printf("leaderX: %f leaderY: %f  Formation: %c\n", leaderX, leaderY, currFormation);
					if(currx >= (leaderX-moe) && currx <= (leaderX + (moe + 0.5)) && curry >= (leaderY-moe) && curry <= (leaderY+moe)){
						printf("IM HERE!\n");
						complete = true;
					}
				}

				break;
				
			/************** FOLLOWER ROBOTS ******************/
				case 7778: 	

						nbytes = listen_to_robot(fl,msg);
						if(nbytes == 0) continue;
						parse_msg2(msg, &leaderX, &leaderY, &currFormation, &messageType);

						if(currFormation == 'l'){
							formX[0] = leaderX;
							formY[0] = leaderY + 1;
						}
						if(currFormation == 'd'){
							formX[0] = leaderX - 1;
							formY[0] = leaderY + 1;
						}
						if(currFormation == 'd'){
							if(leaderX - currx >= 1)
								safewalk(&newspeed, &newturnrate, lp, pp, formX[0], formY[0], currFormation);
							else
								pp.SetSpeed(0.1,0.0);
						}
						if (currFormation == 'l')
						{
							if(leaderX - currx >= 0.3){
								pp.SetSpeed(10,0);
							}
							else
								safewalk(&newspeed, &newturnrate, lp, pp, formX[0], formY[0], currFormation);
						}
						break;

					case 7779: 
			
								nbytes = listen_to_robot(fl,msg);
								if(nbytes == 0) continue;
								parse_msg2(msg, &leaderX, &leaderY, &currFormation, &messageType);

								if(currFormation == 'l'){
									formX[1] = leaderX;
									formY[1] = leaderY - 1;
								}
								if(currFormation == 'd'){
									formX[1] = leaderX - 1;
									formY[1] = leaderY - 1;
								}
								if(currFormation == 'd'){
									if(leaderX - currx >= 1)
										safewalk(&newspeed, &newturnrate, lp, pp, formX[1], formY[1], currFormation);
									else
										pp.SetSpeed(0.1,0.0);
								}
								if (currFormation == 'l')
								{
									if(leaderX - currx >= 0.3){
										pp.SetSpeed(10,0);
									}
									else				
										safewalk(&newspeed, &newturnrate, lp, pp, formX[1], formY[1], currFormation);
								}
						break;

				case 7780:
		
							nbytes = listen_to_robot(fl,msg);
							if(nbytes == 0) continue;
							parse_msg2(msg, &leaderX, &leaderY, &currFormation, &messageType);

							if(currFormation == 'l'){
								formX[2] = leaderX;
								formY[2] = leaderY - 2;
							}
							if(currFormation == 'd'){
								formX[2] = leaderX - 2;
								formY[2] = leaderY;
							}
							if(currFormation == 'd'){
								if(leaderX - currx >= 2)
									safewalk(&newspeed, &newturnrate, lp, pp, formX[2], formY[2], currFormation);
								else{

									pp.SetSpeed(0.1,0.0);
								}
								if(leaderX - currx >= 2.8 && leaderY - curry <= 0.1){
									pp.SetSpeed(0.6,0.0);
								}
							}
							if (currFormation == 'l')
							{
								if(leaderX - currx >= 2){
									pp.SetSpeed(2,0);
								}
								else
									safewalk(&newspeed, &newturnrate, lp, pp, formX[2], formY[2], currFormation);
							}
					break;

				} //end switch	
		} //end while
	}	//end try
	catch (PlayerCc::PlayerError & e)
	{
		std::cerr << e << std::endl;
		return -1;
	}

}//end main