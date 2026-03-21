#ifndef _RP_FILE_H_
#define _RP_FILE_H_

/*
 * This is the file-handling submodule of the Rose Petal
 * library. It has a custom file struct meant for handling
 * the internal C file pointer, the file and line it was
 * allocated on, and the path the file was opened with.
 *
 * TODO: Refactor endian flipping to a separate file. Very useful on it's own2
 *
 * MACRO DEFINE LIST:
 *
 * RP_FILE_IMPLEMENTATION:
 *	This macro is required for all the function declarations to be
 *	accessible. Make sure you define it before including this file
 *	anywhere you want the code to be defined for linkage.
 *
 *	IMPORTANT: Make sure you only ever define this macro in
 *	           ONE C file that gets linked with any given
 *	           program, otherwise the linker will bitch at
 *	           you for a multiple definition thingy-majiggy.
 *
 *	NOTE: All the below macros assume this one is defined.
 *
 * RP_FILE_LOG:
 *	Prints to the specified RP Log stream set by `rp_log.h` whenever
 *	a file function is called, noting where and when things happened
 *	to make debugging the stuff that happens in files easier.
 *
 * RP_FILE_TEST:
 *	This defines the function definiton for testing the file
 *	module. This is only really used in the internal `test.c`
 *	file, but you can define it if you really wanna use it.
 *
 *	NOTE: All test functions in this library are static,
 *	      meaning that they can only be defined once, although
 *	      to be fair, you can only define the implementation once
 *	      anyway, so it doesn't really matter all that much.
 */

#include "rp_types.h"

enum {
	RP_FILE_FLAGS_NONE = 0,
	/* TODO:3IMPLEMENT THIS! */
	RP_FILE_FLAGS_ENDIAN_FLIP	      = (1 << 0),
	RP_FILE_FLAGS_WARN_ON_WRITE_IF_EXISTS = (1 << 1),
	/* FIXME: I could probably compare the __builtin_popcount(). lol */
	RP_FILE_FLAGS_INVALID_MASK = (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) |
				     (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) |
				     (1 << 10) | (1 << 11) | (1 << 12) |
				     (1 << 13) | (1 << 14) | (1 << 15)
};
typedef u16 rp_file_flags_f;

enum {
	RP_FILE_MODE_READ = 0,
	RP_FILE_MODE_WRITE,
	RP_FILE_MODE_APPEND,
	RP_FILE_MODE_COUNT
};
typedef u16 rp_file_mode_e;

struct rp_file {
	FILE	       *handle;
	const char     *path;
	const char     *open_file;
	u32		open_line;
	rp_file_mode_e	mode;
	rp_file_flags_f flags;
};

extern void _rp_file_open_internal(struct rp_file	*fp,
				   const char		*path,
				   const rp_file_mode_e	 mode,
				   const rp_file_flags_f flags,
				   const char		*file,
				   const int		 line);
extern void _rp_file_read_32_internal(struct rp_file *fp,
				      void	     *ptr,
				      const char     *file,
				      const int	      line);
extern void _rp_file_read_16_internal(struct rp_file *fp,
				      void	     *ptr,
				      const char     *file,
				      const int	      line);
extern void _rp_file_read_8_internal(struct rp_file *fp,
				     void	    *ptr,
				     const char	    *file,
				     const int	     line);
extern void _rp_file_write_32_internal(struct rp_file *fp,
				       const void     *ptr,
				       const char     *file,
				       const int       line);
extern void _rp_file_write_16_internal(struct rp_file *fp,
				       const void     *ptr,
				       const char     *file,
				       const int       line);
extern void _rp_file_write_8_internal(struct rp_file *fp,
				      const void     *ptr,
				      const char     *file,
				      const int	      line);
extern void
_rp_file_close_internal(struct rp_file *fp, const char *file, const int line);

#ifdef RP_FILE_IMPLEMENTATION

#ifdef RP_FILE_LOG
#define _rp_file_logf_internal(_file, _line, ...)                              \
	_rp_logf_internal(_file, _line, __VA_ARGS__)
