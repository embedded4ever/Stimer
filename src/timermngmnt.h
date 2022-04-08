#ifndef _TIMER_MGMNT_H_
#define _TIMER_MGMNT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint8_t ut8_timer;
typedef uint32_t ut32_timer;

extern volatile ut32_timer systick_cnt;

typedef enum timer_type
{
  SHOT,
  LOOP
}timer_type_e;

typedef struct timer_s
{   
  struct timer_s* timer_list; ///< Next entry in the single linked list
  timer_type_e type; ///< Single Shot or auto - reloaled
  void *arg; ///< Placeholder for argument will pass when the timer is expired.
  void (*exp_func)(void *); ///< Call the callback function When the timer is expired
  ut32_timer remaining_time_to_expire; ///< Remaining time to the fire!
  ut32_timer starting_timestamp; ///<Placeholder for starting timestamp
  ut32_timer interval; ///< milisecond
}timer_t;

typedef struct timer_mngmnt_s
{
  timer_t* root; ///< Handle root
  ut8_timer count_of_active_timers; ///< Total number of the running timers;
}timer_mngmnt_t;

static timer_mngmnt_t tmr_mngmnt = {.root = NULL, .count_of_active_timers = 0};

static ut8_timer s_get_count_of_active_timers(const timer_mngmnt_t* self)
{
  return self -> count_of_active_timers;
}

static void timer_add(timer_t *tmr)
{
  if (NULL == tmr_mngmnt.root)
  {
    tmr_mngmnt.root = tmr;
  }
  else 
  {
    timer_t *pos = tmr_mngmnt.root;
    
    while (NULL != pos -> timer_list)
    {      
      pos = pos -> timer_list;
    }
    
    pos -> timer_list = tmr;
  }

  ++tmr_mngmnt.count_of_active_timers;
}

static void unlink_timer(timer_t *tmr)
{
  //Check timer is root ?
  //Ex: root (t1) -> t2 -> t3
  // (t1) == (unlinked_timer)
  // root -> t2 -> t3
  if (tmr_mngmnt.root == tmr)
  {
    tmr_mngmnt.root = tmr -> timer_list;
    --tmr_mngmnt.count_of_active_timers;
    return;
  }

  timer_t *prev = tmr_mngmnt.root;

  for (timer_t* pos = tmr_mngmnt.root; NULL != pos; pos = pos -> timer_list)
  {
    // Ex; root (t1) -> t2 -> t3
    // (t1 -- t2) == unlinked_timer
    // t1 -> t3
    if (prev -> timer_list == tmr)
    {
      prev -> timer_list = tmr -> timer_list;
      --tmr_mngmnt.count_of_active_timers;
      break;
    }    
    //Update to previous one
    prev = pos;
  }  
}

static ut32_timer get_systick(void)
{
  return systick_cnt;
}

void init_timer(timer_t *tmr, timer_type_e type, void *arg, void(*exp_func)(void*), ut32_timer interval)
{
  tmr -> timer_list = NULL;
  tmr -> type = type;
  tmr -> exp_func = exp_func;
  tmr -> arg = arg;
  tmr -> remaining_time_to_expire = 0;
  tmr -> interval = interval;
  tmr -> starting_timestamp = 0;
}

void start_timer(timer_t *tmr)
{
  tmr -> starting_timestamp = get_systick();

  timer_add(tmr);
}

void timer_pool(void)
{
  for (timer_t* pos = tmr_mngmnt.root; NULL != pos; pos = pos -> timer_list)
  {
    if ((get_systick() - pos -> starting_timestamp) >= pos -> interval)
    {
      /* Call expire func */
      if (pos -> exp_func != NULL)
      {
        pos -> exp_func(pos -> arg);
      }

      if (pos -> type == SHOT)
      {
        unlink_timer(pos);
      }
      /* Reload the timer */
      else if (pos -> type == LOOP)
      {
        pos -> starting_timestamp = get_systick();
      }
    }
  }
}

ut8_timer get_count_of_active_timers(void)
{
  return s_get_count_of_active_timers(&tmr_mngmnt);
}

void remove_timer(timer_t *tmr)
{
  unlink_timer(tmr);
}

int32_t remaining_time_to_fire(const timer_t *tmr)
{
  if (tmr_mngmnt.root == tmr)
  {
    return (tmr -> interval - (get_systick() - tmr -> starting_timestamp));
  }

  timer_t *prev = tmr_mngmnt.root;

  for (timer_t* pos = tmr_mngmnt.root; NULL != pos; pos = pos -> timer_list)
  {    
    if (prev -> timer_list == tmr)
    {   
      return (tmr -> interval - (get_systick() - tmr -> starting_timestamp));
    }    
    //Update to previous one
    prev = pos;
  }

  // if timer is single shot but you are trying to get remaining time ! 
  return -1; 
}

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* #ifndef _TIMER_MGMNT_H_ */
