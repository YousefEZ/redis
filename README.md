# Redis
Custom built redis cache server and client in C++. This is a prototype not meant for production use.

> This takes inspiration from  Build Your Own Redis with C/C++ Network programming, data structures, and low-level C by James Smith

## ⚙️ Building

First configure and generate the build files in `build/` subdirectory

```bash
cmake -B build
```

and then you can build using 

```bash
cmake --build build
```


## 🚀 Running

You can then spin an instance using the executable in `build/src/Server` and then use the Client defined in redis::SyncClient, or use the cli in `build/src/Client`.



