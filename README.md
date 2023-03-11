# Welcome to Gramado OS


This repository has the source code for:

* Gramado boot loader
* Gramado kernel
* Gramado Window Server
* Applications
* Commands

## What is Gramado?

    Gramado (c) is a 64bit multithreaded operating system.

## Screenshots

![Screenshot](https://raw.githubusercontent.com/frednora/screenshots/main/gramado-7.png)
![Screenshot](https://raw.githubusercontent.com/frednora/screenshots/main/gramado-3.png)

## Kernel features.

> * Paging.
> * Threads.
> * FAT16 file system.
> * e1000 NIC driver.
> * IDE PATA. 
> * ps/2 keyboard. 
> * ps/2 mouse works fine only on qemu.
> * Serial port. 

## Userland features.

> * Window Server. See: gramland/burgundy/server/
> * Some few clients connected to the window server via unix-sockets.
> * Unix-like commands running in the virtual console.

## Libraries.

> * unix-like library. (rtl)
> * Lib for the window server's clients. (libgws)

## Folders.
A brief description of the directories.

    Gramado Kernel - core/
    Gramado DE     - desktop/

 * `core/` - Low level basic components of the system.  
   * `commands` - Unix-like commands and more.
   * `init` - The init process.
   * `libs` - Libraries.
   * `net` - Network stuff.
   * `os` - Main ring0 Components.
     * `boot` - The boot loader.
     * `disk` - The root directory for the boot partition.
     * `kernel` - The Gramado OS kernel.
 * `desktop/` - The window server and it's clients.
   * `apps` - Client applications.
   * `eng` - Graphics engine.
   * `gramland` - Window server.
   * `libs` - Libraries.

## Fred's host machine

```
    Host machine: Ubuntu 20.04.5 LTS
    Linux 5.4.0-137-generic x86_64
    gcc (Ubuntu) 9.4.0 
    GNU ld (GNU Binutils for Ubuntu) 2.34
    NASM version 2.14.02
```

## How to Build?

> See the [Gramado OS build instructions](https://github.com/frednora/gramado/blob/master/core/admin/docs/build.md)

## Building and running on vm

```bash
$ make
$ cd core
$ ./run
```

## Clean up the mess

```bash
$ cd core
$ make clean-all
```

The old versions were tested for a long period of time on my 
real machine. A Gigabyte/Intel Core 2 Duo/Chipset VIA.
Now i'm testing only on qemu and virtualbox.
Please, give us some feedback.

## Documentation

> See the [docs](https://github.com/frednora/gramado/tree/master/core/admin/docs).

## Author

* **Fred Nora** - [frednora](https://twitter.com/frednora)

## Contributors

* **Fred Nora** - [frednora](https://facebook.com/frednora)

## License

Gramado is a Free and Open Source operating system.
The source code uses the BSD license.

# Quotes:
> Oh boy, there is no spoon!
> The beast hates science and the scientist.