#define rp_file_logf(...)                                                      \
	_rp_file_logf_internal(__FILE__, __LINE__, __VA_ARGS__)
#else /* #ifdef RP_FILE_LOG */
#define _rp_file_logf_internal(_file, _line, ...)                              \
	do {                                                                   \
		(void)(_file);                                                 \
		(void)(_line);                                                 \
	} while (0)
#define rp_file_logf(...) ((void)0)
#endif /* #ifdef RP_FILE_LOG #else */

static __inline void _rp_file_mode_verify(const rp_file_mode_e m)
{
	rp_assertf(m >= RP_FILE_MODE_READ && m <= RP_FILE_MODE_APPEND,
		   "Trying to open a file with an invalid mode (%u)\n",
		   m);
}

#ifdef RP_FILE_LOG
static __inline const char *_rp_file_mode_to_str(const rp_file_mode_e m)
{
	static const char *s[RP_FILE_MODE_COUNT] = { "READ",
						     "WRITE",
						     "APPEND" };
	_rp_file_mode_verify(m);
	return s[m];
}
#endif /* #ifdef RP_FILE_LOG */

static void _rp_file_verify(const struct rp_file *fp)
{
	if (fp->handle) {
		/* If we have a handle, the rest of the data better match! */
		rp_assertf(fp->path, "File has handle, but no path");
		rp_assertf(fp->open_file,
			   "File \"%s\" has handle, but "
			   "no file it was opened on",
			   fp->path);
		rp_assertf(fp->open_line != UINT32_MAX,
			   "File \"%s\" has handle, but no "
			   "line of file it was opened on",
			   fp->path);
		_rp_file_mode_verify(fp->mode);
		rp_assertf(!(fp->flags & RP_FILE_FLAGS_INVALID_MASK),
			   "File has invalid flags: 0x%.4X",
			   fp->flags & RP_FILE_FLAGS_INVALID_MASK);
		return;
	}

	/* If we DON'T have a handle, same rules apply, but opposite. */
	rp_assertf(!fp->path,
		   "File has no handle, but has a path (\"%s\")",
		   fp->path);
	rp_assertf(!fp->open_file,
		   "File has no handle, but has a "
		   "file it was opened on attached (\"%s\")",
		   fp->open_file);
	rp_assertf(fp->open_line == UINT32_MAX,
		   "File has no handle, but has a line "
		   "of a file it was opened on attached (%u)",
		   fp->open_line);
	rp_assertf(fp->mode == 0, "File is closed; mode should be 0");
	rp_assertf(fp->flags == 0, "File is closed; flags should be 0");
}

/*
 * Open a file
 *
 * TODO: Add an `atexit()` callback that checks if any files
 *       are still open at the time of the program ending.
 */
void _rp_file_open_internal(struct rp_file	 *fp,
			    const char		 *path,
			    const rp_file_mode_e  mode,
			    const rp_file_flags_f flags,
			    const char		 *file,
			    const int		  line)
{
	/* We're going to assume that we're using binary no matter what */
	static const char *mode_flags[RP_FILE_MODE_COUNT] = { "rb",
							      "wb",
							      "ab" };
	FILE		  *hnd;

	_rp_file_logf_internal(file,
			       line,
			       "FILE: rp_file_open(\"%s\", %s, 0x%.4X);\n",
			       path,
			       _rp_file_mode_to_str(mode),
			       flags);

	_rp_file_mode_verify(mode);
	hnd = fopen(path, mode_flags[mode]);
	switch (mode) {
	case RP_FILE_MODE_READ:
		rp_assertf(hnd, "File \"%s\" does not exist; can't read", path);
		break;

	case RP_FILE_MODE_WRITE:
		/* Just overwrite the file like normal. */
		if (!(fp->flags & RP_FILE_FLAGS_WARN_ON_WRITE_IF_EXISTS))
			break;

		/* Unless we specify otherwise, then it's a problem */
		rp_assertf(!hnd,
			   "File \"%s\" already exists; can't write",
			   path);
		break;

	case RP_FILE_MODE_APPEND:
		rp_assertf(hnd,
			   "File \"%s\" does not exist; can't append",
			   path);
		break;

	default:
		rp_assertf(0,
			   "Trying to open file \"%s\" "
			   "with invalid mode (%u)\n",
			   mode);
		return;
	}

	rp_assertf(line >= 0, "Line %d on file \"%s\" is invalid!", line, file);

	fp->handle    = hnd;
	fp->path      = path;
	fp->open_file = file;
	fp->open_line = (u32)line;
	fp->mode      = mode;
	fp->flags     = flags;
	_rp_file_verify(fp);
}

