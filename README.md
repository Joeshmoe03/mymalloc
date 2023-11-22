to test: simply "make" and run "LD_PRELOAD=./my-malloc.so ls" or some other program of interest

known issues: faulty freeing when it comes to testing the program on "cat some-file" leading to segfault. You can investigate the issue with this: "gdb --args env LD_PRELOAD=./my-malloc.so cat my-malloc.c"
