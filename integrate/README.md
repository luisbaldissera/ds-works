# Integrate

Integrate is a basic integration with trapeze method in a distributed system.
Currently, it only supports one type of function: <img src="https://rawgit.com/in	git@github.com:luisbaldissera/ds-works/None/svgs/ad0609715f335782dffbef1b14a9256a.svg?invert_in_darkmode" align=middle width=82.59141000000001pt height=28.71230999999999pt/>.

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

If `-m` option is not specified, `localhost` is used.

By default, master listens on port 8989, it can be changed using `-p` option.

The discretization interval for the integration calculus can be set using `-s` option, and it is assgined by default to 0.0001

See `integ --help`.
