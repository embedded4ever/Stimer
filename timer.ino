#include "src/timermngmnt.h"

#define USER_LED 13
void init_timer_to_1ms()
{
 // Prescaler 64 ==> resolution changes from 0,0000000625 to 0,000004
 // Need interval of 1Ms ==> 0,001/((1/16000000)*64) = 250 ticks 
 
 // Set prescaler to 64 ; (1 << CS01)|(1 << CS00)
 // Clear Timer on Compare (CTC) mode ; (1 << WGM02)
 TCCR0A = 0 ;    
 TCCR0B |= (1 << WGM02)|(1 << CS01)|(1 << CS00) ;
 

 // set Output Compare Register to (250 - 1) ticks
 OCR0A = 0xF9;
 
 // TCNT0
 // Timer count = (required delay/clock time period) - 1
 // 249 = (0,001/0,000004) - 1
 
 // initialize counter
 TCNT0 = 0;
 
 // TIMSK0
 // Set Timer Interrupt Mask Register to 
 // Clear Timer on Compare channel A for timer 0
 TIMSK0 |= (1 << OCIE0A);
}

volatile ut32_timer systick_cnt;
ISR(TIMER0_COMPA_vect) 
{
 ++systick_cnt;
}

void led_task(void *arg)
{
  int* a = (int *)arg;
  digitalWrite(USER_LED, *a);
  *a = !(*a);
}

void print_task_string(void* arg)
{
  const char* str = (const char*)arg;
  Serial.println(str);
}

void print_task(void* arg)
{
  Serial.println("*****************************************");
}

timer_t t1;
timer_t t2;
timer_t t3;

void setup() {

  Serial.begin(9600);
  Serial.println("Started...");

  
  pinMode(USER_LED, OUTPUT);  
  digitalWrite(USER_LED, LOW);
  
  init_timer_to_1ms();
  int a = 1, b = 2, c = 1;  
  const char* test_string = "test_string";

  init_timer(&t1, LOOP, test_string, print_task_string, 1000);
  init_timer(&t2, LOOP, &b, print_task, 2000);
  init_timer(&t3, LOOP, &c, led_task, 10000);

  start_timer(&t1);
  start_timer(&t2);
  start_timer(&t3);
}

void loop() {
  timer_pool();
}
