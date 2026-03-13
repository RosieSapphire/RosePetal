#ifndef _RP_LOG_H_
#define _RP_LOG_H_

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
 *	This is the default stream that `rp_logf()` writes to. If this is
 *	not defined by the user, it will default to `stdout`. You can
 *	change the stream of output using `rp_log_set_stream()`. This
 *	does mean that you can change the stream to output to a file,
 *	and there's even a helper function `rp_log_file_open()` that
 *	will internally open a log_file. However, you will need to call
 *	`rp_log_file_close()` at the end of the program or it'll bitch.
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

#ifdef RP_LOG_STREAM_FILE
#error "RP_LOG_STREAM_FILE is already defined"
#endif /* #ifdef RP_LOG_STREAM_FILE */

#ifndef RP_LOG_STREAM_DEFAULT
#define RP_LOG_STREAM_DEFAULT stdout
#endif /* #ifndef RP_LOG_STREAM_DEFAULT */

/* This is jank, but it works for now */
#define RP_LOG_STREAM_INVALID ((void *)-1)
#define RP_LOG_STREAM_FILE    NULL

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
 * Sets the current output stream for `rp_logf()`. By default, it is
 * `stdout`, but you can also set it to something like `stderr` and so on.
 *
 * NOTE: You are able to output it to a file, and can even use an
 *	 internal file specifically for this module, however, in order
 *	 to use it, you will need to pass NULL into this function as
 *	 a special case. It's a little hacky and jank, but it works.
 *	 It will also be aliased as `RP_LOG_STREAM_FILE` to make it
 *	 *seem* less jank than it actually is. lmfao
 */
extern void
_rp_log_set_stream_internal(FILE *s, const char *file, const int line);

/*
 * Opens an internal file pointer for logging stuff out to with `rp_logf()`.
 *
 * NOTE: Make sure to call `rp_log_file_close()` before ending the program.
 *	 If you don't, it will bitch at you when the program exits.
 */
extern void
_rp_log_file_open_internal(const char *path, const char *file, const int line);

/*
 * Closes the internal file pointer for logging stuff out to with `rp_logf()`.
 *
 * NOTE: If
 */
extern void _rp_log_file_close_internal(const char *file, const int line);

/******************
 * IMPLEMENTATION *
 ******************/
#ifdef RP_LOG_IMPLEMENTATION
#include <stdint.h>

static struct {
	FILE	   *fp;
	const char *path;
	const char *file_opened;
	u32	    line_opened;
	u32	    _pad;
} _rp_log_file = { NULL, NULL, NULL, UINT32_MAX, 0 };

static FILE	     *_rp_log_stream_current	   = RP_LOG_STREAM_INVALID;
static bool_t	      _rp_exit_callback_registered = FALSE;
static rp_log_state_e _rp_log_state		   = RP_LOG_ON;

/***************
 * DEFINITIONS *
 ***************/

/*
 * Make sure that the state of the internal log file is valid.
 */
static void _rp_log_file_verify(const char *file, const int line)
{
	/* If file is open, all data must corroborate that. */
	if (_rp_log_file.fp) {
		rp_assertf_ex(_rp_log_file.path,
			      file,
			      line,
			      _rp_log_file.path,
			      "Log file is open, but path is NULL");
		rp_assertf_ex(_rp_log_file.file_opened,
			      file,
			      line,
			      "Log file is open, but the file it "
			      "was opened from's name is NULL");
		rp_assertf_ex(_rp_log_file.line_opened != UINT32_MAX,
			      file,
			      line,
			      "Log file is open, but the line of "
			      "the file it was opened on is invalid");
	}

	/* Same thing in the opposite direction. */
	rp_assertf_ex(!_rp_log_file.path,
		      file,
		      line,
		      _rp_log_file.path,
		      "Log file is closed, but path is non-NULL");
	rp_assertf_ex(!_rp_log_file.file_opened,
		      file,
		      line,
		      "Log file is closed, but the file it "
		      "was opened from's name is non-NULL");
	rp_assertf_ex(_rp_log_file.line_opened == UINT32_MAX,
		      file,
		      line,
		      "Log file is closed, but it still has the line "
		      "of the file it was originally opened from");
}

/*
 * POTENTIAL FIXME:
 * It's possible that this is only necessary on any instance of
 * `rp_log_stream_set()`, specifically when it's set to `RP_LOG_STREAM_FILE`,
 * as that's the only case where we would need to check if we left the
 * file open at the end of the program. Granted, I don't know if I'm
 * going to do anything else with this function aside from that.
 *
 * We'll see...
 */
