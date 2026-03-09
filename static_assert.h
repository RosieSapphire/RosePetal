#ifndef _STATIC_ASSERT_H_
#define _STATIC_ASSERT_H_

#ifdef STATIC_ASSERT
#error "The \"STATIC_ASSERT\" macro was already defined elsewhere!"
#endif /* #ifndef STATIC_ASSERT */

#define STATIC_ASSERT(_cond, _msg)                                             \
	extern char static_assert_fail_##_msg[(_cond) ? 1 : -1]

#endif /* #ifndef _STATIC_ASSERT_H_ */
