# SimplifiedShell

A Simplified shell written in C

The Shell can handle multiple commands, such as:


	help

	info
  
	cd
  
	pwd

	ex
  
	exb
  
	ls
  
  
As well as this, the shell supports piping. 

For example: ex ls | ex head -2 | ex tail -3

The shell can take the output from one command and redirect it to another

For example: ex ls > list.txt

## Running the program

open up the directory and type 'make' to compile the shell.

Then type ./sc16lep to begin the program.
