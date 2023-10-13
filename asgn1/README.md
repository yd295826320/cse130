#Assignment 1 directory

This directory contains source code and other files for Assignment 1

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.

I went to vincent's discussion time and got some help to start his assignment. I read through his slide to get the idea how to do read and write.
Then I got some ideas about how to you use strtok using google search. 
The professor's lecture gives me some ideas about the man page and did some research about how is read, write and open works. 
especially the open, for it's flags and mode. For mode, the professor was telling us to use 0 as defult but we need 777 to make the created file have the permission.

There were a lot of problems I had when I debugging myself. But I only wrote down the last two of them:
for the content, what if there is huge amount of content and cannot fit into the buffer? 
Can we consider the first read  can read out all the other context in the command?

I was using some other ways to make sure it can handle the large context. (p.s. I found I could just use the loop in the get function to help me read infinatly)

So my structure is like, you use a while loop at first to make sure it can get all the commands or get full with the buffer. (failed with get_partical)
then you use strtok to tokenlize the order, filename and content length. 
For get, if there is a content length, that means you are getting extra, so return 1 for that.
check if there is a file or not, then check the path max
then open up the file, check if it can open or not, if it can, then read into it and use loop to read and write, then close everything.

For set, I first check if the content length is exist or not. If it exist, I will do a atoi to make it into a integer.
Then, I would to open the file with flags and mode 777. 
After that, I would get the pointer inside the buffer where is the starting of the content.
For the content length, if it's bigger than the buffer size, then I should use the buffer size instead.
I add a check for if it's Null then just close it and send ok to stdout, but this is unnecessary.
Then write everything from the cont to the file.
Then use the get loop to read everything left in the buffer into the file.
Then, close everything and send ok to stdout.


