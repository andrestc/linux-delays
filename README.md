Utility to get per-pid delay accounting statistics from the kernel using
libnl to fetch data from the Netlink interface.

## Usage

`$ make`

`$ sudo ./getdelays <PID>`

It is possible to fetch delay accounting information without running as root,
providing CAP_NET_ADMIN capabilities to the binary:

` $ sudo setcap cap_net_admin+ep ./getdelays`


## Whats linux delay accounting?

Docs available at: https://www.kernel.org/doc/Documentation/accounting/delay-accounting.txt


## Disclaimer

Based on [linux/tools/accounting/getdelays.c](https://github.com/torvalds/linux/blob/master/tools/accounting/getdelays.c), but uses a higher
level abstraction ([libnl](http://www.infradead.org/~tgr/libnl/)) instead of handling the socket and parsing the data manually.
