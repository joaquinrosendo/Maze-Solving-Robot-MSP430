#include  <msp430g2553.h>

#define dutyCycleLM TA1CCR1
#define dutyCycleRM TA1CCR2

unsigned int ADC_Value[6]={0,0,0,0,0,0}; //ADC value
unsigned long delay_Time = 0;
unsigned long delay_Time_Off = 0;
unsigned long x = 0;
unsigned long front = 0;
unsigned long back = 0;
unsigned long right_F = 0;
unsigned long right_B = 0;
unsigned long left_F = 0;
unsigned long left_B = 0;
unsigned long speed_L =  0;
unsigned long speed_R = 0;
unsigned long direction_L = 1;
unsigned long direction_R = 1;
unsigned long ADC_updater = 0;
unsigned long A = 0;
unsigned long VR = 0;
unsigned long VL = 0;
unsigned long turnDuration=0;
unsigned int straightDuration=0;
unsigned int leftSpeed=0;
unsigned int rightSpeed=0;
unsigned int random=0;
unsigned int done = 0;
unsigned int i=0;
unsigned int j=0;
unsigned int flag=0;
unsigned int time=0;
unsigned int TA = 0;
unsigned int TL = 0;
unsigned int TR = 0;
unsigned int wait = 0;
unsigned int WAITING = 375;
unsigned int leftCor=0;
unsigned int rightCor=0;

void ADC(void);
void INT(void);
void straight(void);
void delay(unsigned long d);
void check_adc(void);
void turn(void);
void update_ADC(void);

void leftForward(void);
void rightForward(void);
void leftBack(void);
void rightBack(void);
void moveClockwise(void);
void moveCounterClockwise(void);
void turnRight(void);
void turnLeft(void);
void resetMotors(void);
void chooseLeftDuty(int leftDuty);
void chooseRightDuty(int rightDuty);
void moveStraight(void);

void main(void)
{
  INT();
  ADC();
  __enable_interrupt();
  ADC10CTL0 |= ENC + ADC10SC;  //Starts sampling and convertiong

  while(1)
  {

	  update_ADC();
	  P1OUT |= (BIT1 + BIT2);
	  straight();

  }
}


void straight(){
	update_ADC();update_ADC();

	//***DEAD END***
	if(front >= 475 && left_F > 400 && right_F > 400 || TA == 1){
		  chooseLeftDuty(25);
		  chooseRightDuty(25);
		  P1OUT &= ~(BIT1 + BIT2);
		  if ( front <= 75 ){
			  wait = WAITING;
			  while (wait){
			   moveCounterClockwise();
			   wait = wait -1;
			  }
			  TA = 0;
			  moveStraight();
			}
		  else {
			   TA = 1;
			   moveCounterClockwise();
		  }
	}
//***STABILIZATION***
	else if(left_F > 450){		// left_F < 2.5cm
		chooseLeftDuty(37);
		chooseRightDuty(23);
		leftForward();
		rightForward();
		P1OUT |= (BIT1 + BIT2);
	}

	else if(right_F > 450){ // right_F < 2.5cm
		chooseRightDuty(37);
		chooseLeftDuty(23);
		leftForward();
		rightForward();
		P1OUT |= (BIT1 + BIT2);
	}

	//***LEFT CORNER TURN***
	else if(front > 600 && left_B < 200){
		chooseRightDuty(40);
		chooseLeftDuty(40);
		P1OUT &= ~BIT1;
		while(turnDuration <= 10000){
				turnDuration++;
			//	P1OUT |= BIT2;

				if(turnDuration < 5000){
					moveCounterClockwise();
				}
				else if(turnDuration > 5000 && turnDuration < 10000){
					leftForward();
					rightForward();
				}
				if(turnDuration > 10000){
					turnDuration &= 0x00;
					break;
				}
			}
	}

	//***RIGHT CORNER TURN***
	else if(front > 600 && right_B < 200){
		chooseRightDuty(40);
		chooseLeftDuty(40);
		P1OUT &= ~BIT2;
		while(turnDuration <= 10000){
				turnDuration++;

			//	P1OUT |= BIT1;
				if(turnDuration < 4000){
					moveClockwise();
				}
				else if(turnDuration > 4000 && turnDuration < 10000){
					leftForward();
					rightForward();
				}
				if(turnDuration > 10000){
					turnDuration &= 0x00;
					break;
				}
			}
		}

	//***LEFT CORRIDOR TURN***
	else if(left_F < 200 && left_B < 200 && right_B > 300){

		if(right_F%2 == 0){		//if even number, turn
			chooseRightDuty(45);
			chooseLeftDuty(25);
			time = 10000;
			P1OUT &= ~BIT1;
		}

			else{					//else, go straight
			chooseRightDuty(25);
			chooseLeftDuty(25);
			time = 2500;
			}

			while(turnDuration <= time){
				turnDuration++;
				leftForward();
				rightForward();
			//	P1OUT |= BIT2;

				}
			turnDuration=0;
	}


	//***RIGHT CORRIDOR TURN***
	else if(right_F < 200 && right_B < 200 && left_B > 300){

		if(left_F%2 == 0){		//if even number, turn
			chooseLeftDuty(45);
			chooseRightDuty(25);
			time = 10000;
			P1OUT &= ~BIT2;
			}

			else{					//else, go straight
			chooseRightDuty(25);
			chooseLeftDuty(25);
			time = 2500;

			while(turnDuration <= time){
				turnDuration++;
				leftForward();
				rightForward();

			//	P1OUT |= BIT1;
				}
			turnDuration=0;
			}

	}


	//***RIGHT OPENING, LEFT OPENING, WALL IN FRONT***
	else if(front > 475 && left_B < 200 && right_B < 200){

		if(left_F%2 == 0){			//turns left
			P1OUT &= ~(BIT1);
			while(turnDuration < 1500){
				turnDuration++;

				moveClockwise();
				if(turnDuration > 1500){
					resetMotors();
					turnDuration &=0x00;
				}
			}
		}

		else{						//turns right
			P1OUT &= ~(BIT2);
			while(turnDuration < 1500){
				turnDuration++;
				moveCounterClockwise();
				if(turnDuration > 1500){
					resetMotors();
					turnDuration &=0x00;
				}
			}


		}
	}

	//***GO STRAIGHT BY DEFAULT***
	else{
			chooseLeftDuty(25);
			chooseRightDuty(25);
			leftForward();
			rightForward();
			P1OUT |= BIT1 + BIT2;
		}

}

