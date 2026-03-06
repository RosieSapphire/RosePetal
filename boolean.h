#ifndef _BOOLEAN_H_
#define _BOOLEAN_H_

#ifdef FALSE
#error "\"FALSE\" is already defined!"
#endif /* #ifdef FALSE */

#ifdef TRUE
#error "\"TRUE\" is already defined!"
#endif /* #ifdef TRUE */

#define FALSE 0
#define TRUE  1

typedef unsigned char bool_t;

#endif /* #define _BOOLEAN_H_ */
