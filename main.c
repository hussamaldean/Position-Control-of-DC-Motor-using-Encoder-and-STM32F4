#include "stm32f4xx.h"                  // Device header

uint64_t cnt_current,cnt_prev=0;
uint8_t start=0;
void encoder_init(void)
{
	RCC->AHB1ENR|=RCC_AHB1ENR_GPIOAEN; //enable GPIOA clock
	RCC->APB1ENR|=RCC_APB1ENR_TIM2EN; //enable timer 2 clock
	GPIOA->MODER|=(1<<1)|(1<<3);      //set PA0/PA1 as alternate function
	GPIOA->AFR[0]|=(1<<0)|(1<<4);     //select PA0/PA1 as TIM2CH1/TIM2CH2 
	TIM2->ARR = 39000;								//maximum counter 
	//configure the timer as encoder
	TIM2->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0 ); 
	TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);  
	TIM2->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;   
	TIM2->DIER|=0x01; //enable timer interrupt
	NVIC_EnableIRQ(TIM2_IRQn);  //unmask timer2 interrupt vector
	TIM2->CR1 |= TIM_CR1_CEN ;  //enable timer2
	
}

//crude delay
void delay(int delay)

{

int i;

for(; delay>0 ;delay--)

{

for(i =0; i<3195;i++);

}

}


int  main(void)

{
__disable_irq(); //disable global interrupt
encoder_init();  //initialize timer in encoder mode
	
GPIOA->MODER  |= (1<<10)|(1<<12); //set PA5&PA6 as output
	
//configure the PC13 as input interrupt	
RCC->AHB1ENR|=0x5; //enable GPIOC clock	
RCC->APB2ENR|=RCC_APB2ENR_SYSCFGEN ;
SYSCFG->EXTICR[3]|=SYSCFG_EXTICR4_EXTI13_PC;
EXTI->IMR|=EXTI_IMR_MR13;	
EXTI->FTSR|=EXTI_RTSR_TR13;
	
NVIC_EnableIRQ(EXTI15_10_IRQn); //unmask interrupt for pc13
__enable_irq();	    //ENABLE global interrupt
	
while (1)
{

//blinking an led for fun
GPIOA->ODR ^= (1<<6);
	delay(300);
	
}
}
//timer2 interrupt handler
void TIM2_IRQHandler(void){
GPIOA->BSRR=(1<<(5+16)); //turn off the motor
start=0;	 //reset the start condition
TIM2->SR=0; //clear interrupt flag

}
void EXTI15_10_IRQHandler(void)
{
if(start==0) //if motor is not started
{
TIM2->CNT=0; //clear timer counter
TIM2->SR=0; //clear any pending timer interrupt
GPIOA->BSRR=(1<<5); //turn on the motor
start=1;            //set the  motor start
}
EXTI->PR=EXTI_PR_PR13; //clear the pc13 interrupt
}
