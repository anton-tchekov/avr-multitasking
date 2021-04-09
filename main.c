#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>

#define MAX_TASK_COUNT 10

struct struct_Task
{
	void (*func)(void);
	uint16_t exec_time;
	uint16_t delay_time;
	uint8_t repeat;
};

typedef struct struct_Task Task;

volatile uint16_t ms = 0;
Task tasks[MAX_TASK_COUNT];

void init(void);
int8_t add_task(void (*fn)(void), uint16_t delay, uint8_t rep);
void remove_task(uint8_t taskid);

void blink0(void);
void blink1(void);
void blink2(void);

int main(void)
{
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2);

	init();
	add_task(blink0, 1000, 1);
	add_task(blink1,  500, 1);
	add_task(blink2,  200, 1);

	while(1)
	{

	}

	return 0;
}

void blink0(void)
{
	PORTB ^= (1 << PB0);
}

void blink1(void)
{
	PORTB ^= (1 << PB1);
}

void blink2(void)
{
	PORTB ^= (1 << PB2);
}

void init(void)
{
	for(uint8_t i = 0; i < MAX_TASK_COUNT; i++)
	{
		tasks[i].func = NULL;
	}

	TCCR0 = (1 << WGM01) | (1 << CS01) | (1 << CS00);
	OCR0 = 115;
	TIMSK = (1 << OCIE0);
	asm volatile("sei");
}

int8_t add_task(void (*fn)(void), uint16_t delay, uint8_t rep)
{
	uint8_t sreg = SREG;
	int8_t taskid = -1;
	Task *t = tasks;

	asm volatile("cli");

	while(t->func != NULL && t < tasks+MAX_TASK_COUNT)
	{
		t++;
	}

	if(t->func == NULL)
	{
		taskid = t-tasks;
		t->func = fn;
		t->delay_time = delay;
		t->exec_time = ms+delay;
		t->repeat = rep;
	}

	SREG = sreg;
	return taskid;
}

void remove_task(uint8_t taskid)
{
	tasks[taskid].func = NULL;
}

/* 1 ms timer */
ISR(TIMER0_COMP_vect)
{
	++ms;

	for(uint8_t i = 0; i < MAX_TASK_COUNT; i++)
	{
		Task *t = &(tasks[i]);

		if((t->func != NULL) && (t->exec_time == ms))
		{
			(*(t->func))();
			if(t->repeat)
			{
				t->exec_time = ms + (t->delay_time);
			}
			else
			{
				t->func = NULL;
			}
		}
	}
}