/* Read 32 bits from a file */
void _rp_file_read_32_internal(struct rp_file *fp,
			       void	      *ptr,
			       const char     *file,
			       const int       line)
{
	size_t nbr;
	u32    v;

	rp_assertf(fp, "File pointer is NULL");
	rp_assertf(fp->handle, "File handle is NULL");
	rp_assertf(ptr, "Trying to read a NULL pointer");
	_rp_file_verify(fp);

	_rp_file_logf_internal(file,
			       line,
			       "FILE: rp_file_read_32(.ptr = <%p>);\n",
			       ptr);

	rp_assertf(fp->mode == RP_FILE_MODE_READ,
		   "Calling read function on file "
		   "\"%s\" outside of read mode",
		   fp->path);

	nbr = fread(&v, sizeof(v), 1, fp->handle);
	rp_assertf(nbr == 1,
		   "Failed to read 4 bytes from file \"%s\" (%u)",
		   fp->path,
		   nbr);

	/* If we wanna endian-flip, dew it! */
	if (fp->flags & RP_FILE_FLAGS_ENDIAN_FLIP)
		v = __builtin_bswap32(v);

	memcpy(ptr, &v, sizeof(v));
}

/* Read 16 bits from a file */
void _rp_file_read_16_internal(struct rp_file *fp,
			       void	      *ptr,
			       const char     *file,
			       const int       line)
{
	(void)fp;
	(void)ptr;

	_rp_file_logf_internal(file,
			       line,
			       "FILE: rp_file_read_16(.ptr = <%p>);\n",
			       ptr);
}

/* Read 8 bits from a file */
void _rp_file_read_8_internal(struct rp_file *fp,
			      void	     *ptr,
			      const char     *file,
			      const int	      line)
{
	(void)fp;
	(void)ptr;

	_rp_file_logf_internal(file,
			       line,
			       "FILE: rp_file_read_8(.ptr = <%p>);\n",
			       ptr);
}

#if 0
/*
 * It's called this because it fucking explodes if ANYTHING goes wrong!
 */
static __inline void _dangerously_poke_at_memory(const void *p, const size_t s)
{
	size_t i;

	for (i = 0; i < s; ++i)
		(void)(*((const u8 *)p + i)); /* poke (pls don't kill me :c) */
}
#endif

/* Write 32 bits to a file */
void _rp_file_write_32_internal(struct rp_file *fp,
				const void     *ptr,
				const char     *file,
				const int	line)
{
	size_t niw;
	u32    v;

	rp_assertf(fp, "File pointer is NULL");
	rp_assertf(fp->handle, "File handle is NULL");
	rp_assertf(ptr, "Trying to write a NULL pointer");
	_rp_file_verify(fp);

#if 0 /* Goddammit, I'm gonna miss you... */
	/* Foot-nuke and doesn't even cover edge-cases */
	_dangerously_poke_at_memory(ptr, sizeof(v));
#endif

	rp_assertf(fp->mode == RP_FILE_MODE_WRITE,
		   "Calling write function on file "
		   "\"%s\" outside of write mode",
		   fp->path);

	memcpy(&v, ptr, sizeof(v));

	_rp_file_logf_internal(file,
			       line,
			       "FILE: rp_file_write_32(%u);\n",
			       v);

	/* If we wanna endian-flip, dew it! */
	if (fp->flags & RP_FILE_FLAGS_ENDIAN_FLIP)
		v = __builtin_bswap32(v);

	niw = fwrite(&v, sizeof(v), 1, fp->handle);
	rp_assertf(niw == 1,
		   "Failed to write 4 bytes to file \"%s\",",
		   fp->path);
}

