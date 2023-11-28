questions: The worker thread and the dispatcher thread. : thread create n times and then push them 
Do we only need to put a lock for read and write? What about the log? 
Where do we need to send the message for log?

// infinit looping
// seg fault with the number less than 4

//debugging: core dump

//unnessary struct

the questions on top are the ones I had during this assignment.
The hashtable I was using a simple hash table founded online.

the structure of this assignment is to create t workers threads and use them when popping the request out of the queue.
Then handle them within the reader and writer lock.
