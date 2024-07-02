# gosa2-container

This repository is used to hold the code
necessary to get GOsa² up and running in a container
environment.

It should be possible to run GOsa² on your local machine
and to get it work on your kubernetes environment also.

## Current state

The state of container files is "work in progress".
The amount of options to use this as an basis to
setup an production environment are very limited.

Although you should find any interessting pieces here
to get your own environment up and running.

### What should work

1. Setup GOsa² with openLDAP >= 2.5
2. Use `START_TLS` for GOsa²s LDAP connection
3. Using a multi provider LDAP setup

### Setup

In the future it should be sufficient to apply a
manifest to your kubernetes environment with the
appropriate configuration values.

We do this to avoid the classic setup wizard that
still exists in GOsa².