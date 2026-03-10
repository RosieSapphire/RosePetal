#ifndef _RP_STATIC_ASSERT_H_
#define _RP_STATIC_ASSERT_H_

#ifdef RP_STATIC_ASSERT
#error "The \"RP_STATIC_ASSERT\" macro was already defined elsewhere!"
#endif /* #ifndef RP_STATIC_ASSERT */

#define RP_STATIC_ASSERT(_cond, _msg)                                          \
	extern char rp_static_assert_fail_##_msg[(_cond) ? 1 : -1]

#endif /* #ifndef _RP_STATIC_ASSERT_H_ */
