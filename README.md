# 💻 SO2 
Homework for the second module of Operating Systems. The comments in the files are in italian since the course was in italian.

# Description
The program navigates recursively a directory and copies all the regular/executable files into two dynamically created folders: regulars and executables. Before copying the files in the corresponding folders a low level chown was applied to each file. To keep track of the files I've created a sort of dynamic array that worked as a "dictionary".

## Instructions 
-Compile: gcc -Wall progetto.c -o progetto.out 
# 
-Execute: ./progetto.out "path_name" "desired_depth" "processes"
