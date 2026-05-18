# libftpp
A modular C++ library with reusable building blocks for data handling, design patterns, threading, networking, math utilities and timing.

## Table of Contents
- [Overview](#overview)
- [Modules](#modules)
  - [Data Structures](#data-structures)
  - [Design Patterns](#design-patterns)
  - [IO and Logging](#io-and-logging)
  - [Threading](#threading)
  - [Network](#network)
  - [Mathematics](#mathematics)
  - [Clock](#clock)
- [Build](#build)
- [Test](#test)

## Overview
This repository contains a compact C++ utility library designed for modern projects. It provides:
- a JSON-like `Field` container and a simple `Loader` parser
- common design patterns (`Singleton`, `Factory`, `Observer`, `Memento`, `StateMachine`)
- thread-safe I/O and logging utilities
- concurrent building blocks (`Thread`, `WorkerPool`, `ThreadSafeQueue`, `PersistentWorker`)
- lightweight networking support with `Client`, `Server`, and typed `Message`
- mathematical helpers for vectors, random coordinate generation and Perlin noise
- timing helpers including `Timer` and `Chronometer`

The umbrella header `include/libftpp.hpp` brings together the main categories of the library.

## Modules

### Data Structures
- `Pool<T>`: object pool that stores and reuses a fixed number of items, useful for avoiding repeated allocations.
- `DataBuffer`: a binary buffer with `<<` and `>>` serialization operators for primitive values and strings.
- `Field`: a JSON-like variant container supporting `string`, `double`, `bool`, `null`, `FieldVector`, and `FieldMap`.
- `Registry<T>`: a singleton registry for storing named values or services by key.

### Design Patterns
- `Singleton<T>`: thread-safe base class for creating a single shared instance of a type.
- `Observer<TEvent>`: publish/subscribe pattern for event handling.
- `Memento`: save and restore object state through serialized snapshots.
- `StateMachine<TState>`: add states, transitions and actions, then drive state changes.
- `Factory`: register creation callbacks and instantiate objects from `FieldMap` data.

### IO and Logging
- `ThreadSafeIOStream`: synchronized console I/O with optional prefixes and prompt support across multiple threads.
- `Logger`: singleton logger with severity levels, optional timestamps and formatted log lines.
- `Loader`: lightweight JSON parser returning a `Field` tree representation from a file.

### Threading
- `Thread`: a small wrapper around `std::thread` that runs a named worker function.
- `ThreadSafeQueue<T>`: a mutex-protected deque queue for safe producer/consumer usage.
- `WorkerPool`: a pool of worker threads that execute queued jobs.
- `PersistentWorker`: background worker that manages named tasks and wakes when work is available.

### Network
- `Message`: typed message carrier with a `DataBuffer` payload and value serialization support.
- `Client`: socket client with callback actions typed by `Message::Type`.
- `Server`: socket server that accepts clients, maps message types to actions, and uses a worker pool for handling.

### Mathematics
- `IVector2<T>`: 2D integer vector with common vector operators.
- `IVector3<T>`: 3D integer vector for spatial calculations.
- `Random2DCoordinateGenerator`: generates random 2D coordinates in a range.
- `PerlinNoise2D`: 2D Perlin noise generator built on top of the coordinate generator.

### Clock
- `Timer`: timeout helper that tracks a duration and reports remaining time.
- `Chronometer`: elapsed-time tracker with start/stop/reset support.

## Build
Build the library and tests with the provided Makefile:

```sh
make
```

## Test
Run the full test suite:

```sh
make test
```

Run tests with optional output control:

```sh
make test VERBOSE=0      # hide binary output
make test VALGRIND=1     # run tests under Valgrind
```

Run a single test by name:

```sh
make run test_name
```

Run single test with optional output control:

```sh
make run test_name VERBOSE=0
make run test_name VALGRIND=1
```

Run network examples in separate instances:

```sh
make run server
make run client
```

## Notes
Most modules are header-based and use `.tpp` template implementations for generic logic. The repository is designed to be easy to include or extend in C++20 projects.