//********************************************************************
//****************** PWM & MOTOR DIRECTION ***************************
//********************************************************************

//***********FORWARD********************
void leftForward(void){
		P2OUT |= BIT0;	//P2.0 = 1 , IN1
		P2OUT &= ~BIT1;	//P2.1 = 0 , IN3

	//	P1OUT &= ~(BIT1 + BIT2);
}

void rightForward(void){
		P2OUT |= BIT3;	//P2.3 = 1 , IN2
		P2OUT &= ~BIT4;	//P2.4 = 0 , IN4

	//	P1OUT &= ~(BIT1 + BIT2);
}

//***********BACKWARD*******************
void leftBack(void){
		P2OUT &= ~BIT0;	//P2.0 = 0 , IN1
		P2OUT |= BIT1;	//P2.1 = 1 , IN3

	//	P1OUT &= ~(BIT1 + BIT2);
}

void rightBack(void){
		P2OUT &= ~BIT3;	//P2.3 = 0 , IN2
		P2OUT |= BIT4;	//P2.4 = 1 , IN4

//		P1OUT &= ~(BIT1 + BIT2);
}

//***********TURNING********************
void moveClockwise(void){
		//Left Motor
		P2OUT |= BIT0;	//P2.0 = 1 , IN1
		P2OUT &= ~BIT1;	//P2.1 = 0 , IN3

		//Right Motor
		P2OUT &= ~BIT3;	//P2.3 = 0 , IN2
		P2OUT |= BIT4;	//P2.4 = 1 , IN4

//		P1OUT &= ~BIT2;
}

void moveCounterClockwise(void){
		//Left Motor
		P2OUT &= ~BIT0;	//P2.0 = 0 , IN1
		P2OUT |= BIT1;	//P2.1 = 1 , IN3

		//Right Motor
		P2OUT |= BIT3;	//P2.3 = 1 , IN2
		P2OUT &= ~BIT4;	//P2.4 = 0 , IN4

	//	P1OUT &= ~BIT1;
}

//***********HAULT MOTORS***************
void resetMotors(void){
		//Left Motor
		P2OUT &= ~BIT0;	//P2.0 = 0 , IN1
		P2OUT &= ~BIT1;	//P2.1 = 0 , IN3

		//Right Motor
		P2OUT &= ~BIT3;	//P2.3 = 0 , IN2
		P2OUT &= ~BIT4;	//P2.4 = 0 , IN4
}

void moveStraight(void){

	while(straightDuration <= 1050){
		straightDuration++;
		chooseLeftDuty(30);
		chooseRightDuty(30);

		if(straightDuration < 1000){
		leftForward();
		rightForward();
		}
		else if(straightDuration > 1000){
			resetMotors();
			straightDuration &= 0x00;
			break;
		}
	}
}

void chooseLeftDuty(int leftDuty){
	TA1CCR0 = 10000;
	dutyCycleLM = leftDuty*100+50;
}

void chooseRightDuty(int rightDuty){
	TA1CCR0 = 10000;
	dutyCycleRM = rightDuty*100;
}

/********************************************************************
 * ****************** RANDOM ALGORITHM FUNCTION *********************
 * ******************************************************************
 */
