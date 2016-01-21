Assignment 0_1:

The first program achieves the first one-hundred million rational numbers and
stores them in an array (named arry). I chose to have one global member
variable (named bound) which stores the number of rational numbers the user is
wishing to find. I stored the first number in the array (which in the pdf was
given to us as one (1/1)), and used a for loop with the following rules:
      starts at zero and incrementally goes to bound
      checks to see if either of the two conditions are true:
      	     2*i+1 < one million
	     	   arry[2*i+1] = arry[i];
	     2*i+2 < one million	
	     	   arry[2*i+2] = arry[i] + arry[i+1];

      after all of those tests pass, prints out the value at arry[i].

here are two test cases:

*test1*
bound = 10;
arry[0] = 1;

output:

1/1
1/2
2/1
1/3
3/2
2/3
3/1
1/4
4/3
3/5

Assignment0_2:

The objective of the second was to print out the nth rational number given an integer n (user command line provided). A little bit about what's inside:
    There is my main function, which first checks to make sure the user provided the correct amount of arguments (name of file, and n) and returns -1 if it fails, if it proceeds, it converts the string form of n provided to an integer using the function atoi(). It then uses this integer n in my function 'rec_tree' which takes in an integer, and prints it out in the following format: rec_tree(n-1) / rec_tree(n). If all goes well, the main function will return 0.
    Now a little about what rec_tree(n) does. It has a general case if (n<2) in which it returns 1. The other two cases are determined if n is odd or even. If n is even, it returns the sum of rec_tree((n/2)-1) and rec_tree(n/2). If it's odd, it'll return rec_tree(n/2).


Here's a couple test cases:

Command line: ./assignment0_2 5

output: 3/2



command line: ./assignment0_2 15

output: 4/1
