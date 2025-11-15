# SwiftServe: OS-Inspired HTTP Web Server (C Language)

SwiftServe is a lightweight, OS-inspired HTTP web server built in C to demonstrate core Operating System concepts such as:

* Multithreading
* Scheduling
* Synchronization
* Caching (LRU)
* Inter-Process Communication (IPC)
* Socket-based networking

---

## 🚀 1. Project Overview

SwiftServe is a modular, high‑performance HTTP server built for an OS course project. It is fully implemented in C, using POSIX threads, UNIX domain sockets, file system syscalls, and pipes for logging.

The objective is to simulate how an OS kernel handles:

* Thread scheduling (via a thread pool)
* Medium‑term memory caching (LRU cache)
* Static + dynamic content serving
* Process separation for logging
* Synchronization through mutexes & rwlocks

---

## 📁 2. Folder Structure

```
OS_PBL/
│
├── http_server            # Compiled executable
├── server.log            # Logs written by logger process
│
├── include/              # Header files (modules)
│   ├── cache.h
│   ├── client_handler.h
│   ├── connection_handler.h
│   ├── http_methods.h
│   ├── logger.h
│   ├── router.h
│   ├── routes.h
│   ├── server.h
│   └── thread_pool.h
│
├── public/               # Static files served by HTTP GET
│   ├── index.html
│   └── styles.css
│
├── src/                  # All .c source modules
│   ├── cache.c
│   ├── client_handler.c
│   ├── connection_handler.c
│   ├── http_methods.c
│   ├── logger.c
│   ├── main.c
│   ├── router.c
│   ├── routes.c
│   ├── server.c
│   └── thread_pool.c
│
└── README.md             # This file
```