unsigned int rand( unsigned int right, unsigned int left, unsigned int straight ){
  done = 0;
  // get comparison random value
    random = ADC_Value[A];
  // end get comparison random value
  while ( done == 0 ) {
        // try to choose direction
        if (random <= 341){
              if ( straight == 1 ){
                x = 0;
                done = 1;
              }
          }
          else if ( random >= 342 && random <= 682){
              if ( left == 1 ){
                x = 1;
                done = 1;
              }
          }
          else if ( random >= 683 && random <= 1023){
              if ( right == 1 ){
                x = 2;
                done = 1;
              }
          }
          else {
            x = 10;
            done = 0;
          }
        // make sure it ran correctly
          if (done == 0) {
            if(A >= 5){
              A = 0;
            }
            else{
              A = A + 1;
            }
            random = ADC_Value[A];
         } // if
  } // while
  return x;
}

void INT(void)
{
   // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  BCSCTL1 = CALBC1_1MHZ;	// Set range   DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_0); 	// SMCLK = DCO = 1MHz
  DCOCTL = 0;             		// Select lowest DCOx and MODx
  BCSCTL1 = CALBC1_1MHZ;  		// Set range
  DCOCTL = CALDCO_1MHZ;   		// Set DCO step + modulation
  P1DIR |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);
  P1SEL |= (BIT0 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7);

  //*** Timer A1 ***
  TA1CTL = TASSEL_2 + MC_1;   //SMCLK + UPMODE
  TA1CCTL1 = OUTMOD_7;		//SET/RESET MODE FOR TA1CCR1 (LeftDuty)
  TA1CCTL2 = OUTMOD_7; 		//SET/RESET MODE FOR TA1CCR2 (RightDuty)
  TA1CCR0 = 10000;				//PERIOD - capture/control reg-0
  TA1CCR1 = dutyCycleLM;			//right motor duty cycle - capture/control reg-1
  TA1CCR2 = dutyCycleRM;			//left motor dutry cycle - capture/control reg-2

  //*** PORT 2 PINS ***
  P2DIR |= BIT2 + BIT5;				//SET PIN AS OUTPUT
  P2SEL |= BIT2 + BIT5;				//set up TA1.1-2 function as an output for pin P2.2-3 (PWM-ENA,ENB)
  P2OUT &= 0;						//PORT 2 OUTPUT REGISTER
  P2DIR |= BIT0 | BIT1 | BIT3 | BIT4;	//OUTPUT DIRECTION (IN1, IN3, IN2, IN4)

}

void ADC(void)
{
  ADC10CTL1 = INCH_0 + CONSEQ_1;   //A1 and A0, sequence-of-channels
  ADC10CTL0 = SREF0 + SREF1 + ADC10SHT_0 + ADC10ON + ADC10IE;// 2.5 & Vss as reference, Sample and hold for 4 Clock cycles, ADC on, ADC interrupt enable
  ADC10AE0 |= BIT1;
}

void update_ADC(void)
{
    front = ADC_Value[2];
    right_F = ADC_Value[3];
    right_B = ADC_Value[4];
    left_F = ADC_Value[5];
    left_B = ADC_Value[0];
              if ( ADC_updater == 1){ //FrontZ
                 ADC10CTL0 ^= ENC;
                 A = 2;
                 ADC_updater = 2;
                 ADC10CTL1 = INCH_3 + ADC10DIV_0;
                 ADC10AE0 |= BIT3;
              }
              else if ( ADC_updater == 2){ //Right_F
                 ADC10CTL0 ^= ENC;
                 A = 3;
                 ADC_updater = 3;
                 ADC10CTL1 = INCH_6 + ADC10DIV_0;
                 ADC10AE0 |= BIT6;
              }
              else if ( ADC_updater == 3){ //Right_B
                 ADC10CTL0 ^= ENC;
                 A = 4;
                 ADC_updater = 0;
                 ADC10CTL1 = INCH_5 + ADC10DIV_0;
                 ADC10AE0 |= BIT5;
              }
              else if ( ADC_updater == 0){ //FRONT
                 ADC10CTL0 ^= ENC;
                 A = 2;
                 ADC_updater = 4;
                 ADC10CTL1 = INCH_3 + ADC10DIV_0;
                 ADC10AE0 |= BIT3;
              }
              else if ( ADC_updater == 4){ //Left_F
                 ADC10CTL0 ^= ENC;
                 A = 5;
                 ADC_updater = 5;
                 ADC10CTL1 = INCH_0 + ADC10DIV_0;
                 ADC10AE0 |= BIT0;
              }
              else if ( ADC_updater == 5){ //Left_B
                 ADC10CTL0 ^= ENC;
                 A = 0;
                 ADC_updater = 1;
                 ADC10CTL1 = INCH_7 + ADC10DIV_0;
                 ADC10AE0 |= BIT7;
              }
         ADC10CTL0 |= ENC + ADC10SC;  //Starts sampling and converting
}

//ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  ADC10CTL0 &= ~ADC10IFG;
  ADC_Value[A] = ADC10MEM;
}
