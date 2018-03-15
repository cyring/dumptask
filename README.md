# dumptask

## Build
 1- Download or clone the source code into a working directory.
 
 2- Build the programs.
```
make
```

## Run (as root)

 3- Load the kernel module.
```
modprobe dumptask
```
 _or_
```
insmod dumptask.ko
```
 4- List the tasks.
```
showtask
```

### Unload

 5- Unload the kernel module with the ```rmmod``` command
```
rmmod dumptask.ko
```

## Screenshots
### Linux kernel module
Use ```dmesg``` or ```journalctl -k``` to check the module state
```
dumptask: loaded

dumptask: unload
```

![alt text](http://blog.cyring.free.fr/images/dumptask_first.png "dumptask")  

![alt text](http://blog.cyring.free.fr/images/dumptask_last.png "dumptask")  

# About
[CyrIng](https://github.com/cyring)

Copyright (C) 2017-2018 CYRIL INGENIERIE
 -------