/* Write 16 bits to a file */
void _rp_file_write_16_internal(struct rp_file *fp,
				const void     *ptr,
				const char     *file,
				const int	line)
{
	size_t niw;
	u16    v;

	rp_assertf(fp, "File pointer is NULL");
	rp_assertf(fp->handle, "File handle is NULL");
	rp_assertf(ptr, "Trying to write a NULL pointer");
	_rp_file_verify(fp);

	rp_assertf(fp->mode == RP_FILE_MODE_WRITE,
		   "Calling write function on file "
		   "\"%s\" outside of write mode",
		   fp->path);

	memcpy(&v, ptr, sizeof(v));

	_rp_file_logf_internal(file,
			       line,
			       "FILE: rp_file_write_16(%u);\n",
			       v);

	/* If we wanna endian-flip, dew it! */
	if (fp->flags & RP_FILE_FLAGS_ENDIAN_FLIP)
		v = __builtin_bswap16(v);

	niw = fwrite(&v, sizeof(v), 1, fp->handle);
	rp_assertf(niw == 1,
		   "Failed to write 4 bytes to file \"%s\",",
		   fp->path);
}

/* Write 8 bits to a file */
void _rp_file_write_8_internal(struct rp_file *fp,
			       const void     *ptr,
			       const char     *file,
			       const int       line)
{
	size_t niw;
	u8     v;

	rp_assertf(fp, "File pointer is NULL");
	rp_assertf(fp->handle, "File handle is NULL");
	rp_assertf(ptr, "Trying to write a NULL pointer");
	_rp_file_verify(fp);

	rp_assertf(fp->mode == RP_FILE_MODE_WRITE,
		   "Calling write function on file "
		   "\"%s\" outside of write mode",
		   fp->path);

	memcpy(&v, ptr, sizeof(v));

	_rp_file_logf_internal(file,
			       line,
			       "FILE: rp_file_write_8(%u);\n",
			       v);

	/* No endian-flipping here, it's a single byte! :D */

	niw = fwrite(&v, sizeof(v), 1, fp->handle);
	rp_assertf(niw == 1,
		   "Failed to write 4 bytes to file \"%s\",",
		   fp->path);
}

/* Close a file */
void _rp_file_close_internal(struct rp_file *fp,
			     const char	    *file,
			     const int	     line)
{
	rp_assertf(fp, "File container is NULL");
	rp_assertf(fp->handle, "Trying to close an non-open file.");
	_rp_file_verify(fp);

	_rp_file_logf_internal(file,
			       line,
			       "FILE: rp_file_close(\"%s\");\n",
			       fp->path);

	fclose(fp->handle);
	fp->handle    = NULL;
	fp->path      = NULL;
	fp->open_file = NULL;
	fp->open_line = UINT32_MAX;
	fp->mode      = 0;
	fp->flags     = 0;
	_rp_file_verify(fp);
}

#ifdef RP_FILE_TEST

/*
 * Temporarily define the helper macros for convinience, but
 * undefining them before they're actually included anywhere else.
 */
#define rp_file_open(_fp, _path, _mode, _flags)                                \
	_rp_file_open_internal(_fp, _path, _mode, _flags, __FILE__, __LINE__)
#define rp_file_read_32(_fp, _ptr)                                             \
	_rp_file_read_32_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_read_16(_fp, _ptr)                                             \
	_rp_file_read_16_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_read_8(_fp, _ptr)                                              \
	_rp_file_read_8_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_write_32(_fp, _ptr)                                            \
	_rp_file_write_32_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_write_16(_fp, _ptr)                                            \
	_rp_file_write_16_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_write_8(_fp, _ptr)                                             \
	_rp_file_write_8_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_close(_fp) _rp_file_close_internal(_fp, __FILE__, __LINE__)