static void _rp_log_atexit(void)
{
	_rp_log_file_verify(__FILE__, __LINE__);
	if (!_rp_log_file.fp)
		return;

	/* Only bitch if we left the log_file open. */
	_rp_log_set_stream_internal(stderr, __FILE__, __LINE__);
	_rp_logf_internal(__FILE__,
			  __LINE__,
			  "LOG: You opened a log file \"%s\" "
			  "at %s:%d, but never closed it!\n",
			  _rp_log_file.path,
			  _rp_log_file.file_opened,
			  _rp_log_file.line_opened);
	_rp_log_file_close_internal(__FILE__, __LINE__);
}

void _rp_logf_internal(const char *file, const int line, const char *fmt, ...)
{
	va_list args;

	/*
	 * FIXME: There parameters are not used for now.
	 * If they continue to not be used, remove them.
	 */
	(void)file;
	(void)line;

	/* Make sure to register the callback if it isn't already */
	if (!_rp_exit_callback_registered) {
		atexit(_rp_log_atexit);
		_rp_exit_callback_registered = TRUE;
	}

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
}

void _rp_log_toggle_internal(const rp_log_state_e s,
			     const char		 *file,
			     const int		  line)
{
	rp_assertf_ex(s == RP_LOG_OFF || s == RP_LOG_ON,
		      file,
		      line,
		      "Trying to set logging toggle to an invalid value (%u)",
		      s);
	_rp_log_state = s;
}

void _rp_log_set_stream_internal(FILE *s, const char *file, const int line)
{
	FILE  *accepted_streams[3];
	size_t i;

	/* KLUDGE: Have a preset list of acceptable streams */
	accepted_streams[0] = RP_LOG_STREAM_FILE;
	accepted_streams[1] = stdout;
	accepted_streams[2] = stderr;

	for (i = 0; i < 3; ++i) {
		if (accepted_streams[i] != s)
			continue;

		/* Either `stderr` or `stdout` */
		if (s != RP_LOG_STREAM_FILE) {
			_rp_log_stream_current = s;
			return;
		}

		/* Kludge for logging to a file */
		_rp_log_file_verify(file, line);
		rp_assertf_ex(_rp_log_file.fp,
			      file,
			      line,
			      "Log file must be open in order "
			      "for you to set the stream to it");
		_rp_log_stream_current = _rp_log_file.fp;
		return;
	}

	rp_assertf_ex(0,
		      file,
		      line,
		      "Input an invalid file stream for log file (<%p>)\n",
		      s);
}

/*
 * TODO: Make an `rp_file.h` and have this use that.
 */
void _rp_log_file_open_internal(const char *path,
				const char *file,
				const int   line)
{
	/* Ensure valid path */
	rp_assertf_ex(path,
		      file,
		      line,
		      "Trying to open log file with "
		      "a NULL pointer for a path!");

	/* Ensure file's not already open */
	_rp_log_file_verify(file, line);
	rp_assertf(!_rp_log_file.fp,
		   "Trying to open log file at %s:%d, "
		   "it was already opened from \"%s\"",
		   file,
		   line,
		   _rp_log_file.path);

	/* Open the file pointer */
	_rp_log_file.fp = fopen(path, "wb");
	rp_assertf(_rp_log_file.fp,
		   "Tried to open log file from \"%s\" at %s:%d, but failed.",
		   path,
		   file,
		   line);

	/* Fill in the blanks- so to speak */
	rp_assertf_ex(path, file, line, "Path for file is invalid!");
	rp_assertf_ex(file, file, line, "File opened for file is invalid!");
	rp_assertf_ex(line >= 0, file, line, "Line for file is invalid!");

	_rp_log_file.path	 = path;
	_rp_log_file.file_opened = file;
	_rp_log_file.line_opened = (u32)line;
}

void _rp_log_file_close_internal(const char *file, const int line)
{
	_rp_log_file_verify(file, line);
	rp_assertf(_rp_log_file.fp,
		   "Trying to close log file at "
		   "%s:%d, but it was never opened.",
		   file,
		   line);

	/* Close the file pointer */
	fclose(_rp_log_file.fp);

	/* Nullify the struct so it's valid */
	_rp_log_file.fp		 = NULL;
	_rp_log_file.path	 = NULL;
	_rp_log_file.file_opened = NULL;
	_rp_log_file.line_opened = UINT32_MAX;
}

#endif /* #ifdef RP_LOG_IMPLEMENTATION */

/************
 * WRAPPERS *
 ************/
#define rp_logf(...)	  _rp_logf_internal(__FILE__, __LINE__, __VA_ARGS__)
#define rp_log_toggle(_s) _rp_log_toggle_internal(_s, __FILE__, __LINE__)
#define rp_log_set_stream(_s)                                                  \
	_rp_log_set_stream_internal(_s, __FILE__, __LINE__)
#define rp_log_file_open()  _rp_log_file_open_internal(__FILE__, __LINE__)
#define rp_log_file_close() _rp_log_file_close_internal(__FILE__, __LINE__)

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
	rp_log_file_open("test_logfile.txt");
	rp_log_set_stream(RP_LOG_STREAM_FILE);
	rp_log_file_close();

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
