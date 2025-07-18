<h1>Wu's Line Drawing and Circle Drawing Algorithm in C</h1>
This repository implements the 1991 paper, "An Efficient Anti-Aliasing Technique" in the C programming language for LeetArxiv.

The full coding guide is available at this [link.](https://leetarxiv.substack.com/p/an-efficient-anti-aliasing-technique)

## Summary
Xiaolin Wu published his line-drawing algorithm to ensure fast anti-aliasing. The algorithm implements a two-point anti-aliasing scheme to model the physical image of the curve.

We dive into the paper's details [on LeetArxiv.](https://leetarxiv.substack.com/p/an-efficient-anti-aliasing-technique)
You can run this code using gcc
```
 clear && gcc main.c -lm -lgmp -o m.o && ./m.o
```
and this will generate the image below

![Lines and Circles drawn using Wu's algorithm ](output.png)
