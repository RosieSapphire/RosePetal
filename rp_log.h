#ifndef _RP_LOG_H_
#define _RP_LOG_H_

#include <stdio.h>

#include "rp_types.h"

/*
 * This is the logging submodule for Rose Petal. It's effectively
 * a wrapper for `printf` and other similar functions, but with
 * the ability to change the output for the logging and enable
 * or disable it depending on your particular needs.
 *
 * Here are the macros:
 *
 * RP_LOG_IMPLEMENTATION:
 *	Required to define the implementation for the logging
 *	functionality. As is the case with all header-only
 *	libraries in C, this is required to ONLY be defined
 *	in a single location as to not cause a double-link error.
 *
 *	NOTE: All below macros assume this is enabled.
 *
 * RP_LOG_DISABLE:
 *	This one's pretty self-explanatory. If this macro is
 *	defined, then all calls for `rp_logf()` or whatever will
 *	result in a no-op. Particularly helpful if you wanna
 *	disable all logging functionality in a program, but
 *	don't wanna have to go through and gut all the calls,
 *	as that could be pretty fucking annoying.
 *
 *	NOTE: You can also call `rp_log_toggle()` with `RP_LOG_ON`
 *	      and `RP_LOG_OFF` in order to dynamically decide when
 *	      you want logging to start and end, HOWEVER this will
 *	      only work if this macro is NOT defined, otherwise
 *	      it'll just result in a no-op regardless.
 *
 * RP_LOG_STREAM_DEFAULT:
 *	This is the default stream that `rp_logf()` writes to.
 *	If this is not defined by the user, it will default to
 *	`stdout`. You can change the stream of output using
 *	`rp_log_set_stream()`. This does mean that you can
 *	change the stream to output to a file, however you
 *	will have to provide the file yourself via `rp_file_open()`,
 *	or `fopen()`, as it just takes an STD `FILE *`,
 *	so you've got options. lol
 *
 * RP_LOG_WARN_DUPLICATE_REGISTERS:
 *	This is a bit of a hyper-specific one that deals with how
 *	multiple calls to `rp_log_register_callback()` are handled.
 *
 *	Essentially, there are
 *
 * RP_LOG_TEST:
 *	This defines the `rp_log_test()` function for making sure
 *	that the logging system works, although this is usually only
 *	used in the internal library itself with `test.c`, but you
 *	can call it wherever you want if you really want to. lmfao
 */

#ifdef RP_LOG_STREAM_INVALID
#error "RP_LOG_STREAM_INVALID is already defined"
#endif /* #ifdef RP_LOG_STREAM_INVALID */

#ifndef RP_LOG_STREAM_DEFAULT
#define RP_LOG_STREAM_DEFAULT stdout
#endif /* #ifndef RP_LOG_STREAM_DEFAULT */

/* This is jank, but it works for now */
#define RP_LOG_STREAM_INVALID ((void *)-1)

enum { RP_LOG_OFF = 0, RP_LOG_ON };
typedef u8 rp_log_state_e;

/**************
 * PROTOTYPES *
 **************/

/*
 * Prints to a stream specified by `rp_log_set_stream()`;
 * defaults to `stdout` if `RP_LOG_STREAM_DEFAULT` is
 * not already defined by the user.
 */
extern void
_rp_logf_internal(const char *file, const int line, const char *fmt, ...);

/*
 * Turn the `rp_logf()` statements on or off.
 *
 * NOTE: If `RP_LOG_DISABLE` is defined, then
 *       logging will not work no matter what.
 */
extern void _rp_log_toggle_internal(const rp_log_state_e s,
				    const char		*file,
				    const int		 line);

/*
 * Sets the current output stream for `rp_logf()`. By default,
 * it is `stdout`, but you can also set it to something like
 * `stderr` or even a custom file pointer you provide yourself.
 *
 * NOTE: If you provide a custom file, it MUST already be open
 *       at the time of usage, otherwise, shit will get fucky.
 */
extern void
_rp_log_set_stream_internal(FILE *s, const char *file, const int line);

/*
 * Gets the current output stream for `rp_logf()`. This can be really
 * handy if you need to change the log stream for a specific function
 * or section of code, but then need to set it back once it's over.
 */
extern FILE *_rp_log_get_stream_internal(const char *file, const int line);

/******************
 * IMPLEMENTATION *
 ******************/
#ifdef RP_LOG_IMPLEMENTATION
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

static FILE	     *_rp_log_stream_current = RP_LOG_STREAM_INVALID;
static rp_log_state_e _rp_log_state	     = RP_LOG_ON;

/***************
 * DEFINITIONS *
 ***************/

