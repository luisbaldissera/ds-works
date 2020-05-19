# Integrate

Integrate is a basic integration with trapeze method in a distributed system.
Currently, it only supports one type of function: $\sqrt{100^2 - x^2}$.

## Compiling

To compile, simply use `make` command.

## Running

To run the operation, it first need to start the master, and then, the slaves.

For example, to run the operation with one master and two slaves:

```console
user@host0:...$ integ master -n 2
user@host1:...$ integ slave -m 'host0'
user@host2:...$ integ slave -m 'host0'
```

If `-m` option is not specified, `localhost` is used instead.

By default, master listens on port `8989`, it can be changed using `-p` option.
See `integ --help`.
