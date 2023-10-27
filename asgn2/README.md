#Assignment 2 directory

This directory contains source code and other files for Assignment 2.

Use this README document to store notes about design, testing, and
questions you have while developing your assignment.


For this assignment, I started with the idea of using sscanf but just in case for hidden tests. I learned about regex and switch to it.
For the stucture of this project:
1. get the port number to int from the command line.
2. create a socket, initialize it with the port number
3. Use a while loop to keep accpeting the socket
4. Inside the request, read from the socket for once, then handling the request field in it.
5. using regex to split the request into Method, URI and Version. check if each of them is vaild
6. using a while loop to read through the header field, check each key and value. store the conten length
7. check if it ends with \r\n
8. check if it get or put then go to the order to handle
9. if it's a get, check if it's regular file and readable
10. if it's a put, check if the file exist and if not then give permission to it and write into it
