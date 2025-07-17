# Rose Petal
I have completely lost track of the amount of times I've started a new project and written the same code over and over and over. All of it's just boilerplate necessary for getting things set up. Whether it's a vector type for moving stuff around on the screen or denoting a position, profiling code to see if your optimizations are working, or just keeping track of memory so that you can make sure you freed all of the stuff you've allocated through the course of the program.

That is why I've taken it upon myself to create a library that does all of these base-level things so that making programs is faster and easier as I don't need to worry about rewriting all these low-level processes when I start a new project that utilizes them.

# Features (UNFINISHED)
## Vector Types (UNFINISHED)
One of the cornerstones of any program or involving keeping track of or modifying positions, velocities, etc. is having solid vector types.
### Usage
## Debugger (UNFINISHED)
This is a bit of loose terminology here, since it's not a debugger in the traditional sense--- a separate program that analyzes all of your variables and memory addresses for leaks, errors, and whatnot--- more specifically for printing out different types of messages: info, warnings, errors, fixmes, todo, etc.

As important as it is to print out information to know what is going wrong and where, it's equally important to know the severity or category of what you're printing. But having to type "ERROR:", "WARNING:" all these things before every single message is quite annoying, which is why this API handles this automatically via an enum for the output type.

Additionally, when initializing the debug system, you can pass in the different streams for where these things get output. There is a default parameter list which uses `stderr` for error logs and `stdout` for everything else. However, this setup allows you to setup your own file stream for each of these log types, allowing you to write the output to a file, or multiple files depending on the log, of your choice.
### Usage
