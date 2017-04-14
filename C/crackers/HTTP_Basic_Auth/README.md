# HTTP Basic Access Authentication Cracker

### taken two dictionaries for users and passwords

This is a simpels HTTP Basic Access Authentication Cracker, given two dictionaries ( simple text files ) for users and passwords

There is no current *default* user login, such as *admin*, you can create a simple file with just *admin* in it.

---

*Usage*:

http_basic_crk -H <hostname> -u <user file> -p <pass file> [-P <port>] [-v]

---

**hostname, user and password files are required**

*hostname* could be a simple 'www.hostname.org' or an IP

*dictionary* files should have all user and password terminated with a `new line` character, even the last one, as it could not be used otherwise

*port* is default `80`

*-v* for verbose mode, default `none`

---

there is one dependency that should resolve a `hostname` into a `dotted notation IP`

`#define`d in the main file *http_basic_auth.c* as `DNS`, it's value should be changed into a Path-accessible executable which output should be the IP

there is one available under the *dns* folder here

---

### Compilation

to compile, simple `cd` into the containing folder and `make`

after that, it is suggested that you `mv` or `cp` the executable into a Path-accessible folder, such as **your local bin**

