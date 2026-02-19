
# 🌹Rose Petal🌹
Rose Petal is an API I am writing to serve as something of a base-layer for many programs I may write in the future. It consists of multiple different modules that get built into static libraries that are then linked with whatever program you want.

I decided to go with static libraries since this is mean to be small and compact, and you can only link the modules that you need and leave out all the crap that you don't. I also just really don't like the "header-only" style of APIs since I find them to both be really ugly and extremely cumbersome to work with, although a lot of people feel the same way about linking libraries, so you can't please everyone. lmfao

> Quick Links
> [Modules](#modules)
> - [rose_petal.h](#rose_petal.h)
> - [rp_assert.h](#rp_assert.h)
> - [rp_memory.h](#rp_memory.h)
>
> [Building](#building)

# Modules
## rose_petal.h
This is not really a module and more of a catch-all that includes all the below-specified modules. This is generally ill-advised since the whole point is for it to be minimal and you should only include the stuff you're actually going to use, but if it comes down to it and it's easier in your project and you don't care about size, then go ahead.

__NOTE: You will need to link any libraries that it depends upon, so be weary.__
## rp_assert.h
This is just a janky-ass custom assertion module I wrote for being able to write out custom messages to the terminal when asserting something, instead of it just saying that an assertion failed and that's it. It also mitigates having to do that butt-fucking-ugly method of doing `assert(condition && "String about your assertion");` because let's be honest that's FUCKING DOG SHIT!

Other than that, it's pretty simple and has 3 main macros that are used:

### `RP_ASSERT(cond)`
Your bog-standard assert call. It prints out that your assertion failed as a regular call to `assert()` does and calls `abort()` internally to halt the program in its tracks.

### `RP_ASSERTM(cond, msg)`
This one does the same as above, but it allows you to specify a custom message when the assertion triggers. NOTE: This does not allow you to pass in a format; that is what the one just below is for.

### `RP_ASSERTF(cond, fmt, ...)`
THIS is the one you wanna call when you have a format string you want to output alongside your assertion. This is EXTREMELY handy for actually knowing WHY your code is crashing and makes debugging marginally easier (at least in my opinion, hence why I added it. lmao)
## rp_memory.h
This module is responsible for providing a custom memory allocator. It's a bit rough around the edges, but it's mainly for debugging purposes and wouldn't usually be used in a final shipping product, so whatever.

The idea is that you can either call the `rp_mem_alloc()` and `rp_mem_free()` functions directly to make an explicit link between them, or you can enable the `RP_MEM_WRAP_STD` macro to have it wrap any calls to `malloc()` and `free()` such that when the memory debugger is disabled, it will just use regular calls to malloc and free.

Bear in mind that I'm kind of a dumbass when it comes to memory, so it'll probably be rough around the edges, and the current implementation just uses a fixed-size buffer (defined by `RP_MEM_MAX_ALLOC_CNT`), but at some point I'd like to replace it with a dynamic buffer that gets reallocated to twice its size once the limit is exceeded. That and I'll also have to do sorting of the blocks when a free occurs or at least every-so-often.

One idea I had was that instead of it re-ordering the internal array after every free, it leaves all the empty slots that have been free'd until it tries to allocate a new pointer and isn't able to since there are no free slots. At this point, it would re-order the array, with the free'd slots at the end, marking them all as usable, and then you can keep going like that until it's decided that there needs to be twice the memory allocated again.

That way it doesn't perpetually slow down when trying to re-order the memory after every free, but it also doesn't bog the program down with reallocations of the internal array, since it doubles-up on the memory pool every time it hits the limit, causing it to happen exponentially less frequently as more memory is allocated.

There is also a `rp_mem_free_all()` function that frees all the memory currently being kept track of. This *can* be used as a substitute for actually managing your memory (like you fucking should), but I'd advise against that and use it more as a last resort, although, that's really up to you.

# Building
TODO: Unimplemented. lol