void _rp_logf_internal(const char *file, const int line, const char *fmt, ...)
{
#ifndef RP_LOG_DISABLE
	va_list args;

	/*
	 * FIXME: There parameters are not used for now.
	 * If they continue to not be used, remove them.
	 */
	(void)file;
	(void)line;

	/*
	 * Since I can't set `_rp_log_stream_current` staticly to
	 * `RP_LOG_STREAM_DEFAULT`, here's my shitty fucken work-around
	 * where I check if it's `RP_LOG_STREAM_INVALID` and do mah stuff.
	 *
	 * Admittedly, aside from initialization this would also help
	 * if it just so happens to be invalid, although this is one
	 * invalid value out of ~18.4 quintillion, so maybe `INVALID`
	 * is a bit of a loaded term in this case.
	 *
	 * TODO: Maybe change it from `INVALID` to `INITIAL` or something.
	 */
	if (RP_LOG_STREAM_INVALID == _rp_log_stream_current)
		_rp_log_stream_current = RP_LOG_STREAM_DEFAULT;

	va_start(args, fmt);
	vfprintf(_rp_log_stream_current, fmt, args);
	va_end(args);
#else  /* #ifndef RP_LOG_DISABLE */
	(void)file;
	(void)line;
	(void)fmt;
#endif /* #ifndef RP_LOG_DISABLE #else */
}

void _rp_log_toggle_internal(const rp_log_state_e s,
			     const char		 *file,
			     const int		  line)
{
#ifndef RP_LOG_DISABLE
	rp_assertf_ex(s == RP_LOG_OFF || s == RP_LOG_ON,
		      file,
		      line,
		      "Trying to set logging toggle to an invalid value (%u)",
		      s);
	_rp_log_state = s;
#else  /* #ifndef RP_LOG_DISABLE */
	(void)s;
	(void)file;
	(void)line;
#endif /* #ifndef RP_LOG_DISABLE #else */
}

void _rp_log_set_stream_internal(FILE *s, const char *file, const int line)
{
#ifndef RP_LOG_DISABLE
	rp_assertf_ex(s,
		      file,
		      line,
		      "Trying to set log stream to NULL pointer!");
	_rp_log_stream_current = s;

#else  /* #ifndef RP_LOG_DISABLE */
	(void)s;
	(void)file;
	(void)line;
#endif /* #ifndef RP_LOG_DISABLE #else */
}

FILE *_rp_log_get_stream_internal(const char *file, const int line)
{
	/* TODO: Remove this stupid shit from the whole API... */
	(void)file;
	(void)line;

	rp_assertf(_rp_log_stream_current,
		   "Trying to get RPLog's current stream, but it's NULL.");

	return _rp_log_stream_current;
}

#endif /* #ifdef RP_LOG_IMPLEMENTATION */

/************
 * WRAPPERS *
 ************/
#define rp_logf(...)	  _rp_logf_internal(__FILE__, __LINE__, __VA_ARGS__)
#define rp_log_toggle(_s) _rp_log_toggle_internal(_s, __FILE__, __LINE__)
#define rp_log_set_stream(_s)                                                  \
	_rp_log_set_stream_internal(_s, __FILE__, __LINE__)
#define rp_log_get_stream() _rp_log_get_stream_internal(__FILE__, __LINE__)

/*
 * Test function. Defined outside of the implementation scope,
 * since if you're defining this manually, you're presumably
 * only doing it in a single translation unit for testing.
 *
 * Other than that, I'm mostly putting it down here
 * so it occurs after the macro wrapper defines,
 * and it looks a lot less butt-fuckingly ugly.
 */
#ifdef RP_LOG_TEST

#include "rp_file.h"

static void rp_log_test(void)
{
	/*
	 * On the offchance that `RP_LOG_STREAM_DEFAULT`
	 * is defined elsewhere or as something else, we
	 * wanna make sure the first instance of it
	 * always prints out to `stdout`.
	 */
	rp_log_set_stream(stdout);

	/* Of course, there's the [TEST] convention! */
	rp_logf("\n[TEST] rp_log.h\n\n");

	/* Then we start with `stdout` */
	rp_logf("Testing printing out to `stdout`.\n");

	/* Then try disabling logging temporarily */
	rp_log_toggle(RP_LOG_OFF);
	rp_logf("This message should not show up at all.\n");
	rp_log_toggle(RP_LOG_ON);

	/* Now try `stderr` */
	rp_log_set_stream(stderr);
	rp_logf("Testing printing out to `stderr`.\n");

	/*
	 * TODO: Implement this after implementing `rp_file.h`.
	 * The reason is that it's gonna require reading in the
	 * whole buffer from the file we wrote to and then logging
	 * that out to `stdout`, so that'll be a requirement.
	 */
#if 0
	/* Now we're gonna try logging to a file */
	rp_file_open("test_logfile.txt", RP_FILE_MODE_READ, );
	rp_log_set_stream(RP_LOG_STREAM_FILE);
	rp_file_close(NULL);

	{
		char *lfb;
		size_t sz;

		lfb = rp_file_read_to_buffer("test_logfile.txt", &sz);
		rp_log_set_stream(stdout);
		rp_logf("Log file contents:\n\n%s\n", lfb);
		free(lfb);
	}
#endif
}
#endif /* #ifdef RP_LOG_TEST */

#endif /* #ifndef _RP_LOG_H_ */