static void rp_file_test(void)
{
	/*
	 * TODO: Make a much more brutal test, similar to `rp_memory.h`
	 *       where it randomly generates the sizes and values to
	 *       write to the files and effectively creates a "checksum"
	 *       to compare the read-back data with at the end. This is
	 *       just too simple to thoroughly fuck with yet.
	 *
	 *	Anyway, I started working on this file module because
	 *	I wanted to make a DAW and decided I needed to make
	 *	my file API RIGHT THEN AND THERE, so I'm fucking tired
	 *	and it's 9 PM, and I haven't really gotten anything done
	 *	except for this... And I was supposed to be working on Croc...
	 *	god... fucking... dammit...
	 */

	static const char *fpath   = "afroman.bin";
	static const u32   t32_cmp = 1286969;
	static const u16   t16_cmp = 42069;
	static const u8	   t8_cmp  = 231;
	struct rp_file	   f;
	u32		   t32;
	u16		   t16;
	u8		   t8;

	rp_file_logf("\n[TEST] rp_file.h:\n\n");

	/*
	 * If all goes well, these values should be
	 * the exact same when we go to load the file.
	 */
	t32 = t32_cmp;
	t16 = t16_cmp;
	t8  = t8_cmp;

	/* Write these test values out to a file and close it. */
	rp_file_open(&f, fpath, RP_FILE_MODE_WRITE, RP_FILE_FLAGS_NONE);
	rp_file_write_32(&f, &t32);
	rp_file_write_16(&f, &t16);
	rp_file_write_8(&f, &t8);
	rp_file_close(&f);

	/* Then open the file back up in read mode and load the shit in */
	rp_file_open(&f, fpath, RP_FILE_MODE_READ, RP_FILE_FLAGS_NONE);
	rp_file_read_32(&f, &t32);
	rp_file_read_16(&f, &t16);
	rp_file_read_8(&f, &t8);
	rp_file_close(&f);

	/* Now we have to make sure it's correct. If it's not, we're fucked. */
	rp_assertf(t32 == t32_cmp,
		   "32-bit value from file \"%s\" read %u does not "
		   "match the correct value %u. Test fucking failed.",
		   fpath,
		   t32,
		   t32_cmp);
	rp_assertf(t16 == t16_cmp,
		   "16-bit value from file \"%s\" read %u does not "
		   "match the correct value %u. Test fucking failed.",
		   fpath,
		   t16,
		   t16_cmp);
	rp_assertf(t8 == t8_cmp,
		   "8-bit value from file \"%s\" read %u does not "
		   "match the correct value %u. Test fucking failed.",
		   fpath,
		   t8,
		   t8_cmp);
}
#endif /* #ifdef RP_FILE_TEST */

#undef rp_file_close
#undef rp_file_write_8
#undef rp_file_write_16
#undef rp_file_write_32
#undef rp_file_read_8
#undef rp_file_read_16
#undef rp_file_read_32
#undef rp_file_open

#endif /* #ifdef RP_FILE_IMPLEMENTATION */

#define rp_file_open(_fp, _path, _mode, _flags)                                \
	_rp_file_open_internal(_fp, _path, _mode, _flags, __FILE__, __LINE__)
#define rp_file_read_32(_fp, _ptr)                                             \
	_rp_file_read_32_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_read_16(_fp, _ptr)                                             \
	_rp_file_read_16_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_read_8(_fp, _ptr)                                              \
	_rp_file_read_8_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_write_32(_fp, _ptr)                                            \
	_rp_file_write_32_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_write_16(_fp, _ptr)                                            \
	_rp_file_write_16_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_write_8(_fp, _ptr)                                             \
	_rp_file_write_8_internal(_fp, _ptr, __FILE__, __LINE__)
#define rp_file_close(_fp) _rp_file_close_internal(_fp, __FILE__, __LINE__)

#endif /* #ifndef _RP_FILE_H_ */
